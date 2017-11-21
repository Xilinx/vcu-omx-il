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
#include "omx_convert_module_to_omx.h"
#include "omx_convert_omx_to_module.h"
#include "omx_convert_module_to_omx_dec.h"
#include "omx_convert_omx_to_module_dec.h"

#include <OMX_VideoExt.h>
#include <OMX_ComponentAlg.h>
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
    LOGE("%s", ToStringError.at(e)); \
    return e; \
  } \
  void FORCE_SEMICOLON()

#define OMX_CATCH_PARAMETER() \
  } \
  catch(OMX_ERRORTYPE& e) \
  { \
    LOGE("%s : %s", ToStringIndex.at(index), ToStringError.at(e)); \
    return e; \
  } \
  void FORCE_SEMICOLON()

static inline void clear(std::queue<int>& q)
{
  std::queue<int> empty;
  std::swap(q, empty);
}

static DecModule& ToDecModule(ModuleInterface& module)
{
  return dynamic_cast<DecModule &>(module);
}

DecCodec::DecCodec(OMX_HANDLETYPE component, std::unique_ptr<DecModule>&& module, OMX_STRING name, OMX_STRING role, std::unique_ptr<DecExpertise>&& expertise) : Codec(component, std::move(module), name, role), expertise(std::move(expertise))
{
}

DecCodec::~DecCodec()
{
}

void DecCodec::EmptyThisBufferCallBack(uint8_t* emptied, int offset, int size)
{
  if(!emptied)
    assert(0);
  (void)offset;
  (void)size;

  auto header = map.Get(emptied);
  assert(header);
  map.Remove(emptied);

  ClearPropagatedData(header);

  if(callbacks.EmptyBufferDone)
    callbacks.EmptyBufferDone(component, app, header);
}

void DecCodec::AssociateCallBack(uint8_t* empty, uint8_t* fill)
{
  (void)empty;
  std::lock_guard<std::mutex> lock(mutex);

  if(transmit.empty())
    return;

  auto emptyHeader = transmit.front();
  auto fillHeader = map.Get(fill);
  assert(fillHeader);

  assert(!fillHeader->hMarkTargetComponent);
  assert(!fillHeader->pMarkData);
  assert(!fillHeader->nTimeStamp);
  fillHeader->hMarkTargetComponent = emptyHeader.hMarkTargetComponent;
  fillHeader->pMarkData = emptyHeader.pMarkData;
  fillHeader->nTimeStamp = emptyHeader.nTimeStamp;
  fillHeader->nFlags = emptyHeader.nFlags;
  transmit.pop_front();

  if(IsEOSDetected(emptyHeader.nFlags))
  {
    callbacks.EventHandler(component, app, OMX_EventBufferFlag, output.index, emptyHeader.nFlags, nullptr);
    transmit.clear();
  }

  if(IsCompMarked(emptyHeader.hMarkTargetComponent, component) && callbacks.EventHandler)
    callbacks.EventHandler(component, app, OMX_EventMark, 0, 0, emptyHeader.pMarkData);
}

void DecCodec::FillThisBufferCallBack(uint8_t* filled, int offset, int size)
{
  if(!filled)
    assert(0);

  auto header = map.Get(filled);
  assert(header);
  map.Remove(filled);

  header->nOffset = offset;
  header->nFilledLen = size;

  if(callbacks.FillBufferDone)
    callbacks.FillBufferDone(component, app, header);
}

static OMX_PARAM_PORTDEFINITIONTYPE ConstructPortDefinition(Port const& port, DecModule const& module)
{
  OMX_PARAM_PORTDEFINITIONTYPE d;
  OMXChecker::SetHeaderVersion(d);
  d.nPortIndex = port.index;
  d.eDir = IsInputPort(d.nPortIndex) ? OMX_DirInput : OMX_DirOutput;
  auto const requirements = IsInputPort(d.nPortIndex) ? module.GetBuffersRequirements().input : module.GetBuffersRequirements().output;
  d.nBufferCountActual = port.expected;
  d.bEnabled = ConvertToOMXBool(port.enable);
  d.bPopulated = ConvertToOMXBool(port.playable);
  d.nBufferCountMin = requirements.min;
  d.nBufferSize = requirements.size;
  d.bBuffersContiguous = ConvertToOMXBool(requirements.contiguous);
  d.nBufferAlignment = requirements.bytesAlignment;
  d.eDomain = OMX_PortDomainVideo;

  auto& v = d.format.video;
  auto const moduleResolution = IsInputPort(d.nPortIndex) ? module.GetResolutions().input : module.GetResolutions().output;
  auto const moduleFormat = IsInputPort(d.nPortIndex) ? module.GetFormats().input : module.GetFormats().output;
  auto const moduleClock = IsInputPort(d.nPortIndex) ? module.GetClocks().input : module.GetClocks().output;
  v.pNativeRender = 0; // XXX
  v.nFrameWidth = moduleResolution.width;
  v.nFrameHeight = moduleResolution.height;
  v.nStride = moduleResolution.stride;
  v.nSliceHeight = moduleResolution.sliceHeight;
  v.nBitrate = 0; // XXX
  v.xFramerate = ConvertToOMXFramerate(moduleClock);
  v.bFlagErrorConcealment = ConvertToOMXBool(false); // XXX
  v.eCompressionFormat = ConvertToOMXCompression(moduleFormat.compression);
  v.eColorFormat = ConvertToOMXColor(moduleFormat.color, moduleFormat.bitdepth);
  v.cMIMEType = const_cast<char*>(moduleFormat.mime.c_str());
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

static OMX_VIDEO_PARAM_PORTFORMATTYPE ConstructVideoPortFormat(Port const& port, DecModule const& module)
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE f;
  OMXChecker::SetHeaderVersion(f);
  f.nPortIndex = port.index;
  f.nIndex = 0;
  auto const moduleFormat = IsInputPort(f.nPortIndex) ? module.GetFormats().input : module.GetFormats().output;
  auto const moduleClock = IsInputPort(f.nPortIndex) ? module.GetClocks().input : module.GetClocks().output;
  f.eCompressionFormat = ConvertToOMXCompression(moduleFormat.compression);
  f.eColorFormat = ConvertToOMXColor(moduleFormat.color, moduleFormat.bitdepth);
  f.xFramerate = ConvertToOMXFramerate(moduleClock);
  return f;
}

static OMX_ALG_PARAM_REPORTED_LATENCY ConstructReportedLatency(DecModule const& module)
{
  OMX_ALG_PARAM_REPORTED_LATENCY lat;
  OMXChecker::SetHeaderVersion(lat);
  lat.nLatency = module.GetLatency();
  return lat;
}

OMX_ALG_PORT_PARAM_BUFFER_MODE ConstructPortBufferMode(Port const& port, DecModule const& module)
{
  OMX_ALG_PORT_PARAM_BUFFER_MODE mode;
  OMXChecker::SetHeaderVersion(mode);
  mode.nPortIndex = port.index;
  mode.eMode = IsInputPort(port.index) ? ConvertToOMXBufferMode(module.GetFileDescriptors().input) : ConvertToOMXBufferMode(module.GetFileDescriptors().output);
  return mode;
}

OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER ConstructVideoDecodedPictureBuffer(Port const& port, DecModule const& module)
{
  OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER dpb;
  OMXChecker::SetHeaderVersion(dpb);
  dpb.nPortIndex = port.index;
  dpb.eDecodedPictureBufferMode = ConvertToOMXDecodedPictureBuffer(module.GetDecodedPictureBuffer());
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
  subframe.bEnableSubframe = ConvertToOMXBool(module.IsEnableSubframe());
  return subframe;
}

OMX_ERRORTYPE DecCodec::GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));
  OMXChecker::CheckStateOperation(AL_GetParameter, state);

  auto const getCurrentPort = [=](OMX_PTR param) -> Port const*
                              {
                                auto const index = *(((OMX_U32*)param) + 2);
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
    auto const port = getCurrentPort(param);
    *(OMX_PARAM_PORTDEFINITIONTYPE*)param = ConstructPortDefinition(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    auto const port = getCurrentPort(param);
    *(OMX_PARAM_BUFFERSUPPLIERTYPE*)param = ConstructPortSupplier(*port);
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoPortFormat:
  {
    auto const port = getCurrentPort(param);
    auto p = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)param;

    if(p->nIndex != 0)
      return OMX_ErrorNoMore;

    *p = ConstructVideoPortFormat(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    auto const port = getCurrentPort(param);
    expertise->GetProfileLevel(param, *port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelQuerySupported: // GetParameter Only
  {
    auto const port = getCurrentPort(param);

    if(!expertise->GetProfileLevelSupported(param, *port, ToDecModule(*module)))
      return OMX_ErrorNoMore;

    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoAvc:
  case OMX_ALG_IndexParamVideoHevc:
  {
    auto const port = getCurrentPort(param);
    expertise->GetExpertise(param, *port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamReportedLatency: // GetParameter only
  {
    *(OMX_ALG_PARAM_REPORTED_LATENCY*)param = ConstructReportedLatency(ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexPortParamBufferMode:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_PORT_PARAM_BUFFER_MODE*)param = ConstructPortBufferMode(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoDecodedPictureBuffer:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER*)param = ConstructVideoDecodedPictureBuffer(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInternalEntropyBuffers:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS*)param = ConstructVideoInternalEntropyBuffers(*port, ToDecModule(*module));
    return OMX_ErrorNone;
  }
  default:
    LOGE("%s is unsupported", ToStringIndex.at(index));
    return OMX_ErrorUnsupportedIndex;
  }

  LOGE("%s is unsupported", ToStringIndex.at(index));
  return OMX_ErrorUnsupportedIndex;
  OMX_CATCH_PARAMETER();
}

static bool SetFormats(OMX_COLOR_FORMATTYPE const& color, DecModule& module)
{
  auto moduleFormats = module.GetFormats();
  moduleFormats.input.color = moduleFormats.output.color = ConvertToModuleColor(color);
  moduleFormats.input.bitdepth = moduleFormats.output.bitdepth = ConvertToModuleBitdepth(color);
  return module.SetFormats(moduleFormats);
}

static bool SetClocks(OMX_U32 const& framerateInQ16, DecModule& module)
{
  auto moduleClocks = module.GetClocks();
  auto const clock = ConvertToModuleClock(framerateInQ16);
  moduleClocks.input.framerate = moduleClocks.output.framerate = clock.framerate;
  moduleClocks.input.clockratio = moduleClocks.output.clockratio = clock.clockratio;

  return module.SetClocks(moduleClocks);
}

static bool SetResolution(OMX_VIDEO_PORTDEFINITIONTYPE const& definition, DecModule& module)
{
  auto moduleResolutions = module.GetResolutions();
  moduleResolutions.input.width = moduleResolutions.output.width = definition.nFrameWidth;
  moduleResolutions.input.height = moduleResolutions.output.height = definition.nFrameHeight;
  moduleResolutions.input.stride = moduleResolutions.output.stride = definition.nStride;
  moduleResolutions.input.sliceHeight = moduleResolutions.output.sliceHeight = definition.nSliceHeight;
  return module.SetResolutions(moduleResolutions);
}

static bool SetInputBufferMode(OMX_ALG_BUFFER_MODE mode, DecModule& module)
{
  auto moduleFds = module.GetFileDescriptors();
  moduleFds.input = ConvertToModuleFileDescriptor(mode);
  return module.SetFileDescriptors(moduleFds);
}

static bool SetOutputBufferMode(OMX_ALG_BUFFER_MODE mode, DecModule& module)
{
  auto moduleFds = module.GetFileDescriptors();
  moduleFds.output = ConvertToModuleFileDescriptor(mode);
  return module.SetFileDescriptors(moduleFds);
}

static bool SetDecodedPictureBuffer(OMX_ALG_EDpbMode mode, DecModule& module)
{
  return module.SetDecodedPictureBuffer(ConvertToModuleDecodedPictureBuffer(mode));
}

static bool SetInternalEntropyBuffers(OMX_U32 const& num, DecModule& module)
{
  return module.SetInternalEntropyBuffers(num);
}

static bool SetSubframe(OMX_BOOL const& enableSubframe, DecModule& module)
{
  return module.SetEnableSubframe(ConvertToModuleBool(enableSubframe));
}

static bool SetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port const& port, DecModule& module)
{
  auto const rollback = ConstructPortDefinition(port, module);
  auto const video = settings.format.video;

  if(!SetResolution(video, module))
  {
    SetPortDefinition(rollback, port, module);
    return false;
  }

  if(!SetClocks(video.xFramerate, module))
  {
    SetPortDefinition(rollback, port, module);
    return false;
  }

  if(!SetFormats(video.eColorFormat, module))
  {
    SetPortDefinition(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE const& format, Port const& port, DecModule& module)
{
  auto const rollback = ConstructVideoPortFormat(port, module);

  if(!SetClocks(format.xFramerate, module))
  {
    SetVideoPortFormat(rollback, port, module);
    return false;
  }

  if(!SetFormats(format.eColorFormat, module))
  {
    SetVideoPortFormat(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE const& portBufferMode, Port const& port, DecModule& module)
{
  auto const rollback = ConstructPortBufferMode(port, module);
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
  auto const rollback = ConstructVideoDecodedPictureBuffer(port, module);

  if(!SetDecodedPictureBuffer(dpb.eDecodedPictureBufferMode, module))
  {
    SetVideoDecodedPictureBuffer(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoInternalEntropyBuffers(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS const& ieb, Port const& port, DecModule& module)
{
  auto const rollback = ConstructVideoInternalEntropyBuffers(port, module);

  if(!SetInternalEntropyBuffers(ieb.nNumInternalEntropyBuffers, module))
  {
    SetVideoInternalEntropyBuffers(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME const& subframe, Port const& port, DecModule& module)
{
  auto const rollback = ConstructVideoSubframe(port, module);

  if(!SetSubframe(subframe.bEnableSubframe, module))
  {
    SetVideoSubframe(rollback, port, module);
    return false;
  }
  return true;
}

OMX_ERRORTYPE DecCodec::SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));
  OMXChecker::CheckStateOperation(AL_SetParameter, state);

  auto const getCurrentPort = [=](OMX_PTR param) -> Port const*
                              {
                                auto const index = *(((OMX_U32*)param) + 2);
                                return GetPort(index);
                              };
  switch(static_cast<OMX_U32>(index)) // all indexes are 32u
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
    auto const port = getCurrentPort(param);
    auto const settings = static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(param);

    if(!SetPortDefinition(*settings, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
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
    auto const port = getCurrentPort(param);
    auto const format = static_cast<OMX_VIDEO_PARAM_PORTFORMATTYPE*>(param);

    if(!SetVideoPortFormat(*format, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    auto const port = getCurrentPort(param);

    if(!expertise->SetProfileLevel(param, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoAvc:
  case OMX_ALG_IndexParamVideoHevc:
  {
    auto const port = getCurrentPort(param);

    if(!expertise->SetExpertise(param, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexPortParamBufferMode:
  {
    auto const port = getCurrentPort(param);
    auto const portBufferMode = static_cast<OMX_ALG_PORT_PARAM_BUFFER_MODE*>(param);

    if(!SetPortBufferMode(*portBufferMode, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoDecodedPictureBuffer:
  {
    auto const port = getCurrentPort(param);
    auto const dpb = static_cast<OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER*>(param);

    if(!SetVideoDecodedPictureBuffer(*dpb, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInternalEntropyBuffers:
  {
    auto const port = getCurrentPort(param);
    auto const ieb = static_cast<OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS*>(param);

    if(!SetVideoInternalEntropyBuffers(*ieb, *port, ToDecModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  default:
    LOGE("%s is unsupported", ToStringIndex.at(index));
    return OMX_ErrorUnsupportedIndex;
  }

  LOGE("%s is unsupported", ToStringIndex.at(index));
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

OMX_ERRORTYPE DecCodec::AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckNotNull(size);
  CheckPortIndex(index);

  auto port = GetPort(index);

  if(transientState != TransientLoadedToIdle && !(port->isTransientToEnable))
    throw OMX_ErrorIncorrectStateOperation;

  auto const dmaOnPort = IsInputPort(index) ? ToDecModule(*module).GetFileDescriptors().input : ToDecModule(*module).GetFileDescriptors().output;
  auto buffer = dmaOnPort ? reinterpret_cast<OMX_U8*>(ToDecModule(*module).AllocateDMA(size * sizeof(OMX_U8))) : static_cast<OMX_U8*>(module->Allocate(size * sizeof(OMX_U8)));

  if(dmaOnPort ? (static_cast<int>((intptr_t)buffer) < 0) : !buffer)
    throw OMX_ErrorInsufficientResources;

  *header = AllocateHeader(app, size, buffer, true, index);
  assert(*header);
  port->Add(*header);

  return OMX_ErrorNone;
  OMX_CATCH();
}

OMX_ERRORTYPE DecCodec::FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);

  CheckPortIndex(index);
  auto port = GetPort(index);

  if((transientState != TransientIdleToLoaded) && (!port->isTransientToDisable))
    if(callbacks.EventHandler)
      callbacks.EventHandler(component, app, OMX_EventError, OMX_ErrorPortUnpopulated, 0, nullptr);

  if(isBufferAllocatedByModule(header))
  {
    auto const dmaOnPort = IsInputPort(index) ? ToDecModule(*module).GetFileDescriptors().input : ToDecModule(*module).GetFileDescriptors().output;
    dmaOnPort ? ToDecModule(*module).FreeDMA(static_cast<int>((intptr_t)header->pBuffer)) : module->Free(header->pBuffer);
  }

  port->Remove(header);
  DeleteHeader(header);

  return OMX_ErrorNone;
  OMX_CATCH();
}

void DecCodec::TreatEmptyBufferCommand(Task* task)
{
  std::lock_guard<std::mutex> lock(mutex);
  assert(task);
  assert(task->cmd == EmptyBuffer);
  assert(static_cast<int>((intptr_t)task->data) == input.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task->opt);
  assert(header);
  AttachMark(header);
  map.Add(header->pBuffer, header);
  transmit.push_back(PropagatedData(header->hMarkTargetComponent, header->pMarkData, header->nTimeStamp, header->nFlags));
  auto success = module->Empty(header->pBuffer, header->nOffset, header->nFilledLen);
  assert(success);
}

