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

#include "omx_codec_dec.h"

#include "omx_convert_omx_module.h"

#include <OMX_VideoExt.h>
#include <OMX_ComponentAlg.h>
#include <OMX_IVCommonAlg.h>

#include <cmath>

#include "base/omx_checker/omx_checker.h"
#include "base/omx_utils/omx_log.h"
#include "base/omx_utils/omx_translate.h"

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

#define OMX_CATCH_L(f) \
  } \
  catch(OMX_ERRORTYPE& e) \
  { \
    LOGE("%s", ToStringOMXError.at(e)); \
    f(e); \
    return e; \
  } \
  void FORCE_SEMICOLON()

#define OMX_CATCH_PARAMETER() \
  } \
  catch(OMX_ERRORTYPE& e) \
  { \
    LOGE("%s : %s", ToStringOMXIndex.at(index), ToStringOMXError.at(e)); \
    return e; \
  } \
  void FORCE_SEMICOLON()

using namespace std;

static DecModule& ToDecModule(ModuleInterface& module)
{
  return dynamic_cast<DecModule &>(module);
}

DecCodec::DecCodec(OMX_HANDLETYPE component, shared_ptr<MediatypeInterface> media, std::unique_ptr<DecModule>&& module, OMX_STRING name, OMX_STRING role, std::unique_ptr<DecExpertise>&& expertise) :
  Codec(component, media, std::move(module), name, role), expertise(std::move(expertise))
{
}

DecCodec::~DecCodec() = default;

void DecCodec::EmptyThisBufferCallBack(BufferHandleInterface* handle)
{
  auto header = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)handle)->header;
  delete handle;
  ClearPropagatedData(header);

  if(callbacks.EmptyBufferDone)
    callbacks.EmptyBufferDone(component, app, header);
}

void DecCodec::AssociateCallBack(BufferHandleInterface*, BufferHandleInterface* fill)
{
  std::lock_guard<std::mutex> lock(mutex);

  if(transmit.empty())
    return;

  auto emptyHeader = transmit.front();
  auto fillHeader = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)fill)->header;
  assert(fillHeader);

  assert(!fillHeader->hMarkTargetComponent);
  assert(!fillHeader->pMarkData);
  assert(!fillHeader->nTimeStamp);
  fillHeader->hMarkTargetComponent = emptyHeader.hMarkTargetComponent;
  fillHeader->pMarkData = emptyHeader.pMarkData;
  fillHeader->nTimeStamp = emptyHeader.nTimeStamp;
  transmit.pop_front();

  if(IsEOSDetected(emptyHeader.nFlags))
  {
    callbacks.EventHandler(component, app, OMX_EventBufferFlag, output.index, emptyHeader.nFlags, nullptr);
    transmit.clear();
  }

  if(IsCompMarked(emptyHeader.hMarkTargetComponent, component))
    callbacks.EventHandler(component, app, OMX_EventMark, 0, 0, emptyHeader.pMarkData);
}

void DecCodec::FillThisBufferCallBack(BufferHandleInterface* filled, int offset, int size)
{
  assert(filled);
  auto header = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)filled)->header;
  delete filled;

  header->nOffset = offset;
  header->nFilledLen = size;
  switch(ToDecModule(*module).GetDisplayPictureType())
  {
  case 0:
  {
    // Do nothing : PROGRESSIVE
    break;
  }
  case 1:
  {
    header->nFlags |= OMX_ALG_BUFFERFLAG_TOP_FIELD;
    break;
  }
  case 2:
  {
    header->nFlags |= OMX_ALG_BUFFERFLAG_BOT_FIELD;
    break;
  }
  default: break;
  }

  if(offset == 0 && size == 0)
    header->nFlags = OMX_BUFFERFLAG_EOS;

  if(callbacks.FillBufferDone)
    callbacks.FillBufferDone(component, app, header);
}

void DecCodec::EventCallBack(CallbackEventType type, void* data)
{
  assert(type <= CALLBACK_EVENT_MAX);
  switch(type)
  {
  case CALLBACK_EVENT_RESOLUTION_CHANGE:
  {
    LOGI("%s", ToStringCallbackEvent.at(type));

    auto port = GetPort(1);

    if(port->isTransientToEnable || !port->enable)
      callbacks.EventHandler(component, app, OMX_EventPortSettingsChanged, 1, 0, nullptr);
    break;
  }
  default:
    Codec::EventCallBack(type, data);
    break;
  }
}

static OMX_PARAM_PORTDEFINITIONTYPE ConstructPortDefinition(Port& port, DecModule const& module)
{
  OMX_PARAM_PORTDEFINITIONTYPE d;
  OMXChecker::SetHeaderVersion(d);
  d.nPortIndex = port.index;
  d.eDir = IsInputPort(d.nPortIndex) ? OMX_DirInput : OMX_DirOutput;
  auto requirements = IsInputPort(d.nPortIndex) ? module.GetBufferRequirements().input : module.GetBufferRequirements().output;

  if(port.expected < (size_t)requirements.min)
    port.expected = requirements.min;
  d.nBufferCountActual = port.expected;
  d.bEnabled = ConvertModuleToOMXBool(port.enable);
  d.bPopulated = ConvertModuleToOMXBool(port.playable);
  d.nBufferCountMin = requirements.min;
  d.nBufferSize = requirements.size;
  d.bBuffersContiguous = ConvertModuleToOMXBool(requirements.contiguous);
  d.nBufferAlignment = requirements.bytesAlignment;
  d.eDomain = OMX_PortDomainVideo;

  auto& v = d.format.video;
  auto moduleResolution = module.GetResolution();
  auto moduleFormat = module.GetFormat();
  auto moduleClock = module.GetClock();
  auto moduleMime = IsInputPort(d.nPortIndex) ? module.GetMimes().input : module.GetMimes().output;
  v.pNativeRender = 0; // XXX
  v.nFrameWidth = moduleResolution.width;
  v.nFrameHeight = moduleResolution.height;
  v.nStride = moduleResolution.stride;
  v.nSliceHeight = moduleResolution.sliceHeight;
  v.nBitrate = 0; // XXX
  v.xFramerate = ConvertModuleToOMXFramerate(moduleClock);
  v.bFlagErrorConcealment = ConvertModuleToOMXBool(false); // XXX
  v.eCompressionFormat = ConvertModuleToOMXCompression(moduleMime.compression);
  v.eColorFormat = ConvertModuleToOMXColor(moduleFormat.color, moduleFormat.bitdepth);
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

static OMX_VIDEO_PARAM_PORTFORMATTYPE ConstructVideoPortCurrentFormat(Port const& port, DecModule const& module)
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE f;
  OMXChecker::SetHeaderVersion(f);
  f.nPortIndex = port.index;
  f.nIndex = 0;
  auto moduleFormat = module.GetFormat();
  auto moduleClock = module.GetClock();
  auto moduleMime = IsInputPort(f.nPortIndex) ? module.GetMimes().input : module.GetMimes().output;
  f.eCompressionFormat = ConvertModuleToOMXCompression(moduleMime.compression);
  f.eColorFormat = ConvertModuleToOMXColor(moduleFormat.color, moduleFormat.bitdepth);
  f.xFramerate = ConvertModuleToOMXFramerate(moduleClock);
  return f;
}

static OMX_ALG_PARAM_REPORTED_LATENCY ConstructReportedLatency(DecModule const& module)
{
  OMX_ALG_PARAM_REPORTED_LATENCY lat;
  OMXChecker::SetHeaderVersion(lat);
  lat.nLatency = module.GetLatency();
  return lat;
}

static OMX_ALG_PARAM_PREALLOCATION ConstructPreallocation(bool isEnabled)
{
  OMX_ALG_PARAM_PREALLOCATION prealloc;
  OMXChecker::SetHeaderVersion(prealloc);
  prealloc.bDisablePreallocation = ConvertModuleToOMXBool(!isEnabled);
  return prealloc;
}

OMX_ALG_PORT_PARAM_BUFFER_MODE ConstructPortBufferMode(Port const& port, DecModule const& module)
{
  OMX_ALG_PORT_PARAM_BUFFER_MODE mode;
  OMXChecker::SetHeaderVersion(mode);
  mode.nPortIndex = port.index;
  mode.eMode = IsInputPort(port.index) ? ConvertModuleToOMXBufferMode(module.GetFileDescriptors().input) : ConvertModuleToOMXBufferMode(module.GetFileDescriptors().output);
  return mode;
}

OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER ConstructVideoDecodedPictureBuffer(Port const& port, DecModule const& module)
{
  OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER dpb;
  OMXChecker::SetHeaderVersion(dpb);
  dpb.nPortIndex = port.index;
  dpb.eDecodedPictureBufferMode = ConvertModuleToOMXDecodedPictureBuffer(module.GetDecodedPictureBuffer());
  return dpb;
}

OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS ConstructVideoInternalEntropyBuffers(Port const& port, DecModule const& module)
{
  OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS ieb;
  OMXChecker::SetHeaderVersion(ieb);
  ieb.nPortIndex = port.index;
  ieb.nNumInternalEntropyBuffers = module.GetInternalEntropyBuffers();
  return ieb;
}

OMX_ALG_VIDEO_PARAM_SUBFRAME ConstructVideoSubframe(Port const& port, DecModule const& module)
{
  OMX_ALG_VIDEO_PARAM_SUBFRAME subframe;
  OMXChecker::SetHeaderVersion(subframe);
  subframe.nPortIndex = port.index;
  subframe.bEnableSubframe = ConvertModuleToOMXBool(module.IsEnableSubframe());
  return subframe;
}

static bool GetVideoPortFormatSupported(OMX_VIDEO_PARAM_PORTFORMATTYPE& format, DecModule const& module)
{
  auto supported = module.GetFormatsSupported();

  if(format.nIndex >= supported.size())
    return false;

  auto moduleMime = IsInputPort(format.nPortIndex) ? module.GetMimes().input : module.GetMimes().output;
  format.eCompressionFormat = ConvertModuleToOMXCompression(moduleMime.compression);
  format.eColorFormat = ConvertModuleToOMXColor(supported[format.nIndex].color, supported[format.nIndex].bitdepth);
  auto moduleClock = module.GetClock();
  format.xFramerate = ConvertModuleToOMXFramerate(moduleClock);

  return true;
}

bool ConstructCommonSequencePictureModesSupported(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE* mode, shared_ptr<MediatypeInterface> media)
{
  vector<SequencePictureModeType> modes;
  auto supported = media->Get(SETTINGS_INDEX_SEQUENCE_PICTURE_MODES_SUPPORTED, &modes);
  assert(supported == MediatypeInterface::ERROR_SETTINGS_NONE);

  if(mode->nModeIndex >= modes.size())
    return false;

  mode->eMode = ConvertModuleToOMXSequencePictureMode(modes[mode->nModeIndex]);
  return true;
}

OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE ConstructCommonSequencePictureMode(Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE mode;
  OMXChecker::SetHeaderVersion(mode);
  mode.nPortIndex = port.index;
  auto supported = media->Get(SETTINGS_INDEX_SEQUENCE_PICTURE_MODE, &mode.eMode);
  assert(supported == MediatypeInterface::ERROR_SETTINGS_NONE);

  return mode;
}

OMX_ERRORTYPE DecCodec::GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));
  OMXChecker::CheckStateOperation(AL_GetParameter, state);

  auto getCurrentPort = [=](OMX_PTR param) -> Port*
                        {
                          auto index = *(((OMX_U32*)param) + 2);
                          return GetPort(index);
                        };
  switch(static_cast<OMX_U32>(index)) // all indexes are 32u
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
    auto port = getCurrentPort(param);
    *(OMX_PARAM_PORTDEFINITIONTYPE*)param = ConstructPortDefinition(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    auto port = getCurrentPort(param);
    *(OMX_PARAM_BUFFERSUPPLIERTYPE*)param = ConstructPortSupplier(*port);
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoPortFormat:
  {
    auto port = getCurrentPort(param);
    auto p = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)param;

    if(IsInputPort(index))
    {
      if(!GetVideoPortFormatSupported(*p, ToDecModule(*module)))
        return OMX_ErrorNoMore;
    }
    else
      *p = ConstructVideoPortCurrentFormat(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    auto port = getCurrentPort(param);
    expertise->GetProfileLevel(param, *port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelQuerySupported: // GetParameter only
  {
    auto port = getCurrentPort(param);
    (void)port;

    if(!expertise->GetProfileLevelSupported(param, ToDecModule(*module)))
      return OMX_ErrorNoMore;

    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoAvc:
  case OMX_ALG_IndexParamVideoHevc:
  {
    auto port = getCurrentPort(param);
    expertise->GetExpertise(param, *port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamReportedLatency: // GetParameter only
  {
    *(OMX_ALG_PARAM_REPORTED_LATENCY*)param = ConstructReportedLatency(ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamPreallocation:
  {
    *(OMX_ALG_PARAM_PREALLOCATION*)param = ConstructPreallocation(this->shouldPrealloc);
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexPortParamBufferMode:
  {
    auto port = getCurrentPort(param);
    *(OMX_ALG_PORT_PARAM_BUFFER_MODE*)param = ConstructPortBufferMode(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoDecodedPictureBuffer:
  {
    auto port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER*)param = ConstructVideoDecodedPictureBuffer(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInternalEntropyBuffers:
  {
    auto port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS*)param = ConstructVideoInternalEntropyBuffers(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoSubframe:
  {
    auto port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_SUBFRAME*)param = ConstructVideoSubframe(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamCommonSequencePictureModeQuerySupported:
  {
    auto mode = (OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE*)param;

    if(!ConstructCommonSequencePictureModesSupported(mode, media))
      return OMX_ErrorNoMore;

    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamCommonSequencePictureModeCurrent:
  {
    auto port = getCurrentPort(param);
    *(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE*)param = ConstructCommonSequencePictureMode(*port, media);
    return OMX_ErrorNone;
  }
  default:
    LOGE("%s is unsupported", ToStringOMXIndex.at(index));
    return OMX_ErrorUnsupportedIndex;
  }

  LOGE("%s is unsupported", ToStringOMXIndex.at(index));
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_PARAMETER();
}

static bool SetFormat(OMX_COLOR_FORMATTYPE const& color, DecModule& module)
{
  auto moduleFormat = module.GetFormat();
  moduleFormat.color = ConvertOMXToModuleColor(color);
  moduleFormat.bitdepth = ConvertOMXToModuleBitdepth(color);
  return module.SetFormat(moduleFormat);
}

static bool SetClock(OMX_U32 framerateInQ16, DecModule& module)
{
  auto moduleClock = module.GetClock();
  auto clock = ConvertOMXToModuleClock(framerateInQ16);
  moduleClock.framerate = clock.framerate;
  moduleClock.clockratio = clock.clockratio;
  return module.SetClock(moduleClock);
}

static bool SetResolution(OMX_VIDEO_PORTDEFINITIONTYPE const& definition, DecModule& module)
{
  auto moduleResolution = module.GetResolution();
  moduleResolution.width = definition.nFrameWidth;
  moduleResolution.height = definition.nFrameHeight;
  moduleResolution.stride = definition.nStride;
  moduleResolution.sliceHeight = definition.nSliceHeight;
  return module.SetResolution(moduleResolution);
}

static bool SetInputBufferMode(OMX_ALG_BUFFER_MODE mode, DecModule& module)
{
  auto moduleFds = module.GetFileDescriptors();
  moduleFds.input = ConvertOMXToModuleFileDescriptor(mode);
  return module.SetFileDescriptors(moduleFds);
}

static bool SetOutputBufferMode(OMX_ALG_BUFFER_MODE mode, DecModule& module)
{
  auto moduleFds = module.GetFileDescriptors();
  moduleFds.output = ConvertOMXToModuleFileDescriptor(mode);
  return module.SetFileDescriptors(moduleFds);
}

static bool SetDecodedPictureBuffer(OMX_ALG_EDpbMode mode, DecModule& module)
{
  return module.SetDecodedPictureBuffer(ConvertOMXToModuleDecodedPictureBuffer(mode));
}

static bool SetInternalEntropyBuffers(OMX_U32 num, DecModule& module)
{
  return module.SetInternalEntropyBuffers(num);
}

static bool SetSubframe(OMX_BOOL enableSubframe, DecModule& module)
{
  return module.SetEnableSubframe(ConvertOMXToModuleBool(enableSubframe));
}

static bool SetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, DecModule& module)
{
  auto rollback = ConstructPortDefinition(port, module);
  auto video = settings.format.video;

  if(!SetFormat(video.eColorFormat, module))
  {
    SetPortDefinition(rollback, port, module);
    return false;
  }

  if(!SetClock(video.xFramerate, module))
  {
    SetPortDefinition(rollback, port, module);
    return false;
  }

  if(!SetResolution(video, module))
  {
    SetPortDefinition(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE const& format, Port const& port, DecModule& module)
{
  auto rollback = ConstructVideoPortCurrentFormat(port, module);

  if(!SetClock(format.xFramerate, module))
  {
    SetVideoPortFormat(rollback, port, module);
    return false;
  }

  if(!SetFormat(format.eColorFormat, module))
  {
    SetVideoPortFormat(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE const& portBufferMode, Port const& port, DecModule& module)
{
  auto rollback = ConstructPortBufferMode(port, module);
  auto& setBufferMode = IsInputPort(portBufferMode.nPortIndex) ? SetInputBufferMode : SetOutputBufferMode;

  if(!setBufferMode(portBufferMode.eMode, module))
  {
    SetPortBufferMode(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoDecodedPictureBuffer(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER const& dpb, Port const& port, DecModule& module)
{
  auto rollback = ConstructVideoDecodedPictureBuffer(port, module);

  if(!SetDecodedPictureBuffer(dpb.eDecodedPictureBufferMode, module))
  {
    SetVideoDecodedPictureBuffer(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoInternalEntropyBuffers(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS const& ieb, Port const& port, DecModule& module)
{
  auto rollback = ConstructVideoInternalEntropyBuffers(port, module);

  if(!SetInternalEntropyBuffers(ieb.nNumInternalEntropyBuffers, module))
  {
    SetVideoInternalEntropyBuffers(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME const& subframe, Port const& port, DecModule& module)
{
  auto rollback = ConstructVideoSubframe(port, module);

  if(!SetSubframe(subframe.bEnableSubframe, module))
  {
    SetVideoSubframe(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetPortExpectedBuffer(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, DecModule const& module)
{
  auto min = IsInputPort(settings.nPortIndex) ? module.GetBufferRequirements().input.min : module.GetBufferRequirements().output.min;
  auto actual = static_cast<int>(settings.nBufferCountActual);

  if(actual < min)
    return false;

  port.expected = actual;

  return true;
}

static bool SetSequencePictureMode(OMX_ALG_SEQUENCE_PICTURE_MODE mode, shared_ptr<MediatypeInterface> media)
{
  auto ret = media->Set(SETTINGS_INDEX_SEQUENCE_PICTURE_MODE, &mode);

  if(ret == MediatypeInterface::ERROR_SETTINGS_BAD_PARAMETER)
    return false;

  return true;
}

static bool SetCommonSequencePictureMode(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE mode, Port const& port, shared_ptr<MediatypeInterface> media)
{
  auto rollback = ConstructCommonSequencePictureMode(port, media);

  if(!SetSequencePictureMode(mode.eMode, media))
  {
    SetCommonSequencePictureMode(rollback, port, media);
    return false;
  }

  return true;
}

OMX_ERRORTYPE DecCodec::SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));

  auto getCurrentPort = [=](OMX_PTR param) -> Port*
                        {
                          auto index = *(((OMX_U32*)param) + 2);
                          return GetPort(index);
                        };
  switch(static_cast<OMX_U32>(index)) // all indexes are 32u
  {
  case OMX_IndexParamStandardComponentRole:
  {
    OMXChecker::CheckStateOperation(AL_SetParameter, state);
    auto p = (OMX_PARAM_COMPONENTROLETYPE*)param;

    if(!strncmp((char*)role, (char*)p->cRole, strlen((char*)role)))
    {
      module->ResetRequirements();
      return OMX_ErrorNone;
    }
    throw OMX_ErrorBadParameter;
  }
  case OMX_ALG_IndexParamPreallocation:
  {
    auto p = (OMX_ALG_PARAM_PREALLOCATION*)param;
    this->shouldPrealloc = p->bDisablePreallocation == OMX_FALSE;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamPortDefinition:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);

    auto settings = static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(param);

    if(!SetPortExpectedBuffer(*settings, const_cast<Port &>(*port), ToDecModule(*module)))
      throw OMX_ErrorBadParameter;

    if(!SetPortDefinition(*settings, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);

    // Do nothing
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoPortFormat:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);

    auto format = static_cast<OMX_VIDEO_PARAM_PORTFORMATTYPE*>(param);

    if(!SetVideoPortFormat(*format, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);

    if(!expertise->SetProfileLevel(param, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoAvc:
  case OMX_ALG_IndexParamVideoHevc:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);

    if(!expertise->SetExpertise(param, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexPortParamBufferMode:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);
    auto portBufferMode = static_cast<OMX_ALG_PORT_PARAM_BUFFER_MODE*>(param);

    if(!SetPortBufferMode(*portBufferMode, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoDecodedPictureBuffer:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);
    auto dpb = static_cast<OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER*>(param);

    if(!SetVideoDecodedPictureBuffer(*dpb, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInternalEntropyBuffers:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);
    auto ieb = static_cast<OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS*>(param);

    if(!SetVideoInternalEntropyBuffers(*ieb, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoSubframe:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);

    auto subframe = static_cast<OMX_ALG_VIDEO_PARAM_SUBFRAME*>(param);

    if(!SetVideoSubframe(*subframe, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamCommonSequencePictureModeCurrent:
  {
    auto port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);

    auto spm = static_cast<OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE*>(param);

    if(!SetCommonSequencePictureMode(*spm, *port, media))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  default:
    LOGE("%s is unsupported", ToStringOMXIndex.at(index));
    return OMX_ErrorUnsupportedIndex;
  }

  LOGE("%s is unsupported", ToStringOMXIndex.at(index));
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_PARAMETER();
}

OMX_ERRORTYPE DecCodec::GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(name);
  OMXChecker::CheckNotNull(index);
  OMXChecker::CheckStateOperation(AL_GetExtensionIndex, state);
  return OMX_ErrorNoMore;
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

  auto isInputAllocated = *(static_cast<bool*>(header->pInputPortPrivate));
  auto isOutputAllocated = *(static_cast<bool*>(header->pOutputPortPrivate));

  return isInputAllocated || isOutputAllocated;
}

static void DeleteHeader(OMX_BUFFERHEADERTYPE* header)
{
  delete static_cast<bool*>(header->pInputPortPrivate);
  delete static_cast<bool*>(header->pOutputPortPrivate);
  delete header;
}

OMX_ERRORTYPE DecCodec::AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckNotNull(size);
  CheckPortIndex(index);

  auto port = GetPort(index);

  if(transientState != TransientLoadedToIdle && !(port->isTransientToEnable))
    throw OMX_ErrorIncorrectStateOperation;

  auto dmaOnPort = IsInputPort(index) ? ToDecModule(*module).GetFileDescriptors().input : ToDecModule(*module).GetFileDescriptors().output;
  auto buffer = dmaOnPort ? reinterpret_cast<OMX_U8*>(ToDecModule(*module).AllocateDMA(size * sizeof(OMX_U8))) : static_cast<OMX_U8*>(module->Allocate(size * sizeof(OMX_U8)));

  if(dmaOnPort ? (static_cast<int>((intptr_t)buffer) < 0) : !buffer)
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

OMX_ERRORTYPE DecCodec::FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);

  CheckPortIndex(index);
  auto port = GetPort(index);

  if((transientState != TransientIdleToLoaded) && (!port->isTransientToDisable))
    callbacks.EventHandler(component, app, OMX_EventError, OMX_ErrorPortUnpopulated, 0, nullptr);

  auto dmaOnPort = IsInputPort(index) ? ToDecModule(*module).GetFileDescriptors().input : ToDecModule(*module).GetFileDescriptors().output;
  dmaOnPort ? ToDecModule(*module).FreeDMA(static_cast<int>((intptr_t)header->pBuffer)) : module->Free(header->pBuffer);

  port->Remove(header);
  DeleteHeader(header);

  return OMX_ErrorNone;
  OMX_CATCH_L([&](OMX_ERRORTYPE& e)
  {
    if(e != OMX_ErrorBadPortIndex)
      GetPort(index)->ErrorOccured();
  });
}

void DecCodec::TreatEmptyBufferCommand(Task* task)
{
  std::lock_guard<std::mutex> lock(mutex);
  assert(task);
  assert(task->cmd == EmptyBuffer);
  assert(static_cast<int>((intptr_t)task->data) == input.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task->opt.get());
  assert(header);

  if(state == OMX_StateInvalid)
  {
    callbacks.EmptyBufferDone(component, app, header);
    return;
  }

  AttachMark(header);

  if(header->nFlags & OMX_BUFFERFLAG_ENDOFFRAME)
    transmit.push_back(PropagatedData(header->hMarkTargetComponent, header->pMarkData, header->nTimeStamp, header->nFlags));

  auto handle = new OMXBufferHandle(header);
  auto success = module->Empty(handle);
  assert(success);
}

