/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
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

#include "omx_codec.h"
#include "base/omx_checker/omx_checker.h"
#include "base/omx_utils/omx_translate.h"
#include "base/omx_utils/omx_log.h"
#include <assert.h>
#include <string.h>
#include <future>

#include <OMX_VideoExt.h>
#include "omx_convert_omx_module.h"

using std::mutex;
using std::lock_guard;
using std::shared_ptr;

#define OMX_TRY() \
  try \
  { \
    void FORCE_SEMICOLON()

#define OMX_CATCH() \
  } \
  catch(OMX_ERRORTYPE& e) \
  { \
    LOGE("%s", ToStringOMXError.at(e)); \
    return e; \
  } \
  void FORCE_SEMICOLON()

#define OMX_CATCH_CONFIG() \
  } \
  catch(OMX_ERRORTYPE& e) \
  { \
    LOGE("%s : %s", ToStringOMXIndex.at(index), ToStringOMXError.at(e)); \
    return e; \
  } \
  void FORCE_SEMICOLON()

void Codec::ReturnEmptiedBuffer(uint8_t* emptied)
{
  if(!emptied)
    assert(0);

  auto header = map.Pop(emptied);
  ClearPropagatedData(header);

  if(callbacks.EmptyBufferDone)
    callbacks.EmptyBufferDone(component, app, header);
}

void Codec::EmptyThisBufferCallBack(uint8_t* emptied, int, int, void*)
{
  printf("[omx] emptied: %p\n", emptied);
  ReturnEmptiedBuffer(emptied);
}

void Codec::AssociateCallBack(uint8_t* empty, uint8_t* fill)
{
  auto emptyHeader = map.Get(empty);
  assert(emptyHeader);
  auto fillHeader = map.Get(fill);
  assert(fillHeader);

  PropagateHeaderData(emptyHeader, fillHeader);

  if(IsEOSDetected(emptyHeader->nFlags))
    callbacks.EventHandler(component, app, OMX_EventBufferFlag, output.index, emptyHeader->nFlags, nullptr);

  if(IsCompMarked(emptyHeader->hMarkTargetComponent, component) && callbacks.EventHandler)
    callbacks.EventHandler(component, app, OMX_EventMark, 0, 0, emptyHeader->pMarkData);
}

void Codec::ReturnFilledBuffer(uint8_t* filled, int offset, int size)
{
  if(!filled)
    assert(0);

  auto header = map.Pop(filled);

  header->nOffset = offset;
  header->nFilledLen = size;

  if(callbacks.FillBufferDone)
    callbacks.FillBufferDone(component, app, header);
}

void Codec::FillThisBufferCallBack(uint8_t* filled, int offset, int size)
{
  printf("[omx] filled: %p\n", filled);
  ReturnFilledBuffer(filled, offset, size);
}

void Codec::ReleaseCallBack(bool isInput, uint8_t* released)
{
  std::string type;
  if(isInput)
    type = "empty";
  else
    type = "fill";
  printf("[omx] (%s) released: %p\n", type.c_str(), released);
  if(isInput)
    ReturnEmptiedBuffer(released);
  else
    ReturnFilledBuffer(released, 0, 0);
}

void Codec::EventCallBack(CallbackEventType type, void* data)
{
  (void)data;

  if(type > CALLBACK_EVENT_MAX)
    assert(0);
  switch(type)
  {
  case CALLBACK_EVENT_ERROR:
  {
    LOGE("%s", ToStringCallbackEvent.at(type));

    if(callbacks.EventHandler)
      callbacks.EventHandler(component, app, OMX_EventError, OMX_ErrorUndefined, 0, nullptr);
    break;
  }
  default:
    LOGE("%s is unsupported", ToStringCallbackEvent.at(type));
  }
}

void Codec::CheckPortIndex(int index)
{
  if(index < static_cast<int>(ports.nStartPortNumber))
    throw OMX_ErrorBadPortIndex;

  auto const maxIndex = static_cast<int>(ports.nStartPortNumber + ports.nPorts - 1);

  if(index > maxIndex)
    throw OMX_ErrorBadPortIndex;
}

Port* Codec::GetPort(int index)
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

Codec::Codec(OMX_HANDLETYPE component, std::unique_ptr<ModuleInterface>&& module, OMX_STRING name, OMX_STRING role) :
  component(component), module(std::move(module)),
  input(0, this->module->GetBufferRequirements().input.min), output(1, this->module->GetBufferRequirements().output.min)
{
  assert(name);
  assert(role);

  CreateName(name);
  CreateRole(role);
  shouldPrealloc = true;
  shouldClearROI = false;
  shouldPushROI = false;
  version.nVersion = ALLEGRODVT_OMX_VERSION;
  AssociateSpecVersion(spec);

  OMXChecker::SetHeaderVersion(ports);
  SetPortsParam(ports);
  auto const p = std::bind(&Codec::_Process, this, std::placeholders::_1);
  auto const d = std::bind(&Codec::_Delete, this, std::placeholders::_1);
  auto const p2 = std::bind(&Codec::_ProcessFillBuffer, this, std::placeholders::_1);
  processor.reset(new ProcessorFifo(p, d));
  processorFill.reset(new ProcessorFifo(p2, d));

  transientState = TransientMax;
  state = OMX_StateLoaded;
}

Codec::~Codec()
{
}

void Codec::CreateName(OMX_STRING name)
{
  this->name = (OMX_STRING)malloc(OMX_MAX_STRINGNAME_SIZE * sizeof(char));
  strncpy(this->name, name, OMX_MAX_STRINGNAME_SIZE);
}

void Codec::CreateRole(OMX_STRING role)
{
  this->role = (OMX_STRING)malloc(OMX_MAX_STRINGNAME_SIZE * sizeof(char));
  strncpy(this->role, role, OMX_MAX_STRINGNAME_SIZE);
}

Task* CreateTask(Command cmd, OMX_U32 data, shared_ptr<void> opt)
{
  auto task = new Task();
  task->cmd = cmd;
  task->data = reinterpret_cast<uintptr_t*>(data);
  task->opt = opt;
  return task;
}

static TransientState GetTransientState(OMX_STATETYPE const& curState, OMX_STATETYPE const& nextState)
{
  if(curState == OMX_StateIdle && nextState == OMX_StateLoaded)
    return TransientIdleToLoaded;

  if(curState == OMX_StateLoaded && nextState == OMX_StateIdle)
    return TransientLoadedToIdle;

  return TransientMax;
}

void nullDeleter(void*)
{
}

void Codec::CreateCommand(OMX_COMMANDTYPE command, OMX_U32 param, OMX_PTR data)
{
  Command taskCommand;
  switch(command)
  {
  case OMX_CommandStateSet:
  {
    OMXChecker::CheckNull(data);

    auto const nextState = static_cast<OMX_STATETYPE>(param);
    OMXChecker::CheckStateExistance(nextState);

    TransientState NewtransientState = GetTransientState(state, nextState);

    if(NewtransientState == TransientLoadedToIdle)
      if(!module->CheckParam())
        throw OMX_ErrorUndefined;

    transientState = NewtransientState;
    taskCommand = SetState;
    break;
  }
  case OMX_CommandFlush:
  {
    OMXChecker::CheckNull(data);

    auto promise = std::make_shared<std::promise<int>>();
    processor->queue(CreateTask(Fence, param, promise));
    processorFill->queue(CreateTask(RemoveFence, param, promise));

    if(param == OMX_ALL)
    {
      for(auto i = ports.nStartPortNumber; i < ports.nPorts; i++)
        processor->queue(CreateTask(Flush, i, shared_ptr<void>(data, nullDeleter)));

      return;
    }

    printf("[omx] flush port %d\n", param); 
    CheckPortIndex(param);
    taskCommand = Flush;
    break;
  }
  case OMX_CommandPortDisable:
  {
    OMXChecker::CheckNull(data);

    if(param == OMX_ALL)
    {
      for(auto i = ports.nStartPortNumber; i < ports.nPorts; i++)
      {
        GetPort(i)->enable = false;
        GetPort(i)->isTransientToDisable = true;
        processor->queue(CreateTask(DisablePort, i, shared_ptr<void>(data, nullDeleter)));
      }

      return;
    }

    CheckPortIndex(param);
    GetPort(param)->enable = false;
    GetPort(param)->isTransientToDisable = true;
    taskCommand = DisablePort;
    break;
  }
  case OMX_CommandPortEnable:
  {
    OMXChecker::CheckNull(data);

    if(param == OMX_ALL)
    {
      for(auto i = ports.nStartPortNumber; i < ports.nPorts; i++)
      {
        GetPort(i)->enable = true;
        GetPort(i)->isTransientToEnable = true;
        processor->queue(CreateTask(EnablePort, i, shared_ptr<void>(data, nullDeleter)));
      }

      return;
    }

    CheckPortIndex(param);
    GetPort(param)->enable = true;
    GetPort(param)->isTransientToEnable = true;
    taskCommand = EnablePort;
    break;
  }
  case OMX_CommandMarkBuffer:
  {
    OMXChecker::CheckNotNull(data);

    if(!IsInputPort(param))
      throw OMX_ErrorBadPortIndex;

    taskCommand = MarkBuffer;
    break;
  }
  default:
    throw OMX_ErrorBadParameter;
  }

  processor->queue(CreateTask(taskCommand, param, shared_ptr<void>(data, nullDeleter)));
}

OMX_ERRORTYPE Codec::SendCommand(OMX_IN OMX_COMMANDTYPE cmd, OMX_IN OMX_U32 param, OMX_IN OMX_PTR data)
{
  OMX_TRY();
  OMXChecker::CheckStateOperation(AL_SendCommand, state);

  CreateCommand(cmd, param, data);

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Codec::GetState(OMX_OUT OMX_STATETYPE* state)
{
  OMX_TRY();
  OMXChecker::CheckNotNull<OMX_STATETYPE*>(state);

  *state = this->state;

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Codec::SetCallbacks(OMX_IN OMX_CALLBACKTYPE* callbacks, OMX_IN OMX_PTR app)
{
  OMX_TRY();
  OMXChecker::CheckNotNull<OMX_CALLBACKTYPE*>(callbacks);
  OMXChecker::CheckStateOperation(AL_SetCallbacks, state);

  auto empty = std::bind(&Codec::EmptyThisBufferCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  auto associate = std::bind(&Codec::AssociateCallBack, this, std::placeholders::_1, std::placeholders::_2);
  auto filled = std::bind(&Codec::FillThisBufferCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto event = std::bind(&Codec::EventCallBack, this, std::placeholders::_1, std::placeholders::_2);
  auto release = std::bind(&Codec::ReleaseCallBack, this, std::placeholders::_1, std::placeholders::_2);
  auto success = module->SetCallbacks((Callbacks) {empty, associate, filled, release, event });

  if(!success)
    throw OMX_ErrorUndefined;

  this->callbacks = *callbacks;
  this->app = app;

  return OMX_ErrorNone;
  OMX_CATCH();
}

static OMX_PARAM_PORTDEFINITIONTYPE ConstructPortDefinition(Port& port, ModuleInterface const& module)
{
  OMX_PARAM_PORTDEFINITIONTYPE d;
  OMXChecker::SetHeaderVersion(d);
  d.nPortIndex = port.index;
  d.eDir = IsInputPort(d.nPortIndex) ? OMX_DirInput : OMX_DirOutput;
  auto const requirements = IsInputPort(d.nPortIndex) ? module.GetBufferRequirements().input : module.GetBufferRequirements().output;

  if(port.expected < (size_t)requirements.min)
    port.expected = requirements.min;
  d.nBufferCountActual = port.expected;
  d.bEnabled = ConvertToOMXBool(port.enable);
  d.bPopulated = ConvertToOMXBool(port.playable);
  d.nBufferCountMin = requirements.min;
  d.nBufferSize = requirements.size;
  d.bBuffersContiguous = ConvertToOMXBool(requirements.contiguous);
  d.nBufferAlignment = requirements.bytesAlignment;
  d.eDomain = OMX_PortDomainVideo;

  auto& v = d.format.video;
  auto const moduleResolution = module.GetResolution();
  auto const moduleFormat = module.GetFormat();
  auto const moduleClock = module.GetClock();
  auto const moduleMime = IsInputPort(d.nPortIndex) ? module.GetMimes().input : module.GetMimes().output;
  v.pNativeRender = 0; // XXX
  v.nFrameWidth = moduleResolution.width;
  v.nFrameHeight = moduleResolution.height;
  v.nStride = moduleResolution.stride;
  v.nSliceHeight = moduleResolution.sliceHeight;
  v.nBitrate = 0; // XXX
  v.xFramerate = ConvertToOMXFramerate(moduleClock);
  v.bFlagErrorConcealment = ConvertToOMXBool(false); // XXX
  v.eCompressionFormat = ConvertToOMXCompression(moduleMime.compression);
  v.eColorFormat = ConvertToOMXColor(moduleFormat.color, moduleFormat.bitdepth);
  v.cMIMEType = const_cast<char*>(moduleMime.mime.c_str());
  v.pNativeWindow = 0; // XXX
  return d;
}

static OMX_PARAM_BUFFERSUPPLIERTYPE ConstructPortSupplier(Port const& port)
{
  OMX_PARAM_BUFFERSUPPLIERTYPE s;
  OMXChecker::SetHeaderVersion(s);
  s.nPortIndex = port.index;
  s.eBufferSupplier = OMX_BufferSupplyUnspecified; // We don't care
  return s;
}

static OMX_VIDEO_PARAM_PORTFORMATTYPE ConstructVideoPortFormat(Port const& port, ModuleInterface const& module)
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE f;
  OMXChecker::SetHeaderVersion(f);
  f.nPortIndex = port.index;
  f.nIndex = 0;
  auto const moduleFormat = module.GetFormat();
  auto const moduleClock = module.GetClock();
  auto const moduleMime = IsInputPort(f.nPortIndex) ? module.GetMimes().input : module.GetMimes().output;
  f.eCompressionFormat = ConvertToOMXCompression(moduleMime.compression);
  f.eColorFormat = ConvertToOMXColor(moduleFormat.color, moduleFormat.bitdepth);
  f.xFramerate = ConvertToOMXFramerate(moduleClock);
  return f;
}

OMX_ERRORTYPE Codec::GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));
  OMXChecker::CheckStateOperation(AL_GetParameter, state);
  switch(index)
  {
  case OMX_IndexParamVideoInit:
  {
    *(OMX_PORT_PARAM_TYPE*)param = ports;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamStandardComponentRole:
  {
    auto p = (OMX_PARAM_COMPONENTROLETYPE*)param;

    strncpy((char*)p->cRole, (char*)role, strlen((char*)role));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamPortDefinition:
  {
    auto const index = *(((OMX_U32*)param) + 2);
    auto port = GetPort(index);
    *(OMX_PARAM_PORTDEFINITIONTYPE*)param = ConstructPortDefinition(*port, *module);
    return OMX_ErrorNone;
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    auto const index = *(((OMX_U32*)param) + 2);
    auto port = GetPort(index);
    *(OMX_PARAM_BUFFERSUPPLIERTYPE*)param = ConstructPortSupplier(*port);
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoPortFormat:
  {
    auto const index = *(((OMX_U32*)param) + 2);
    auto port = GetPort(index);
    auto p = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)param;

    if(p->nIndex != 0)
      return OMX_ErrorNoMore;

    *p = ConstructVideoPortFormat(*port, *module);
    return OMX_ErrorNone;
  }
  default:
    LOGE("%s is unsupported", ToStringOMXIndex.at(index));
    return OMX_ErrorUnsupportedIndex;
  }

  LOGE("%s is unsupported", ToStringOMXIndex.at(index));
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH();
}

static bool SetFormat(OMX_COLOR_FORMATTYPE const& color, ModuleInterface& module)
{
  auto moduleFormat = module.GetFormat();
  moduleFormat.color = ConvertToModuleColor(color);
  moduleFormat.bitdepth = ConvertToModuleBitdepth(color);
  return module.SetFormat(moduleFormat);
}

static bool SetClock(OMX_U32 const& framerateInQ16, ModuleInterface& module)
{
  auto moduleClock = module.GetClock();
  auto const clock = ConvertToModuleClock(framerateInQ16);
  moduleClock.framerate = clock.framerate;
  moduleClock.clockratio = clock.clockratio;
  return module.SetClock(moduleClock);
}

static bool SetResolution(OMX_VIDEO_PORTDEFINITIONTYPE const& definition, ModuleInterface& module)
{
  auto moduleResolution = module.GetResolution();
  moduleResolution.width = definition.nFrameWidth;
  moduleResolution.height = definition.nFrameHeight;
  moduleResolution.stride = definition.nStride;
  moduleResolution.sliceHeight = definition.nSliceHeight;
  return module.SetResolution(moduleResolution);
}

static bool SetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE const& settings, ModuleInterface& module)
{
  auto const video = settings.format.video;

  if(!SetResolution(video, module))
    return false;

  if(!SetClock(video.xFramerate, module))
    return false;

  if(!SetFormat(video.eColorFormat, module))
    return false;
  return true;
}

static bool SetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE const& format, ModuleInterface& module)
{
  if(!SetClock(format.xFramerate, module))
    return false;

  if(!SetFormat(format.eColorFormat, module))
    return false;
  return true;
}

static bool SetPortExpectedBuffer(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, ModuleInterface const& module)
{
  auto const min = IsInputPort(settings.nPortIndex) ? module.GetBufferRequirements().input.min : module.GetBufferRequirements().output.min;
  auto const actual = static_cast<int>(settings.nBufferCountActual);

  if(actual < min)
    return false;

  port.expected = actual;

  return true;
}

OMX_ERRORTYPE Codec::SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));
  OMXChecker::CheckStateOperation(AL_SetParameter, state);
  switch(index)
  {
  case OMX_IndexParamStandardComponentRole:
  {
    auto p = (OMX_PARAM_COMPONENTROLETYPE*)param;

    if(!strncmp((char*)role, (char*)p->cRole, strlen((char*)role)))
    {
      module->ResetRequirements();
      return OMX_ErrorNone;
    }
    throw OMX_ErrorBadParameter;
  }
  case OMX_IndexParamPortDefinition:
  {
    auto const index = *(((OMX_U32*)param) + 2);
    CheckPortIndex(index);
    auto const settings = static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(param);
    auto const rollback = ConstructPortDefinition(*GetPort(index), *module);

    if(!SetPortExpectedBuffer(*settings, *GetPort(index), *module))
      throw OMX_ErrorBadParameter;

    if(!SetPortDefinition(*settings, *module))
    {
      auto success = SetPortDefinition(rollback, *module);
      assert(success);
      throw OMX_ErrorBadParameter;
    }
    return OMX_ErrorNone;
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    auto const index = *(((OMX_U32*)param) + 2);
    CheckPortIndex(index);
    // Do nothing
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoPortFormat:
  {
    auto const index = *(((OMX_U32*)param) + 2);
    CheckPortIndex(index);
    auto const rollback = ConstructVideoPortFormat(*GetPort(index), *module);
    auto const format = static_cast<OMX_VIDEO_PARAM_PORTFORMATTYPE*>(param);

    if(!SetVideoPortFormat(*format, *module))
    {
      auto success = SetVideoPortFormat(rollback, *module);
      assert(success);
      throw OMX_ErrorBadParameter;
    }
    return OMX_ErrorNone;
  }
  default:
    LOGE("%s is unsupported", ToStringOMXIndex.at(index));
    return OMX_ErrorUnsupportedIndex;
  }

  LOGE("%s is unsupported", ToStringOMXIndex.at(index));
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH();
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

OMX_ERRORTYPE Codec::UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size, OMX_IN OMX_U8* buffer)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckNotNull(size);

  CheckPortIndex(index);
  auto port = GetPort(index);

  if(transientState != TransientLoadedToIdle && !(port->isTransientToEnable))
    throw OMX_ErrorIncorrectStateOperation;

  *header = AllocateHeader(app, size, buffer, false, index);
  assert(*header);
  port->Add(*header);

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Codec::AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckNotNull(size);
  CheckPortIndex(index);

  auto port = GetPort(index);

  if(transientState != TransientLoadedToIdle && !(port->isTransientToEnable))
    throw OMX_ErrorIncorrectStateOperation;

  auto buffer = static_cast<OMX_U8*>(module->Allocate(size * sizeof(OMX_U8)));

  if(!buffer)
    throw OMX_ErrorInsufficientResources;

  *header = AllocateHeader(app, size, buffer, true, index);
  assert(*header);
  port->Add(*header);

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Codec::FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);

  CheckPortIndex(index);
  auto port = GetPort(index);

  if((transientState != TransientIdleToLoaded) && (!port->isTransientToDisable))
    if(callbacks.EventHandler)
      callbacks.EventHandler(component, app, OMX_EventError, OMX_ErrorPortUnpopulated, 0, nullptr);

  if(isBufferAllocatedByModule(header))
    module->Free(header->pBuffer);

  port->Remove(header);
  DeleteHeader(header);

  return OMX_ErrorNone;
  OMX_CATCH();
}

void Codec::AttachMark(OMX_BUFFERHEADERTYPE* header)
{
  assert(header);

  if(marks.empty())
    return;

  if(header->hMarkTargetComponent)
    return;

  auto const mark = marks.front();
  header->hMarkTargetComponent = mark->hMarkTargetComponent;
  header->pMarkData = mark->pMarkData;
  marks.pop();

  if(callbacks.EventHandler && header->hMarkTargetComponent == component)
    callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandMarkBuffer, input.index, mark);
}

OMX_ERRORTYPE Codec::EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  printf("[omx] empty: %p\n", header->pBuffer);
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckStateOperation(AL_EmptyThisBuffer, state);
  CheckPortIndex(header->nInputPortIndex);

  processor->queue(CreateTask(EmptyBuffer, static_cast<OMX_U32>(input.index), shared_ptr<void>(header, nullDeleter)));

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Codec::FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  printf("[omx] fill: %p\n", header->pBuffer);
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckStateOperation(AL_FillThisBuffer, state);
  CheckPortIndex(header->nOutputPortIndex);

  header->nTimeStamp = 0;
  header->hMarkTargetComponent = NULL;
  header->pMarkData = NULL;
  header->nFlags = 0;

  processorFill->queue(CreateTask(FillBuffer, static_cast<OMX_U32>(output.index), shared_ptr<void>(header, nullDeleter)));

  return OMX_ErrorNone;
  OMX_CATCH();
}

void Codec::ComponentDeInit()
{
  free(role);
  free(name);
}

OMX_ERRORTYPE Codec::GetComponentVersion(OMX_OUT OMX_STRING name, OMX_OUT OMX_VERSIONTYPE* version, OMX_OUT OMX_VERSIONTYPE* spec)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(name);
  OMXChecker::CheckNotNull(version);
  OMXChecker::CheckNotNull(spec);
  OMXChecker::CheckStateOperation(AL_GetComponentVersion, state);

  strncpy(name, this->name, OMX_MAX_STRINGNAME_SIZE);
  *version = this->version;
  *spec = this->spec;

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE Codec::GetConfig(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR config)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(config);
  OMXChecker::CheckHeaderVersion(GetVersion(config));
  OMXChecker::CheckStateOperation(AL_GetConfig, state);
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
    framerate.xEncodeFramerate = ConvertToOMXFramerate(clock);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoGroupOfPictures:
  {
    Gop moduleGop;
    module->GetDynamic(DYNAMIC_INDEX_GOP, &moduleGop);
    auto& gop = *(static_cast<OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES*>(config));
    gop.nBFrames = ConvertToOMXBFrames(moduleGop);
    gop.nPFrames = ConvertToOMXPFrames(moduleGop);
    return OMX_ErrorNone;
  }
  default:
    LOGE("%s is unsupported", ToStringOMXIndex.at(index));
    return OMX_ErrorUnsupportedIndex;
  }

  LOGE("%s is unsupported", ToStringOMXIndex.at(index));

  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_CONFIG();
}

OMX_ERRORTYPE Codec::SetConfig(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR config)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(config);
  OMXChecker::CheckHeaderVersion(GetVersion(config));
  OMXChecker::CheckStateOperation(AL_SetConfig, state);
  switch(static_cast<OMX_U32>(index))
  {
  case OMX_IndexConfigVideoBitrate:
  {
    OMX_VIDEO_CONFIG_BITRATETYPE* bitrate = new OMX_VIDEO_CONFIG_BITRATETYPE;
    memcpy(bitrate, static_cast<OMX_VIDEO_CONFIG_BITRATETYPE*>(config), sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));

    if(bitrate->nEncodeBitrate == 0)
      throw OMX_ErrorBadParameter;

    processor->queue(CreateTask(SetDynamic, OMX_IndexConfigVideoBitrate, shared_ptr<void>(bitrate)));

    return OMX_ErrorNone;
  }
  case OMX_IndexConfigVideoFramerate:
  {
    OMX_CONFIG_FRAMERATETYPE* framerate = new OMX_CONFIG_FRAMERATETYPE;
    memcpy(framerate, static_cast<OMX_CONFIG_FRAMERATETYPE*>(config), sizeof(OMX_CONFIG_FRAMERATETYPE));

    processor->queue(CreateTask(SetDynamic, OMX_IndexConfigVideoFramerate, shared_ptr<void>(framerate)));

    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh:
  {
    OMX_ALG_VIDEO_CONFIG_INSERT_INSTANTANEOUS_DECODING_REFRESH* idr = new OMX_ALG_VIDEO_CONFIG_INSERT_INSTANTANEOUS_DECODING_REFRESH;
    memcpy(idr, static_cast<OMX_ALG_VIDEO_CONFIG_INSERT_INSTANTANEOUS_DECODING_REFRESH*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_INSERT_INSTANTANEOUS_DECODING_REFRESH));
    processor->queue(CreateTask(SetDynamic, OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh, shared_ptr<void>(idr)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoGroupOfPictures:
  {
    OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES* gop = new OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES;
    memcpy(gop, static_cast<OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES));
    processor->queue(CreateTask(SetDynamic, OMX_ALG_IndexConfigVideoGroupOfPictures, shared_ptr<void>(gop)));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexConfigVideoRegionOfInterest:
  {
    OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST* roi = new OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST;
    memcpy(roi, static_cast<OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST*>(config), sizeof(OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST));
    processor->queue(CreateTask(SetDynamic, OMX_ALG_IndexConfigVideoRegionOfInterest, shared_ptr<void>(roi)));
    return OMX_ErrorNone;
  }
  default:
    LOGE("%s is unsupported", ToStringOMXIndex.at(index));
    return OMX_ErrorUnsupportedIndex;
  }

  LOGE("%s is unsupported", ToStringOMXIndex.at(index));
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_CONFIG();
}

OMX_ERRORTYPE Codec::GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(name);
  OMXChecker::CheckNotNull(index);
  OMXChecker::CheckStateOperation(AL_GetExtensionIndex, state);
  return OMX_ErrorNoMore;
  OMX_CATCH();
}

OMX_ERRORTYPE Codec::ComponentTunnelRequest(OMX_IN OMX_U32 index, OMX_IN OMX_HANDLETYPE comp, OMX_IN OMX_U32 tunneledIndex, OMX_INOUT OMX_TUNNELSETUPTYPE* setup)
{
  (void)index, (void)tunneledIndex, (void)comp, (void)setup;
  return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE Codec::UseEGLImage(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN void* eglImage)
{
  (void)header, (void)index, (void)app, (void)eglImage;
  return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE Codec::ComponentRoleEnum(OMX_OUT OMX_U8* role, OMX_IN OMX_U32 index)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(role);
  OMXChecker::CheckStateOperation(AL_ComponentRoleEnum, state);

  if(index != 0)
    return OMX_ErrorNoMore;

  strncpy((OMX_STRING)role, this->role, OMX_MAX_STRINGNAME_SIZE);

  return OMX_ErrorNone;
  OMX_CATCH();
}

static inline bool isTransitionToIdle(OMX_STATETYPE previousState, OMX_STATETYPE state)
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

static inline bool isTransitionToRun(OMX_STATETYPE previousState, OMX_STATETYPE state)
{
  if(state != OMX_StateExecuting)
    return false;

  if((previousState != OMX_StateIdle) && (previousState != OMX_StatePause))
    return false;
  return true;
}

static inline bool isTransitionToStop(OMX_STATETYPE previousState, OMX_STATETYPE state)
{
  if(state != OMX_StateIdle)
    return false;

  if((previousState != OMX_StateExecuting))
    return false;
  return true;
}

static inline bool isTransitionToPause(OMX_STATETYPE previousState, OMX_STATETYPE state)
{
  if(state != OMX_StatePause)
    return false;

  if((previousState != OMX_StateExecuting) && (previousState != OMX_StateIdle))
    return false;
  return true;
}

inline void Codec::PopulatingPorts()
{
  for(auto i = ports.nStartPortNumber; i < ports.nPorts; i++)
  {
    auto port = GetPort(i);

    if(port->enable)
      port->WaitFull();
  }
}

inline void Codec::UnpopulatingPorts()
{
  for(auto i = ports.nStartPortNumber; i < ports.nPorts; i++)
  {
    auto port = GetPort(i);
    port->WaitEmpty();
  }
}

void Codec::TreatSetStateCommand(Task* task)
{
  try
  {
    assert(task);
    assert(task->cmd == SetState);
    assert(task->opt.get() == nullptr);

    auto const newState = (OMX_STATETYPE)((uintptr_t)task->data);
    LOGI("Set State : %s", ToStringOMXState.at(newState));
    OMXChecker::CheckStateTransition(state, newState);

    if(isTransitionToIdle(state, newState))
    {
      PopulatingPorts();

      if(!module->Create())
        throw OMX_ErrorInsufficientResources;
    }

    if(isTransitionToLoaded(state, newState) && (state != OMX_StateWaitForResources))
    {
      module->Destroy();
      UnpopulatingPorts();
      transientState = TransientMax;
    }

    if(isTransitionToRun(state, newState))
      if(!module->Run(shouldPrealloc))
        throw OMX_ErrorUndefined;

    if(isTransitionToPause(state, newState))
      module->Pause();

    if(isTransitionToStop(state, newState))
      module->Stop();

    state = newState;

    if(callbacks.EventHandler)
      callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandStateSet, newState, nullptr);
  }
  catch(OMX_ERRORTYPE& e)
  {
    LOGE("%s", ToStringOMXError.at(e));

    if(e == OMX_ErrorInvalidState)
    {
      module->Stop();
      module->Destroy();
      state = OMX_StateInvalid;
    }

    if(callbacks.EventHandler)
      callbacks.EventHandler(component, app, OMX_EventError, e, 0, nullptr);
  }
}

void Codec::TreatFlushCommand(Task* task)
{
  assert(task);
  assert(task->cmd == Flush);
  assert(task->opt.get() == nullptr);
  auto const index = static_cast<OMX_U32>((uintptr_t)task->data);

  LOGI("Flush port : %i", index);
  module->Flush();
  printf("END FLUSH\n");

  if(callbacks.EventHandler)
    callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandFlush, index, nullptr);
}

void Codec::TreatDisablePortCommand(Task* task)
{
  assert(task);
  assert(task->cmd == DisablePort);
  assert(task->opt.get() == nullptr);
  auto const index = static_cast<OMX_U32>((uintptr_t)task->data);
  auto const port = GetPort(index);

  if(port->isTransientToDisable)
  {
    port->WaitEmpty();
    port->isTransientToDisable = false;
  }

  if(callbacks.EventHandler)
    callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandPortDisable, index, nullptr);
}

void Codec::TreatEnablePortCommand(Task* task)
{
  assert(task);
  assert(task->cmd == EnablePort);
  assert(task->opt.get() == nullptr);
  auto const index = static_cast<OMX_U32>((uintptr_t)task->data);
  auto const port = GetPort(index);

  if(port->isTransientToEnable)
  {
    if(state != OMX_StateLoaded && state != OMX_StateWaitForResources)
      port->WaitFull();

    port->isTransientToEnable = false;
  }

  if(callbacks.EventHandler)
    callbacks.EventHandler(component, app, OMX_EventCmdComplete, OMX_CommandPortEnable, index, nullptr);
}

void Codec::TreatMarkBufferCommand(Task* task)
{
  assert(task);
  assert(task->cmd == MarkBuffer);
  assert(static_cast<int>((uintptr_t)task->data) == input.index);
  auto mark = static_cast<OMX_MARKTYPE*>(task->opt.get());
  assert(mark);

  marks.push(mark);
}

void Codec::TreatEmptyBufferCommand(Task* task)
{
  assert(task);
  assert(task->cmd == EmptyBuffer);
  assert(static_cast<int>((uintptr_t)task->data) == input.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task->opt.get());
  assert(header);
  AttachMark(header);
  map.Add(header->pBuffer, header);
  auto success = module->Empty(header->pBuffer, header->nOffset, header->nFilledLen, header);
  assert(success);
}

void Codec::TreatFillBufferCommand(Task* task)
{
  assert(task);
  assert(task->cmd == FillBuffer);
  assert(static_cast<int>((uintptr_t)task->data) == output.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task->opt.get());
  assert(header);
  map.Add(header->pBuffer, header);
  auto success = module->Fill(header->pBuffer, header->nOffset, (header->nAllocLen - header->nOffset));
  assert(success);
}

RegionQuality CreateRegionQuality(OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST const& roi)
{
  RegionQuality rq;
  rq.region.x = roi.nLeft;
  rq.region.y = roi.nTop;
  rq.region.width = roi.nWidth;
  rq.region.height = roi.nHeight;
  rq.quality = ConvertToModuleQuality(roi.eQuality);
  return rq;
}

void Codec::TreatDynamicCommand(Task* task)
{
  assert(task);
  assert(task->cmd == SetDynamic);
  auto const index = static_cast<OMX_U32>((uintptr_t)task->data);
  void* opt = task->opt.get();
  assert(opt);
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
    auto moduleClock = module->GetClock();
    auto const clock = ConvertToModuleClock(framerate->xEncodeFramerate);
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
    auto moduleGop = module->GetGop();
    moduleGop.b = ConvertToModuleBFrames(gop->nBFrames, gop->nPFrames);
    moduleGop.length = ConvertToModuleGopLength(gop->nBFrames, gop->nPFrames);
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

    auto rq = CreateRegionQuality(*roi);
    module->SetDynamic(DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD, &rq);
    shouldPushROI = true;
    return;
  }

  default:
    return;
  }
}

static void TreatRemoveFenceCommand(Task* task)
{
  auto promise = (std::promise<int>*)task->opt.get();
  promise->set_value(0);
}

static void TreatFenceCommand(Task* task)
{
  auto promise = (std::promise<int>*)task->opt.get();
  promise->get_future().wait();
}

void Codec::_Delete(void* data)
{
  auto task = static_cast<Task*>(data);
  delete task;
}

void Codec::_Process(void* data)
{
  auto task = static_cast<Task*>(data);
  switch(task->cmd)
  {
  case SetState:
  {
    lock_guard<mutex> lock(moduleMutex);
    TreatSetStateCommand(task);
    break;
  }
  case Flush:
  {
    lock_guard<mutex> lock(moduleMutex);
    TreatFlushCommand(task);
    break;
  }
  case DisablePort:
  {
    TreatDisablePortCommand(task);
    break;
  }
  case EnablePort:
  {
    TreatEnablePortCommand(task);
    break;
  }
  case MarkBuffer:
  {
    TreatMarkBufferCommand(task);
    break;
  }
  case EmptyBuffer:
  {
    TreatEmptyBufferCommand(task);
    break;
  }
  case SetDynamic:
  {
    TreatDynamicCommand(task);
    break;
  }
  case Fence:
  {
    TreatFenceCommand(task);
    break;
  }
  default:
    assert(0);
  }

  delete task;
}

void Codec::_ProcessFillBuffer(void* data)
{
  auto task = static_cast<Task*>(data);

  if(task->cmd == FillBuffer)
  {
    lock_guard<mutex> lock(moduleMutex);
    TreatFillBufferCommand(task);
  }
  else if(task->cmd == RemoveFence)
    TreatRemoveFenceCommand(task);
  else
    assert(0 == "bad command");
  delete task;
}

