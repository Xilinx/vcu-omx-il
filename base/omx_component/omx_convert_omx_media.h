/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

#include <OMX_VideoExt.h>
#include <OMX_ComponentAlg.h>
#include <OMX_IVCommonAlg.h>

#include "base/omx_module/omx_module_enums.h"
#include "base/omx_module/omx_module_structs.h"

ColorType ConvertOMXToMediaColor(OMX_COLOR_FORMATTYPE format);
OMX_COLOR_FORMATTYPE ConvertMediaToOMXColor(ColorType color, int bitdepth);

CompressionType ConvertOMXToMediaCompression(OMX_VIDEO_CODINGTYPE coding);
OMX_VIDEO_CODINGTYPE ConvertMediaToOMXCompression(CompressionType compression);

OMX_U32 ConvertMediaToOMXFramerate(Clock clock);

int ConvertOMXToMediaBitdepth(OMX_COLOR_FORMATTYPE format);
Clock ConvertOMXToMediaClock(OMX_U32 framerateInQ16);

bool ConvertOMXToMediaBool(OMX_BOOL boolean);
OMX_BOOL ConvertMediaToOMXBool(bool boolean);

BufferHandleType ConvertOMXToMediaBufferHandle(OMX_ALG_BUFFER_MODE bufferMode);
OMX_ALG_BUFFER_MODE ConvertMediaToOMXBufferHandle(BufferHandleType handle);

DecodedPictureBufferType ConvertOMXToMediaDecodedPictureBuffer(OMX_ALG_EDpbMode mode);
OMX_ALG_EDpbMode ConvertMediaToOMXDecodedPictureBuffer(DecodedPictureBufferType mode);

ProfileLevel ConvertOMXToMediaAVCProfileLevel(OMX_VIDEO_AVCPROFILETYPE profile, OMX_VIDEO_AVCLEVELTYPE level);

OMX_VIDEO_AVCPROFILETYPE ConvertMediaToOMXAVCProfile(ProfileLevel profileLevel);
OMX_VIDEO_AVCLEVELTYPE ConvertMediaToOMXAVCLevel(ProfileLevel profileLevel);

ProfileLevel ConvertOMXToMediaHEVCProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE profile, OMX_ALG_VIDEO_HEVCLEVELTYPE level);

OMX_ALG_VIDEO_HEVCPROFILETYPE ConvertMediaToOMXHEVCProfile(ProfileLevel profileLevel);
OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertMediaToOMXHEVCLevel(ProfileLevel profileLevel);

LoopFilterType ConvertOMXToMediaAVCLoopFilter(OMX_VIDEO_AVCLOOPFILTERTYPE loopFilter);
OMX_VIDEO_AVCLOOPFILTERTYPE ConvertMediaToOMXAVCLoopFilter(LoopFilterType loopFilter);

LoopFilterType ConvertOMXToMediaHEVCLoopFilter(OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE loopFilter);
OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE ConvertMediaToOMXHEVCLoopFilter(LoopFilterType loopFilter);

int ConvertOMXToMediaBFrames(OMX_U32 bFrames, OMX_U32 pFrames);
int ConvertOMXToMediaGopLength(OMX_U32 bFrames, OMX_U32 pFrames);
OMX_U32 ConvertMediaToOMXBFrames(Gop gop);
OMX_U32 ConvertMediaToOMXPFrames(Gop gop);

EntropyCodingType ConvertOMXToMediaEntropyCoding(OMX_BOOL isCabac);
OMX_BOOL ConvertMediaToOMXEntropyCoding(EntropyCodingType mode);

int ConvertOMXToMediaQPInitial(OMX_U32 qpI);
int ConvertOMXToMediaQPDeltaIP(OMX_U32 qpI, OMX_U32 qpP);
int ConvertOMXToMediaQPDeltaPB(OMX_U32 pP, OMX_U32 qpB);
OMX_U32 ConvertMediaToOMXQpI(QPs qps);
OMX_U32 ConvertMediaToOMXQpP(QPs qps);
OMX_U32 ConvertMediaToOMXQpB(QPs qps);

int ConvertOMXToMediaQPMin(OMX_S32 qpMin);
OMX_S32 ConvertMediaToOMXQpMin(QPs qps);

int ConvertOMXToMediaQPMax(OMX_S32 qpMax);
OMX_S32 ConvertMediaToOMXQpMax(QPs qps);

QPControlType ConvertOMXToMediaQPControl(OMX_ALG_EQpCtrlMode mode);
OMX_ALG_EQpCtrlMode ConvertMediaToOMXQpControl(QPs qps);

RateControlType ConvertOMXToMediaControlRate(OMX_VIDEO_CONTROLRATETYPE mode);
OMX_VIDEO_CONTROLRATETYPE ConvertMediaToOMXControlRate(RateControlType mode);

AspectRatioType ConvertOMXToMediaAspectRatio(OMX_ALG_EAspectRatio aspectRatio);
OMX_ALG_EAspectRatio ConvertMediaToOMXAspectRatio(AspectRatioType aspectRatio);

GopControlType ConvertOMXToMediaGopControl(OMX_ALG_EGopCtrlMode mode);
OMX_ALG_EGopCtrlMode ConvertMediaToOMXGopControl(GopControlType mode);

GdrType ConvertOMXToMediaGdr(OMX_ALG_EGdrMode gdr);
OMX_ALG_EGdrMode ConvertMediaToOMXGdr(GdrType gdr);

ScalingListType ConvertOMXToMediaScalingList(OMX_ALG_EScalingList scalingListMode);
OMX_ALG_EScalingList ConvertMediaToOMXScalingList(ScalingListType scalingLisgt);

BufferModeType ConvertOMXToMediaBufferMode(OMX_ALG_VIDEO_BUFFER_MODE mode);
OMX_ALG_VIDEO_BUFFER_MODE ConvertMediaToOMXBufferMode(BufferModeType mode);

QualityType ConvertOMXToMediaQuality(OMX_ALG_ERoiQuality quality);

OMX_U32 ConvertMediaToOMXInterlaceFlag(VideoModeType mode);
VideoModeType ConvertOMXToMediaVideoMode(OMX_U32 flag);

SequencePictureModeType ConvertOMXToMediaSequencePictureMode(OMX_ALG_SEQUENCE_PICTURE_MODE mode);
OMX_ALG_SEQUENCE_PICTURE_MODE ConvertMediaToOMXSequencePictureMode(SequencePictureModeType mode);

OMX_ALG_VIDEO_COLOR_PRIMARIESTYPE ConvertMediaToOMXColorPrimaries(ColorPrimariesType colorPrimaries);
ColorPrimariesType ConvertOMXToMediaColorPrimaries(OMX_ALG_VIDEO_COLOR_PRIMARIESTYPE colorPrimaries);

