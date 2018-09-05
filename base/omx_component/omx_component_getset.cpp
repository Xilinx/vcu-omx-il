/******************************************************************************
*
* Copyright (C) 2018 Allegro DVT2.  All rights reserved.
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

#include "omx_component_getset.h"

#include "base/omx_checker/omx_checker.h"

using namespace std;

#define OMX_CHECK_MEDIA_GET(ret) \
  if(ret == MediatypeInterface::ERROR_SETTINGS_BAD_INDEX) \
    throw OMX_ErrorUnsupportedIndex; \
  assert(ret == MediatypeInterface::ERROR_SETTINGS_NONE);

#define OMX_CHECK_MEDIA_SET(ret) \
  if(ret == MediatypeInterface::ERROR_SETTINGS_BAD_INDEX) \
    return OMX_ErrorUnsupportedIndex; \
  if(ret == MediatypeInterface::ERROR_SETTINGS_BAD_PARAMETER) \
    return OMX_ErrorBadParameter; \
  assert(ret == MediatypeInterface::ERROR_SETTINGS_NONE);

// Common

OMX_ERRORTYPE ConstructPortSupplier(OMX_PARAM_BUFFERSUPPLIERTYPE& s, Port const& port)
{
  OMXChecker::SetHeaderVersion(s);
  s.nPortIndex = port.index;
  s.eBufferSupplier = OMX_BufferSupplyUnspecified; // We don't care
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructReportedLatency(OMX_ALG_PARAM_REPORTED_LATENCY& lat, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(lat);
  auto ret = media->Get(SETTINGS_INDEX_LATENCY, &lat.nLatency);
  OMX_CHECK_MEDIA_GET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetPortExpectedBuffer(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, ModuleInterface const& module)
{
  auto min = IsInputPort(settings.nPortIndex) ? module.GetBufferRequirements().input.min : module.GetBufferRequirements().output.min;
  auto actual = static_cast<int>(settings.nBufferCountActual);

  if(actual < min)
    throw OMX_ErrorBadParameter;

  port.expected = actual;

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME& subframe, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(subframe);
  subframe.nPortIndex = port.index;
  bool isEnabledSubFrame;
  auto ret = media->Get(SETTINGS_INDEX_SUBFRAME, &isEnabledSubFrame);
  OMX_CHECK_MEDIA_GET(ret);
  subframe.bEnableSubframe = ConvertMediaToOMXBool(isEnabledSubFrame);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetSubframe(OMX_BOOL enableSubframe, shared_ptr<MediatypeInterface> media)
{
  auto isEnabled = ConvertOMXToMediaBool(enableSubframe);
  auto ret = media->Set(SETTINGS_INDEX_SUBFRAME, &isEnabled);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME const& subframe, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_SUBFRAME rollback;
  ConstructVideoSubframe(rollback, port, media);

  auto ret = SetSubframe(subframe.bEnableSubframe, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoSubframe(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE& mode, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(mode);
  mode.nPortIndex = port.index;
  BufferHandles handles;
  auto ret = media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &handles);
  OMX_CHECK_MEDIA_GET(ret);
  mode.eMode = IsInputPort(port.index) ? ConvertMediaToOMXBufferHandle(handles.input) : ConvertMediaToOMXBufferHandle(handles.output);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetInputBufferMode(OMX_ALG_BUFFER_MODE mode, shared_ptr<MediatypeInterface> media)
{
  BufferHandles handles;
  auto ret = media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &handles);
  OMX_CHECK_MEDIA_GET(ret);
  handles.input = ConvertOMXToMediaBufferHandle(mode);
  ret = media->Set(SETTINGS_INDEX_BUFFER_HANDLES, &handles);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetOutputBufferMode(OMX_ALG_BUFFER_MODE mode, shared_ptr<MediatypeInterface> media)
{
  BufferHandles handles;
  auto ret = media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &handles);
  OMX_CHECK_MEDIA_GET(ret);
  handles.output = ConvertOMXToMediaBufferHandle(mode);
  ret = media->Set(SETTINGS_INDEX_BUFFER_HANDLES, &handles);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE const& portBufferMode, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_PORT_PARAM_BUFFER_MODE rollback;
  ConstructPortBufferMode(rollback, port, media);
  auto& setBufferMode = IsInputPort(portBufferMode.nPortIndex) ? SetInputBufferMode : SetOutputBufferMode;

  auto ret = setBufferMode(portBufferMode.eMode, media);

  if(ret != OMX_ErrorNone)
  {
    SetPortBufferMode(rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE GetVideoPortFormatSupported(OMX_VIDEO_PARAM_PORTFORMATTYPE& format, shared_ptr<MediatypeInterface> media)
{
  SupportedFormats supportedFormats;
  auto ret = media->Get(SETTINGS_INDEX_FORMATS_SUPPORTED, &supportedFormats);
  OMX_CHECK_MEDIA_GET(ret);
  vector<Format> supported = IsInputPort(format.nPortIndex) ? supportedFormats.input : supportedFormats.output;

  if(format.nIndex >= supported.size())
    return OMX_ErrorNoMore;

  Clock clock;
  Mimes mimes;
  ret = media->Get(SETTINGS_INDEX_CLOCK, &clock);
  OMX_CHECK_MEDIA_GET(ret);
  ret = media->Get(SETTINGS_INDEX_MIMES, &mimes);
  OMX_CHECK_MEDIA_GET(ret);
  Mime mime = IsInputPort(format.nPortIndex) ? mimes.input : mimes.output;
  format.eCompressionFormat = ConvertMediaToOMXCompression(mime.compression);
  format.eColorFormat = ConvertMediaToOMXColor(supported[format.nIndex].color, supported[format.nIndex].bitdepth);
  format.xFramerate = ConvertMediaToOMXFramerate(clock);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoPortCurrentFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE& f, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(f);
  f.nPortIndex = port.index;
  f.nIndex = 0;
  Format format;
  Clock clock;
  Mimes mimes;
  auto ret = media->Get(SETTINGS_INDEX_FORMAT, &format);
  OMX_CHECK_MEDIA_GET(ret);
  ret = media->Get(SETTINGS_INDEX_CLOCK, &clock);
  OMX_CHECK_MEDIA_GET(ret);
  ret = media->Get(SETTINGS_INDEX_MIMES, &mimes);
  OMX_CHECK_MEDIA_GET(ret);
  Mime mime = IsInputPort(f.nPortIndex) ? mimes.input : mimes.output;
  f.eCompressionFormat = ConvertMediaToOMXCompression(mime.compression);
  f.eColorFormat = ConvertMediaToOMXColor(format.color, format.bitdepth);
  f.xFramerate = ConvertMediaToOMXFramerate(clock);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetFormat(OMX_COLOR_FORMATTYPE const& color, shared_ptr<MediatypeInterface> media)
{
  Format format;
  auto ret = media->Get(SETTINGS_INDEX_FORMAT, &format);
  OMX_CHECK_MEDIA_GET(ret);
  format.color = ConvertOMXToMediaColor(color);
  format.bitdepth = ConvertOMXToMediaBitdepth(color);
  ret = media->Set(SETTINGS_INDEX_FORMAT, &format);
  OMX_CHECK_MEDIA_SET(ret)
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE const& format, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE rollback;
  ConstructVideoPortCurrentFormat(rollback, port, media);

  auto ret = SetClock(format.xFramerate, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoPortFormat(rollback, port, media);
    throw ret;
  }

  ret = SetFormat(format.eColorFormat, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoPortFormat(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetResolution(OMX_VIDEO_PORTDEFINITIONTYPE const& definition, shared_ptr<MediatypeInterface> media)
{
  Resolution resolution;
  auto ret = media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
  OMX_CHECK_MEDIA_GET(ret);
  resolution.width = definition.nFrameWidth;
  resolution.height = definition.nFrameHeight;
  resolution.stride.widthStride = definition.nStride;
  resolution.stride.heightStride = definition.nSliceHeight;
  ret = media->Set(SETTINGS_INDEX_RESOLUTION, &resolution);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetClock(OMX_U32 framerateInQ16, shared_ptr<MediatypeInterface> media)
{
  Clock curClock;
  auto ret = media->Get(SETTINGS_INDEX_CLOCK, &curClock);
  OMX_CHECK_MEDIA_GET(ret);
  auto clock = ConvertOMXToMediaClock(framerateInQ16);
  curClock.framerate = clock.framerate;
  curClock.clockratio = clock.clockratio;
  ret = media->Set(SETTINGS_INDEX_CLOCK, &curClock);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE& def, Port& port, ModuleInterface const& module, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(def);
  def.nPortIndex = port.index;
  def.eDir = IsInputPort(def.nPortIndex) ? OMX_DirInput : OMX_DirOutput;
  auto requirements = IsInputPort(def.nPortIndex) ? module.GetBufferRequirements().input : module.GetBufferRequirements().output;

  if(port.expected < (size_t)requirements.min)
    port.expected = requirements.min;
  def.nBufferCountActual = port.expected;
  def.bEnabled = ConvertMediaToOMXBool(port.enable);
  def.bPopulated = ConvertMediaToOMXBool(port.playable);
  def.nBufferCountMin = requirements.min;
  def.nBufferSize = requirements.size;
  def.bBuffersContiguous = ConvertMediaToOMXBool(requirements.contiguous);
  def.nBufferAlignment = requirements.bytesAlignment;
  def.eDomain = OMX_PortDomainVideo;

  auto& v = def.format.video;
  Resolution resolution;
  Format format;
  Clock clock;
  Mimes mimes;
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
  OMX_CHECK_MEDIA_GET(ret);
  ret = media->Get(SETTINGS_INDEX_FORMAT, &format);
  OMX_CHECK_MEDIA_GET(ret);
  ret = media->Get(SETTINGS_INDEX_CLOCK, &clock);
  OMX_CHECK_MEDIA_GET(ret);
  ret = media->Get(SETTINGS_INDEX_MIMES, &mimes);
  OMX_CHECK_MEDIA_GET(ret);
  ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);

  // Get Bitrate is encoder only
  if(ret == MediatypeInterface::ERROR_SETTINGS_BAD_INDEX)
    bitrate.target = 0; // 0 by default for Decoder
  Mime mime = IsInputPort(def.nPortIndex) ? mimes.input : mimes.output;
  v.pNativeRender = 0; // XXX
  v.nFrameWidth = resolution.width;
  v.nFrameHeight = resolution.height;
  v.nStride = resolution.stride.widthStride;
  v.nSliceHeight = resolution.stride.heightStride;
  v.nBitrate = bitrate.target;
  v.xFramerate = ConvertMediaToOMXFramerate(clock);
  v.bFlagErrorConcealment = ConvertMediaToOMXBool(false); // XXX
  v.eCompressionFormat = ConvertMediaToOMXCompression(mime.compression);
  v.eColorFormat = ConvertMediaToOMXColor(format.color, format.bitdepth);
  v.cMIMEType = const_cast<char*>(mime.mime.c_str());
  v.pNativeWindow = 0; // XXX
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, ModuleInterface& module, shared_ptr<MediatypeInterface> media)
{
  OMX_PARAM_PORTDEFINITIONTYPE rollback;
  ConstructPortDefinition(rollback, port, module, media);
  auto video = settings.format.video;

  auto ret = SetFormat(video.eColorFormat, media);

  if(ret != OMX_ErrorNone)
  {
    SetPortDefinition(rollback, port, module, media);
    throw ret;
  }

  ret = SetClock(video.xFramerate, media);

  if(ret != OMX_ErrorNone)
  {
    SetPortDefinition(rollback, port, module, media);
    throw ret;
  }

  ret = SetResolution(video, media);

  if(ret != OMX_ErrorNone)
  {
    SetPortDefinition(rollback, port, module, media);
    throw ret;
  }

  // Set Target is only used for encoder, ignored for decoder
  ret = SetTargetBitrate(video.nBitrate, media);

  if(ret != OMX_ErrorNone && ret != OMX_ErrorUnsupportedIndex)
  {
    SetPortDefinition(rollback, port, module, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoLookAhead(OMX_ALG_VIDEO_PARAM_LOOKAHEAD& la, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(la);
  la.nPortIndex = port.index;
  LookAhead lookAhead;
  auto ret = media->Get(SETTINGS_INDEX_LOOKAHEAD, &lookAhead);
  OMX_CHECK_MEDIA_GET(ret);
  la.nLookAhead = lookAhead.nLookAhead;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetLookAhead(OMX_U32 nLookAhead, shared_ptr<MediatypeInterface> media)
{
  LookAhead lookAhead;
  auto ret = media->Get(SETTINGS_INDEX_LOOKAHEAD, &lookAhead);
  OMX_CHECK_MEDIA_GET(ret);
  lookAhead.nLookAhead = nLookAhead;
  ret = media->Set(SETTINGS_INDEX_LOOKAHEAD, &lookAhead);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoLookAhead(OMX_ALG_VIDEO_PARAM_LOOKAHEAD const& la, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_LOOKAHEAD rollback;
  ConstructVideoLookAhead(rollback, port, media);

  auto ret = SetLookAhead(la.nLookAhead, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoLookAhead(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

// Encoder

OMX_ERRORTYPE ConstructVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE& b, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(b);
  b.nPortIndex = port.index;
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_GET(ret);
  b.eControlRate = ConvertMediaToOMXControlRate(bitrate.mode);
  b.nTargetBitrate = bitrate.target;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetModeBitrate(OMX_U32 target, OMX_VIDEO_CONTROLRATETYPE mode, shared_ptr<MediatypeInterface> media)
{
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_GET(ret);
  bitrate.mode = ConvertOMXToMediaControlRate(mode);
  bitrate.target = target;

  if(bitrate.max < bitrate.target)
    bitrate.max = bitrate.target;

  ret = media->Set(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE const& bitrate, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_VIDEO_PARAM_BITRATETYPE rollback;
  ConstructVideoBitrate(rollback, port, media);

  auto ret = SetModeBitrate(bitrate.nTargetBitrate, bitrate.eControlRate, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoBitrate(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE& q, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(q);
  q.nPortIndex = port.index;
  QPs qps;
  auto ret = media->Get(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &qps);
  OMX_CHECK_MEDIA_GET(ret);
  q.nQpI = ConvertMediaToOMXQpI(qps);
  q.nQpP = ConvertMediaToOMXQpP(qps);
  q.nQpB = ConvertMediaToOMXQpB(qps);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetQuantization(OMX_U32 qpI, OMX_U32 qpP, OMX_U32 qpB, shared_ptr<MediatypeInterface> media)
{
  QPs curQPs;
  auto ret = media->Get(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &curQPs);
  OMX_CHECK_MEDIA_GET(ret);
  curQPs.initial = ConvertOMXToMediaQPInitial(qpI);
  curQPs.deltaIP = ConvertOMXToMediaQPDeltaIP(qpI, qpP);
  curQPs.deltaPB = ConvertOMXToMediaQPDeltaPB(qpP, qpB);
  ret = media->Set(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &curQPs);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE const& quantization, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_VIDEO_PARAM_QUANTIZATIONTYPE rollback;
  ConstructVideoQuantization(rollback, port, media);

  auto ret = SetQuantization(quantization.nQpI, quantization.nQpP, quantization.nQpB, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoQuantization(rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoQuantizationControl(OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL& q, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(q);
  q.nPortIndex = port.index;
  QPs qps;
  auto ret = media->Get(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &qps);
  OMX_CHECK_MEDIA_GET(ret);
  q.eQpControlMode = ConvertMediaToOMXQpControl(qps);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetQuantizationControl(OMX_ALG_EQpCtrlMode const& mode, shared_ptr<MediatypeInterface> media)
{
  QPs curQPs;
  auto ret = media->Get(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &curQPs);
  OMX_CHECK_MEDIA_GET(ret);
  curQPs.mode = ConvertOMXToMediaQPControl(mode);
  ret = media->Set(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &curQPs);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoQuantizationControl(OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL const& quantizationControl, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL rollback;
  ConstructVideoQuantizationControl(rollback, port, media);

  auto ret = SetQuantizationControl(quantizationControl.eQpControlMode, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoQuantizationControl(rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoQuantizationExtension(OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION& q, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(q);
  q.nPortIndex = port.index;
  QPs qps;
  auto ret = media->Get(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &qps);
  OMX_CHECK_MEDIA_GET(ret);
  q.nQpMin = ConvertMediaToOMXQpMin(qps);
  q.nQpMax = ConvertMediaToOMXQpMax(qps);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetQuantizationExtension(OMX_S32 qpMin, OMX_S32 qpMax, shared_ptr<MediatypeInterface> media)
{
  QPs curQPs;
  auto ret = media->Get(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &curQPs);
  OMX_CHECK_MEDIA_GET(ret);
  curQPs.min = ConvertOMXToMediaQPMin(qpMin);
  curQPs.max = ConvertOMXToMediaQPMax(qpMax);
  ret = media->Set(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &curQPs);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoQuantizationExtension(OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION const& quantizationExtension, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION rollback;
  ConstructVideoQuantizationExtension(rollback, port, media);

  auto ret = SetQuantizationExtension(quantizationExtension.nQpMin, quantizationExtension.nQpMax, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoQuantizationExtension(rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoAspectRatio(OMX_ALG_VIDEO_PARAM_ASPECT_RATIO& a, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(a);
  a.nPortIndex = port.index;
  AspectRatioType aspectRatio;
  auto ret = media->Get(SETTINGS_INDEX_ASPECT_RATIO, &aspectRatio);
  OMX_CHECK_MEDIA_GET(ret);
  a.eAspectRatio = ConvertMediaToOMXAspectRatio(aspectRatio);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetAspectRatio(OMX_ALG_EAspectRatio const& aspectRatio, shared_ptr<MediatypeInterface> media)
{
  auto ratio = ConvertOMXToMediaAspectRatio(aspectRatio);
  auto ret = media->Set(SETTINGS_INDEX_ASPECT_RATIO, &ratio);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoAspectRatio(OMX_ALG_VIDEO_PARAM_ASPECT_RATIO const& aspectRatio, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_ASPECT_RATIO rollback;
  ConstructVideoAspectRatio(rollback, port, media);

  auto ret = SetAspectRatio(aspectRatio.eAspectRatio, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoAspectRatio(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoMaxBitrate(OMX_ALG_VIDEO_PARAM_MAX_BITRATE& b, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(b);
  b.nPortIndex = port.index;
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_GET(ret);
  b.nMaxBitrate = bitrate.max;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetMaxBitrate(OMX_U32 max, shared_ptr<MediatypeInterface> media)
{
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_GET(ret);
  bitrate.max = max;

  if(bitrate.target > bitrate.max)
    bitrate.target = bitrate.max;

  ret = media->Set(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoMaxBitrate(OMX_ALG_VIDEO_PARAM_MAX_BITRATE const& maxBitrate, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_MAX_BITRATE rollback;
  ConstructVideoMaxBitrate(rollback, port, media);

  auto ret = SetMaxBitrate(maxBitrate.nMaxBitrate, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoMaxBitrate(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoLowBandwidth(OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH& bw, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(bw);
  bw.nPortIndex = port.index;
  bool isLowBandwidthEnabled;
  auto ret = media->Get(SETTINGS_INDEX_LOW_BANDWIDTH, &isLowBandwidthEnabled);
  OMX_CHECK_MEDIA_GET(ret);
  bw.bEnableLowBandwidth = ConvertMediaToOMXBool(isLowBandwidthEnabled);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetLowBandwidth(OMX_BOOL enableLowBandwidth, shared_ptr<MediatypeInterface> media)
{
  auto enabled = ConvertOMXToMediaBool(enableLowBandwidth);
  auto ret = media->Set(SETTINGS_INDEX_LOW_BANDWIDTH, &enabled);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoLowBandwidth(OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH const& lowBandwidth, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH rollback;
  ConstructVideoLowBandwidth(rollback, port, media);

  auto ret = SetLowBandwidth(lowBandwidth.bEnableLowBandwidth, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoLowBandwidth(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoGopControl(OMX_ALG_VIDEO_PARAM_GOP_CONTROL& gc, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(gc);
  gc.nPortIndex = port.index;
  Gop gop;
  auto ret = media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_GET(ret);
  gc.eGopControlMode = ConvertMediaToOMXGopControl(gop.mode);
  gc.eGdrMode = ConvertMediaToOMXGdr(gop.gdr);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetGopControl(OMX_ALG_EGopCtrlMode const& mode, OMX_ALG_EGdrMode const& gdr, shared_ptr<MediatypeInterface> media)
{
  Gop gop;
  auto ret = media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_GET(ret);
  gop.mode = ConvertOMXToMediaGopControl(mode);
  gop.gdr = ConvertOMXToMediaGdr(gdr);
  ret = media->Set(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoGopControl(OMX_ALG_VIDEO_PARAM_GOP_CONTROL const& gopControl, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_GOP_CONTROL rollback;
  ConstructVideoGopControl(rollback, port, media);

  auto ret = SetGopControl(gopControl.eGopControlMode, gopControl.eGdrMode, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoGopControl(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoSceneChangeResilience(OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE& r, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(r);
  r.nPortIndex = port.index;
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_GET(ret);
  r.bDisableSceneChangeResilience = ConvertMediaToOMXDisableSceneChangeResilience(bitrate.option);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetSceneChangeResilience(OMX_BOOL disableSceneChangeResilience, shared_ptr<MediatypeInterface> media)
{
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_GET(ret);
  bitrate.option = ConvertOMXToMediaDisableSceneChangeResilience(disableSceneChangeResilience);
  ret = media->Set(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoSceneChangeResilience(OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE const& sceneChangeResilience, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE rollback;
  ConstructVideoSceneChangeResilience(rollback, port, media);

  auto ret = SetSceneChangeResilience(sceneChangeResilience.bDisableSceneChangeResilience, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoSceneChangeResilience(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoInstantaneousDecodingRefresh(OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH& idr, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(idr);
  idr.nPortIndex = port.index;
  Gop gop;
  auto ret = media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_GET(ret);
  idr.nInstantaneousDecodingRefreshFrequency = gop.idrFrequency;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetInstantaneousDecodingRefresh(OMX_U32 instantaneousDecodingRefreshFrequency, shared_ptr<MediatypeInterface> media)
{
  Gop gop;
  auto ret = media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_GET(ret);
  gop.idrFrequency = instantaneousDecodingRefreshFrequency;
  ret = media->Set(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoInstantaneousDecodingRefresh(OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH const& instantaneousDecodingRefresh, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH rollback;
  ConstructVideoInstantaneousDecodingRefresh(rollback, port, media);

  auto ret = SetInstantaneousDecodingRefresh(instantaneousDecodingRefresh.nInstantaneousDecodingRefreshFrequency, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoInstantaneousDecodingRefresh(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoPrefetchBuffer(OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER& pb, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(pb);
  pb.nPortIndex = port.index;
  bool isCacheLevel2Enabled;
  auto ret = media->Get(SETTINGS_INDEX_CACHE_LEVEL2, &isCacheLevel2Enabled);
  OMX_CHECK_MEDIA_GET(ret);
  pb.bEnablePrefetchBuffer = ConvertMediaToOMXBool(isCacheLevel2Enabled);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetPrefetchBuffer(OMX_BOOL enablePrefetchBuffer, shared_ptr<MediatypeInterface> media)
{
  auto enabled = ConvertOMXToMediaBool(enablePrefetchBuffer);
  auto ret = media->Set(SETTINGS_INDEX_CACHE_LEVEL2, &enabled);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoPrefetchBuffer(OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER const& prefetchBuffer, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER rollback;
  ConstructVideoPrefetchBuffer(rollback, port, media);

  auto ret = SetPrefetchBuffer(prefetchBuffer.bEnablePrefetchBuffer, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoPrefetchBuffer(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoCodedPictureBuffer(OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER& cpb, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(cpb);
  cpb.nPortIndex = port.index;
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_GET(ret);
  cpb.nCodedPictureBufferSize = bitrate.cpb;
  cpb.nInitialRemovalDelay = bitrate.ird;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetCodedPictureBuffer(OMX_U32 codedPictureBufferSize, OMX_U32 initialRemovalDelay, shared_ptr<MediatypeInterface> media)
{
  Bitrate bitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_GET(ret);
  bitrate.cpb = codedPictureBufferSize;
  bitrate.ird = initialRemovalDelay;
  ret = media->Set(SETTINGS_INDEX_BITRATE, &bitrate);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoCodedPictureBuffer(OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER const& codedPictureBuffer, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER rollback;
  ConstructVideoCodedPictureBuffer(rollback, port, media);

  auto ret = SetCodedPictureBuffer(codedPictureBuffer.nCodedPictureBufferSize, codedPictureBuffer.nInitialRemovalDelay, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoCodedPictureBuffer(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoScalingList(OMX_ALG_VIDEO_PARAM_SCALING_LIST& scl, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(scl);
  scl.nPortIndex = port.index;
  ScalingListType scalingList;
  auto ret = media->Get(SETTINGS_INDEX_SCALING_LIST, &scalingList);
  OMX_CHECK_MEDIA_GET(ret);
  scl.eScalingListMode = ConvertMediaToOMXScalingList(scalingList);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetScalingList(OMX_ALG_EScalingList const& scalingListMode, shared_ptr<MediatypeInterface> media)
{
  auto scalingList = ConvertOMXToMediaScalingList(scalingListMode);
  auto ret = media->Set(SETTINGS_INDEX_SCALING_LIST, &scalingList);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoScalingList(OMX_ALG_VIDEO_PARAM_SCALING_LIST const& scalingList, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_SCALING_LIST rollback;
  ConstructVideoScalingList(rollback, port, media);

  auto ret = SetScalingList(scalingList.eScalingListMode, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoScalingList(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoFillerData(OMX_ALG_VIDEO_PARAM_FILLER_DATA& f, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(f);
  f.nPortIndex = port.index;
  bool isFillerDataEnabled;
  auto ret = media->Get(SETTINGS_INDEX_FILLER_DATA, &isFillerDataEnabled);
  OMX_CHECK_MEDIA_GET(ret);
  f.bDisableFillerData = ConvertMediaToOMXBool(!isFillerDataEnabled);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetFillerData(OMX_BOOL disableFillerData, shared_ptr<MediatypeInterface> media)
{
  auto enableFillerData = !ConvertOMXToMediaBool(disableFillerData);
  auto ret = media->Set(SETTINGS_INDEX_FILLER_DATA, &enableFillerData);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoFillerData(OMX_ALG_VIDEO_PARAM_FILLER_DATA const& fillerData, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_FILLER_DATA rollback;
  ConstructVideoFillerData(rollback, port, media);

  auto ret = SetFillerData(fillerData.bDisableFillerData, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoFillerData(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoSlices(OMX_ALG_VIDEO_PARAM_SLICES& s, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(s);
  s.nPortIndex = port.index;
  Slices slices;
  auto ret = media->Get(SETTINGS_INDEX_SLICE_PARAMETER, &slices);
  OMX_CHECK_MEDIA_GET(ret);
  s.nNumSlices = slices.num;
  s.nSlicesSize = slices.size;
  s.bDependentSlices = ConvertMediaToOMXBool(slices.dependent);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetSlices(OMX_U32 numSlices, OMX_U32 slicesSize, OMX_BOOL dependentSlices, shared_ptr<MediatypeInterface> media)
{
  Slices slices;
  auto ret = media->Get(SETTINGS_INDEX_SLICE_PARAMETER, &slices);
  OMX_CHECK_MEDIA_GET(ret);
  slices.num = numSlices;
  slices.size = slicesSize;
  slices.dependent = ConvertOMXToMediaBool(dependentSlices);
  ret = media->Set(SETTINGS_INDEX_SLICE_PARAMETER, &slices);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoSlices(OMX_ALG_VIDEO_PARAM_SLICES const& slices, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_SLICES rollback;
  ConstructVideoSlices(rollback, port, media);

  auto ret = SetSlices(slices.nNumSlices, slices.nSlicesSize, slices.bDependentSlices, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoSlices(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoModesSupported(OMX_INTERLACEFORMATTYPE& interlace, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(interlace);
  interlace.nPortIndex = port.index;
  vector<VideoModeType> modes;
  auto ret = media->Get(SETTINGS_INDEX_VIDEO_MODES_SUPPORTED, &modes);
  OMX_CHECK_MEDIA_GET(ret);

  for(auto mode : modes)
    interlace.nFormat |= ConvertMediaToOMXInterlaceFlag(mode);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoModeCurrent(OMX_INTERLACEFORMATTYPE& interlace, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(interlace);
  interlace.nPortIndex = port.index;
  VideoModeType videoMode;
  auto ret = media->Get(SETTINGS_INDEX_VIDEO_MODE, &videoMode);
  OMX_CHECK_MEDIA_GET(ret);
  interlace.nFormat = ConvertMediaToOMXInterlaceFlag(videoMode);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetInterlaceMode(OMX_U32 flag, shared_ptr<MediatypeInterface> media)
{
  auto videoMode = ConvertOMXToMediaVideoMode(flag);
  auto ret = media->Set(SETTINGS_INDEX_VIDEO_MODE, &videoMode);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoModeCurrent(OMX_INTERLACEFORMATTYPE const& interlace, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_INTERLACEFORMATTYPE rollback;
  ConstructVideoModeCurrent(rollback, port, media);

  auto ret = SetInterlaceMode(interlace.nFormat, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoModeCurrent(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoLongTerm(OMX_ALG_VIDEO_PARAM_LONG_TERM& longTerm, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(longTerm);
  longTerm.nPortIndex = port.index;
  Gop gop;
  auto ret = media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_GET(ret);
  longTerm.bEnableLongTerm = ConvertMediaToOMXBool(gop.isLongTermEnabled);
  longTerm.nLongTermFrequency = gop.ltFrequency;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetLongTerm(OMX_BOOL isLongTermEnabled, OMX_S32 ltFrequency, shared_ptr<MediatypeInterface> media)
{
  Gop gop;
  auto ret = media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_GET(ret);
  gop.isLongTermEnabled = ConvertOMXToMediaBool(isLongTermEnabled);
  gop.ltFrequency = ltFrequency;
  ret = media->Set(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoLongTerm(OMX_ALG_VIDEO_PARAM_LONG_TERM const& longTerm, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_LONG_TERM rollback;
  ConstructVideoLongTerm(rollback, port, media);

  auto ret = SetLongTerm(longTerm.bEnableLongTerm, longTerm.nLongTermFrequency, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoLongTerm(rollback, port, media);
    throw ret;
  }
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetTargetBitrate(OMX_U32 bitrate, shared_ptr<MediatypeInterface> media)
{
  Bitrate curBitrate;
  auto ret = media->Get(SETTINGS_INDEX_BITRATE, &curBitrate);

  if(ret == MediatypeInterface::ERROR_SETTINGS_BAD_INDEX)
    return OMX_ErrorUnsupportedIndex;
  assert(ret == MediatypeInterface::ERROR_SETTINGS_NONE);
  curBitrate.target = bitrate;

  if(curBitrate.max < curBitrate.target)
    curBitrate.max = curBitrate.target;
  ret = media->Set(SETTINGS_INDEX_BITRATE, &curBitrate);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

// Decoder

OMX_ERRORTYPE ConstructPreallocation(OMX_ALG_PARAM_PREALLOCATION& prealloc, bool isEnabled)
{
  OMXChecker::SetHeaderVersion(prealloc);
  prealloc.bDisablePreallocation = ConvertMediaToOMXBool(!isEnabled);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructCommonSequencePictureModesSupported(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE* mode, shared_ptr<MediatypeInterface> media)
{
  vector<SequencePictureModeType> modes;
  auto ret = media->Get(SETTINGS_INDEX_SEQUENCE_PICTURE_MODES_SUPPORTED, &modes);
  OMX_CHECK_MEDIA_GET(ret);

  if(mode->nModeIndex >= modes.size())
    return OMX_ErrorNoMore;

  mode->eMode = ConvertMediaToOMXSequencePictureMode(modes[mode->nModeIndex]);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoDecodedPictureBuffer(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER& dpb, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(dpb);
  dpb.nPortIndex = port.index;
  DecodedPictureBufferType pictureBuffer;
  auto ret = media->Get(SETTINGS_INDEX_DECODED_PICTURE_BUFFER, &pictureBuffer);
  OMX_CHECK_MEDIA_GET(ret);
  dpb.eDecodedPictureBufferMode = ConvertMediaToOMXDecodedPictureBuffer(pictureBuffer);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetDecodedPictureBuffer(OMX_ALG_EDpbMode mode, shared_ptr<MediatypeInterface> media)
{
  DecodedPictureBufferType decodedPictureBuffer = ConvertOMXToMediaDecodedPictureBuffer(mode);
  auto ret = media->Set(SETTINGS_INDEX_DECODED_PICTURE_BUFFER, &decodedPictureBuffer);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoDecodedPictureBuffer(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER const& dpb, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER rollback;
  ConstructVideoDecodedPictureBuffer(rollback, port, media);

  auto ret = SetDecodedPictureBuffer(dpb.eDecodedPictureBufferMode, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoDecodedPictureBuffer(rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructVideoInternalEntropyBuffers(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS& ieb, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(ieb);
  int stack;
  auto ret = media->Get(SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER, &stack);
  OMX_CHECK_MEDIA_GET(ret);
  ieb.nPortIndex = port.index;
  ieb.nNumInternalEntropyBuffers = stack;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetInternalEntropyBuffers(OMX_U32 num, shared_ptr<MediatypeInterface> media)
{
  auto ret = media->Set(SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER, &num);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetVideoInternalEntropyBuffers(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS const& ieb, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS rollback;
  ConstructVideoInternalEntropyBuffers(rollback, port, media);

  auto ret = SetInternalEntropyBuffers(ieb.nNumInternalEntropyBuffers, media);

  if(ret != OMX_ErrorNone)
  {
    SetVideoInternalEntropyBuffers(rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ConstructCommonSequencePictureMode(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE& mode, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMXChecker::SetHeaderVersion(mode);
  mode.nPortIndex = port.index;
  auto ret = media->Get(SETTINGS_INDEX_SEQUENCE_PICTURE_MODE, &mode.eMode);
  OMX_CHECK_MEDIA_GET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetSequencePictureMode(OMX_ALG_SEQUENCE_PICTURE_MODE mode, shared_ptr<MediatypeInterface> media)
{
  auto ret = media->Set(SETTINGS_INDEX_SEQUENCE_PICTURE_MODE, &mode);
  OMX_CHECK_MEDIA_SET(ret);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE SetCommonSequencePictureMode(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE mode, Port const& port, shared_ptr<MediatypeInterface> media)
{
  OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE rollback;
  ConstructCommonSequencePictureMode(rollback, port, media);

  auto ret = SetSequencePictureMode(mode.eMode, media);

  if(ret != OMX_ErrorNone)
  {
    SetCommonSequencePictureMode(rollback, port, media);
    throw ret;
  }

  return OMX_ErrorNone;
}

