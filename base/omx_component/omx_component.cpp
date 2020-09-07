/******************************************************************************
*
* Copyright (C) 2016-2020 Allegro DVT2.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX OR ALLEGRO DVT2 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of  Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
* Except as contained in this notice, the name of Allegro DVT2 shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Allegro DVT2.
*
******************************************************************************/

#include "omx_component.h"
#include "base/omx_checker/omx_checker.h"
#include <cassert>
#include <cstring>
#include <string>

#include <OMX_VideoExt.h>
#include <utility/logger.h>
#include <utility/omx_translate.h>

using namespace std;

#define OMX_CATCH_PARAMETER_OR_CONFIG() \
  } \
  catch(OMX_ERRORTYPE& e) \
  { \
    LOG_ERROR(ToStringOMXIndex(index) + string { ": " } +ToStringOMXError(e)); \
    return e; \
  } \
  void FORCE_SEMICOLON()

static void ClearPropagatedData(OMX_BUFFERHEADERTYPE* header)
{
  header->hMarkTargetComponent = nullptr;
  header->pMarkData = nullptr;
  header->nFilledLen = 0;
  header->nTickCount = 0;
  header->nFlags = 0;
}

void Component::ReturnEmptiedBuffer(OMX_BUFFERHEADERTYPE* emptied)
{
  assert(emptied);

  auto header = emptied;
  ClearPropagatedData(header);

  if(callbacks.EmptyBufferDone)
    callbacks.EmptyBufferDone(component, app, header);
}

void nullDeleter(void*)
{
}

static Task CreateTask(Command cmd, OMX_U32 data, shared_ptr<void> opt)
{
  Task task {};
  task.cmd = cmd;
  task.data = reinterpret_cast<uintptr_t*>(data);
  task.opt = opt;
  return task;
}

void Component::EmptyThisBufferCallBack(BufferHandleInterface* handle)
{
  auto emptied = ((OMXBufferHandle*)(handle))->header;
  ReturnEmptiedBuffer(emptied);
  delete handle;
}

void Component::AssociateCallBack(BufferHandleInterface* empty, BufferHandleInterface* fill)
{
  auto emptyHeader = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)(empty))->header;
  assert(emptyHeader);
  auto fillHeader = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)(fill))->header;
  assert(fillHeader);

  PropagateHeaderData(*emptyHeader, *fillHeader);

  if(IsEOSDetected(emptyHeader->nFlags))
    callbacks.EventHandler(component, app, OMX_EventBufferFlag, output.index, emptyHeader->nFlags, nullptr);

  if(IsCompMarked(emptyHeader->hMarkTargetComponent, component))
    callbacks.EventHandler(component, app, OMX_EventMark, 0, 0, emptyHeader->pMarkData);
}

void Component::ReturnFilledBuffer(OMX_BUFFERHEADERTYPE* filled, int offset, int size)
{
  assert(filled);

  auto header = filled;

  header->nOffset = offset;
  header->nFilledLen = size;

  if(callbacks.FillBufferDone)
    callbacks.FillBufferDone(component, app, header);
}

void Component::FillThisBufferCallBack(BufferHandleInterface* filled)
{
  auto header = ((OMXBufferHandle*)filled)->header;
  auto offset = ((OMXBufferHandle*)filled)->offset;
  auto payload = ((OMXBufferHandle*)filled)->payload;
  delete filled;
  ReturnFilledBuffer(header, offset, payload);
}

void Component::ReleaseCallBack(bool isInput, BufferHandleInterface* released)
{
  auto header = ((OMXBufferHandle*)released)->header;
  delete released;

  if(isInput)
    ReturnEmptiedBuffer(header);
  else
    ReturnFilledBuffer(header, 0, 0);
}

static OMX_ERRORTYPE ToOmxError(ModuleInterface::ErrorType error)
{
  switch(error)
  {
  case ModuleInterface::CHANNEL_CREATION_NO_CHANNEL_AVAILABLE: return static_cast<OMX_ERRORTYPE>(OMX_ALG_ErrorNoChannelLeft);
  case ModuleInterface::CHANNEL_CREATION_RESOURCE_UNAVAILABLE: return static_cast<OMX_ERRORTYPE>(OMX_ALG_ErrorChannelResourceUnavailable);
  case ModuleInterface::CHANNEL_CREATION_RESOURCE_FRAGMENTED: return static_cast<OMX_ERRORTYPE>(OMX_ALG_ErrorChannelResourceFragmented);
  case ModuleInterface::NO_MEMORY: return OMX_ErrorInsufficientResources;
  case ModuleInterface::BAD_PARAMETER: return OMX_ErrorBadParameter;
  default: return OMX_ErrorUndefined;
  }

  return OMX_ErrorUndefined;
}

void Component::EventCallBack(Callbacks::Event type, void* data)
{
  assert(type <= Callbacks::Event::MAX);
  switch(type)
  {
  case Callbacks::Event::RESOLUTION_DETECTED:
  {
    LOG_IMPORTANT(ToStringCallbackEvent.at(type));

    callbacks.EventHandler(component, app, OMX_EventPortSettingsChanged, 1, 0, nullptr);
    break;
  }
  case Callbacks::Event::RESOLUTION_CHANGED:
  {
    LOG_IMPORTANT(ToStringCallbackEvent.at(type));
    auto dimension = static_cast<Dimension<int>*>(data);

    callbacks.EventHandler(component, app, static_cast<OMX_EVENTTYPE>(OMX_ALG_EventResolutionChanged), dimension->horizontal, dimension->vertical, nullptr);
    break;
  }
  case Callbacks::Event::ERROR:
  {
    ModuleInterface::ErrorType errorCode = static_cast<ModuleInterface::ErrorType>((uintptr_t)data);
    processorMain->queue(CreateTask(Command::SetState, OMX_StateInvalid, shared_ptr<void>((uintptr_t*)ToOmxError(errorCode), nullDeleter)));
    break;
  }
  default:
    LOG_ERROR(ToStringCallbackEvent.at(type) + string { " is unsupported" });
  }
}

void Component::CheckPortIndex(int index)
{
  if(index < static_cast<int>(videoPortParams.nStartPortNumber))
    throw OMX_ErrorBadPortIndex;

  auto maxIndex = static_cast<int>(videoPortParams.nStartPortNumber + videoPortParams.nPorts - 1);

  if(index > maxIndex)
    throw OMX_ErrorBadPortIndex;
}

Port* Component::GetPort(int index)
{
  CheckPortIndex(index);
  return IsInputPort(index) ? &input : &output;
}

static void AssociateSpecVersion(OMX_VERSIONTYPE& spec)
{
  spec.s.nVersionMajor = OMX_VERSION_MAJOR;
  spec.s.nVersionMinor = OMX_VERSION_MINOR;
  spec.s.nRevision = OMX_VERSION_REVISION;
  spec.s.nStep = OMX_VERSION_STEP;
}

static BufferCounts MinBufferCounts(shared_ptr<MediatypeInterface> media)
{
  BufferCounts bufferCounts {};
  assert(media);
  media->Get(SETTINGS_INDEX_BUFFER_COUNTS, &bufferCounts);
  return bufferCounts;
}

static void SetPortsParam(OMX_PORT_PARAM_TYPE& portParams)
{
  portParams.nPorts = VIDEO_PORTS_COUNT;
  portParams.nStartPortNumber = VIDEO_START_PORT;
}

Component::Component(OMX_HANDLETYPE component, shared_ptr<MediatypeInterface> media, unique_ptr<ModuleInterface>&& module, std::unique_ptr<ExpertiseInterface>&& expertise, OMX_STRING name, OMX_STRING role) :
  component{component},
  media{media},
  module{move(module)},
  expertise{move(expertise)},
  input{0, MinBufferCounts(media).input},
  output{1, MinBufferCounts(media).output}
{
  assert(name);
  assert(role);
  CreateName(name);
  CreateRole(role);
  shouldPrealloc = true;
  shouldClearROI = false;
  shouldPushROI = false;
  shouldFireEventPortSettingsChanges = true;
  version.nVersion = ALLEGRODVT_OMX_VERSION;
  AssociateSpecVersion(spec);

  OMXChecker::SetHeaderVersion(videoPortParams);
  SetPortsParam(videoPortParams);
  auto deleteEmpty = bind(&Component::_DeleteEmpty, this, placeholders::_1);
  auto deleteFill = bind(&Component::_DeleteFill, this, placeholders::_1);
  auto p = bind(&Component::_ProcessMain, this, placeholders::_1);
  auto p2 = bind(&Component::_ProcessFillBuffer, this, placeholders::_1);
  auto p3 = bind(&Component::_ProcessEmptyBuffer, this, placeholders::_1);
  processorMain.reset(new ProcessorFifo<Task> { p, nullptr, "OMX - Sched" });
  processorFill.reset(new ProcessorFifo<Task> { p2, deleteFill, "OMX - Out" });
  processorEmpty.reset(new ProcessorFifo<Task> { p3, deleteEmpty, "OMX - In" });
  pauseFillPromise = nullptr;
  pauseEmptyPromise = nullptr;
  eosHandles.input = nullptr;
  eosHandles.output = nullptr;

  transientState = TransientState::Max;
  state = OMX_StateLoaded;
}

Component::~Component()
{
  if(pauseFillPromise || pauseEmptyPromise)
    UnblockFillEmptyBuffers();
}

void Component::CreateName(OMX_STRING name)
{
  this->name = (OMX_STRING)malloc(OMX_MAX_STRINGNAME_SIZE * sizeof(char));
  strncpy(this->name, name, OMX_MAX_STRINGNAME_SIZE);
}

void Component::CreateRole(OMX_STRING role)
{
  this->role = (OMX_STRING)malloc(OMX_MAX_STRINGNAME_SIZE * sizeof(char));
  strncpy(this->role, role, OMX_MAX_STRINGNAME_SIZE);
}

static TransientState GetTransientState(OMX_STATETYPE const& curState, OMX_STATETYPE const& nextState)
{
  if(curState == OMX_StateLoaded && nextState == OMX_StateIdle)
    return TransientState::LoadedToIdle;

  if(curState == OMX_StateIdle && nextState == OMX_StatePause)
    return TransientState::IdleToPause;

  if(curState == OMX_StateIdle && nextState == OMX_StateLoaded)
    return TransientState::IdleToLoaded;

  if(curState == OMX_StateIdle && nextState == OMX_StateExecuting)
    return TransientState::IdleToExecuting;

  if(curState == OMX_StatePause && nextState == OMX_StateExecuting)
    return TransientState::PauseToExecuting;

  if(curState == OMX_StatePause && nextState == OMX_StateIdle)
    return TransientState::PauseToIdle;

  if(curState == OMX_StateExecuting && nextState == OMX_StateIdle)
    return TransientState::ExecutingToIdle;

  if(curState == OMX_StateExecuting && nextState == OMX_StatePause)
    return TransientState::ExecutingToPause;

  return TransientState::Max;
}

void Component::CreateCommand(OMX_COMMANDTYPE command, OMX_U32 param, OMX_PTR data)
{
  Command taskCommand;
  switch(command)
  {
  case OMX_CommandStateSet:
  {
    OMXChecker::CheckNull(data);

    auto nextState = static_cast<OMX_STATETYPE>(param);
    OMXChecker::CheckStateExistance(nextState);

    TransientState NewtransientState = GetTransientState(state, nextState);

    if(NewtransientState == TransientState::LoadedToIdle)
      if(media && !media->Check())
        throw OMX_ErrorUndefined;

    transientState = NewtransientState;
    taskCommand = Command::SetState;
    break;
  }
  case OMX_CommandFlush:
  {
    OMXChecker::CheckNull(data);

    if(param == OMX_ALL)
    {
      for(auto i = videoPortParams.nStartPortNumber; i < videoPortParams.nPorts; i++)
        processorMain->queue(CreateTask(Command::Flush, i, shared_ptr<void>(data, nullDeleter)));

      return;
    }
    CheckPortIndex(param);
    taskCommand = Command::Flush;
    break;
  }
  case OMX_CommandPortDisable:
  {
    OMXChecker::CheckNull(data);

    if(param == OMX_ALL)
    {
      for(auto i = videoPortParams.nStartPortNumber; i < videoPortParams.nPorts; i++)
      {
        GetPort(i)->enable = false;
        GetPort(i)->isTransientToDisable = true;
        shouldFireEventPortSettingsChanges = true;
        processorMain->queue(CreateTask(Command::DisablePort, i, shared_ptr<void>(data, nullDeleter)));
      }

      return;
    }

    CheckPortIndex(param);
    GetPort(param)->enable = false;
    GetPort(param)->isTransientToDisable = true;
    taskCommand = Command::DisablePort;
    shouldFireEventPortSettingsChanges = true;
    break;
  }
  case OMX_CommandPortEnable:
  {
    OMXChecker::CheckNull(data);

    if(param == OMX_ALL)
    {
      for(auto i = videoPortParams.nStartPortNumber; i < videoPortParams.nPorts; i++)
      {
        GetPort(i)->enable = true;
        GetPort(i)->isTransientToEnable = true;
        shouldFireEventPortSettingsChanges = false;
        processorMain->queue(CreateTask(Command::EnablePort, i, shared_ptr<void>(data, nullDeleter)));
      }

      return;
    }

    CheckPortIndex(param);
    GetPort(param)->enable = true;
    GetPort(param)->isTransientToEnable = true;
    taskCommand = Command::EnablePort;
    shouldFireEventPortSettingsChanges = false;
    break;
  }
  case OMX_CommandMarkBuffer:
  {
    OMXChecker::CheckNotNull(data);

    if(!IsInputPort(param))
      throw OMX_ErrorBadPortIndex;

    taskCommand = Command::MarkBuffer;
    break;
  }
  default:
    throw OMX_ErrorBadParameter;
  }

  processorMain->queue(CreateTask(taskCommand, param, shared_ptr<void>(data, nullDeleter)));
}

OMX_ERRORTYPE Component::SendCommand(OMX_IN OMX_COMMANDTYPE cmd, OMX_IN OMX_U32 param, OMX_IN OMX_PTR data)
{
  OMX_TRY();
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::SendCommand, state);

  CreateCommand(cmd, param, data);

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Component::GetState(OMX_OUT OMX_STATETYPE* state)
{
  OMX_TRY();
  OMXChecker::CheckNotNull<OMX_STATETYPE*>(state);

  *state = this->state;

  return OMX_ErrorNone;
  OMX_CATCH();
}

static OMX_ERRORTYPE StubEventHandler(OMX_IN OMX_HANDLETYPE, OMX_IN OMX_PTR, OMX_IN OMX_EVENTTYPE, OMX_IN OMX_U32, OMX_IN OMX_U32, OMX_IN OMX_PTR)
{
  return OMX_ErrorNone;
}

OMX_ERRORTYPE Component::SetCallbacks(OMX_IN OMX_CALLBACKTYPE* callbacks, OMX_IN OMX_PTR app)
{
  OMX_TRY();
  OMXChecker::CheckNotNull<OMX_CALLBACKTYPE*>(callbacks);
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::SetCallbacks, state);

  auto empty = bind(&Component::EmptyThisBufferCallBack, this, placeholders::_1);
  auto associate = bind(&Component::AssociateCallBack, this, placeholders::_1, placeholders::_2);
  auto filled = bind(&Component::FillThisBufferCallBack, this, placeholders::_1);
  auto event = bind(&Component::EventCallBack, this, placeholders::_1, placeholders::_2);
  auto release = bind(&Component::ReleaseCallBack, this, placeholders::_1, placeholders::_2);
  Callbacks cbs {};
  cbs.emptied = empty;
  cbs.associate = associate;
  cbs.filled = filled;
  cbs.release = release;
  cbs.event = event;
  auto success = module->SetCallbacks(cbs);

  if(!success)
    throw OMX_ErrorUndefined;

  this->callbacks = *callbacks;

  if(this->callbacks.EventHandler == nullptr)
    this->callbacks.EventHandler = &StubEventHandler;

  this->app = app;

  return OMX_ErrorNone;
  OMX_CATCH();
}

static void CheckVersionExistance(OMX_PTR ptr)
{
  auto size = *static_cast<OMX_U32*>(ptr);

  if(size < (sizeof(OMX_U32) + sizeof(OMX_VERSIONTYPE)))
    throw OMX_ErrorBadParameter;
}

static OMX_VERSIONTYPE GetVersion(OMX_PTR ptr)
{
  CheckVersionExistance(ptr);
  auto tmp = ptr;
  tmp = static_cast<OMX_U32*>(tmp) + 1; // nVersion is always after nSize

  return *static_cast<OMX_VERSIONTYPE*>(tmp);
}

OMX_ERRORTYPE Component::GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::GetParameter, state);

  auto getCurrentPort = [=](OMX_PTR param) -> Port*
                        {
                          auto index = *(((OMX_U32*)param) + 2);
                          return GetPort(index);
                        };
  switch(static_cast<OMX_U32>(index)) // all indexes are 32u
  {
  case OMX_IndexParamVideoInit:
  {
    *(OMX_PORT_PARAM_TYPE*)param = videoPortParams;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamStandardComponentRole:
  {
    auto p = (OMX_PARAM_COMPONENTROLETYPE*)param;
    strncpy((char*)p->cRole, (char*)role, OMX_MAX_STRINGNAME_SIZE);
    return OMX_ErrorNone;
  }
  case OMX_IndexParamPortDefinition:
  {
    auto port = getCurrentPort(param);
    auto def = static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(param);
    return ConstructPortDefinition(*def, *port, media);
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    auto port = getCurrentPort(param);
    auto s = static_cast<OMX_PARAM_BUFFERSUPPLIERTYPE*>(param);
    return ConstructPortSupplier(*s, *port);
  }
  case OMX_IndexParamVideoPortFormat:
  {
    auto p = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)param;
    return GetVideoPortFormatSupported(*p, media);
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    auto port = getCurrentPort(param);
    return expertise->GetProfileLevel(param, *port, media);
  }
  case OMX_IndexParamVideoProfileLevelQuerySupported: // GetParameter only
  {
    return expertise->GetProfileLevelSupported(param, media);
  }
  case OMX_IndexParamVideoAvc:
  case OMX_ALG_IndexParamVideoHevc:
  {
    auto port = getCurrentPort(param);
    return expertise->GetExpertise(param, *port, media);
  }
  case OMX_ALG_IndexParamReportedLatency: // GetParameter only
  {
    auto lat = static_cast<OMX_ALG_PARAM_REPORTED_LATENCY*>(param);
    return ConstructReportedLatency(*lat, media);
  }
  case OMX_ALG_IndexPortParamEarlyCallback:
  {
    auto earlyCB = static_cast<OMX_ALG_PORT_PARAM_EARLY_CALLBACK*>(param);
    auto port = getCurrentPort(param);
    return ConstructPortEarlyCallback(*earlyCB, *port, media);
  }
  case OMX_ALG_IndexPortParamBufferMode:
  {
    auto port = getCurrentPort(param);
    auto mode = static_cast<OMX_ALG_PORT_PARAM_BUFFER_MODE*>(param);
    return ConstructPortBufferMode(*mode, *port, media);
  }
  case OMX_ALG_IndexParamVideoSubframe:
  {
    auto port = getCurrentPort(param);
    auto subframe = static_cast<OMX_ALG_VIDEO_PARAM_SUBFRAME*>(param);
    return ConstructVideoSubframe(*subframe, *port, media);
  }
  // only encoder
  case OMX_IndexParamVideoQuantization:
  {
    auto port = getCurrentPort(param);
    auto q = static_cast<OMX_VIDEO_PARAM_QUANTIZATIONTYPE*>(param);
    return ConstructVideoQuantization(*q, *port, media);
  }
  case OMX_IndexParamVideoBitrate:
  {
    auto port = getCurrentPort(param);
    auto b = static_cast<OMX_VIDEO_PARAM_BITRATETYPE*>(param);
    return ConstructVideoBitrate(*b, *port, media);
  }
  case OMX_ALG_IndexParamVideoInterlaceFormatSupported: // GetParameter only
  {
    auto port = getCurrentPort(param);
    auto interlace = static_cast<OMX_INTERLACEFORMATTYPE*>(param);
    return ConstructVideoModesSupported(*interlace, *port, media);
  }
  case OMX_ALG_IndexParamVideoInterlaceFormatCurrent:
  {
    auto port = getCurrentPort(param);
    auto interlace = static_cast<OMX_INTERLACEFORMATTYPE*>(param);
    return ConstructVideoModeCurrent(*interlace, *port, media);
  }
  case OMX_ALG_IndexParamVideoQuantizationControl:
  {
    auto port = getCurrentPort(param);
    auto q = static_cast<OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL*>(param);
    return ConstructVideoQuantizationControl(*q, *port, media);
  }
  case OMX_ALG_IndexParamVideoQuantizationExtension:
  {
    auto port = getCurrentPort(param);
    auto q = static_cast<OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION*>(param);
    return ConstructVideoQuantizationExtension(*q, *port, media);
  }
  case OMX_ALG_IndexParamVideoAspectRatio:
  {
    auto port = getCurrentPort(param);
    auto a = static_cast<OMX_ALG_VIDEO_PARAM_ASPECT_RATIO*>(param);
    return ConstructVideoAspectRatio(*a, *port, media);
  }
  case OMX_ALG_IndexParamVideoMaxBitrate:
  {
    auto port = getCurrentPort(param);
    auto b = static_cast<OMX_ALG_VIDEO_PARAM_MAX_BITRATE*>(param);
    return ConstructVideoMaxBitrate(*b, *port, media);
  }
  case OMX_ALG_IndexParamVideoLowBandwidth:
  {
    auto port = getCurrentPort(param);
    auto bw = static_cast<OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH*>(param);
    return ConstructVideoLowBandwidth(*bw, *port, media);
  }
  case OMX_ALG_IndexParamVideoGopControl:
  {
    auto port = getCurrentPort(param);
    auto gc = static_cast<OMX_ALG_VIDEO_PARAM_GOP_CONTROL*>(param);
    return ConstructVideoGopControl(*gc, *port, media);
  }
  case OMX_ALG_IndexParamVideoSceneChangeResilience:
  {
    auto port = getCurrentPort(param);
    auto r = static_cast<OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE*>(param);
    return ConstructVideoSceneChangeResilience(*r, *port, media);
  }
  case OMX_ALG_IndexParamVideoInstantaneousDecodingRefresh:
  {
    auto port = getCurrentPort(param);
    auto idr = static_cast<OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH*>(param);
    return ConstructVideoInstantaneousDecodingRefresh(*idr, *port, media);
  }
  case OMX_ALG_IndexParamVideoCodedPictureBuffer:
  {
    auto port = getCurrentPort(param);
    auto cpb = static_cast<OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER*>(param);
    return ConstructVideoCodedPictureBuffer(*cpb, *port, media);
  }
  case OMX_ALG_IndexParamVideoPrefetchBuffer:
  {
    auto port = getCurrentPort(param);
    auto pb = static_cast<OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER*>(param);
    return ConstructVideoPrefetchBuffer(*pb, *port, media);
  }
  case OMX_ALG_IndexParamVideoScalingList:
  {
    auto port = getCurrentPort(param);
    auto scl = static_cast<OMX_ALG_VIDEO_PARAM_SCALING_LIST*>(param);
    return ConstructVideoScalingList(*scl, *port, media);
  }
  case OMX_ALG_IndexParamVideoFillerData:
  {
    auto port = getCurrentPort(param);
    auto f = static_cast<OMX_ALG_VIDEO_PARAM_FILLER_DATA*>(param);
    return ConstructVideoFillerData(*f, *port, media);
  }
  case OMX_ALG_IndexParamVideoSlices:
  {
    auto port = getCurrentPort(param);
    auto s = static_cast<OMX_ALG_VIDEO_PARAM_SLICES*>(param);
    return ConstructVideoSlices(*s, *port, media);
  }
  case OMX_ALG_IndexParamVideoLongTerm:
  {
    auto port = getCurrentPort(param);
    auto longTerm = static_cast<OMX_ALG_VIDEO_PARAM_LONG_TERM*>(param);
    return ConstructVideoLongTerm(*longTerm, *port, media);
  }
  case OMX_ALG_IndexParamVideoLookAhead:
  {
    auto port = getCurrentPort(param);
    auto la = static_cast<OMX_ALG_VIDEO_PARAM_LOOKAHEAD*>(param);
    return ConstructVideoLookAhead(*la, *port, media);
  }
  case OMX_ALG_IndexParamVideoTwoPass:
  {
    auto port = getCurrentPort(param);
    auto tp = static_cast<OMX_ALG_VIDEO_PARAM_TWOPASS*>(param);
    return ConstructVideoTwoPass(*tp, *port, media);
  }
  case OMX_ALG_IndexParamVideoSkipFrame:
  {
    auto port = getCurrentPort(param);
    auto skip = static_cast<OMX_ALG_VIDEO_PARAM_SKIP_FRAME*>(param);
    return ConstructVideoSkipFrame(*skip, *port, media);
  }
  case OMX_ALG_IndexParamVideoColorPrimaries:
  {
    auto port = getCurrentPort(param);
    auto c = static_cast<OMX_ALG_VIDEO_PARAM_COLOR_PRIMARIES*>(param);
    return ConstructVideoColorPrimaries(*c, *port, media);
  }
  case OMX_ALG_IndexParamVideoTransferCharacteristics:
  {
    auto port = getCurrentPort(param);
    auto tc = static_cast<OMX_ALG_VIDEO_PARAM_TRANSFER_CHARACTERISTICS*>(param);
    return ConstructVideoTransferCharacteristics(*tc, *port, media);
  }
  case OMX_ALG_IndexParamVideoColorMatrix:
  {
    auto port = getCurrentPort(param);
    auto cm = static_cast<OMX_ALG_VIDEO_PARAM_COLOR_MATRIX*>(param);
    return ConstructVideoColorMatrix(*cm, *port, media);
  }
  case OMX_ALG_IndexParamVideoMaxPictureSize:
  {
    auto port = getCurrentPort(param);
    auto mps = static_cast<OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE*>(param);
    return ConstructVideoMaxPictureSize(*mps, *port, media);
  }
  case OMX_ALG_IndexParamVideoMaxPictureSizes:
  {
    auto port = getCurrentPort(param);
    auto mps = static_cast<OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES*>(param);
    return ConstructVideoMaxPictureSizes(*mps, *port, media);
  }
  // only decoder
  case OMX_ALG_IndexParamPreallocation:
  {
    auto prealloc = static_cast<OMX_ALG_PARAM_PREALLOCATION*>(param);
    return ConstructPreallocation(*prealloc, this->shouldPrealloc);
  }
  case OMX_ALG_IndexParamVideoDecodedPictureBuffer:
  {
    auto port = getCurrentPort(param);
    auto dpb = static_cast<OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER*>(param);
    return ConstructVideoDecodedPictureBuffer(*dpb, *port, media);
  }
  case OMX_ALG_IndexParamVideoInternalEntropyBuffers:
  {
    auto port = getCurrentPort(param);
    auto ieb = static_cast<OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS*>(param);
    return ConstructVideoInternalEntropyBuffers(*ieb, *port, media);
  }
  case OMX_ALG_IndexParamCommonSequencePictureModeQuerySupported:
  {
    auto mode = (OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE*)param;
    return ConstructCommonSequencePictureModesSupported(mode, media);
  }
  case OMX_ALG_IndexParamCommonSequencePictureModeCurrent:
  {
    auto port = getCurrentPort(param);
    auto mode = static_cast<OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE*>(param);
    return ConstructCommonSequencePictureMode(*mode, *port, media);
  }
  case OMX_ALG_IndexParamVideoInputParsed:
  {
    auto port = getCurrentPort(param);
    auto ip = static_cast<OMX_ALG_VIDEO_PARAM_INPUT_PARSED*>(param);
    return ConstructVideoInputParsed(*ip, *port, media);
  }
  case OMX_ALG_IndexParamVideoLoopFilterBeta:
  {
    auto port = getCurrentPort(param);
    auto beta = static_cast<OMX_ALG_VIDEO_PARAM_LOOP_FILTER_BETA*>(param);
    return ConstructVideoLoopFilterBeta(*beta, *port, media);
  }
  case OMX_ALG_IndexParamVideoLoopFilterTc:
  {
    auto port = getCurrentPort(param);
    auto tc = static_cast<OMX_ALG_VIDEO_PARAM_LOOP_FILTER_TC*>(param);
    return ConstructVideoLoopFilterTc(*tc, *port, media);
  }
  case OMX_ALG_IndexParamVideoQuantizationTable:
  {
    auto port = getCurrentPort(param);
    auto qpTable = static_cast<OMX_ALG_VIDEO_PARAM_QUANTIZATION_TABLE*>(param);
    return ConstructVideoQuantizationTable(*qpTable, *port, media);
  }
  case OMX_ALG_IndexParamVideoAccessUnitDelimiter:
  {
    auto port = getCurrentPort(param);
    auto aud = static_cast<OMX_ALG_VIDEO_PARAM_ACCESS_UNIT_DELIMITER*>(param);
    return ConstructVideoAccessUnitDelimiter(*aud, *port, media);
  }
  case OMX_ALG_IndexParamVideoBufferingPeriodSEI:
  {
    auto port = getCurrentPort(param);
    auto bpSEI = static_cast<OMX_ALG_VIDEO_PARAM_BUFFERING_PERIOD_SEI*>(param);
    return ConstructVideoBufferingPeriodSEI(*bpSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoPictureTimingSEI:
  {
    auto port = getCurrentPort(param);
    auto ptSEI = static_cast<OMX_ALG_VIDEO_PARAM_PICTURE_TIMING_SEI*>(param);
    return ConstructVideoPictureTimingSEI(*ptSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoRecoveryPointSEI:
  {
    auto port = getCurrentPort(param);
    auto rpSEI = static_cast<OMX_ALG_VIDEO_PARAM_RECOVERY_POINT_SEI*>(param);
    return ConstructVideoRecoveryPointSEI(*rpSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoMasteringDisplayColourVolumeSEI:
  {
    auto port = getCurrentPort(param);
    auto mdcvSEI = static_cast<OMX_ALG_VIDEO_PARAM_MASTERING_DISPLAY_COLOUR_VOLUME_SEI*>(param);
    return ConstructVideoMasteringDisplayColourVolumeSEI(*mdcvSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoContentLightLevelSEI:
  {
    auto port = getCurrentPort(param);
    auto cllSEI = static_cast<OMX_ALG_VIDEO_PARAM_CONTENT_LIGHT_LEVEL_SEI*>(param);
    return ConstructVideoContentLightLevelSEI(*cllSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoST209410SEI:
  {
    auto port = getCurrentPort(param);
    auto st209410SEI = static_cast<OMX_ALG_VIDEO_PARAM_ST2094_10_SEI*>(param);
    return ConstructVideoST209410SEI(*st209410SEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoST209440SEI:
  {
    auto port = getCurrentPort(param);
    auto st209440SEI = static_cast<OMX_ALG_VIDEO_PARAM_ST2094_40_SEI*>(param);
    return ConstructVideoST209440SEI(*st209440SEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoRateControlPlugin:
  {
    auto port = getCurrentPort(param);
    auto rcp = static_cast<OMX_ALG_VIDEO_PARAM_RATE_CONTROL_PLUGIN*>(param);
    return ConstructVideoRateControlPlugin(*rcp, *port, media);
  }
  case OMX_ALG_IndexParamVideoCrop:
  {
    auto port = getCurrentPort(param);
    auto crop = static_cast<OMX_CONFIG_RECTTYPE*>(param);
    return ConstructVideoCrop(*crop, *port, media);
  }
  case OMX_ALG_IndexParamVideoMaxPictureSizesInBits:
  {
    auto port = getCurrentPort(param);
    auto mps = static_cast<OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES_IN_BITS*>(param);
    return ConstructVideoMaxPictureSizesInBits(*mps, *port, media);
  }
  default:
    LOG_ERROR(ToStringOMXIndex(index) + string { " is unsupported" });
    return OMX_ErrorUnsupportedIndex;
  }

  LOG_ERROR(ToStringOMXIndex(index) + string { " is unsupported" });
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_PARAMETER_OR_CONFIG();
}

OMX_ERRORTYPE Component::SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));

  auto getCurrentPort = [=](OMX_PTR param) -> Port*
                        {
                          auto index = *(((OMX_U32*)param) + 2);
                          return GetPort(index);
                        };

  Port* port = nullptr;
  shouldFireEventPortSettingsChanges = true;

  if(OMX_U32(index) != OMX_IndexParamStandardComponentRole)
  {
    port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::SetParameter, state);
  }
  switch(static_cast<OMX_U32>(index)) // all indexes are 32u
  {
  case OMX_IndexParamStandardComponentRole:
  {
    OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::SetParameter, state);
    auto p = (OMX_PARAM_COMPONENTROLETYPE*)param;

    if(!strncmp((char*)role, (char*)p->cRole, strlen((char*)role)))
    {
      bool shouldUseLLP2EarlyCallback {};
      media->Get(SETTINGS_INDEX_LLP2_EARLY_CB, &shouldUseLLP2EarlyCallback);
      media->Reset();
      media->Set(SETTINGS_INDEX_LLP2_EARLY_CB, &shouldUseLLP2EarlyCallback);
      return OMX_ErrorNone;
    }
    throw OMX_ErrorBadParameter;
  }
  case OMX_IndexParamPortDefinition:
  {
    auto settings = static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(param);
    SetPortExpectedBuffer(*settings, const_cast<Port &>(*port), media);
    return SetPortDefinition(*settings, *port, *module, media);
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    // Do nothing
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexPortParamEarlyCallback:
  {
    auto earlyCB = static_cast<OMX_ALG_PORT_PARAM_EARLY_CALLBACK*>(param);
    return SetPortEarlyCallback(*earlyCB, *port, media);
  }
  case OMX_IndexParamVideoPortFormat:
  {
    auto format = static_cast<OMX_VIDEO_PARAM_PORTFORMATTYPE*>(param);
    return SetVideoPortFormat(*format, *port, media);
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    return expertise->SetProfileLevel(param, *port, media);
  }
  case OMX_IndexParamVideoAvc:
  case OMX_ALG_IndexParamVideoHevc:
  {
    return expertise->SetExpertise(param, *port, media);
  }
  case OMX_ALG_IndexParamVideoSubframe:
  {
    auto subframe = static_cast<OMX_ALG_VIDEO_PARAM_SUBFRAME*>(param);
    return SetVideoSubframe(*subframe, *port, media);
  }
  case OMX_ALG_IndexPortParamBufferMode:
  {
    auto portBufferMode = static_cast<OMX_ALG_PORT_PARAM_BUFFER_MODE*>(param);
    return SetPortBufferMode(*portBufferMode, *port, media);
  }
  // only encoder
  case OMX_IndexParamVideoQuantization:
  {
    auto quantization = static_cast<OMX_VIDEO_PARAM_QUANTIZATIONTYPE*>(param);
    return SetVideoQuantization(*quantization, *port, media);
  }
  case OMX_IndexParamVideoBitrate:
  {
    auto bitrate = static_cast<OMX_VIDEO_PARAM_BITRATETYPE*>(param);
    return SetVideoBitrate(*bitrate, *port, media);
  }
  case OMX_ALG_IndexParamVideoInterlaceFormatCurrent:
  {
    auto interlaced = static_cast<OMX_INTERLACEFORMATTYPE*>(param);
    return SetVideoModeCurrent(*interlaced, *port, media);
  }
  case OMX_ALG_IndexParamVideoQuantizationControl:
  {
    auto quantizationControl = static_cast<OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL*>(param);
    return SetVideoQuantizationControl(*quantizationControl, *port, media);
  }
  case OMX_ALG_IndexParamVideoQuantizationExtension:
  {
    auto quantizationExtension = static_cast<OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION*>(param);
    return SetVideoQuantizationExtension(*quantizationExtension, *port, media);
  }
  case OMX_ALG_IndexParamVideoAspectRatio:
  {
    auto aspectRatio = static_cast<OMX_ALG_VIDEO_PARAM_ASPECT_RATIO*>(param);
    return SetVideoAspectRatio(*aspectRatio, *port, media);
  }
  case OMX_ALG_IndexParamVideoMaxBitrate:
  {
    auto maxBitrate = static_cast<OMX_ALG_VIDEO_PARAM_MAX_BITRATE*>(param);
    return SetVideoMaxBitrate(*maxBitrate, *port, media);
  }
  case OMX_ALG_IndexParamVideoLowBandwidth:
  {
    auto lowBandwidth = static_cast<OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH*>(param);
    return SetVideoLowBandwidth(*lowBandwidth, *port, media);
  }
  case OMX_ALG_IndexParamVideoGopControl:
  {
    auto gopControl = static_cast<OMX_ALG_VIDEO_PARAM_GOP_CONTROL*>(param);
    return SetVideoGopControl(*gopControl, *port, media);
  }
  case OMX_ALG_IndexParamVideoSceneChangeResilience:
  {
    auto sceneChangeResilience = static_cast<OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE*>(param);
    return SetVideoSceneChangeResilience(*sceneChangeResilience, *port, media);
  }
  case OMX_ALG_IndexParamVideoInstantaneousDecodingRefresh:
  {
    auto instantaneousDecodingRefresh = static_cast<OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH*>(param);
    return SetVideoInstantaneousDecodingRefresh(*instantaneousDecodingRefresh, *port, media);
  }
  case OMX_ALG_IndexParamVideoCodedPictureBuffer:
  {
    auto codedPictureBuffer = static_cast<OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER*>(param);
    return SetVideoCodedPictureBuffer(*codedPictureBuffer, *port, media);
  }
  case OMX_ALG_IndexParamVideoPrefetchBuffer:
  {
    auto prefetchBuffer = static_cast<OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER*>(param);
    return SetVideoPrefetchBuffer(*prefetchBuffer, *port, media);
  }
  case OMX_ALG_IndexParamVideoScalingList:
  {
    auto scalingList = static_cast<OMX_ALG_VIDEO_PARAM_SCALING_LIST*>(param);
    return SetVideoScalingList(*scalingList, *port, media);
  }
  case OMX_ALG_IndexParamVideoFillerData:
  {
    auto fillerData = static_cast<OMX_ALG_VIDEO_PARAM_FILLER_DATA*>(param);
    return SetVideoFillerData(*fillerData, *port, media);
  }
  case OMX_ALG_IndexParamVideoSlices:
  {
    auto slices = static_cast<OMX_ALG_VIDEO_PARAM_SLICES*>(param);
    return SetVideoSlices(*slices, *port, media);
  }
  case OMX_ALG_IndexParamVideoLongTerm:
  {
    auto longTerm = static_cast<OMX_ALG_VIDEO_PARAM_LONG_TERM*>(param);
    return SetVideoLongTerm(*longTerm, *port, media);
  }
  case OMX_ALG_IndexParamVideoLookAhead:
  {
    auto la = static_cast<OMX_ALG_VIDEO_PARAM_LOOKAHEAD*>(param);
    return SetVideoLookAhead(*la, *port, media);
  }
  case OMX_ALG_IndexParamVideoTwoPass:
  {
    auto tp = static_cast<OMX_ALG_VIDEO_PARAM_TWOPASS*>(param);
    return SetVideoTwoPass(*tp, *port, media);
  }
  case OMX_ALG_IndexParamVideoSkipFrame:
  {
    auto skipFrame = static_cast<OMX_ALG_VIDEO_PARAM_SKIP_FRAME*>(param);
    return SetVideoSkipFrame(*skipFrame, *port, media);
  }
  case OMX_ALG_IndexParamVideoColorPrimaries:
  {
    auto c = static_cast<OMX_ALG_VIDEO_PARAM_COLOR_PRIMARIES*>(param);
    return SetVideoColorPrimaries(*c, *port, media);
  }
  case OMX_ALG_IndexParamVideoTransferCharacteristics:
  {
    auto c = static_cast<OMX_ALG_VIDEO_PARAM_TRANSFER_CHARACTERISTICS*>(param);
    return SetVideoTransferCharacteristics(*c, *port, media);
  }
  case OMX_ALG_IndexParamVideoColorMatrix:
  {
    auto c = static_cast<OMX_ALG_VIDEO_PARAM_COLOR_MATRIX*>(param);
    return SetVideoColorMatrix(*c, *port, media);
  }
  case OMX_ALG_IndexParamVideoMaxPictureSize:
  {
    auto mps = static_cast<OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE*>(param);
    return SetVideoMaxPictureSize(*mps, *port, media);
  }
  case OMX_ALG_IndexParamVideoMaxPictureSizes:
  {
    auto mps = static_cast<OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES*>(param);
    return SetVideoMaxPictureSizes(*mps, *port, media);
  }
  case OMX_ALG_IndexParamVideoLoopFilterBeta:
  {
    auto lfb = static_cast<OMX_ALG_VIDEO_PARAM_LOOP_FILTER_BETA*>(param);
    return SetVideoLoopFilterBeta(*lfb, *port, media);
  }
  case OMX_ALG_IndexParamVideoLoopFilterTc:
  {
    auto lftc = static_cast<OMX_ALG_VIDEO_PARAM_LOOP_FILTER_TC*>(param);
    return SetVideoLoopFilterTc(*lftc, *port, media);
  }
  // only decoder
  case OMX_ALG_IndexParamPreallocation:
  {
    auto p = (OMX_ALG_PARAM_PREALLOCATION*)param;
    this->shouldPrealloc = (p->bDisablePreallocation == OMX_FALSE);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoDecodedPictureBuffer:
  {
    auto dpb = static_cast<OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER*>(param);

    return SetVideoDecodedPictureBuffer(*dpb, *port, media);
  }
  case OMX_ALG_IndexParamVideoInternalEntropyBuffers:
  {
    auto ieb = static_cast<OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS*>(param);
    return SetVideoInternalEntropyBuffers(*ieb, *port, media);
  }
  case OMX_ALG_IndexParamCommonSequencePictureModeCurrent:
  {
    auto spm = static_cast<OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE*>(param);
    return SetCommonSequencePictureMode(*spm, *port, media);
  }
  case OMX_ALG_IndexParamVideoInputParsed:
  {
    auto ip = static_cast<OMX_ALG_VIDEO_PARAM_INPUT_PARSED*>(param);
    return SetVideoInputParsed(*ip, *port, media);
  }
  case OMX_ALG_IndexParamVideoQuantizationTable:
  {
    auto table = static_cast<OMX_ALG_VIDEO_PARAM_QUANTIZATION_TABLE*>(param);
    return SetVideoQuantizationTable(*table, *port, media);
  }
  case OMX_ALG_IndexParamVideoAccessUnitDelimiter:
  {
    auto aud = static_cast<OMX_ALG_VIDEO_PARAM_ACCESS_UNIT_DELIMITER*>(param);
    return SetVideoAccessUnitDelimiter(*aud, *port, media);
  }
  case OMX_ALG_IndexParamVideoBufferingPeriodSEI:
  {
    auto bpSEI = static_cast<OMX_ALG_VIDEO_PARAM_BUFFERING_PERIOD_SEI*>(param);
    return SetVideoBufferingPeriodSEI(*bpSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoPictureTimingSEI:
  {
    auto ptSEI = static_cast<OMX_ALG_VIDEO_PARAM_PICTURE_TIMING_SEI*>(param);
    return SetVideoPictureTimingSEI(*ptSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoRecoveryPointSEI:
  {
    auto rpSEI = static_cast<OMX_ALG_VIDEO_PARAM_RECOVERY_POINT_SEI*>(param);
    return SetVideoRecoveryPointSEI(*rpSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoMasteringDisplayColourVolumeSEI:
  {
    auto mdcvSEI = static_cast<OMX_ALG_VIDEO_PARAM_MASTERING_DISPLAY_COLOUR_VOLUME_SEI*>(param);
    return SetVideoMasteringDisplayColourVolumeSEI(*mdcvSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoContentLightLevelSEI:
  {
    auto cllSEI = static_cast<OMX_ALG_VIDEO_PARAM_CONTENT_LIGHT_LEVEL_SEI*>(param);
    return SetVideoContentLightLevelSEI(*cllSEI, *port, media);
  }
  case OMX_ALG_IndexParamVideoST209410SEI:
  {
    auto st209410 = static_cast<OMX_ALG_VIDEO_PARAM_ST2094_10_SEI*>(param);
    return SetVideoST209410SEI(*st209410, *port, media);
  }
  case OMX_ALG_IndexParamVideoST209440SEI:
  {
    auto st209440 = static_cast<OMX_ALG_VIDEO_PARAM_ST2094_40_SEI*>(param);
    return SetVideoST209440SEI(*st209440, *port, media);
  }
  case OMX_ALG_IndexParamVideoRateControlPlugin:
  {
    auto rcPlugin = static_cast<OMX_ALG_VIDEO_PARAM_RATE_CONTROL_PLUGIN*>(param);
    return SetVideoRateControlPlugin(*rcPlugin, *port, media);
  }
  case OMX_ALG_IndexParamVideoCrop:
  {
    auto crop = static_cast<OMX_CONFIG_RECTTYPE*>(param);
    return SetVideoCrop(*crop, *port, media);
  }
  case OMX_ALG_IndexParamVideoMaxPictureSizesInBits:
  {
    auto mps = static_cast<OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES_IN_BITS*>(param);
    return SetVideoMaxPictureSizesInBits(*mps, *port, media);
  }
  default:
    LOG_ERROR(ToStringOMXIndex(index) + string { " is unsupported" });
    return OMX_ErrorUnsupportedIndex;
  }

  LOG_ERROR(ToStringOMXIndex(index) + string { " is unsupported" });
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_PARAMETER_OR_CONFIG();
}

static OMX_BUFFERHEADERTYPE* AllocateHeader(OMX_PTR app, int size, OMX_U8* buffer, bool isBufferAllocatedByModule, int index)
{
  auto header = new OMX_BUFFERHEADERTYPE;
  OMXChecker::SetHeaderVersion(*header);
  header->pBuffer = buffer;
  header->nAllocLen = size;
  header->pAppPrivate = app;
  header->pInputPortPrivate = new bool(isBufferAllocatedByModule);
  header->pOutputPortPrivate = new bool(isBufferAllocatedByModule);
  auto& p = IsInputPort(index) ? header->nInputPortIndex : header->nOutputPortIndex;
  p = index;

  return header;
}

static inline bool isBufferAllocatedByModule(OMX_BUFFERHEADERTYPE const* header)
{
  if(!header->pInputPortPrivate || !header->pOutputPortPrivate)
    return false;

  auto isInputAllocated = *(bool*)(header->pInputPortPrivate);
  auto isOutputAllocated = *(bool*)(header->pOutputPortPrivate);

  return isInputAllocated || isOutputAllocated;
}

static void DeleteHeader(OMX_BUFFERHEADERTYPE* header)
{
  delete (bool*)header->pInputPortPrivate;
  delete (bool*)header->pOutputPortPrivate;
  delete header;
}

OMX_ERRORTYPE Component::UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size, OMX_IN OMX_U8* buffer)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckNotNull(size);

  CheckPortIndex(index);
  auto port = GetPort(index);

  if(transientState != TransientState::LoadedToIdle && !(port->isTransientToEnable))
    throw OMX_ErrorIncorrectStateOperation;

  *header = AllocateHeader(app, size, buffer, false, index);
  assert(*header);
  port->Add(*header);

  return OMX_ErrorNone;
  OMX_CATCH_L([&](OMX_ERRORTYPE& e)
  {
    if(e != OMX_ErrorBadPortIndex)
      GetPort(index)->ErrorOccured();
  });
}

OMX_ERRORTYPE Component::AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckNotNull(size);
  CheckPortIndex(index);

  auto port = GetPort(index);

  if(transientState != TransientState::LoadedToIdle && !(port->isTransientToEnable))
    throw OMX_ErrorIncorrectStateOperation;

  auto buffer = static_cast<OMX_U8*>(module->Allocate(size * sizeof(OMX_U8)));

  if(!buffer)
    throw OMX_ErrorInsufficientResources;

  *header = AllocateHeader(app, size, buffer, true, index);
  assert(*header);
  port->Add(*header);

  return OMX_ErrorNone;
  OMX_CATCH_L([&](OMX_ERRORTYPE& e)
  {
    if(e != OMX_ErrorBadPortIndex)
      GetPort(index)->ErrorOccured();
  });
}

OMX_ERRORTYPE Component::FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);

  CheckPortIndex(index);
  auto port = GetPort(index);

  if((transientState != TransientState::IdleToLoaded) && (!port->isTransientToDisable))
    callbacks.EventHandler(component, app, OMX_EventError, OMX_ErrorPortUnpopulated, 0, nullptr);

  if(isBufferAllocatedByModule(header))
    module->Free(header->pBuffer);

  port->Remove(header);
  DeleteHeader(header);

  return OMX_ErrorNone;
  OMX_CATCH_L([&](OMX_ERRORTYPE& e)
  {
    if(e != OMX_ErrorBadPortIndex)
      GetPort(index)->ErrorOccured();
  });
}

void Component::AttachMark(OMX_BUFFERHEADERTYPE* header)
{
  assert(header);

  if(marks.empty())
    return;

  if(header->hMarkTargetComponent)
    return;

  auto mark = marks.front();
  header->hMarkTargetComponent = mark->hMarkTargetComponent;
  header->pMarkData = mark->pMarkData;
  marks.pop();

  if(header->hMarkTargetComponent == component)
    callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandMarkBuffer, input.index, mark);
}

OMX_ERRORTYPE Component::EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::EmptyThisBuffer, state);
  CheckPortIndex(header->nInputPortIndex);

  processorMain->queue(CreateTask(Command::EmptyBuffer, static_cast<OMX_U32>(input.index), shared_ptr<void>(header, nullDeleter)));

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Component::FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::FillThisBuffer, state);
  CheckPortIndex(header->nOutputPortIndex);

  header->nTimeStamp = 0;
  header->hMarkTargetComponent = nullptr;
  header->pMarkData = nullptr;
  header->nFlags = 0;

  processorMain->queue(CreateTask(Command::FillBuffer, static_cast<OMX_U32>(output.index), shared_ptr<void>(header, nullDeleter)));

  return OMX_ErrorNone;
  OMX_CATCH();
}

void Component::ComponentDeInit()
{
  if(eosHandles.input)
  {
    delete eosHandles.input;
    eosHandles.input = nullptr;
  }

  if(eosHandles.output)
  {
    delete eosHandles.output;
    eosHandles.output = nullptr;
  }
  free(role);
  free(name);
}

OMX_ERRORTYPE Component::GetComponentVersion(OMX_OUT OMX_STRING name, OMX_OUT OMX_VERSIONTYPE* version, OMX_OUT OMX_VERSIONTYPE* spec)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(name);
  OMXChecker::CheckNotNull(version);
  OMXChecker::CheckNotNull(spec);
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::GetComponentVersion, state);

  strncpy(name, this->name, OMX_MAX_STRINGNAME_SIZE);
  *version = this->version;
  *spec = this->spec;

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Component::GetConfig(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR config)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(config);
  OMXChecker::CheckHeaderVersion(GetVersion(config));
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::GetConfig, state);
  switch(static_cast<OMX_U32>(index))
  {
  case OMX_IndexConfigVideoBitrate:
  {
    auto& bitrate = *(static_cast<OMX_VIDEO_CONFIG_BITRATETYPE*>(config));
    module->GetDynamic(DYNAMIC_INDEX_BITRATE, &bitrate.nEncodeBitrate);
    return OMX_ErrorNone;
  }
  case OMX_IndexConfigVideoFramerate:
  {
    Clock clock;
    module->GetDynamic(DYNAMIC_INDEX_CLOCK, &clock);
    auto& framerate = *(static_cast<OMX_CONFIG_FRAMERATETYPE*>(config));
    framerate.xEncodeFramerate = ConvertMediaToOMXFramerate(clock);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoGroupOfPictures:
  {
    Gop moduleGop;
    module->GetDynamic(DYNAMIC_INDEX_GOP, &moduleGop);
    auto& gop = *(static_cast<OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES*>(config));
    gop.nBFrames = ConvertMediaToOMXBFrames(moduleGop);
    gop.nPFrames = ConvertMediaToOMXPFrames(moduleGop);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoTransferCharacteristics:
  {
    TransferCharacteristicsType modTC;
    module->GetDynamic(DYNAMIC_INDEX_TRANSFER_CHARACTERISTICS, &modTC);
    auto& tc = *(static_cast<OMX_ALG_VIDEO_CONFIG_TRANSFER_CHARACTERISTICS*>(config));
    tc.eTransferCharac = ConvertMediaToOMXTransferCharacteristics(modTC);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoColorMatrix:
  {
    ColourMatrixType modCM;
    module->GetDynamic(DYNAMIC_INDEX_COLOUR_MATRIX, &modCM);
    auto& cm = *(static_cast<OMX_ALG_VIDEO_CONFIG_COLOR_MATRIX*>(config));
    cm.eColorMatrix = ConvertMediaToOMXColourMatrix(modCM);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoColorPrimaries:
  {
    ColorPrimariesType modCP;
    module->GetDynamic(DYNAMIC_INDEX_COLOR_PRIMARIES, &modCP);
    auto& cp = *(static_cast<OMX_ALG_VIDEO_CONFIG_COLOR_PRIMARIES*>(config));
    cp.eColorPrimaries = ConvertMediaToOMXColorPrimaries(modCP);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoHighDynamicRangeSEI:
  {
    HighDynamicRangeSeis modHDRSEIs;
    module->GetDynamic(DYNAMIC_INDEX_HIGH_DYNAMIC_RANGE_SEIS, &modHDRSEIs);
    auto& hdrSEIs = *(static_cast<OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI*>(config));
    hdrSEIs = ConvertMediaToOMXHDRSEI(modHDRSEIs);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoMaxResolutionChange:
  {
    Dimension<int> maxDimensionSupported;
    module->GetDynamic(DYNAMIC_INDEX_MAX_RESOLUTION_CHANGE_SUPPORTED, &maxDimensionSupported);
    auto& dimension = *(static_cast<OMX_ALG_VIDEO_CONFIG_MAX_RESOLUTION_CHANGE*>(config));
    dimension.nWidth = maxDimensionSupported.horizontal;
    dimension.nHeight = maxDimensionSupported.vertical;
    return OMX_ErrorNone;
  }
  default:
    LOG_ERROR(ToStringOMXIndex(index) + string { " is unsupported" });
    return OMX_ErrorUnsupportedIndex;
  }

  LOG_ERROR(ToStringOMXIndex(index) + string { " is unsupported" });
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_PARAMETER_OR_CONFIG();
}

OMX_ERRORTYPE Component::SetConfig(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR config)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(config);
  OMXChecker::CheckHeaderVersion(GetVersion(config));
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::SetConfig, state);
  switch(static_cast<OMX_U32>(index))
  {
  case OMX_IndexConfigVideoBitrate:
  {
    OMX_VIDEO_CONFIG_BITRATETYPE* bitrate = new OMX_VIDEO_CONFIG_BITRATETYPE;
    memcpy(bitrate, static_cast<OMX_VIDEO_CONFIG_BITRATETYPE*>(config), sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));

    if(bitrate->nEncodeBitrate == 0)
      throw OMX_ErrorBadParameter;

    processorMain->queue(CreateTask(Command::SetDynamic, OMX_IndexConfigVideoBitrate, shared_ptr<void>(bitrate)));

    return OMX_ErrorNone;
  }
  case OMX_IndexConfigVideoFramerate:
  {
    OMX_CONFIG_FRAMERATETYPE* framerate = new OMX_CONFIG_FRAMERATETYPE;
    memcpy(framerate, static_cast<OMX_CONFIG_FRAMERATETYPE*>(config), sizeof(OMX_CONFIG_FRAMERATETYPE));

    processorMain->queue(CreateTask(Command::SetDynamic, OMX_IndexConfigVideoFramerate, shared_ptr<void>(framerate)));

    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh:
  {
    OMX_ALG_VIDEO_CONFIG_INSERT* idr = new OMX_ALG_VIDEO_CONFIG_INSERT;
    memcpy(idr, static_cast<OMX_ALG_VIDEO_CONFIG_INSERT*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_INSERT));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh, shared_ptr<void>(idr)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoGroupOfPictures:
  {
    OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES* gop = new OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES;
    memcpy(gop, static_cast<OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoGroupOfPictures, shared_ptr<void>(gop)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoRegionOfInterest:
  {
    OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST* roi = new OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST;
    memcpy(roi, static_cast<OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoRegionOfInterest, shared_ptr<void>(roi)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoRegionOfInterestByValue:
  {
    OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST_BY_VALUE* roi = new OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST_BY_VALUE;
    memcpy(roi, static_cast<OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST_BY_VALUE*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST_BY_VALUE));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoRegionOfInterestByValue, shared_ptr<void>(roi)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoNotifySceneChange:
  {
    OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE* notifySceneChange = new OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE;
    memcpy(notifySceneChange, static_cast<OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoNotifySceneChange, shared_ptr<void>(notifySceneChange)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoInsertLongTerm:
  {
    OMX_ALG_VIDEO_CONFIG_INSERT* lt = new OMX_ALG_VIDEO_CONFIG_INSERT;
    memcpy(lt, static_cast<OMX_ALG_VIDEO_CONFIG_INSERT*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_INSERT));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoInsertLongTerm, shared_ptr<void>(lt)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoUseLongTerm:
  {
    OMX_ALG_VIDEO_CONFIG_INSERT* lt = new OMX_ALG_VIDEO_CONFIG_INSERT;
    memcpy(lt, static_cast<OMX_ALG_VIDEO_CONFIG_INSERT*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_INSERT));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoUseLongTerm, shared_ptr<void>(lt)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoInsertPrefixSEI:
  {
    OMX_ALG_VIDEO_CONFIG_SEI* userPrefixSei = static_cast<OMX_ALG_VIDEO_CONFIG_SEI*>(config);
    OMX_ALG_VIDEO_CONFIG_SEI* seiPrefix = new OMX_ALG_VIDEO_CONFIG_SEI {};
    memcpy(seiPrefix, userPrefixSei, sizeof(OMX_ALG_VIDEO_CONFIG_SEI));
    seiPrefix->pBuffer = new OMX_U8[userPrefixSei->nAllocLen] {};
    memcpy(seiPrefix->pBuffer, userPrefixSei->pBuffer, userPrefixSei->nAllocLen);
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoInsertPrefixSEI, shared_ptr<void>(seiPrefix)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoInsertSuffixSEI:
  {
    OMX_ALG_VIDEO_CONFIG_SEI* userSuffixSei = static_cast<OMX_ALG_VIDEO_CONFIG_SEI*>(config);
    OMX_ALG_VIDEO_CONFIG_SEI* seiSuffix = new OMX_ALG_VIDEO_CONFIG_SEI {};
    memcpy(seiSuffix, userSuffixSei, sizeof(OMX_ALG_VIDEO_CONFIG_SEI));
    seiSuffix->pBuffer = new OMX_U8[userSuffixSei->nAllocLen] {};
    memcpy(seiSuffix->pBuffer, userSuffixSei->pBuffer, userSuffixSei->nAllocLen);
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoInsertSuffixSEI, shared_ptr<void>(seiSuffix)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoHighDynamicRangeSEI:
  {
    OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI* hdrSEIS = new OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI {};
    memcpy(hdrSEIS, static_cast<OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoHighDynamicRangeSEI, shared_ptr<void>(hdrSEIS)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoNotifyResolutionChange:
  {
    OMX_ALG_VIDEO_CONFIG_NOTIFY_RESOLUTION_CHANGE* drc = new OMX_ALG_VIDEO_CONFIG_NOTIFY_RESOLUTION_CHANGE;
    memcpy(drc, static_cast<OMX_ALG_VIDEO_CONFIG_NOTIFY_RESOLUTION_CHANGE*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_NOTIFY_RESOLUTION_CHANGE));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoNotifyResolutionChange, shared_ptr<void>(drc)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoQuantizationParameterTable:
  {
    QPs qps;
    media->Get(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &qps);
    OMX_ALG_VIDEO_CONFIG_DATA* userData = static_cast<OMX_ALG_VIDEO_CONFIG_DATA*>(config);
    OMX_ALG_VIDEO_CONFIG_DATA* data = new OMX_ALG_VIDEO_CONFIG_DATA {};
    memcpy(data, userData, sizeof(OMX_ALG_VIDEO_CONFIG_DATA));
    data->pBuffer = new OMX_U8[userData->nAllocLen] {};
    memcpy(data->pBuffer, userData->pBuffer, userData->nAllocLen);
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoQuantizationParameterTable, shared_ptr<void>(data)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoLoopFilterBeta:
  {
    OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_BETA* lfb = new OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_BETA;
    memcpy(lfb, static_cast<OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_BETA*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_BETA));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoLoopFilterBeta, shared_ptr<void>(lfb)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoLoopFilterTc:
  {
    OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_TC* lftc = new OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_TC;
    memcpy(lftc, static_cast<OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_TC*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_TC));
    processorMain->queue(CreateTask(Command::SetDynamic, OMX_ALG_IndexConfigVideoLoopFilterTc, shared_ptr<void>(lftc)));
    return OMX_ErrorNone;
  }
  default:
    LOG_ERROR(ToStringOMXIndex(index) + string { " is unsupported" });
    return OMX_ErrorUnsupportedIndex;
  }

  LOG_ERROR(ToStringOMXIndex(index) + string { " is unsupported" });
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_PARAMETER_OR_CONFIG();
}

OMX_ERRORTYPE Component::GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(name);
  OMXChecker::CheckNotNull(index);
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::GetExtensionIndex, state);
  return OMX_ErrorNoMore;
  OMX_CATCH();
}

OMX_ERRORTYPE Component::ComponentTunnelRequest(OMX_IN OMX_U32 index, OMX_IN OMX_HANDLETYPE comp, OMX_IN OMX_U32 tunneledIndex, OMX_INOUT OMX_TUNNELSETUPTYPE* setup)
{
  (void)index, (void)tunneledIndex, (void)comp, (void)setup;
  return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE Component::UseEGLImage(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN void* eglImage)
{
  (void)header, (void)index, (void)app, (void)eglImage;
  return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE Component::ComponentRoleEnum(OMX_OUT OMX_U8* role, OMX_IN OMX_U32 index)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(role);
  OMXChecker::CheckStateOperation(OMXChecker::ComponentMethods::ComponentRoleEnum, state);

  if(index != 0)
    return OMX_ErrorNoMore;

  strncpy((OMX_STRING)role, this->role, OMX_MAX_STRINGNAME_SIZE);

  return OMX_ErrorNone;
  OMX_CATCH();
}

static inline bool isTransitionToIdleFromLoadedOrWaitRessource(OMX_STATETYPE previousState, OMX_STATETYPE state)
{
  if(state != OMX_StateIdle)
    return false;

  if((previousState != OMX_StateLoaded) && (previousState != OMX_StateWaitForResources))
    return false;
  return true;
}

static inline bool isTransitionToLoaded(OMX_STATETYPE previousState, OMX_STATETYPE state)
{
  if(state != OMX_StateLoaded)
    return false;

  if((previousState != OMX_StateIdle) && (previousState != OMX_StateWaitForResources))
    return false;
  return true;
}

static inline bool isTransitionToExecutingOrPause(OMX_STATETYPE previousState, OMX_STATETYPE state)
{
  if(state != OMX_StateExecuting && state != OMX_StatePause)
    return false;

  if(previousState != OMX_StateIdle)
    return false;
  return true;
}

static inline bool isTransitionToIdleFromExecutingOrPause(OMX_STATETYPE previousState, OMX_STATETYPE state)
{
  if(state != OMX_StateIdle)
    return false;

  if((previousState != OMX_StateExecuting) && (previousState != OMX_StatePause))
    return false;
  return true;
}

static inline bool isTransitionFromPause(OMX_STATETYPE previousState)
{
  return previousState == OMX_StatePause;
}

static inline bool isTransitionToPause(OMX_STATETYPE previousState, OMX_STATETYPE state)
{
  if(state != OMX_StatePause)
    return false;

  if((previousState != OMX_StateExecuting) && (previousState != OMX_StateIdle))
    return false;
  return true;
}

void Component::PopulatingPorts()
{
  for(auto i = videoPortParams.nStartPortNumber; i < videoPortParams.nPorts; i++)
  {
    auto port = GetPort(i);

    if(port->enable)
      port->WaitFull();

    if(port->error)
    {
      port->ResetError();
      throw OMX_ErrorInsufficientResources;
    }
  }
}

void Component::UnpopulatingPorts()
{
  for(auto i = videoPortParams.nStartPortNumber; i < videoPortParams.nPorts; i++)
  {
    auto port = GetPort(i);
    port->WaitEmpty();

    if(port->error)
    {
      port->ResetError();
      throw OMX_ErrorUndefined;
    }
  }
}

void Component::FlushFillEmptyBuffers(bool fill, bool empty)
{
  auto buffersFillBlocked = (pauseFillPromise != nullptr);
  auto buffersEmptyBlocked = (pauseEmptyPromise != nullptr);

  if(buffersFillBlocked || buffersEmptyBlocked)
    UnblockFillEmptyBuffers();

  if(fill)
  {
    auto deleteFill = bind(&Component::_DeleteFill, this, placeholders::_1);
    auto processFill = bind(&Component::_ProcessFillBuffer, this, placeholders::_1);
    processorFill.reset(new ProcessorFifo<Task> { processFill, deleteFill, "OMX - Out" });
  }

  if(empty)
  {
    auto deleteEmpty = bind(&Component::_DeleteEmpty, this, placeholders::_1);
    auto processEmpty = bind(&Component::_ProcessEmptyBuffer, this, placeholders::_1);
    processorEmpty.reset(new ProcessorFifo<Task> { processEmpty, deleteEmpty, "OMX - In" });
  }

  if(buffersFillBlocked || buffersEmptyBlocked)
    BlockFillEmptyBuffers(buffersFillBlocked, buffersEmptyBlocked);
}

void Component::CleanFlushFillEmptyBuffers()
{
  shared_ptr<promise<void>> signalPromise;
  signalPromise.reset(new promise<void> );
  processorFill->queue(CreateTask(Command::Signal, state, signalPromise));
  signalPromise->get_future().wait();
  signalPromise.reset(new promise<void> );
  processorEmpty->queue(CreateTask(Command::Signal, state, signalPromise));
  signalPromise->get_future().wait();
}

void Component::BlockFillEmptyBuffers(bool fill, bool empty)
{
  if(fill && !pauseFillPromise)
  {
    pauseFillPromise.reset(new promise<void> );
    auto pauseFillFuture = make_shared<shared_future<void>>(pauseFillPromise->get_future());
    processorFill->queue(CreateTask(Command::SharedFence, state, pauseFillFuture));
  }

  if(empty && !pauseEmptyPromise)
  {
    pauseEmptyPromise.reset(new promise<void> );
    auto pauseEmptyFuture = make_shared<shared_future<void>>(pauseEmptyPromise->get_future());
    processorEmpty->queue(CreateTask(Command::SharedFence, state, pauseEmptyFuture));
  }
}

void Component::UnblockFillEmptyBuffers()
{
  if(pauseFillPromise)
  {
    pauseFillPromise->set_value();
    pauseFillPromise = nullptr;
  }

  if(pauseEmptyPromise)
  {
    pauseEmptyPromise->set_value();
    pauseEmptyPromise = nullptr;
  }
}

void Component::FlushComponent()
{
  FlushFillEmptyBuffers(true, true);
}

void Component::FlushEosHandles()
{
  if(eosHandles.input)
    ReleaseCallBack(true, eosHandles.input);

  if(eosHandles.output)
    ReleaseCallBack(false, eosHandles.output);

  eosHandles.input = nullptr;
  eosHandles.output = nullptr;
}

static bool isFlushingRequired(OMX_STATETYPE prevState, OMX_STATETYPE newState)
{
  try
  {
    OMXChecker::CheckStateExistance(prevState);
    OMXChecker::CheckStateExistance(newState);
    OMXChecker::CheckStateTransition(prevState, newState);
    auto transientState = GetTransientState(prevState, newState);

    if(transientState == TransientState::ExecutingToPause || transientState == TransientState::ExecutingToIdle || newState == OMX_StateInvalid)
      return true;
    return false;
  }
  catch(OMX_ERRORTYPE& e)
  {
    return false;
  }
}

void Component::TreatSetStateCommand(Task task)
{
  try
  {
    assert(task.cmd == Command::SetState);
    auto newState = (OMX_STATETYPE)((uintptr_t)task.data);
    LOG_IMPORTANT(string { "Set State: " } +ToStringOMXState(newState));
    OMXChecker::CheckStateTransition(state, newState);

    if(isTransitionToIdleFromLoadedOrWaitRessource(state, newState))
      PopulatingPorts();

    if(isTransitionToLoaded(state, newState) && (state != OMX_StateWaitForResources))
    {
      UnpopulatingPorts();
      transientState = TransientState::Max;
    }

    if(isTransitionToExecutingOrPause(state, newState))
    {
      auto error = module->Start(shouldPrealloc);

      if(error)
        throw ToOmxError(error);

      if(shouldPrealloc && shouldFireEventPortSettingsChanges)
      {
        callbacks.EventHandler(component, app, static_cast<OMX_EVENTTYPE>(OMX_EventPortSettingsChanged), 1, 0, nullptr);
        shouldFireEventPortSettingsChanges = false;
      }
    }

    if(isTransitionToIdleFromExecutingOrPause(state, newState))
      module->Stop();

    if(isTransitionToPause(state, newState))
      BlockFillEmptyBuffers(true, true);

    if(isTransitionFromPause(state) && input.enable && output.enable && !input.isTransientToEnable && !output.isTransientToEnable)
      UnblockFillEmptyBuffers();

    state = newState;

    callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandStateSet, newState, nullptr);
  }
  catch(OMX_ERRORTYPE& e)
  {
    if(e == OMX_ErrorInvalidState)
    {
      module->Stop();
      state = OMX_StateInvalid;
    }

    if(task.opt.get() != nullptr)
      e = (OMX_ERRORTYPE)((uintptr_t)task.opt.get());

    LOG_ERROR(ToStringOMXError(e));
    callbacks.EventHandler(component, app, OMX_EventError, e, 0, nullptr);
  }
}

void Component::TreatFlushCommand(Task task)
{
  assert(task.cmd == Command::Flush);
  assert(task.opt.get() == nullptr);
  auto index = static_cast<OMX_U32>((uintptr_t)task.data);

  LOG_IMPORTANT(string { "Flush port: " } +to_string(index));

  if(module->Stop())
    module->Start(true);

  FlushEosHandles();

  callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandFlush, index, nullptr);
}

void Component::TreatDisablePortCommand(Task task)
{
  assert(task.cmd == Command::DisablePort);
  assert(task.opt.get() == nullptr);
  auto index = static_cast<OMX_U32>((uintptr_t)task.data);
  auto port = GetPort(index);

  if(!port->isTransientToDisable)
  {
    callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandPortDisable, index, nullptr);
    return;
  }

  bool isInput = IsInputPort(index);
  FlushFillEmptyBuffers(!isInput, isInput);
  BlockFillEmptyBuffers(true, shouldPrealloc);

  if(shouldPrealloc && shouldFireEventPortSettingsChanges && (state == OMX_StateExecuting || state == OMX_StatePause))
  {
    if(module->Stop())
      module->Start(true);

    if(shouldFireEventPortSettingsChanges)
    {
      callbacks.EventHandler(component, app, static_cast<OMX_EVENTTYPE>(OMX_EventPortSettingsChanged), 1, 0, nullptr);
      shouldFireEventPortSettingsChanges = false;
    }
  }

  port->WaitEmpty();

  if(port->error)
    return;

  port->isTransientToDisable = false;

  callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandPortDisable, index, nullptr);
}

void Component::TreatEnablePortCommand(Task task)
{
  assert(task.cmd == Command::EnablePort);
  assert(task.opt.get() == nullptr);
  auto index = static_cast<OMX_U32>((uintptr_t)task.data);
  auto port = GetPort(index);

  if(!port->isTransientToEnable)
  {
    callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandPortEnable, index, nullptr);
    return;
  }

  if(state != OMX_StateLoaded && state != OMX_StateWaitForResources)
    port->WaitFull();

  if(port->error)
    return;

  port->isTransientToEnable = false;

  if(shouldPrealloc && shouldFireEventPortSettingsChanges && (state == OMX_StateExecuting || state == OMX_StatePause))
  {
    if(module->Stop())
      module->Start(true);

    if(shouldFireEventPortSettingsChanges)
    {
      callbacks.EventHandler(component, app, static_cast<OMX_EVENTTYPE>(OMX_EventPortSettingsChanged), 1, 0, nullptr);
      shouldFireEventPortSettingsChanges = false;
    }
  }

  if(input.enable && output.enable && !input.isTransientToEnable && !output.isTransientToEnable && state != OMX_StatePause)
    UnblockFillEmptyBuffers();

  callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandPortEnable, index, nullptr);
}

void Component::TreatMarkBufferCommand(Task task)
{
  assert(task.cmd == Command::MarkBuffer);
  assert(static_cast<int>((uintptr_t)task.data) == input.index);
  auto mark = static_cast<OMX_MARKTYPE*>(task.opt.get());
  assert(mark);

  marks.push(mark);
}

void Component::TreatEmptyBufferCommand(Task* task)
{
  assert(task);
  assert(task->cmd == Command::EmptyBuffer);
  assert(static_cast<int>((uintptr_t)task->data) == input.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task->opt.get());
  assert(header);

  if(state == OMX_StateInvalid)
  {
    callbacks.EmptyBufferDone(component, app, header);
    return;
  }

  AttachMark(header);

  if(header->nFilledLen == 0)
  {
    if(header->nFlags & OMX_BUFFERFLAG_EOS)
    {
      auto handle = new OMXBufferHandle(header);
      eosHandles.input = handle;
      auto success = module->Empty(handle);
      assert(success);
      return;
    }
    callbacks.EmptyBufferDone(component, app, header);
    return;
  }

  auto handle = new OMXBufferHandle(header);

  auto success = module->Empty(handle);
  assert(success);

  if(header->nFlags & OMX_BUFFERFLAG_EOS)
  {
    success = module->Empty(nullptr);
    assert(success);
    return;
  }
}

void Component::TreatFillBufferCommand(Task task)
{
  assert(task.cmd == Command::FillBuffer);
  assert(static_cast<int>((uintptr_t)task.data) == output.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task.opt.get());
  assert(header);

  if(state == OMX_StateInvalid)
  {
    callbacks.FillBufferDone(component, app, header);
    return;
  }

  auto handle = new OMXBufferHandle(header);

  if(!eosHandles.output)
  {
    eosHandles.output = handle;
    return;
  }
  auto success = module->Fill(handle);
  assert(success);
}

static RegionQuality CreateRegionQualityByPreset(OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST const& roi)
{
  RegionQuality rq {};
  rq.region.point.x = roi.nLeft;
  rq.region.point.y = roi.nTop;
  rq.region.dimension.horizontal = roi.nWidth;
  rq.region.dimension.vertical = roi.nHeight;
  rq.quality.byPreset = ConvertOMXToMediaQualityPreset(roi.eQuality);
  return rq;
}

static RegionQuality CreateRegionQualityByValue(OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST_BY_VALUE const& roi)
{
  RegionQuality rq {};
  rq.region.point.x = roi.nLeft;
  rq.region.point.y = roi.nTop;
  rq.region.dimension.horizontal = roi.nWidth;
  rq.region.dimension.vertical = roi.nHeight;
  rq.quality.byValue = ConvertOMXToMediaQualityValue(roi.nQuality);
  return rq;
}

void Component::TreatDynamicCommand(Task task)
{
  assert(task.cmd == Command::SetDynamic);
  auto index = static_cast<OMX_U32>((uintptr_t)task.data);
  void* opt = task.opt.get();
  assert(opt);

  if(state == OMX_StateInvalid)
    return;
  switch(index)
  {
  case OMX_IndexConfigVideoBitrate:
  {
    auto bitrate = static_cast<OMX_VIDEO_CONFIG_BITRATETYPE*>(opt);
    module->SetDynamic(DYNAMIC_INDEX_BITRATE, (void*)(static_cast<intptr_t>(bitrate->nEncodeBitrate)));
    return;
  }
  case OMX_IndexConfigVideoFramerate:
  {
    auto framerate = static_cast<OMX_CONFIG_FRAMERATETYPE*>(opt);
    auto clock = ConvertOMXToMediaClock(framerate->xEncodeFramerate);
    Clock moduleClock {};
    media->Get(SETTINGS_INDEX_CLOCK, &moduleClock);
    moduleClock.framerate = clock.framerate;
    moduleClock.clockratio = clock.clockratio;
    module->SetDynamic(DYNAMIC_INDEX_CLOCK, &moduleClock);
    return;
  }
  case OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh:
  {
    module->SetDynamic(DYNAMIC_INDEX_INSERT_IDR, nullptr);
    return;
  }
  case OMX_ALG_IndexConfigVideoGroupOfPictures:
  {
    auto gop = static_cast<OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES*>(opt);
    Gop moduleGop {};
    media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &moduleGop);
    moduleGop.b = ConvertOMXToMediaBFrames(gop->nBFrames, gop->nPFrames);
    moduleGop.length = ConvertOMXToMediaGopLength(gop->nBFrames, gop->nPFrames);
    module->SetDynamic(DYNAMIC_INDEX_GOP, &moduleGop);
    return;
  }
  case OMX_ALG_IndexConfigVideoRegionOfInterest:
  {
    auto roi = static_cast<OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST*>(opt);

    if(shouldClearROI)
    {
      module->SetDynamic(DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_CLEAR, nullptr);
      shouldClearROI = false;
    }

    auto rq = CreateRegionQualityByPreset(*roi);
    module->SetDynamic(DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD_BY_PRESET, &rq);
    shouldPushROI = true;
    return;
  }
  case OMX_ALG_IndexConfigVideoRegionOfInterestByValue:
  {
    auto roi = static_cast<OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST_BY_VALUE*>(opt);

    if(shouldClearROI)
    {
      module->SetDynamic(DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_CLEAR, nullptr);
      shouldClearROI = false;
    }

    auto rq = CreateRegionQualityByValue(*roi);
    module->SetDynamic(DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD_BY_VALUE, &rq);
    shouldPushROI = true;
    return;
  }
  case OMX_ALG_IndexConfigVideoNotifySceneChange:
  {
    auto notifySceneChange = static_cast<OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE*>(opt);
    module->SetDynamic(DYNAMIC_INDEX_NOTIFY_SCENE_CHANGE, (void*)(static_cast<intptr_t>(notifySceneChange->nLookAhead)));
    return;
  }
  case OMX_ALG_IndexConfigVideoInsertLongTerm:
  {
    module->SetDynamic(DYNAMIC_INDEX_IS_LONG_TERM, nullptr);
    return;
  }
  case OMX_ALG_IndexConfigVideoUseLongTerm:
  {
    module->SetDynamic(DYNAMIC_INDEX_USE_LONG_TERM, nullptr);
    return;
  }
  case OMX_ALG_IndexConfigVideoInsertPrefixSEI:
  {
    OMXSei prefix {
      *static_cast<OMX_ALG_VIDEO_CONFIG_SEI*>(opt), true
    };
    tmpSeis.push_back(prefix);
    return;
  }
  case OMX_ALG_IndexConfigVideoInsertSuffixSEI:
  {
    OMXSei suffix {
      *static_cast<OMX_ALG_VIDEO_CONFIG_SEI*>(opt), false
    };
    tmpSeis.push_back(suffix);
    return;
  }
  case OMX_ALG_IndexConfigVideoHighDynamicRangeSEI:
  {
    auto hdrSEIs = ConvertOMXToMediaHDRSEI(*static_cast<OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI*>(opt));
    module->SetDynamic(DYNAMIC_INDEX_HIGH_DYNAMIC_RANGE_SEIS, &hdrSEIs);
    return;
  }
  case OMX_ALG_IndexConfigVideoNotifyResolutionChange:
  {
    Resolution resolution {};
    auto ret = media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
    assert(ret == MediatypeInterface::SUCCESS);
    auto drc = static_cast<OMX_ALG_VIDEO_CONFIG_NOTIFY_RESOLUTION_CHANGE*>(opt);
    resolution.dimension.horizontal = drc->nWidth;
    resolution.dimension.vertical = drc->nHeight;
    module->SetDynamic(DYNAMIC_INDEX_RESOLUTION, (void*)(&resolution));
    return;
  }
  case OMX_ALG_IndexConfigVideoQuantizationParameterTable:
  {
    auto data = static_cast<OMX_ALG_VIDEO_CONFIG_DATA*>(opt);
    module->SetDynamic(DYNAMIC_INDEX_INSERT_QUANTIZATION_PARAMETER_BUFFER, data->pBuffer + data->nOffset);
    delete[] data->pBuffer;
    return;
  }
  case OMX_ALG_IndexConfigVideoLoopFilterBeta:
  {
    auto beta = static_cast<OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_BETA*>(opt);
    module->SetDynamic(DYNAMIC_INDEX_LOOP_FILTER_BETA, (void*)(static_cast<intptr_t>(beta->nLoopFilterBeta)));
    return;
  }
  case OMX_ALG_IndexConfigVideoLoopFilterTc:
  {
    auto tc = static_cast<OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_TC*>(opt);
    module->SetDynamic(DYNAMIC_INDEX_LOOP_FILTER_TC, (void*)(static_cast<intptr_t>(tc->nLoopFilterTc)));
    return;
  }
  default:
    return;
  }
}

static void TreatSharedFenceCommand(Task* task)
{
  auto p = (shared_future<void>*)task->opt.get();
  p->wait();
}

static void TreatSignalCommand(Task* task)
{
  auto p = (promise<void>*)task->opt.get();
  p->set_value();
}

void Component::_ProcessMain(Task task)
{
  switch(task.cmd)
  {
  case Command::SetState:
  {
    auto newState = (OMX_STATETYPE)((uintptr_t)task.data);

    if(isFlushingRequired(state, newState))
      FlushFillEmptyBuffers(true, true);

    TreatSetStateCommand(task);
    break;
  }
  case Command::Flush:
  {
    FlushComponent();
    TreatFlushCommand(task);
    break;
  }
  case Command::DisablePort:
  {
    TreatDisablePortCommand(task);
    break;
  }
  case Command::EnablePort:
  {
    TreatEnablePortCommand(task);
    break;
  }
  case Command::MarkBuffer:
  {
    TreatMarkBufferCommand(task);
    break;
  }
  case Command::EmptyBuffer:
  {
    auto index = static_cast<int>((uintptr_t)task.data);
    assert(IsInputPort(index));
    processorEmpty->queue(task);
    break;
  }
  case Command::FillBuffer:
  {
    auto index = static_cast<int>((uintptr_t)task.data);
    assert(!IsInputPort(index));
    processorFill->queue(task);
    break;
  }
  case Command::SetDynamic:
  {
    processorEmpty->queue(task);
    break;
  }
  default:
    assert(0 == "bad command");
  }
}

void Component::_DeleteEmpty(Task task)
{
  if(task.cmd != Command::EmptyBuffer)
    return;

  assert(static_cast<int>((uintptr_t)task.data) == input.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task.opt.get());
  assert(header);
  callbacks.EmptyBufferDone(component, app, header);
}

void Component::_DeleteFill(Task task)
{
  if(task.cmd != Command::FillBuffer)
    return;

  assert(static_cast<int>((uintptr_t)task.data) == output.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task.opt.get());
  assert(header);
  callbacks.FillBufferDone(component, app, header);
}

void Component::_ProcessFillBuffer(Task task)
{
  if(task.cmd == Command::FillBuffer)
    TreatFillBufferCommand(task);
  else if(task.cmd == Command::SharedFence)
    TreatSharedFenceCommand(&task);
  else if(task.cmd == Command::Signal)
    TreatSignalCommand(&task);
  else
    assert(0 == "bad command");
}

void Component::_ProcessEmptyBuffer(Task task)
{
  if(task.cmd == Command::EmptyBuffer)
    TreatEmptyBufferCommand(&task);
  else if(task.cmd == Command::SharedFence)
    TreatSharedFenceCommand(&task);
  else if(task.cmd == Command::Signal)
    TreatSignalCommand(&task);
  else if(task.cmd == Command::SetDynamic)
    TreatDynamicCommand(task);
  else
    assert(0 == "bad command");
}

