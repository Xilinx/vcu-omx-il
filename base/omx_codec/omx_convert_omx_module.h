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

#include "base/omx_module/omx_module_enums.h"
#include "base/omx_module/omx_module_structs.h"

ColorType ConvertToModuleColor(OMX_COLOR_FORMATTYPE const& format);
OMX_COLOR_FORMATTYPE ConvertToOMXColor(ColorType const& color, int const& bitdepth);

CompressionType ConvertToModuleCompression(OMX_VIDEO_CODINGTYPE const& coding);
OMX_VIDEO_CODINGTYPE ConvertToOMXCompression(CompressionType const& compression);

OMX_U32 ConvertToOMXFramerate(Clock const& clock);

int ConvertToModuleBitdepth(OMX_COLOR_FORMATTYPE const& format);
Clock ConvertToModuleClock(OMX_U32 const& framerateInQ16);

bool ConvertToModuleBool(OMX_BOOL const& boolean);
OMX_BOOL ConvertToOMXBool(bool const& boolean);

bool ConvertToModuleFileDescriptor(OMX_ALG_BUFFER_MODE const& bufferMode);
OMX_ALG_BUFFER_MODE ConvertToOMXBufferMode(bool const& useFd);

DecodedPictureBufferType ConvertToModuleDecodedPictureBuffer(OMX_ALG_EDpbMode const& mode);
OMX_ALG_EDpbMode ConvertToOMXDecodedPictureBuffer(DecodedPictureBufferType const& mode);

ProfileLevelType ConvertToModuleAVCProfileLevel(OMX_VIDEO_AVCPROFILETYPE const& profile, OMX_VIDEO_AVCLEVELTYPE const& level);

OMX_VIDEO_AVCPROFILETYPE ConvertToOMXAVCProfile(ProfileLevelType const& profileLevel);
OMX_VIDEO_AVCLEVELTYPE ConvertToOMXAVCLevel(ProfileLevelType const& profileLevel);

ProfileLevelType ConvertToModuleHEVCProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile, OMX_ALG_VIDEO_HEVCLEVELTYPE const& level);

OMX_ALG_VIDEO_HEVCPROFILETYPE ConvertToOMXHEVCProfile(ProfileLevelType const& profileLevel);
OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertToOMXHEVCLevel(ProfileLevelType const& profileLevel);

LoopFilterType ConvertToModuleAVCLoopFilter(OMX_VIDEO_AVCLOOPFILTERTYPE const& loopFilter);
OMX_VIDEO_AVCLOOPFILTERTYPE ConvertToOMXAVCLoopFilter(LoopFilterType const& loopFilter);

LoopFilterType ConvertToModuleHEVCLoopFilter(OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE const& loopFilter);
OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE ConvertToOMXHEVCLoopFilter(LoopFilterType const& loopFilter);

int ConvertToModuleBFrames(OMX_U32 const& bFrames, OMX_U32 const& pFrames);
int ConvertToModuleGopLength(OMX_U32 const& bFrames, OMX_U32 const& pFrames);
OMX_U32 ConvertToOMXBFrames(Gop const& gop);
OMX_U32 ConvertToOMXPFrames(Gop const& gop);

EntropyCodingType ConvertToModuleEntropyCoding(OMX_BOOL const& isCabac);
OMX_BOOL ConvertToOMXEntropyCoding(EntropyCodingType const& mode);

int ConvertToModuleQPInitial(OMX_U32 const& qpI);
int ConvertToModuleQPDeltaIP(OMX_U32 const& qpI, OMX_U32 const& qpP);
int ConvertToModuleQPDeltaPB(OMX_U32 const& pP, OMX_U32 const& qpB);
OMX_U32 ConvertToOMXQpI(QPs const& qps);
OMX_U32 ConvertToOMXQpP(QPs const& qps);
OMX_U32 ConvertToOMXQpB(QPs const& qps);

int ConvertToModuleQPMin(OMX_S32 const& qpMin);
OMX_S32 ConvertToOMXQpMin(QPs const& qps);

int ConvertToModuleQPMax(OMX_S32 const& qpMax);
OMX_S32 ConvertToOMXQpMax(QPs const& qps);

QPControlType ConvertToModuleQPControl(OMX_ALG_EQpCtrlMode const& mode);
OMX_ALG_EQpCtrlMode ConvertToOMXQpControl(QPs const& qps);

RateControlType ConvertToModuleControlRate(OMX_VIDEO_CONTROLRATETYPE const& mode);
OMX_VIDEO_CONTROLRATETYPE ConvertToOMXControlRate(RateControlType const& mode);

AspectRatioType ConvertToModuleAspectRatio(OMX_ALG_EAspectRatio const& aspectRatio);
OMX_ALG_EAspectRatio ConvertToOMXAspectRatio(AspectRatioType const& aspectRatio);

GopControlType ConvertToModuleGopControl(OMX_ALG_EGopCtrlMode const& mode);
OMX_ALG_EGopCtrlMode ConvertToOMXGopControl(GopControlType const& mode);

GdrType ConvertToModuleGdr(OMX_ALG_EGdrMode const& gdr);
OMX_ALG_EGdrMode ConvertToOMXGdr(GdrType const& gdr);

RateControlOptionType ConvertToModuleDisableSceneChangeResilience(OMX_BOOL const& disable);
OMX_BOOL ConvertToOMXDisableSceneChangeResilience(RateControlOptionType const& option);

ScalingListType ConvertToModuleScalingList(OMX_ALG_EScalingList const& scalingListMode);
OMX_ALG_EScalingList ConvertToOMXScalingList(ScalingListType const& scalingLisgt);

BufferModeType ConvertToModuleBufferMode(OMX_ALG_VIDEO_BUFFER_MODE const& mode);
OMX_ALG_VIDEO_BUFFER_MODE ConvertToOMXBufferMode(BufferModeType const& mode);

QualityType ConvertToModuleQuality(OMX_ALG_ERoiQuality const& quality);

