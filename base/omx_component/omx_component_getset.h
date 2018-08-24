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

#pragma once

/*
 * Component Get/Set :
 * Functions used for getParameter/SetParameter
 */

#include "omx_component.h"
// Common

OMX_ERRORTYPE ConstructPortSupplier(OMX_PARAM_BUFFERSUPPLIERTYPE& s, Port const& port);

OMX_ERRORTYPE ConstructReportedLatency(OMX_ALG_PARAM_REPORTED_LATENCY& lat, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetPortExpectedBuffer(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, ModuleInterface const& module);

OMX_ERRORTYPE ConstructVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME& subframe, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetSubframe(OMX_BOOL enableSubframe, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME const& subframe, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE& mode, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetInputBufferMode(OMX_ALG_BUFFER_MODE mode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetOutputBufferMode(OMX_ALG_BUFFER_MODE mode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE const& portBufferMode, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE GetVideoPortFormatSupported(OMX_VIDEO_PARAM_PORTFORMATTYPE& format, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE ConstructVideoPortCurrentFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE& f, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetFormat(OMX_COLOR_FORMATTYPE const& color, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE const& format, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetResolution(OMX_VIDEO_PORTDEFINITIONTYPE const& definition, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetClock(OMX_U32 framerateInQ16, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE& def, Port& port, ModuleInterface const& module, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, ModuleInterface& module, std::shared_ptr<MediatypeInterface> media);

// Encoder

OMX_ERRORTYPE ConstructVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE& b, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetModeBitrate(OMX_U32 target, OMX_VIDEO_CONTROLRATETYPE mode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE const& bitrate, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE& q, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetQuantization(OMX_U32 qpI, OMX_U32 qpP, OMX_U32 qpB, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE const& quantization, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoQuantizationControl(OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL& q, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetQuantizationControl(OMX_ALG_EQpCtrlMode const& mode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoQuantizationControl(OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL const& quantizationControl, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoQuantizationExtension(OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION& q, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetQuantizationExtension(OMX_S32 qpMin, OMX_S32 qpMax, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoQuantizationExtension(OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION const& quantizationExtension, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoAspectRatio(OMX_ALG_VIDEO_PARAM_ASPECT_RATIO& a, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetAspectRatio(OMX_ALG_EAspectRatio const& aspectRatio, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoAspectRatio(OMX_ALG_VIDEO_PARAM_ASPECT_RATIO const& aspectRatio, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoMaxBitrate(OMX_ALG_VIDEO_PARAM_MAX_BITRATE& b, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetMaxBitrate(OMX_U32 max, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoMaxBitrate(OMX_ALG_VIDEO_PARAM_MAX_BITRATE const& maxBitrate, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoLowBandwidth(OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH& bw, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetLowBandwidth(OMX_BOOL enableLowBandwidth, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoLowBandwidth(OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH const& lowBandwidth, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoGopControl(OMX_ALG_VIDEO_PARAM_GOP_CONTROL& gc, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetGopControl(OMX_ALG_EGopCtrlMode const& mode, OMX_ALG_EGdrMode const& gdr, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoGopControl(OMX_ALG_VIDEO_PARAM_GOP_CONTROL const& gopControl, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoSceneChangeResilience(OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE& r, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetSceneChangeResilience(OMX_BOOL disableSceneChangeResilience, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoSceneChangeResilience(OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE const& sceneChangeResilience, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoInstantaneousDecodingRefresh(OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH& idr, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetInstantaneousDecodingRefresh(OMX_U32 instantaneousDecodingRefreshFrequency, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoInstantaneousDecodingRefresh(OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH const& instantaneousDecodingRefresh, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoPrefetchBuffer(OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER& pb, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetPrefetchBuffer(OMX_BOOL enablePrefetchBuffer, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoPrefetchBuffer(OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER const& prefetchBuffer, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoCodedPictureBuffer(OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER& cpb, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetCodedPictureBuffer(OMX_U32 codedPictureBufferSize, OMX_U32 initialRemovalDelay, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoCodedPictureBuffer(OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER const& codedPictureBuffer, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoScalingList(OMX_ALG_VIDEO_PARAM_SCALING_LIST& scl, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetScalingList(OMX_ALG_EScalingList const& scalingListMode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoScalingList(OMX_ALG_VIDEO_PARAM_SCALING_LIST const& scalingList, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoFillerData(OMX_ALG_VIDEO_PARAM_FILLER_DATA& f, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetFillerData(OMX_BOOL disableFillerData, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoFillerData(OMX_ALG_VIDEO_PARAM_FILLER_DATA const& fillerData, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoSlices(OMX_ALG_VIDEO_PARAM_SLICES& s, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetSlices(OMX_U32 numSlices, OMX_U32 slicesSize, OMX_BOOL dependentSlices, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoSlices(OMX_ALG_VIDEO_PARAM_SLICES const& slices, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoModesSupported(OMX_INTERLACEFORMATTYPE& interlace, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoModeCurrent(OMX_INTERLACEFORMATTYPE& interlace, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetInterlaceMode(OMX_U32 flag, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoModeCurrent(OMX_INTERLACEFORMATTYPE const& interlace, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoLongTerm(OMX_ALG_VIDEO_PARAM_LONG_TERM& longTerm, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetLongTerm(OMX_BOOL isLongTermEnabled, OMX_S32 ltFrequency, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoLongTerm(OMX_ALG_VIDEO_PARAM_LONG_TERM const& longTerm, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetTargetBitrate(OMX_U32 bitrate, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoLookAhead(OMX_ALG_VIDEO_PARAM_LOOKAHEAD& la, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetLookAhead(OMX_U32 nLookAhead, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoLookAhead(OMX_ALG_VIDEO_PARAM_LOOKAHEAD const& la, Port const& port, std::shared_ptr<MediatypeInterface> media);

// Decoder

OMX_ERRORTYPE ConstructPreallocation(OMX_ALG_PARAM_PREALLOCATION& prealloc, bool isEnabled);

OMX_ERRORTYPE ConstructCommonSequencePictureModesSupported(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE* mode, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoDecodedPictureBuffer(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER& dpb, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetDecodedPictureBuffer(OMX_ALG_EDpbMode mode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoDecodedPictureBuffer(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER const& dpb, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoInternalEntropyBuffers(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS& ieb, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE OMX_ERRORTYPESetInternalEntropyBuffers(OMX_U32 num, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoInternalEntropyBuffers(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS const& ieb, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructCommonSequencePictureMode(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE& mode, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetSequencePictureMode(OMX_ALG_SEQUENCE_PICTURE_MODE mode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetCommonSequencePictureMode(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE mode, Port const& port, std::shared_ptr<MediatypeInterface> media);

