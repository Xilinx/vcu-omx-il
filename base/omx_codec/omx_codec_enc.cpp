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

#include "omx_codec_enc.h"
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

static EncModule& ToEncModule(ModuleInterface& module)
{
  return dynamic_cast<EncModule &>(module);
}

EncCodec::EncCodec(OMX_HANDLETYPE component, shared_ptr<MediatypeInterface> media, std::unique_ptr<EncModule>&& module, OMX_STRING name, OMX_STRING role, std::unique_ptr<EncExpertise>&& expertise) :
  Codec(component, media, std::move(module), name, role), expertise(std::move(expertise))
{
}

EncCodec::~EncCodec() = default;

void EncCodec::EmptyThisBufferCallBack(BufferHandleInterface* handle)
{
  auto header = ((OMXBufferHandle*)(handle))->header;

  ClearPropagatedData(header);

  if(roiMap.Exist(header))
  {
    auto roiBuffer = roiMap.Pop(header);
    roiFreeBuffers.push(roiBuffer);
  }

  if(callbacks.EmptyBufferDone)
    callbacks.EmptyBufferDone(component, app, header);

  delete handle;
}

static void AddEncoderFlags(OMXBufferHandle* handle, EncModule& module)
{
  auto const flags = module.GetFlags(handle);

  if(flags.isSync)
    handle->header->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;

  if(flags.isEndOfFrame)
    handle->header->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

  if(flags.isEndOfSlice)
    handle->header->nFlags |= OMX_BUFFERFLAG_ENDOFSUBFRAME;
}

void EncCodec::AssociateCallBack(BufferHandleInterface* empty_, BufferHandleInterface* fill_)
{
  auto empty = (OMXBufferHandle*)(empty_);
  auto fill = (OMXBufferHandle*)(fill_);
  auto emptyHeader = empty->header;
  auto fillHeader = fill->header;

  PropagateHeaderData(emptyHeader, fillHeader);
  AddEncoderFlags(fill, ToEncModule(*module));

  if(IsEOSDetected(emptyHeader->nFlags))
    callbacks.EventHandler(component, app, OMX_EventBufferFlag, output.index, emptyHeader->nFlags, nullptr);

  if(IsCompMarked(emptyHeader->hMarkTargetComponent, component))
    callbacks.EventHandler(component, app, OMX_EventMark, 0, 0, emptyHeader->pMarkData);
}

void EncCodec::FillThisBufferCallBack(BufferHandleInterface* filled, int offset, int size)
{
  assert(filled);
  auto header = (OMX_BUFFERHEADERTYPE*)(((OMXBufferHandle*)(filled))->header);

  header->nOffset = offset;
  header->nFilledLen = size;

  if(callbacks.FillBufferDone)
    callbacks.FillBufferDone(component, app, header);
  delete filled;
}

static OMX_PARAM_PORTDEFINITIONTYPE ConstructPortDefinition(Port& port, EncModule const& module)
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
  v.nBitrate = module.GetBitrate().target;
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

static OMX_VIDEO_PARAM_PORTFORMATTYPE ConstructVideoPortCurrentFormat(Port const& port, EncModule const& module)
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

static OMX_VIDEO_PARAM_QUANTIZATIONTYPE ConstructVideoQuantization(Port const& port, EncModule const& module)
{
  OMX_VIDEO_PARAM_QUANTIZATIONTYPE q;
  OMXChecker::SetHeaderVersion(q);
  q.nPortIndex = port.index;
  auto const moduleQPs = module.GetQPs();
  q.nQpI = ConvertToOMXQpI(moduleQPs);
  q.nQpP = ConvertToOMXQpP(moduleQPs);
  q.nQpB = ConvertToOMXQpB(moduleQPs);
  return q;
}

static OMX_VIDEO_PARAM_BITRATETYPE ConstructVideoBitrate(Port const& port, EncModule const& module)
{
  OMX_VIDEO_PARAM_BITRATETYPE b;
  OMXChecker::SetHeaderVersion(b);
  b.nPortIndex = port.index;
  auto const moduleBitrate = module.GetBitrate();
  b.eControlRate = ConvertToOMXControlRate(moduleBitrate.mode);
  b.nTargetBitrate = moduleBitrate.target;
  return b;
}

static OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL ConstructVideoQuantizationControl(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL q;
  OMXChecker::SetHeaderVersion(q);
  q.nPortIndex = port.index;
  auto const moduleQPs = module.GetQPs();
  q.eQpControlMode = ConvertToOMXQpControl(moduleQPs);
  return q;
}

static OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION ConstructVideoQuantizationExtension(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION q;
  OMXChecker::SetHeaderVersion(q);
  q.nPortIndex = port.index;
  auto const moduleQPs = module.GetQPs();
  q.nQpMin = ConvertToOMXQpMin(moduleQPs);
  q.nQpMax = ConvertToOMXQpMax(moduleQPs);
  return q;
}

static OMX_ALG_VIDEO_PARAM_ASPECT_RATIO ConstructVideoAspectRatio(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_ASPECT_RATIO a;
  OMXChecker::SetHeaderVersion(a);
  a.nPortIndex = port.index;
  auto const moduleAspectRatio = module.GetAspectRatio();
  a.eAspectRatio = ConvertToOMXAspectRatio(moduleAspectRatio);
  return a;
}

static OMX_ALG_VIDEO_PARAM_MAX_BITRATE ConstructVideoMaxBitrate(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_MAX_BITRATE b;
  OMXChecker::SetHeaderVersion(b);
  b.nPortIndex = port.index;
  auto const moduleBitrate = module.GetBitrate();
  b.nMaxBitrate = moduleBitrate.max;
  return b;
}

static OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH ConstructVideoLowBandwidth(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH bw;
  OMXChecker::SetHeaderVersion(bw);
  bw.nPortIndex = port.index;
  bw.bEnableLowBandwidth = ConvertToOMXBool(module.IsEnableLowBandwidth());
  return bw;
}

static OMX_ALG_VIDEO_PARAM_GOP_CONTROL ConstructVideoGopControl(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_GOP_CONTROL gc;
  OMXChecker::SetHeaderVersion(gc);
  gc.nPortIndex = port.index;
  auto const moduleGop = module.GetGop();
  gc.eGopControlMode = ConvertToOMXGopControl(moduleGop.mode);
  gc.eGdrMode = ConvertToOMXGdr(moduleGop.gdr);
  return gc;
}

static OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE ConstructVideoSceneChangeResilience(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE r;
  OMXChecker::SetHeaderVersion(r);
  r.nPortIndex = port.index;
  auto const moduleBitrate = module.GetBitrate();
  r.bDisableSceneChangeResilience = ConvertToOMXDisableSceneChangeResilience(moduleBitrate.option);
  return r;
}

static OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH ConstructVideoInstantaneousDecodingRefresh(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH idr;
  OMXChecker::SetHeaderVersion(idr);
  idr.nPortIndex = port.index;
  auto const moduleGop = module.GetGop();
  idr.nInstantaneousDecodingRefreshFrequency = moduleGop.idrFrequency;
  return idr;
}

static OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER ConstructVideoPrefetchBuffer(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER pb;
  OMXChecker::SetHeaderVersion(pb);
  pb.nPortIndex = port.index;
  pb.bEnablePrefetchBuffer = ConvertToOMXBool(module.IsEnablePrefetchBuffer());
  return pb;
}

static OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER ConstructVideoCodedPictureBuffer(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER cpb;
  OMXChecker::SetHeaderVersion(cpb);
  cpb.nPortIndex = port.index;
  auto const moduleBitrate = module.GetBitrate();
  cpb.nCodedPictureBufferSize = moduleBitrate.cpb;
  cpb.nInitialRemovalDelay = moduleBitrate.ird;
  return cpb;
}

static OMX_ALG_VIDEO_PARAM_SCALING_LIST ConstructVideoScalingList(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_SCALING_LIST scl;
  OMXChecker::SetHeaderVersion(scl);
  scl.nPortIndex = port.index;
  scl.eScalingListMode = ConvertToOMXScalingList(module.GetScalingList());
  return scl;
}

static OMX_ALG_VIDEO_PARAM_FILLER_DATA ConstructVideoFillerData(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_FILLER_DATA f;
  OMXChecker::SetHeaderVersion(f);
  f.nPortIndex = port.index;
  f.bDisableFillerData = ConvertToOMXBool(!module.IsEnableFillerData());
  return f;
}

static OMX_ALG_VIDEO_PARAM_SLICES ConstructVideoSlices(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_SLICES s;
  OMXChecker::SetHeaderVersion(s);
  s.nPortIndex = port.index;
  auto const moduleSlices = module.GetSlices();
  s.nNumSlices = moduleSlices.num;
  s.nSlicesSize = moduleSlices.size;
  s.bDependentSlices = ConvertToOMXBool(moduleSlices.dependent);
  return s;
}

static OMX_ALG_VIDEO_PARAM_SUBFRAME ConstructVideoSubframe(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_SUBFRAME sub;
  OMXChecker::SetHeaderVersion(sub);
  sub.nPortIndex = port.index;
  sub.bEnableSubframe = ConvertToOMXBool(module.IsEnableSubframe());
  return sub;
}

static OMX_ALG_PARAM_REPORTED_LATENCY ConstructReportedLatency(EncModule const& module)
{
  OMX_ALG_PARAM_REPORTED_LATENCY lat;
  OMXChecker::SetHeaderVersion(lat);
  lat.nLatency = module.GetLatency();
  return lat;
}

OMX_ALG_PORT_PARAM_BUFFER_MODE ConstructPortBufferMode(Port const& port, EncModule const& module)
{
  OMX_ALG_PORT_PARAM_BUFFER_MODE mode;
  OMXChecker::SetHeaderVersion(mode);
  mode.nPortIndex = port.index;
  mode.eMode = IsInputPort(port.index) ? ConvertToOMXBufferMode(module.GetFileDescriptors().input) : ConvertToOMXBufferMode(module.GetFileDescriptors().output);
  return mode;
}

static bool GetVideoPortFormatSupported(OMX_VIDEO_PARAM_PORTFORMATTYPE& format, EncModule const& module)
{
  auto supported = module.GetFormatsSupported();

  if(format.nIndex >= supported.size())
    return false;

  auto const moduleMime = IsInputPort(format.nPortIndex) ? module.GetMimes().input : module.GetMimes().output;
  format.eCompressionFormat = ConvertToOMXCompression(moduleMime.compression);
  format.eColorFormat = ConvertToOMXColor(supported[format.nIndex].color, supported[format.nIndex].bitdepth);
  auto const moduleClock = module.GetClock();
  format.xFramerate = ConvertToOMXFramerate(moduleClock);

  return true;
}

static OMX_INTERLACEFORMATTYPE ConstructVideoModesSupported(Port const& port, EncModule const& module)
{
  OMX_INTERLACEFORMATTYPE interlace;
  OMXChecker::SetHeaderVersion(interlace);
  interlace.nPortIndex = port.index;
  auto const modes = module.GetVideoModesSupported();

  for(auto const mode : modes)
    interlace.nFormat |= ConvertToOMXInterlaceFlag(mode);

  return interlace;
}

static OMX_INTERLACEFORMATTYPE ConstructVideoModeCurrent(Port const& port, EncModule const& module)
{
  OMX_INTERLACEFORMATTYPE interlace;
  OMXChecker::SetHeaderVersion(interlace);
  interlace.nPortIndex = port.index;
  interlace.nFormat = ConvertToOMXInterlaceFlag(module.GetVideoMode());

  return interlace;
}

static OMX_ALG_VIDEO_PARAM_LONG_TERM ConstructVideoLongTerm(Port const& port, EncModule const& module)
{
  OMX_ALG_VIDEO_PARAM_LONG_TERM longTerm;
  OMXChecker::SetHeaderVersion(longTerm);
  longTerm.nPortIndex = port.index;
  longTerm.bEnableLongTerm = ConvertToOMXBool(module.GetGop().isLongTermEnabled);
  return longTerm;
}

OMX_ERRORTYPE EncCodec::GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));
  OMXChecker::CheckStateOperation(AL_GetParameter, state);

  auto getCurrentPort = [=](OMX_PTR param) -> Port*
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
    auto port = getCurrentPort(param);
    *(OMX_PARAM_PORTDEFINITIONTYPE*)param = ConstructPortDefinition(*port, ToEncModule(*module));
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
    (void)port;
    auto p = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)param;

    if(!GetVideoPortFormatSupported(*p, ToEncModule(*module)))
      return OMX_ErrorNoMore;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    auto const port = getCurrentPort(param);
    expertise->GetProfileLevel(param, *port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelQuerySupported: // GetParameter Only
  {
    auto const port = getCurrentPort(param);
    (void)port;

    if(!expertise->GetProfileLevelSupported(param, ToEncModule(*module)))
      return OMX_ErrorNoMore;

    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoQuantization:
  {
    auto const port = getCurrentPort(param);
    *(OMX_VIDEO_PARAM_QUANTIZATIONTYPE*)param = ConstructVideoQuantization(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoBitrate:
  {
    auto const port = getCurrentPort(param);
    *(OMX_VIDEO_PARAM_BITRATETYPE*)param = ConstructVideoBitrate(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoAvc:
  case OMX_ALG_IndexParamVideoHevc:
  {
    auto const port = getCurrentPort(param);
    expertise->GetExpertise(param, *port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInterlaceFormatSupported: // GetParameter only
  {
    auto const port = getCurrentPort(param);
    *(OMX_INTERLACEFORMATTYPE*)param = ConstructVideoModesSupported(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInterlaceFormatCurrent:
  {
    auto const port = getCurrentPort(param);
    *(OMX_INTERLACEFORMATTYPE*)param = ConstructVideoModeCurrent(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoQuantizationControl:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL*)param = ConstructVideoQuantizationControl(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoQuantizationExtension:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION*)param = ConstructVideoQuantizationExtension(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoAspectRatio:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_ASPECT_RATIO*)param = ConstructVideoAspectRatio(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoMaxBitrate:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_MAX_BITRATE*)param = ConstructVideoMaxBitrate(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoLowBandwidth:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH*)param = ConstructVideoLowBandwidth(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoGopControl:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_GOP_CONTROL*)param = ConstructVideoGopControl(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoSceneChangeResilience:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE*)param = ConstructVideoSceneChangeResilience(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInstantaneousDecodingRefresh:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH*)param = ConstructVideoInstantaneousDecodingRefresh(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoCodedPictureBuffer:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER*)param = ConstructVideoCodedPictureBuffer(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoPrefetchBuffer:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER*)param = ConstructVideoPrefetchBuffer(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoScalingList:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_SCALING_LIST*)param = ConstructVideoScalingList(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoFillerData:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_FILLER_DATA*)param = ConstructVideoFillerData(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoSlices:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_SLICES*)param = ConstructVideoSlices(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoSubframe:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_SUBFRAME*)param = ConstructVideoSubframe(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamReportedLatency: // GetParameter only
  {
    *(OMX_ALG_PARAM_REPORTED_LATENCY*)param = ConstructReportedLatency(ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexPortParamBufferMode:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_PORT_PARAM_BUFFER_MODE*)param = ConstructPortBufferMode(*port, ToEncModule(*module));
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoLongTerm:
  {
    auto const port = getCurrentPort(param);
    *(OMX_ALG_VIDEO_PARAM_LONG_TERM*)param = ConstructVideoLongTerm(*port, ToEncModule(*module));
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

static bool SetFormat(OMX_COLOR_FORMATTYPE const& color, EncModule& module)
{
  auto moduleFormat = module.GetFormat();
  moduleFormat.color = ConvertToModuleColor(color);
  moduleFormat.bitdepth = ConvertToModuleBitdepth(color);
  return module.SetFormat(moduleFormat);
}

static bool SetClock(OMX_U32 framerateInQ16, EncModule& module)
{
  auto moduleClock = module.GetClock();
  auto const clock = ConvertToModuleClock(framerateInQ16);
  moduleClock.framerate = clock.framerate;
  moduleClock.clockratio = clock.clockratio;
  return module.SetClock(moduleClock);
}

static bool SetResolution(OMX_VIDEO_PORTDEFINITIONTYPE const& definition, EncModule& module)
{
  auto moduleResolution = module.GetResolution();
  moduleResolution.width = definition.nFrameWidth;
  moduleResolution.height = definition.nFrameHeight;
  moduleResolution.stride = definition.nStride;
  moduleResolution.sliceHeight = definition.nSliceHeight;
  return module.SetResolution(moduleResolution);
}

static bool SetTargetBitrate(OMX_U32 bitrate, EncModule& module)
{
  auto moduleBitrate = module.GetBitrate();
  moduleBitrate.target = bitrate;

  if(moduleBitrate.max < moduleBitrate.target)
    moduleBitrate.max = moduleBitrate.target;
  return module.SetBitrate(moduleBitrate);
}

static bool SetQuantization(OMX_U32 qpI, OMX_U32 qpP, OMX_U32 qpB, EncModule& module)
{
  auto moduleQPs = module.GetQPs();
  moduleQPs.initial = ConvertToModuleQPInitial(qpI);
  moduleQPs.deltaIP = ConvertToModuleQPDeltaIP(qpI, qpP);
  moduleQPs.deltaPB = ConvertToModuleQPDeltaPB(qpP, qpB);
  return module.SetQPs(moduleQPs);
}

static bool SetQuantizationControl(OMX_ALG_EQpCtrlMode const& mode, EncModule& module)
{
  auto moduleQPs = module.GetQPs();
  moduleQPs.mode = ConvertToModuleQPControl(mode);
  return module.SetQPs(moduleQPs);
}

static bool SetQuantizationExtension(OMX_S32 qpMin, OMX_S32 qpMax, EncModule& module)
{
  auto moduleQPs = module.GetQPs();
  moduleQPs.min = ConvertToModuleQPMin(qpMin);
  moduleQPs.max = ConvertToModuleQPMax(qpMax);
  return module.SetQPs(moduleQPs);
}

static bool SetModeBitrate(OMX_U32 target, OMX_VIDEO_CONTROLRATETYPE mode, EncModule& module)
{
  auto moduleBitrate = module.GetBitrate();
  moduleBitrate.mode = ConvertToModuleControlRate(mode);
  moduleBitrate.target = target;

  if(moduleBitrate.max < moduleBitrate.target)
    moduleBitrate.max = moduleBitrate.target;

  return module.SetBitrate(moduleBitrate);
}

static bool SetAspectRatio(OMX_ALG_EAspectRatio const& aspectRatio, EncModule& module)
{
  return module.SetAspectRatio(ConvertToModuleAspectRatio(aspectRatio));
}

static bool SetMaxBitrate(OMX_U32 max, EncModule& module)
{
  auto moduleBitrate = module.GetBitrate();
  moduleBitrate.max = max;

  if(moduleBitrate.target > moduleBitrate.max)
    moduleBitrate.target = moduleBitrate.max;

  return module.SetBitrate(moduleBitrate);
}

static bool SetLowBandwidth(OMX_BOOL enableLowBandwidth, EncModule& module)
{
  return module.SetEnableLowBandwidth(ConvertToModuleBool(enableLowBandwidth));
}

static bool SetGopControl(OMX_ALG_EGopCtrlMode const& mode, OMX_ALG_EGdrMode const& gdr, EncModule& module)
{
  auto moduleGop = module.GetGop();
  moduleGop.mode = ConvertToModuleGopControl(mode);
  moduleGop.gdr = ConvertToModuleGdr(gdr);
  return module.SetGop(moduleGop);
}

static bool SetSceneChangeResilience(OMX_BOOL disableSceneChangeResilience, EncModule& module)
{
  auto moduleBitrate = module.GetBitrate();
  moduleBitrate.option = ConvertToModuleDisableSceneChangeResilience(disableSceneChangeResilience);
  return module.SetBitrate(moduleBitrate);
}

static bool SetInstantaneousDecodingRefresh(OMX_U32 instantaneousDecodingRefreshFrequency, EncModule& module)
{
  auto moduleGop = module.GetGop();
  moduleGop.idrFrequency = instantaneousDecodingRefreshFrequency;
  return module.SetGop(moduleGop);
}

static bool SetCodedPictureBuffer(OMX_U32 codedPictureBufferSize, OMX_U32 initialRemovalDelay, EncModule& module)
{
  auto moduleBitrate = module.GetBitrate();
  moduleBitrate.cpb = codedPictureBufferSize;
  moduleBitrate.ird = initialRemovalDelay;
  return module.SetBitrate(moduleBitrate);
}

static bool SetPrefetchBuffer(OMX_BOOL enablePrefetchBuffer, EncModule& module)
{
  return module.SetEnablePrefetchBuffer(ConvertToModuleBool(enablePrefetchBuffer));
}

static bool SetScalingList(OMX_ALG_EScalingList const& scalingListMode, EncModule& module)
{
  return module.SetScalingList(ConvertToModuleScalingList(scalingListMode));
}

static bool SetFillerData(OMX_BOOL disableFillerData, EncModule& module)
{
  return module.SetEnableFillerData(!ConvertToModuleBool(disableFillerData));
}

static bool SetSlices(OMX_U32 numSlices, OMX_U32 slicesSize, OMX_BOOL dependentSlices, EncModule& module)
{
  auto moduleSlices = module.GetSlices();
  moduleSlices.num = numSlices;
  moduleSlices.size = slicesSize;
  moduleSlices.dependent = ConvertToModuleBool(dependentSlices);
  return module.SetSlices(moduleSlices);
}

static bool SetSubframe(OMX_BOOL enableSubframe, EncModule& module)
{
  return module.SetEnableSubframe(ConvertToModuleBool(enableSubframe));
}

static bool SetInputBufferMode(OMX_ALG_BUFFER_MODE mode, EncModule& module)
{
  auto moduleFds = module.GetFileDescriptors();
  moduleFds.input = ConvertToModuleFileDescriptor(mode);
  return module.SetFileDescriptors(moduleFds);
}

static bool SetOutputBufferMode(OMX_ALG_BUFFER_MODE mode, EncModule& module)
{
  auto moduleFds = module.GetFileDescriptors();
  moduleFds.output = ConvertToModuleFileDescriptor(mode);
  return module.SetFileDescriptors(moduleFds);
}

static bool SetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, EncModule& module)
{
  auto const rollback = ConstructPortDefinition(port, module);
  auto const video = settings.format.video;

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

  if(!SetTargetBitrate(video.nBitrate, module))
  {
    SetPortDefinition(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE const& format, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoPortCurrentFormat(port, module);

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

static bool SetVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE const& quantization, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoQuantization(port, module);

  if(!SetQuantization(quantization.nQpI, quantization.nQpP, quantization.nQpB, module))
  {
    SetVideoQuantization(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoQuantizationControl(OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL const& quantizationControl, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoQuantizationControl(port, module);

  if(!SetQuantizationControl(quantizationControl.eQpControlMode, module))
  {
    SetVideoQuantizationControl(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoQuantizationExtension(OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION const& quantizationExtension, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoQuantizationExtension(port, module);

  if(!SetQuantizationExtension(quantizationExtension.nQpMin, quantizationExtension.nQpMax, module))
  {
    SetVideoQuantizationExtension(rollback, port, module);
    return false;
  }

  return true;
}

static bool SetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE const& bitrate, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoBitrate(port, module);

  if(!SetModeBitrate(bitrate.nTargetBitrate, bitrate.eControlRate, module))
  {
    SetVideoBitrate(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoAspectRatio(OMX_ALG_VIDEO_PARAM_ASPECT_RATIO const& aspectRatio, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoAspectRatio(port, module);

  if(!SetAspectRatio(aspectRatio.eAspectRatio, module))
  {
    SetVideoAspectRatio(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoMaxBitrate(OMX_ALG_VIDEO_PARAM_MAX_BITRATE const& maxBitrate, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoMaxBitrate(port, module);

  if(!SetMaxBitrate(maxBitrate.nMaxBitrate, module))
  {
    SetVideoMaxBitrate(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoLowBandwidth(OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH const& lowBandwidth, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoLowBandwidth(port, module);

  if(!SetLowBandwidth(lowBandwidth.bEnableLowBandwidth, module))
  {
    SetVideoLowBandwidth(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoGopControl(OMX_ALG_VIDEO_PARAM_GOP_CONTROL const& gopControl, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoGopControl(port, module);

  if(!SetGopControl(gopControl.eGopControlMode, gopControl.eGdrMode, module))
  {
    SetVideoGopControl(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoSceneChangeResilience(OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE const& sceneChangeResilience, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoSceneChangeResilience(port, module);

  if(!SetSceneChangeResilience(sceneChangeResilience.bDisableSceneChangeResilience, module))
  {
    SetVideoSceneChangeResilience(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoInstantaneousDecodingRefresh(OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH const& instantaneousDecodingRefresh, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoInstantaneousDecodingRefresh(port, module);

  if(!SetInstantaneousDecodingRefresh(instantaneousDecodingRefresh.nInstantaneousDecodingRefreshFrequency, module))
  {
    SetVideoInstantaneousDecodingRefresh(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoCodedPictureBuffer(OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER const& codedPictureBuffer, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoCodedPictureBuffer(port, module);

  if(!SetCodedPictureBuffer(codedPictureBuffer.nCodedPictureBufferSize, codedPictureBuffer.nInitialRemovalDelay, module))
  {
    SetVideoCodedPictureBuffer(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoPrefetchBuffer(OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER const& prefetchBuffer, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoPrefetchBuffer(port, module);

  if(!SetPrefetchBuffer(prefetchBuffer.bEnablePrefetchBuffer, module))
  {
    SetVideoPrefetchBuffer(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoScalingList(OMX_ALG_VIDEO_PARAM_SCALING_LIST const& scalingList, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoScalingList(port, module);

  if(!SetScalingList(scalingList.eScalingListMode, module))
  {
    SetVideoScalingList(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoFillerData(OMX_ALG_VIDEO_PARAM_FILLER_DATA const& fillerData, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoFillerData(port, module);

  if(!SetFillerData(fillerData.bDisableFillerData, module))
  {
    SetVideoFillerData(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoSlices(OMX_ALG_VIDEO_PARAM_SLICES const& slices, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoSlices(port, module);

  if(!SetSlices(slices.nNumSlices, slices.nSlicesSize, slices.bDependentSlices, module))
  {
    SetVideoSlices(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME const& subframe, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoSubframe(port, module);

  if(!SetSubframe(subframe.bEnableSubframe, module))
  {
    SetVideoSubframe(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE const& portBufferMode, Port const& port, EncModule& module)
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

static bool SetPortExpectedBuffer(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, EncModule const& module)
{
  auto const min = IsInputPort(settings.nPortIndex) ? module.GetBufferRequirements().input.min : module.GetBufferRequirements().output.min;
  auto const actual = static_cast<int>(settings.nBufferCountActual);

  if(actual < min)
    return false;

  port.expected = actual;

  return true;
}

static bool SetInterlaceMode(OMX_U32 flag, EncModule& module)
{
  return module.SetVideoMode(ConvertToModuleVideoMode(flag));
}

static bool SetVideoModeCurrent(OMX_INTERLACEFORMATTYPE const& interlace, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoModeCurrent(port, module);

  if(!SetInterlaceMode(interlace.nFormat, module))
  {
    SetVideoModeCurrent(rollback, port, module);
    return false;
  }
  return true;
}

static bool SetLongTerm(OMX_BOOL isLongTermEnabled, EncModule& module)
{
  auto moduleGop = module.GetGop();
  moduleGop.isLongTermEnabled = ConvertToModuleBool(isLongTermEnabled);
  return module.SetGop(moduleGop);
}

static bool SetVideoLongTerm(OMX_ALG_VIDEO_PARAM_LONG_TERM const& longTerm, Port const& port, EncModule& module)
{
  auto const rollback = ConstructVideoLongTerm(port, module);

  if(!SetLongTerm(longTerm.bEnableLongTerm, module))
  {
    SetVideoLongTerm(rollback, port, module);
    return false;
  }
  return true;
}

OMX_ERRORTYPE EncCodec::SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(param);
  OMXChecker::CheckHeaderVersion(GetVersion(param));

  auto getCurrentPort = [=](OMX_PTR param) -> Port*
                        {
                          auto const index = *(((OMX_U32*)param) + 2);
                          return GetPort(index);
                        };

  Port* port;

  if(OMX_U32(index) != OMX_IndexParamStandardComponentRole)
  {
    port = getCurrentPort(param);

    if(!port->isTransientToDisable && port->enable)
      OMXChecker::CheckStateOperation(AL_SetParameter, state);
  }
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
  case OMX_IndexParamPortDefinition:
  {
    auto const settings = static_cast<OMX_PARAM_PORTDEFINITIONTYPE*>(param);

    if(!SetPortExpectedBuffer(*settings, const_cast<Port &>(*port), ToEncModule(*module)))
      throw OMX_ErrorBadParameter;

    if(!SetPortDefinition(*settings, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    // Do nothing
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoPortFormat:
  {
    auto const format = static_cast<OMX_VIDEO_PARAM_PORTFORMATTYPE*>(param);

    if(!SetVideoPortFormat(*format, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    if(!expertise->SetProfileLevel(param, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoQuantization:
  {
    auto const quantization = static_cast<OMX_VIDEO_PARAM_QUANTIZATIONTYPE*>(param);

    if(!SetVideoQuantization(*quantization, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoBitrate:
  {
    auto const bitrate = static_cast<OMX_VIDEO_PARAM_BITRATETYPE*>(param);

    if(!SetVideoBitrate(*bitrate, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_IndexParamVideoAvc:
  case OMX_ALG_IndexParamVideoHevc:
  {
    if(!expertise->SetExpertise(param, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInterlaceFormatCurrent:
  {
    auto const interlaced = static_cast<OMX_INTERLACEFORMATTYPE*>(param);

    if(!SetVideoModeCurrent(*interlaced, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoQuantizationControl:
  {
    auto const quantizationControl = static_cast<OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL*>(param);

    if(!SetVideoQuantizationControl(*quantizationControl, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoQuantizationExtension:
  {
    auto const quantizationExtension = static_cast<OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION*>(param);

    if(!SetVideoQuantizationExtension(*quantizationExtension, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoAspectRatio:
  {
    auto const aspectRatio = static_cast<OMX_ALG_VIDEO_PARAM_ASPECT_RATIO*>(param);

    if(!SetVideoAspectRatio(*aspectRatio, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoMaxBitrate:
  {
    auto const maxBitrate = static_cast<OMX_ALG_VIDEO_PARAM_MAX_BITRATE*>(param);

    if(!SetVideoMaxBitrate(*maxBitrate, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoLowBandwidth:
  {
    auto const lowBandwidth = static_cast<OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH*>(param);

    if(!SetVideoLowBandwidth(*lowBandwidth, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoGopControl:
  {
    auto const gopControl = static_cast<OMX_ALG_VIDEO_PARAM_GOP_CONTROL*>(param);

    if(!SetVideoGopControl(*gopControl, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoSceneChangeResilience:
  {
    auto const sceneChangeResilience = static_cast<OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE*>(param);

    if(!SetVideoSceneChangeResilience(*sceneChangeResilience, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoInstantaneousDecodingRefresh:
  {
    auto const instantaneousDecodingRefresh = static_cast<OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH*>(param);

    if(!SetVideoInstantaneousDecodingRefresh(*instantaneousDecodingRefresh, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoCodedPictureBuffer:
  {
    auto const codedPictureBuffer = static_cast<OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER*>(param);

    if(!SetVideoCodedPictureBuffer(*codedPictureBuffer, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoPrefetchBuffer:
  {
    auto const prefetchBuffer = static_cast<OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER*>(param);

    if(!SetVideoPrefetchBuffer(*prefetchBuffer, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoScalingList:
  {
    auto const scalingList = static_cast<OMX_ALG_VIDEO_PARAM_SCALING_LIST*>(param);

    if(!SetVideoScalingList(*scalingList, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoFillerData:
  {
    auto const fillerData = static_cast<OMX_ALG_VIDEO_PARAM_FILLER_DATA*>(param);

    if(!SetVideoFillerData(*fillerData, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoSlices:
  {
    auto const slices = static_cast<OMX_ALG_VIDEO_PARAM_SLICES*>(param);

    if(!SetVideoSlices(*slices, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoSubframe:
  {
    auto const subframe = static_cast<OMX_ALG_VIDEO_PARAM_SUBFRAME*>(param);

    if(!SetVideoSubframe(*subframe, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexPortParamBufferMode:
  {
    auto const portBufferMode = static_cast<OMX_ALG_PORT_PARAM_BUFFER_MODE*>(param);

    if(!SetPortBufferMode(*portBufferMode, *port, ToEncModule(*module)))
      throw OMX_ErrorBadParameter;
    return OMX_ErrorNone;
  }
  case OMX_ALG_IndexParamVideoLongTerm:
  {
    auto const longTerm = static_cast<OMX_ALG_VIDEO_PARAM_LONG_TERM*>(param);

    if(!SetVideoLongTerm(*longTerm, *port, ToEncModule(*module)))
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

OMX_ERRORTYPE EncCodec::GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index)
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

uint8_t* EncCodec::AllocateROIBuffer()
{
  int roiSize;
  module->GetDynamic(DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_SIZE, &roiSize);
  return static_cast<uint8_t*>(calloc(roiSize, sizeof(uint8_t)));
}

OMX_ERRORTYPE EncCodec::UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size, OMX_IN OMX_U8* buffer)
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

  if(IsInputPort(index))
  {
    auto roiBuffer = AllocateROIBuffer();
    roiFreeBuffers.push(roiBuffer);
    roiDestroyMap.Add(*header, roiBuffer);
  }

  return OMX_ErrorNone;
  OMX_CATCH_L([&](OMX_ERRORTYPE& e)
  {
    if(e != OMX_ErrorBadPortIndex)
      GetPort(index)->ErrorOccured();
  });
}

OMX_ERRORTYPE EncCodec::AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckNotNull(size);
  CheckPortIndex(index);

  auto port = GetPort(index);

  if(transientState != TransientLoadedToIdle && !(port->isTransientToEnable))
    throw OMX_ErrorIncorrectStateOperation;

  auto const dmaOnPort = IsInputPort(index) ? ToEncModule(*module).GetFileDescriptors().input : ToEncModule(*module).GetFileDescriptors().output;
  auto buffer = dmaOnPort ? reinterpret_cast<OMX_U8*>(ToEncModule(*module).AllocateDMA(size * sizeof(OMX_U8))) : static_cast<OMX_U8*>(module->Allocate(size * sizeof(OMX_U8)));

  if(dmaOnPort ? (static_cast<int>((intptr_t)buffer) < 0) : !buffer)
    throw OMX_ErrorInsufficientResources;

  *header = AllocateHeader(app, size, buffer, true, index);
  assert(*header);
  port->Add(*header);

  if(IsInputPort(index))
  {
    auto roiBuffer = AllocateROIBuffer();
    roiFreeBuffers.push(roiBuffer);
    roiDestroyMap.Add(*header, roiBuffer);
  }

  return OMX_ErrorNone;
  OMX_CATCH_L([&](OMX_ERRORTYPE& e)
  {
    if(e != OMX_ErrorBadPortIndex)
      GetPort(index)->ErrorOccured();
  });
}

void EncCodec::DestroyROIBuffer(uint8_t* roiBuffer)
{
  free(roiBuffer);
}

OMX_ERRORTYPE EncCodec::FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);

  CheckPortIndex(index);
  auto port = GetPort(index);

  if((transientState != TransientIdleToLoaded) && (!port->isTransientToDisable))
    callbacks.EventHandler(component, app, OMX_EventError, OMX_ErrorPortUnpopulated, 0, nullptr);

  if(isBufferAllocatedByModule(header))
  {
    auto const dmaOnPort = IsInputPort(index) ? ToEncModule(*module).GetFileDescriptors().input : ToEncModule(*module).GetFileDescriptors().output;
    dmaOnPort ? ToEncModule(*module).FreeDMA(static_cast<int>((intptr_t)header->pBuffer)) : module->Free(header->pBuffer);
  }

  if(IsInputPort(index))
  {
    if(roiDestroyMap.Exist(header))
    {
      auto roiBuffer = roiDestroyMap.Pop(header);
      DestroyROIBuffer(roiBuffer);
    }
  }

  port->Remove(header);
  DeleteHeader(header);

  return OMX_ErrorNone;
  OMX_CATCH();
}

void EncCodec::TreatEmptyBufferCommand(Task* task)
{
  assert(task);
  assert(task->cmd == EmptyBuffer);
  assert(static_cast<int>((intptr_t)task->data) == input.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task->opt.get());
  assert(header);
  AttachMark(header);

  if(shouldPushROI && header->nFilledLen)
  {
    auto roiBuffer = roiFreeBuffers.pop();
    module->GetDynamic(DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_FILL, roiBuffer);
    module->SetDynamic(DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_EMPTY, roiBuffer);
    roiMap.Add(header, roiBuffer);
  }

  auto handle = new OMXBufferHandle(header);
  auto success = module->Empty(handle);

  shouldClearROI = true;
  assert(success);
}

