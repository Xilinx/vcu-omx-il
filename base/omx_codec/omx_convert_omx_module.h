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

#pragma once

#include <OMX_VideoExt.h>
#include <OMX_ComponentAlg.h>
#include <OMX_IVCommonAlg.h>

#include "base/omx_module/omx_module_enums.h"
#include "base/omx_module/omx_module_structs.h"

ColorType ConvertOMXToModuleColor(OMX_COLOR_FORMATTYPE format);
OMX_COLOR_FORMATTYPE ConvertModuleToOMXColor(ColorType color, int bitdepth);

CompressionType ConvertOMXToModuleCompression(OMX_VIDEO_CODINGTYPE coding);
OMX_VIDEO_CODINGTYPE ConvertModuleToOMXCompression(CompressionType compression);

OMX_U32 ConvertModuleToOMXFramerate(Clock clock);

int ConvertOMXToModuleBitdepth(OMX_COLOR_FORMATTYPE format);
Clock ConvertOMXToModuleClock(OMX_U32 framerateInQ16);

bool ConvertOMXToModuleBool(OMX_BOOL boolean);
OMX_BOOL ConvertModuleToOMXBool(bool boolean);

bool ConvertOMXToModuleFileDescriptor(OMX_ALG_BUFFER_MODE bufferMode);
OMX_ALG_BUFFER_MODE ConvertModuleToOMXBufferMode(bool useFd);

DecodedPictureBufferType ConvertOMXToModuleDecodedPictureBuffer(OMX_ALG_EDpbMode mode);
OMX_ALG_EDpbMode ConvertModuleToOMXDecodedPictureBuffer(DecodedPictureBufferType mode);

ProfileLevelType ConvertOMXToModuleAVCProfileLevel(OMX_VIDEO_AVCPROFILETYPE profile, OMX_VIDEO_AVCLEVELTYPE level);

OMX_VIDEO_AVCPROFILETYPE ConvertModuleToOMXAVCProfile(ProfileLevelType profileLevel);
OMX_VIDEO_AVCLEVELTYPE ConvertModuleToOMXAVCLevel(ProfileLevelType profileLevel);

ProfileLevelType ConvertOMXToModuleHEVCProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE profile, OMX_ALG_VIDEO_HEVCLEVELTYPE level);

OMX_ALG_VIDEO_HEVCPROFILETYPE ConvertModuleToOMXHEVCProfile(ProfileLevelType profileLevel);
OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertModuleToOMXHEVCLevel(ProfileLevelType profileLevel);

LoopFilterType ConvertOMXToModuleAVCLoopFilter(OMX_VIDEO_AVCLOOPFILTERTYPE loopFilter);
OMX_VIDEO_AVCLOOPFILTERTYPE ConvertModuleToOMXAVCLoopFilter(LoopFilterType loopFilter);

LoopFilterType ConvertOMXToModuleHEVCLoopFilter(OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE loopFilter);
OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE ConvertModuleToOMXHEVCLoopFilter(LoopFilterType loopFilter);

int ConvertOMXToModuleBFrames(OMX_U32 bFrames, OMX_U32 pFrames);
int ConvertOMXToModuleGopLength(OMX_U32 bFrames, OMX_U32 pFrames);
OMX_U32 ConvertModuleToOMXBFrames(Gop gop);
OMX_U32 ConvertModuleToOMXPFrames(Gop gop);

EntropyCodingType ConvertOMXToModuleEntropyCoding(OMX_BOOL isCabac);
OMX_BOOL ConvertModuleToOMXEntropyCoding(EntropyCodingType mode);

int ConvertOMXToModuleQPInitial(OMX_U32 qpI);
int ConvertOMXToModuleQPDeltaIP(OMX_U32 qpI, OMX_U32 qpP);
int ConvertOMXToModuleQPDeltaPB(OMX_U32 pP, OMX_U32 qpB);
OMX_U32 ConvertModuleToOMXQpI(QPs qps);
OMX_U32 ConvertModuleToOMXQpP(QPs qps);
OMX_U32 ConvertModuleToOMXQpB(QPs qps);

int ConvertOMXToModuleQPMin(OMX_S32 qpMin);
OMX_S32 ConvertModuleToOMXQpMin(QPs qps);

int ConvertOMXToModuleQPMax(OMX_S32 qpMax);
OMX_S32 ConvertModuleToOMXQpMax(QPs qps);

QPControlType ConvertOMXToModuleQPControl(OMX_ALG_EQpCtrlMode mode);
OMX_ALG_EQpCtrlMode ConvertModuleToOMXQpControl(QPs qps);

RateControlType ConvertOMXToModuleControlRate(OMX_VIDEO_CONTROLRATETYPE mode);
OMX_VIDEO_CONTROLRATETYPE ConvertModuleToOMXControlRate(RateControlType mode);

AspectRatioType ConvertOMXToModuleAspectRatio(OMX_ALG_EAspectRatio aspectRatio);
OMX_ALG_EAspectRatio ConvertModuleToOMXAspectRatio(AspectRatioType aspectRatio);

GopControlType ConvertOMXToModuleGopControl(OMX_ALG_EGopCtrlMode mode);
OMX_ALG_EGopCtrlMode ConvertModuleToOMXGopControl(GopControlType mode);

GdrType ConvertOMXToModuleGdr(OMX_ALG_EGdrMode gdr);
OMX_ALG_EGdrMode ConvertModuleToOMXGdr(GdrType gdr);

RateControlOptionType ConvertOMXToModuleDisableSceneChangeResilience(OMX_BOOL disable);
OMX_BOOL ConvertModuleToOMXDisableSceneChangeResilience(RateControlOptionType option);

ScalingListType ConvertOMXToModuleScalingList(OMX_ALG_EScalingList scalingListMode);
OMX_ALG_EScalingList ConvertModuleToOMXScalingList(ScalingListType scalingLisgt);

BufferModeType ConvertOMXToModuleBufferMode(OMX_ALG_VIDEO_BUFFER_MODE mode);
OMX_ALG_VIDEO_BUFFER_MODE ConvertModuleToOMXBufferMode(BufferModeType mode);

QualityType ConvertOMXToModuleQuality(OMX_ALG_ERoiQuality quality);

OMX_U32 ConvertModuleToOMXInterlaceFlag(VideoModeType mode);
VideoModeType ConvertOMXToModuleVideoMode(OMX_U32 flag);

SequencePictureModeType ConvertOMXToModuleSequencePictureMode(OMX_ALG_SEQUENCE_PICTURE_MODE mode);
OMX_ALG_SEQUENCE_PICTURE_MODE ConvertModuleToOMXSequencePictureMode(SequencePictureModeType mode);

