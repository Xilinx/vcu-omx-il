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

#pragma once

/*
 * Component Get/Set :
 * Functions used for getParameter/SetParameter
 */

#include "omx_component.h"
// Common

OMX_ERRORTYPE ConstructPortSupplier(OMX_PARAM_BUFFERSUPPLIERTYPE& s, Port const& port);

OMX_ERRORTYPE ConstructReportedLatency(OMX_ALG_PARAM_REPORTED_LATENCY& lat, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetPortExpectedBuffer(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME& subframe, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoSubframe(OMX_ALG_VIDEO_PARAM_SUBFRAME const& subframe, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE& mode, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetInputBufferMode(OMX_ALG_BUFFER_MODE mode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetOutputBufferMode(OMX_ALG_BUFFER_MODE mode, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetPortBufferMode(OMX_ALG_PORT_PARAM_BUFFER_MODE const& portBufferMode, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE GetVideoPortFormatSupported(OMX_VIDEO_PARAM_PORTFORMATTYPE& format, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE ConstructVideoPortCurrentFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE& f, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE const& format, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetClock(OMX_U32 framerateInQ16, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE& def, Port& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE const& settings, Port& port, ModuleInterface& module, std::shared_ptr<MediatypeInterface> media);

// Encoder

OMX_ERRORTYPE ConstructVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE& b, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE const& bitrate, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE& q, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE const& quantization, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoQuantizationControl(OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL& q, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoQuantizationControl(OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL const& quantizationControl, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoQuantizationExtension(OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION& q, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoQuantizationExtension(OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION const& quantizationExtension, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoAspectRatio(OMX_ALG_VIDEO_PARAM_ASPECT_RATIO& a, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoAspectRatio(OMX_ALG_VIDEO_PARAM_ASPECT_RATIO const& aspectRatio, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoMaxBitrate(OMX_ALG_VIDEO_PARAM_MAX_BITRATE& b, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoMaxBitrate(OMX_ALG_VIDEO_PARAM_MAX_BITRATE const& maxBitrate, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoLowBandwidth(OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH& bw, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoLowBandwidth(OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH const& lowBandwidth, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoGopControl(OMX_ALG_VIDEO_PARAM_GOP_CONTROL& gc, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoGopControl(OMX_ALG_VIDEO_PARAM_GOP_CONTROL const& gopControl, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoSceneChangeResilience(OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE& r, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoSceneChangeResilience(OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE const& sceneChangeResilience, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoSkipFrame(OMX_ALG_VIDEO_PARAM_SKIP_FRAME& skip, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoSkipFrame(OMX_ALG_VIDEO_PARAM_SKIP_FRAME const& skipFrame, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoInstantaneousDecodingRefresh(OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH& idr, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoInstantaneousDecodingRefresh(OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH const& instantaneousDecodingRefresh, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoPrefetchBuffer(OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER& pb, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoPrefetchBuffer(OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER const& prefetchBuffer, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoCodedPictureBuffer(OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER& cpb, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoCodedPictureBuffer(OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER const& codedPictureBuffer, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoScalingList(OMX_ALG_VIDEO_PARAM_SCALING_LIST& scl, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoScalingList(OMX_ALG_VIDEO_PARAM_SCALING_LIST const& scalingList, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoFillerData(OMX_ALG_VIDEO_PARAM_FILLER_DATA& f, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoFillerData(OMX_ALG_VIDEO_PARAM_FILLER_DATA const& fillerData, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoSlices(OMX_ALG_VIDEO_PARAM_SLICES& s, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoSlices(OMX_ALG_VIDEO_PARAM_SLICES const& slices, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoModesSupported(OMX_INTERLACEFORMATTYPE& interlace, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoModeCurrent(OMX_INTERLACEFORMATTYPE& interlace, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoModeCurrent(OMX_INTERLACEFORMATTYPE const& interlace, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoLongTerm(OMX_ALG_VIDEO_PARAM_LONG_TERM& longTerm, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoLongTerm(OMX_ALG_VIDEO_PARAM_LONG_TERM const& longTerm, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoLookAhead(OMX_ALG_VIDEO_PARAM_LOOKAHEAD& la, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoLookAhead(OMX_ALG_VIDEO_PARAM_LOOKAHEAD const& la, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoTwoPass(OMX_ALG_VIDEO_PARAM_TWOPASS& tp, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoTwoPass(OMX_ALG_VIDEO_PARAM_TWOPASS const& tp, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoColorPrimaries(OMX_ALG_VIDEO_PARAM_COLOR_PRIMARIES& colorPrimaries, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoColorPrimaries(OMX_ALG_VIDEO_PARAM_COLOR_PRIMARIES const& colorPrimaries, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoTransferCharacteristics(OMX_ALG_VIDEO_PARAM_TRANSFER_CHARACTERISTICS& transferCharac, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoTransferCharacteristics(OMX_ALG_VIDEO_PARAM_TRANSFER_CHARACTERISTICS const& transferCharac, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoColorMatrix(OMX_ALG_VIDEO_PARAM_COLOR_MATRIX& colorMatrix, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoColorMatrix(OMX_ALG_VIDEO_PARAM_COLOR_MATRIX const& colorMatrix, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoMaxPictureSize(OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE& maxPictureSizes, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoMaxPictureSize(OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE const& maxPictureSizes, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoMaxPictureSizeInBits(OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE_IN_BITS& maxPictureSizes, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoMaxPictureSizeInBits(OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE_IN_BITS const& maxPictureSizes, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoMaxPictureSizes(OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES& maxPictureSizes, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoMaxPictureSizes(OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES const& maxPictureSizes, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoMaxPictureSizesInBits(OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES_IN_BITS& maxPictureSizes, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoMaxPictureSizesInBits(OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES_IN_BITS const& maxPictureSizes, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoLoopFilterBeta(OMX_ALG_VIDEO_PARAM_LOOP_FILTER_BETA& loopFilterBeta, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoLoopFilterBeta(OMX_ALG_VIDEO_PARAM_LOOP_FILTER_BETA const& loopFilterBeta, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoLoopFilterTc(OMX_ALG_VIDEO_PARAM_LOOP_FILTER_TC& loopFilterTc, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoLoopFilterTc(OMX_ALG_VIDEO_PARAM_LOOP_FILTER_TC const& loopFilterTc, Port const& port, std::shared_ptr<MediatypeInterface> media);

// Decoder

OMX_ERRORTYPE ConstructPreallocation(OMX_ALG_PARAM_PREALLOCATION& prealloc, bool isPreallocationEnabled);

OMX_ERRORTYPE ConstructCommonSequencePictureModesSupported(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE* mode, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoDecodedPictureBuffer(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER& dpb, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoDecodedPictureBuffer(OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER const& dpb, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoInternalEntropyBuffers(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS& ieb, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoInternalEntropyBuffers(OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS const& ieb, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructCommonSequencePictureMode(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE& mode, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetCommonSequencePictureMode(OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE mode, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoInputParsed(OMX_ALG_VIDEO_PARAM_INPUT_PARSED& ip, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoInputParsed(OMX_ALG_VIDEO_PARAM_INPUT_PARSED const& ip, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoQuantizationTable(OMX_ALG_VIDEO_PARAM_QUANTIZATION_TABLE& table, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoQuantizationTable(OMX_ALG_VIDEO_PARAM_QUANTIZATION_TABLE const& table, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructPortEarlyCallback(OMX_ALG_PORT_PARAM_EARLY_CALLBACK& earlyCB, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetPortEarlyCallback(OMX_ALG_PORT_PARAM_EARLY_CALLBACK const& earlyCB, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoAccessUnitDelimiter(OMX_ALG_VIDEO_PARAM_ACCESS_UNIT_DELIMITER& aud, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoAccessUnitDelimiter(OMX_ALG_VIDEO_PARAM_ACCESS_UNIT_DELIMITER const& aud, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoBufferingPeriodSEI(OMX_ALG_VIDEO_PARAM_BUFFERING_PERIOD_SEI& bpSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoBufferingPeriodSEI(OMX_ALG_VIDEO_PARAM_BUFFERING_PERIOD_SEI const& bpSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoPictureTimingSEI(OMX_ALG_VIDEO_PARAM_PICTURE_TIMING_SEI& ptSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoPictureTimingSEI(OMX_ALG_VIDEO_PARAM_PICTURE_TIMING_SEI const& ptSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoRecoveryPointSEI(OMX_ALG_VIDEO_PARAM_RECOVERY_POINT_SEI& rpSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoRecoveryPointSEI(OMX_ALG_VIDEO_PARAM_RECOVERY_POINT_SEI const& rpSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoMasteringDisplayColourVolumeSEI(OMX_ALG_VIDEO_PARAM_MASTERING_DISPLAY_COLOUR_VOLUME_SEI& mdcvSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoMasteringDisplayColourVolumeSEI(OMX_ALG_VIDEO_PARAM_MASTERING_DISPLAY_COLOUR_VOLUME_SEI const& mdcvSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoContentLightLevelSEI(OMX_ALG_VIDEO_PARAM_CONTENT_LIGHT_LEVEL_SEI& cllSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoContentLightLevelSEI(OMX_ALG_VIDEO_PARAM_CONTENT_LIGHT_LEVEL_SEI const& cllSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoAlternativeTransferCharacteristicsSEI(OMX_ALG_VIDEO_PARAM_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI& atcSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoAlternativeTransferCharacteristicsSEI(OMX_ALG_VIDEO_PARAM_ALTERNATIVE_TRANSFER_CHARACTERISTICS_SEI const& atcSEI, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoST209410SEI(OMX_ALG_VIDEO_PARAM_ST2094_10_SEI& st2094_10SEI, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoST209410SEI(OMX_ALG_VIDEO_PARAM_ST2094_10_SEI const& st2094_10SEI, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE ConstructVideoST209440SEI(OMX_ALG_VIDEO_PARAM_ST2094_40_SEI& st2094_40SEI, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE SetVideoST209440SEI(OMX_ALG_VIDEO_PARAM_ST2094_40_SEI const& st2094_40SEI, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetVideoRateControlPlugin(OMX_ALG_VIDEO_PARAM_RATE_CONTROL_PLUGIN const& rateCtrlPlugin, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE ConstructVideoRateControlPlugin(OMX_ALG_VIDEO_PARAM_RATE_CONTROL_PLUGIN& rateCtrlPlugin, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetVideoCrop(OMX_CONFIG_RECTTYPE const& crop, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE ConstructVideoCrop(OMX_CONFIG_RECTTYPE& crop, Port const& port, std::shared_ptr<MediatypeInterface> media);

OMX_ERRORTYPE SetVideoUniformSliceType(OMX_ALG_VIDEO_PARAM_UNIFORM_SLICE_TYPE const& ust, Port const& port, std::shared_ptr<MediatypeInterface> media);
OMX_ERRORTYPE ConstructVideoUniformSliceType(OMX_ALG_VIDEO_PARAM_UNIFORM_SLICE_TYPE& ust, Port const& port, std::shared_ptr<MediatypeInterface> media);
