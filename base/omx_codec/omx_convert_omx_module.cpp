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

#include "omx_convert_omx_module.h"

#include <stdexcept>
#include <cassert>
#include <cmath>

using namespace std;

OMX_BOOL ConvertModuleToOMXBool(bool boolean)
{
  return (!boolean) ? OMX_FALSE : OMX_TRUE;
}

OMX_COLOR_FORMATTYPE ConvertModuleToOMXColor(ColorType color, int bitdepth)
{
  switch(color)
  {
  case ColorType::COLOR_400:
  {
    if(bitdepth == 8)
      return OMX_COLOR_FormatL8;

    if(bitdepth == 10)
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL10bitPacked);

    throw invalid_argument("bitdepth");
  }
  case ColorType::COLOR_420:
  {
    if(bitdepth == 8)
      return OMX_COLOR_FormatYUV420SemiPlanar;

    if(bitdepth == 10)
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked);

    throw invalid_argument("bitdepth");
  }
  case ColorType::COLOR_422:
  {
    if(bitdepth == 8)
      return OMX_COLOR_FormatYUV422SemiPlanar;

    if(bitdepth == 10)
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked);

    throw invalid_argument("bitdepth");
  }
  case ColorType::COLOR_MAX_ENUM: return OMX_COLOR_FormatMax;
  default:
    throw invalid_argument("color");
  }
}

OMX_VIDEO_CODINGTYPE ConvertModuleToOMXCompression(CompressionType compression)
{
  switch(compression)
  {
  case CompressionType::COMPRESSION_UNUSED: return OMX_VIDEO_CodingUnused;
  case CompressionType::COMPRESSION_MAX_ENUM: return OMX_VIDEO_CodingMax;
  case CompressionType::COMPRESSION_AVC: return OMX_VIDEO_CodingAVC;
  case CompressionType::COMPRESSION_VP9: return static_cast<OMX_VIDEO_CODINGTYPE>(OMX_ALG_VIDEO_CodingVP9);
  case CompressionType::COMPRESSION_HEVC: return static_cast<OMX_VIDEO_CODINGTYPE>(OMX_ALG_VIDEO_CodingHEVC);
  default:
    throw invalid_argument("compression");
  }
}

OMX_U32 ConvertModuleToOMXFramerate(Clock clock)
{
  auto f = ((clock.framerate * 1000.0) / clock.clockratio) * 65536.0;
  return ceil(f);
}

OMX_ALG_BUFFER_MODE ConvertModuleToOMXBufferMode(bool useFd)
{
  if(useFd)
    return OMX_ALG_BUF_DMA;
  else
    return OMX_ALG_BUF_NORMAL;
}

bool ConvertOMXToModuleBool(OMX_BOOL boolean)
{
  if(boolean == OMX_FALSE)
    return false;
  return true;
}

CompressionType ConvertOMXToModuleCompression(OMX_VIDEO_CODINGTYPE coding)
{
  switch(static_cast<OMX_U32>(coding))
  {
  case OMX_VIDEO_CodingUnused: return CompressionType::COMPRESSION_UNUSED;
  case OMX_VIDEO_CodingAVC: return CompressionType::COMPRESSION_AVC;
  case OMX_ALG_VIDEO_CodingHEVC: return CompressionType::COMPRESSION_HEVC;
  case OMX_ALG_VIDEO_CodingVP9: return CompressionType::COMPRESSION_VP9;
  case OMX_VIDEO_CodingMax: return CompressionType::COMPRESSION_MAX_ENUM;
  default:
    throw invalid_argument("coding");
  }

  throw invalid_argument("coding");
}

ColorType ConvertOMXToModuleColor(OMX_COLOR_FORMATTYPE format)
{
  switch(static_cast<OMX_U32>(format))
  {
  case OMX_COLOR_FormatL8:
  case OMX_ALG_COLOR_FormatL10bitPacked:
    return ColorType::COLOR_400;
  case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked:
    return ColorType::COLOR_420;
  case OMX_COLOR_FormatYUV422SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked:
    return ColorType::COLOR_422;
  case OMX_COLOR_FormatMax: return ColorType::COLOR_MAX_ENUM;
  default:
    throw invalid_argument("format");
  }

  throw invalid_argument("format");
}

int ConvertOMXToModuleBitdepth(OMX_COLOR_FORMATTYPE format)
{
  switch(static_cast<OMX_U32>(format))
  {
  case OMX_COLOR_FormatL8:
  case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_COLOR_FormatYUV422SemiPlanar:
    return 8;
  case OMX_ALG_COLOR_FormatL10bitPacked:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked:
    return 10;
  case OMX_COLOR_FormatUnused: return 0; // XXX
  default:
    throw invalid_argument("color");
  }

  throw invalid_argument("color");
}

Clock ConvertOMXToModuleClock(OMX_U32 framerateInQ16)
{
  Clock clock;
  clock.framerate = ceil(framerateInQ16 / 65536.0);
  clock.clockratio = rint((clock.framerate * 1000.0 * 65536.0) / framerateInQ16);
  return clock;
}

bool ConvertOMXToModuleFileDescriptor(OMX_ALG_BUFFER_MODE bufferMode)
{
  switch(bufferMode)
  {
  case OMX_ALG_BUF_NORMAL: return false;
  case OMX_ALG_BUF_DMA: return true;
  case OMX_ALG_BUF_MAX_ENUM: // fallthrough
  default:
    throw invalid_argument("bufferMode");
  }

  throw invalid_argument("bufferMode");
}

DecodedPictureBufferType ConvertOMXToModuleDecodedPictureBuffer(OMX_ALG_EDpbMode mode)
{
  switch(mode)
  {
  case OMX_ALG_DPB_NORMAL: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NORMAL;
  case OMX_ALG_DPB_LOW_REF: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_LOW_REFERENCE;
  case OMX_ALG_DPB_MAX_ENUM: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
  default: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
  }

  return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
}

OMX_ALG_EDpbMode ConvertModuleToOMXDecodedPictureBuffer(DecodedPictureBufferType mode)
{
  switch(mode)
  {
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NORMAL: return OMX_ALG_DPB_NORMAL;
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_LOW_REFERENCE: return OMX_ALG_DPB_LOW_REF;
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM: return OMX_ALG_DPB_MAX_ENUM;
  default: return OMX_ALG_DPB_MAX_ENUM;
  }

  return OMX_ALG_DPB_MAX_ENUM;
}

static inline AVCProfileType ConvertOMXToModuleAVCProfile(OMX_VIDEO_AVCPROFILETYPE profile)
{
  switch(static_cast<OMX_U32>(profile))
  {
  case OMX_VIDEO_AVCProfileBaseline: return AVCProfileType::AVC_PROFILE_BASELINE;
  case OMX_VIDEO_AVCProfileMain: return AVCProfileType::AVC_PROFILE_MAIN;
  case OMX_VIDEO_AVCProfileExtended: return AVCProfileType::AVC_PROFILE_EXTENDED;
  case OMX_VIDEO_AVCProfileHigh: return AVCProfileType::AVC_PROFILE_HIGH;
  case OMX_VIDEO_AVCProfileHigh10: return AVCProfileType::AVC_PROFILE_HIGH_10;
  case OMX_VIDEO_AVCProfileHigh422: return AVCProfileType::AVC_PROFILE_HIGH_422;
  case OMX_VIDEO_AVCProfileHigh444: return AVCProfileType::AVC_PROFILE_HIGH_444_PREDICTIVE;
  case OMX_ALG_VIDEO_AVCProfileConstrainedBaseline: return AVCProfileType::AVC_PROFILE_CONSTRAINED_BASELINE;
  case OMX_ALG_VIDEO_AVCProfileProgressiveHigh: return AVCProfileType::AVC_PROFILE_PROGRESSIVE_HIGH;
  case OMX_ALG_VIDEO_AVCProfileConstrainedHigh: return AVCProfileType::AVC_PROFILE_CONSTRAINED_HIGH;
  case OMX_ALG_VIDEO_AVCProfileHigh10_Intra: return AVCProfileType::AVC_PROFILE_HIGH_10_INTRA;
  case OMX_ALG_VIDEO_AVCProfileHigh422_Intra: return AVCProfileType::AVC_PROFILE_HIGH_422_INTRA;
  case OMX_ALG_VIDEO_AVCProfileHigh444_Intra: return AVCProfileType::AVC_PROFILE_HIGH_444_INTRA;
  case OMX_ALG_VIDEO_AVCProfileCAVLC444_Intra: return AVCProfileType::AVC_PROFILE_CAVLC_444_INTRA;
  case OMX_VIDEO_AVCProfileMax: return AVCProfileType::AVC_PROFILE_MAX_ENUM;
  default: return AVCProfileType::AVC_PROFILE_MAX_ENUM;
  }

  return AVCProfileType::AVC_PROFILE_MAX_ENUM;
}

static inline int ConvertOMXToModuleAVCLevel(OMX_VIDEO_AVCLEVELTYPE level)
{
  switch(static_cast<OMX_U32>(level))
  {
  case OMX_VIDEO_AVCLevel1b: return 9;
  case OMX_VIDEO_AVCLevel1: return 10;
  case OMX_VIDEO_AVCLevel11: return 11;
  case OMX_VIDEO_AVCLevel12: return 12;
  case OMX_VIDEO_AVCLevel13: return 13;
  case OMX_VIDEO_AVCLevel2: return 20;
  case OMX_VIDEO_AVCLevel21: return 21;
  case OMX_VIDEO_AVCLevel22: return 22;
  case OMX_VIDEO_AVCLevel3: return 30;
  case OMX_VIDEO_AVCLevel31: return 31;
  case OMX_VIDEO_AVCLevel32: return 32;
  case OMX_VIDEO_AVCLevel4: return 40;
  case OMX_VIDEO_AVCLevel41: return 41;
  case OMX_VIDEO_AVCLevel42: return 42;
  case OMX_VIDEO_AVCLevel5: return 50;
  case OMX_VIDEO_AVCLevel51: return 51;
  case OMX_ALG_VIDEO_AVCLevel52: return 52;
  case OMX_ALG_VIDEO_AVCLevel60: return 60;
  case OMX_ALG_VIDEO_AVCLevel61: return 61;
  case OMX_ALG_VIDEO_AVCLevel62: return 62;
  default:
    return 0;
  }

  return 0;
}

ProfileLevelType ConvertOMXToModuleAVCProfileLevel(OMX_VIDEO_AVCPROFILETYPE profile, OMX_VIDEO_AVCLEVELTYPE level)
{
  ProfileLevelType pf;
  pf.profile.avc = ConvertOMXToModuleAVCProfile(profile);
  pf.level = ConvertOMXToModuleAVCLevel(level);
  return pf;
}

OMX_VIDEO_AVCPROFILETYPE ConvertModuleToOMXAVCProfile(ProfileLevelType profileLevel)
{
  switch(profileLevel.profile.avc)
  {
  case AVCProfileType::AVC_PROFILE_BASELINE: return OMX_VIDEO_AVCProfileBaseline;
  case AVCProfileType::AVC_PROFILE_MAIN: return OMX_VIDEO_AVCProfileMain;
  case AVCProfileType::AVC_PROFILE_EXTENDED: return OMX_VIDEO_AVCProfileExtended;
  case AVCProfileType::AVC_PROFILE_HIGH: return OMX_VIDEO_AVCProfileHigh;
  case AVCProfileType::AVC_PROFILE_HIGH_10: return OMX_VIDEO_AVCProfileHigh10;
  case AVCProfileType::AVC_PROFILE_HIGH_422: return OMX_VIDEO_AVCProfileHigh422;
  case AVCProfileType::AVC_PROFILE_HIGH_444_PREDICTIVE: return OMX_VIDEO_AVCProfileHigh444;
  case AVCProfileType::AVC_PROFILE_CONSTRAINED_BASELINE: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileConstrainedBaseline);
  case AVCProfileType::AVC_PROFILE_PROGRESSIVE_HIGH: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileProgressiveHigh);
  case AVCProfileType::AVC_PROFILE_CONSTRAINED_HIGH: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileConstrainedHigh);
  case AVCProfileType::AVC_PROFILE_HIGH_10_INTRA: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileHigh10_Intra);
  case AVCProfileType::AVC_PROFILE_HIGH_422_INTRA: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileHigh422_Intra);
  case AVCProfileType::AVC_PROFILE_HIGH_444_INTRA: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileHigh444_Intra);
  case AVCProfileType::AVC_PROFILE_CAVLC_444_INTRA: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileCAVLC444_Intra);
  default: return OMX_VIDEO_AVCProfileMax;
  }

  return OMX_VIDEO_AVCProfileMax;
}

OMX_VIDEO_AVCLEVELTYPE ConvertModuleToOMXAVCLevel(ProfileLevelType profileLevel)
{
  switch(profileLevel.level)
  {
  case 9: return OMX_VIDEO_AVCLevel1b;
  case 10: return OMX_VIDEO_AVCLevel1;
  case 11: return OMX_VIDEO_AVCLevel11;
  case 12: return OMX_VIDEO_AVCLevel12;
  case 13: return OMX_VIDEO_AVCLevel13;
  case 20: return OMX_VIDEO_AVCLevel2;
  case 21: return OMX_VIDEO_AVCLevel21;
  case 22: return OMX_VIDEO_AVCLevel22;
  case 30: return OMX_VIDEO_AVCLevel3;
  case 31: return OMX_VIDEO_AVCLevel31;
  case 32: return OMX_VIDEO_AVCLevel32;
  case 40: return OMX_VIDEO_AVCLevel4;
  case 41: return OMX_VIDEO_AVCLevel41;
  case 42: return OMX_VIDEO_AVCLevel42;
  case 50: return OMX_VIDEO_AVCLevel5;
  case 51: return OMX_VIDEO_AVCLevel51;
  case 52: return static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel52);
  case 60: return static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel60);
  case 61: return static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel61);
  case 62: return static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel62);
  default: return OMX_VIDEO_AVCLevelMax;
  }

  return OMX_VIDEO_AVCLevelMax;
}

LoopFilterType ConvertOMXToModuleAVCLoopFilter(OMX_VIDEO_AVCLOOPFILTERTYPE loopFilter)
{
  switch(loopFilter)
  {
  case OMX_VIDEO_AVCLoopFilterDisable: return LoopFilterType::LOOP_FILTER_DISABLE;
  case OMX_VIDEO_AVCLoopFilterDisableSliceBoundary: return LoopFilterType::LOOP_FILTER_ENABLE;
  case OMX_VIDEO_AVCLoopFilterEnable: return LoopFilterType::LOOP_FILTER_ENABLE_CROSS_SLICE;
  case OMX_VIDEO_AVCLoopFilterMax: return LoopFilterType::LOOP_FILTER_MAX_ENUM;
  default: return LoopFilterType::LOOP_FILTER_MAX_ENUM;
  }

  return LoopFilterType::LOOP_FILTER_MAX_ENUM;
}

OMX_VIDEO_AVCLOOPFILTERTYPE ConvertModuleToOMXAVCLoopFilter(LoopFilterType loopFilter)
{
  switch(loopFilter)
  {
  case LoopFilterType::LOOP_FILTER_DISABLE: return OMX_VIDEO_AVCLoopFilterDisable;
  case LoopFilterType::LOOP_FILTER_ENABLE: return OMX_VIDEO_AVCLoopFilterDisableSliceBoundary;
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_SLICE: return OMX_VIDEO_AVCLoopFilterEnable;
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE: return OMX_VIDEO_AVCLoopFilterMax;
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE: return OMX_VIDEO_AVCLoopFilterMax;
  case LoopFilterType::LOOP_FILTER_MAX_ENUM: return OMX_VIDEO_AVCLoopFilterMax;
  default: return OMX_VIDEO_AVCLoopFilterMax;
  }

  return OMX_VIDEO_AVCLoopFilterMax;
}

int ConvertOMXToModuleBFrames(OMX_U32 bFrames, OMX_U32 pFrames)
{
  return bFrames / (pFrames + 1);
}

int ConvertOMXToModuleGopLength(OMX_U32 bFrames, OMX_U32 pFrames)
{
  return pFrames + bFrames + 1;
}

EntropyCodingType ConvertOMXToModuleEntropyCoding(OMX_BOOL isCabac)
{
  if(isCabac == OMX_FALSE)
    return EntropyCodingType::ENTROPY_CODING_CAVLC;
  return EntropyCodingType::ENTROPY_CODING_CABAC;
}

int ConvertOMXToModuleQPInitial(OMX_U32 qpI)
{
  return qpI;
}

int ConvertOMXToModuleQPDeltaIP(OMX_U32 qpI, OMX_U32 qpP)
{
  return qpP - qpI;
}

int ConvertOMXToModuleQPDeltaPB(OMX_U32 qpP, OMX_U32 qpB)
{
  return qpB - qpP;
}

int ConvertOMXToModuleQPMin(OMX_S32 qpMin)
{
  return qpMin;
}

int ConvertOMXToModuleQPMax(OMX_S32 qpMax)
{
  return qpMax;
}

QPControlType ConvertOMXToModuleQPControl(OMX_ALG_EQpCtrlMode mode)
{
  switch(mode)
  {
  case OMX_ALG_UNIFORM_QP: return QPControlType::QP_UNIFORM;
  case OMX_ALG_ROI_QP: return QPControlType::QP_ROI;
  case OMX_ALG_AUTO_QP: return QPControlType::QP_AUTO;
  case OMX_ALG_MAX_ENUM_QP: return QPControlType::QP_MAX_ENUM;
  default: return QPControlType::QP_MAX_ENUM;
  }

  return QPControlType::QP_MAX_ENUM;
}

RateControlType ConvertOMXToModuleControlRate(OMX_VIDEO_CONTROLRATETYPE mode)
{
  switch(static_cast<OMX_U32>(mode)) // all indexes are in OMX_U32
  {
  case OMX_Video_ControlRateDisable: return RateControlType::RATE_CONTROL_CONSTANT_QUANTIZATION;
  case OMX_Video_ControlRateConstant: return RateControlType::RATE_CONTROL_CONSTANT_BITRATE;
  case OMX_Video_ControlRateVariable: return RateControlType::RATE_CONTROL_VARIABLE_BITRATE;
  case OMX_ALG_Video_ControlRateLowLatency: return RateControlType::RATE_CONTROL_LOW_LATENCY;
  case OMX_ALG_Video_ControlRateMaxEnum: return RateControlType::RATE_CONTROL_MAX_ENUM;
  default: return RateControlType::RATE_CONTROL_MAX_ENUM;
  }

  return RateControlType::RATE_CONTROL_MAX_ENUM;
}

AspectRatioType ConvertOMXToModuleAspectRatio(OMX_ALG_EAspectRatio aspectRatio)
{
  switch(aspectRatio)
  {
  case OMX_ALG_ASPECT_RATIO_NONE: return AspectRatioType::ASPECT_RATIO_NONE;
  case OMX_ALG_ASPECT_RATIO_4_3: return AspectRatioType::ASPECT_RATIO_4_3;
  case OMX_ALG_ASPECT_RATIO_16_9: return AspectRatioType::ASPECT_RATIO_16_9;
  case OMX_ALG_ASPECT_RATIO_AUTO: return AspectRatioType::ASPECT_RATIO_AUTO;
  case OMX_ALG_ASPECT_RATIO_MAX_ENUM: return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
  default: return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
  }

  return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
}

GopControlType ConvertOMXToModuleGopControl(OMX_ALG_EGopCtrlMode mode)
{
  switch(mode)
  {
  case OMX_ALG_GOP_MODE_DEFAULT: return GopControlType::GOP_CONTROL_DEFAULT;
  case OMX_ALG_GOP_MODE_PYRAMIDAL: return GopControlType::GOP_CONTROL_PYRAMIDAL;
  case OMX_ALG_GOP_MODE_ADAPTIVE: return GopControlType::GOP_CONTROL_ADAPTIVE;
  case OMX_ALG_GOP_MODE_LOW_DELAY_P: return GopControlType::GOP_CONTROL_LOW_DELAY_P;
  case OMX_ALG_GOP_MODE_LOW_DELAY_B: return GopControlType::GOP_CONTROL_LOW_DELAY_B;
  case OMX_ALG_GOP_MODE_MAX_ENUM: return GopControlType::GOP_CONTROL_MAX_ENUM;
  default: return GopControlType::GOP_CONTROL_MAX_ENUM;
  }

  return GopControlType::GOP_CONTROL_MAX_ENUM;
}

GdrType ConvertOMXToModuleGdr(OMX_ALG_EGdrMode gdr)
{
  switch(gdr)
  {
  case OMX_ALG_GDR_OFF: return GdrType::GDR_OFF;
  case OMX_ALG_GDR_HORIZONTAL: return GdrType::GDR_HORTIZONTAL;
  case OMX_ALG_GDR_VERTICAL: return GdrType::GDR_VERTICAL;
  case OMX_ALG_GDR_MAX_ENUM: return GdrType::GDR_MAX_ENUM;
  default: return GdrType::GDR_MAX_ENUM;
  }

  return GdrType::GDR_MAX_ENUM;
}

RateControlOptionType ConvertOMXToModuleDisableSceneChangeResilience(OMX_BOOL disable)
{
  return (disable == OMX_TRUE) ? RateControlOptionType::RATE_CONTROL_OPTION_NONE : RateControlOptionType::RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE;
}

ScalingListType ConvertOMXToModuleScalingList(OMX_ALG_EScalingList scalingListMode)
{
  switch(scalingListMode)
  {
  case OMX_ALG_SCL_DEFAULT: return ScalingListType::SCALING_LIST_DEFAULT;
  case OMX_ALG_SCL_FLAT: return ScalingListType::SCALING_LIST_FLAT;
  case OMX_ALG_SCL_MAX_ENUM: return ScalingListType::SCALING_LIST_MAX_ENUM;
  default: return ScalingListType::SCALING_LIST_MAX_ENUM;
  }

  return ScalingListType::SCALING_LIST_MAX_ENUM;
}

OMX_BOOL ConvertModuleToOMXEntropyCoding(EntropyCodingType mode)
{
  switch(mode)
  {
  case EntropyCodingType::ENTROPY_CODING_CABAC: return OMX_TRUE;
  case EntropyCodingType::ENTROPY_CODING_CAVLC: return OMX_FALSE;
  case EntropyCodingType::ENTROPY_CODING_MAX_ENUM: return OMX_FALSE;
  default: return OMX_FALSE;
  }

  return OMX_FALSE;
}

OMX_U32 ConvertModuleToOMXBFrames(Gop gop)
{
  assert(gop.b >= 0);
  return (gop.b * gop.length) / (1 + gop.b);
}

OMX_U32 ConvertModuleToOMXPFrames(Gop gop)
{
  assert(gop.b >= 0);
  return (gop.b - gop.length + 1) / (-gop.b - 1);
}

OMX_U32 ConvertModuleToOMXQpI(QPs qps)
{
  return qps.initial;
}

OMX_U32 ConvertModuleToOMXQpP(QPs qps)
{
  return qps.deltaIP + qps.initial;
}

OMX_U32 ConvertModuleToOMXQpB(QPs qps)
{
  return qps.deltaIP + qps.deltaPB + qps.initial;
}

OMX_S32 ConvertModuleToOMXQpMin(QPs qps)
{
  return qps.min;
}

OMX_S32 ConvertModuleToOMXQpMax(QPs qps)
{
  return qps.max;
}

OMX_ALG_EQpCtrlMode ConvertModuleToOMXQpControl(QPs qps)
{
  switch(qps.mode)
  {
  case QPControlType::QP_UNIFORM: return OMX_ALG_UNIFORM_QP;
  case QPControlType::QP_AUTO: return OMX_ALG_AUTO_QP;
  case QPControlType::QP_MAX_ENUM: return OMX_ALG_MAX_ENUM_QP;
  default: return OMX_ALG_MAX_ENUM_QP;
  }

  return OMX_ALG_MAX_ENUM_QP;
}

OMX_VIDEO_CONTROLRATETYPE ConvertModuleToOMXControlRate(RateControlType mode)
{
  switch(mode)
  {
  case RateControlType::RATE_CONTROL_CONSTANT_QUANTIZATION: return OMX_Video_ControlRateDisable;
  case RateControlType::RATE_CONTROL_CONSTANT_BITRATE: return OMX_Video_ControlRateConstant;
  case RateControlType::RATE_CONTROL_VARIABLE_BITRATE: return OMX_Video_ControlRateVariable;
  case RateControlType::RATE_CONTROL_LOW_LATENCY: return static_cast<OMX_VIDEO_CONTROLRATETYPE>(OMX_ALG_Video_ControlRateLowLatency);
  case RateControlType::RATE_CONTROL_MAX_ENUM: return OMX_Video_ControlRateMax;
  default: return OMX_Video_ControlRateMax;
  }

  return OMX_Video_ControlRateMax;
}

OMX_ALG_EAspectRatio ConvertModuleToOMXAspectRatio(AspectRatioType aspectRatio)
{
  switch(aspectRatio)
  {
  case AspectRatioType::ASPECT_RATIO_NONE: return OMX_ALG_ASPECT_RATIO_NONE;
  case AspectRatioType::ASPECT_RATIO_4_3: return OMX_ALG_ASPECT_RATIO_4_3;
  case AspectRatioType::ASPECT_RATIO_16_9: return OMX_ALG_ASPECT_RATIO_16_9;
  case AspectRatioType::ASPECT_RATIO_AUTO: return OMX_ALG_ASPECT_RATIO_AUTO;
  case AspectRatioType::ASPECT_RATIO_MAX_ENUM: return OMX_ALG_ASPECT_RATIO_MAX_ENUM;
  default: return OMX_ALG_ASPECT_RATIO_MAX_ENUM;
  }

  return OMX_ALG_ASPECT_RATIO_MAX_ENUM;
}

OMX_ALG_EGopCtrlMode ConvertModuleToOMXGopControl(GopControlType mode)
{
  switch(mode)
  {
  case GopControlType::GOP_CONTROL_DEFAULT: return OMX_ALG_GOP_MODE_DEFAULT;
  case GopControlType::GOP_CONTROL_PYRAMIDAL: return OMX_ALG_GOP_MODE_PYRAMIDAL;
  case GopControlType::GOP_CONTROL_ADAPTIVE: return OMX_ALG_GOP_MODE_ADAPTIVE;
  case GopControlType::GOP_CONTROL_LOW_DELAY_P: return OMX_ALG_GOP_MODE_LOW_DELAY_P;
  case GopControlType::GOP_CONTROL_LOW_DELAY_B: return OMX_ALG_GOP_MODE_LOW_DELAY_B;
  default: return OMX_ALG_GOP_MODE_MAX_ENUM;
  }

  return OMX_ALG_GOP_MODE_MAX_ENUM;
}

OMX_ALG_EGdrMode ConvertModuleToOMXGdr(GdrType gdr)
{
  switch(gdr)
  {
  case GdrType::GDR_OFF: return OMX_ALG_GDR_OFF;
  case GdrType::GDR_VERTICAL: return OMX_ALG_GDR_VERTICAL;
  case GdrType::GDR_HORTIZONTAL: return OMX_ALG_GDR_HORIZONTAL;
  default: return OMX_ALG_GDR_MAX_ENUM;
  }

  return OMX_ALG_GDR_MAX_ENUM;
}

OMX_BOOL ConvertModuleToOMXDisableSceneChangeResilience(RateControlOptionType option)
{
  if(option == RateControlOptionType::RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE)
    return OMX_FALSE; // Because it's bDisableSceneChangeResilience

  return OMX_TRUE;
}

OMX_ALG_EScalingList ConvertModuleToOMXScalingList(ScalingListType scalingLisgt)
{
  switch(scalingLisgt)
  {
  case ScalingListType::SCALING_LIST_DEFAULT: return OMX_ALG_SCL_DEFAULT;
  case ScalingListType::SCALING_LIST_FLAT: return OMX_ALG_SCL_FLAT;
  case ScalingListType::SCALING_LIST_MAX_ENUM: return OMX_ALG_SCL_MAX_ENUM;
  default: return OMX_ALG_SCL_MAX_ENUM;
  }

  return OMX_ALG_SCL_MAX_ENUM;
}

LoopFilterType ConvertOMXToModuleHEVCLoopFilter(OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE loopFilter)
{
  switch(loopFilter)
  {
  case OMX_ALG_VIDEO_HEVCLoopFilterDisable: return LoopFilterType::LOOP_FILTER_DISABLE;
  case OMX_ALG_VIDEO_HEVCLoopFilterEnable: return LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSlice: return LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossTile: return LoopFilterType::LOOP_FILTER_ENABLE_CROSS_SLICE;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSliceAndTile: return LoopFilterType::LOOP_FILTER_ENABLE;
  case OMX_ALG_VIDEO_HEVCLoopFilterMaxEnum: return LoopFilterType::LOOP_FILTER_MAX_ENUM;
  default: return LoopFilterType::LOOP_FILTER_MAX_ENUM;
  }

  return LoopFilterType::LOOP_FILTER_MAX_ENUM;
}

OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE ConvertModuleToOMXHEVCLoopFilter(LoopFilterType loopFilter)
{
  switch(loopFilter)
  {
  case LoopFilterType::LOOP_FILTER_DISABLE: return OMX_ALG_VIDEO_HEVCLoopFilterDisable;
  case LoopFilterType::LOOP_FILTER_ENABLE: return OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSliceAndTile;
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_SLICE: return OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossTile;
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE: return OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSlice;
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE: return OMX_ALG_VIDEO_HEVCLoopFilterEnable;
  case LoopFilterType::LOOP_FILTER_MAX_ENUM: return OMX_ALG_VIDEO_HEVCLoopFilterMaxEnum;
  default: return OMX_ALG_VIDEO_HEVCLoopFilterMaxEnum;
  }

  return OMX_ALG_VIDEO_HEVCLoopFilterMaxEnum;
}

static inline HEVCProfileType ConvertOMXToModuleHEVCMainTierProfile(OMX_ALG_VIDEO_HEVCPROFILETYPE profile)
{
  switch(profile)
  {
  case OMX_ALG_VIDEO_HEVCProfileMain: return HEVCProfileType::HEVC_PROFILE_MAIN;
  case OMX_ALG_VIDEO_HEVCProfileMain10: return HEVCProfileType::HEVC_PROFILE_MAIN_10;
  case OMX_ALG_VIDEO_HEVCProfileMainStill: return HEVCProfileType::HEVC_PROFILE_MAIN_STILL;
  case OMX_ALG_VIDEO_HEVCProfileMonochrome: return HEVCProfileType::HEVC_PROFILE_MONOCHROME;
  case OMX_ALG_VIDEO_HEVCProfileMonochrome10: return HEVCProfileType::HEVC_PROFILE_MONOCHROME_10;
  case OMX_ALG_VIDEO_HEVCProfileMonochrome12: return HEVCProfileType::HEVC_PROFILE_MONOCHROME_12;
  case OMX_ALG_VIDEO_HEVCProfileMonochrome16: return HEVCProfileType::HEVC_PROFILE_MONOCHROME_16;
  case OMX_ALG_VIDEO_HEVCProfileMain12: return HEVCProfileType::HEVC_PROFILE_MAIN_12;
  case OMX_ALG_VIDEO_HEVCProfileMain422: return HEVCProfileType::HEVC_PROFILE_MAIN_422;
  case OMX_ALG_VIDEO_HEVCProfileMain422_10: return HEVCProfileType::HEVC_PROFILE_MAIN_422_10;
  case OMX_ALG_VIDEO_HEVCProfileMain422_12: return HEVCProfileType::HEVC_PROFILE_MAIN_422_12;
  case OMX_ALG_VIDEO_HEVCProfileMain444: return HEVCProfileType::HEVC_PROFILE_MAIN_444;
  case OMX_ALG_VIDEO_HEVCProfileMain444_10: return HEVCProfileType::HEVC_PROFILE_MAIN_444_10;
  case OMX_ALG_VIDEO_HEVCProfileMain444_12: return HEVCProfileType::HEVC_PROFILE_MAIN_444_12;
  case OMX_ALG_VIDEO_HEVCProfileMain_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain10_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain12_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_12_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain422_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain422_10_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain422_12_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_422_12_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain444_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_444_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain444_10_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_444_10_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain444_12_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_444_12_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain444_16_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_444_16_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMain444_Still: return HEVCProfileType::HEVC_PROFILE_MAIN_444_STILL;
  case OMX_ALG_VIDEO_HEVCProfileMain444_16_Still: return HEVCProfileType::HEVC_PROFILE_MAIN_444_16_STILL;
  case OMX_ALG_VIDEO_HEVCProfileHighThroughtPut444_16_Intra: return HEVCProfileType::HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA;
  case OMX_ALG_VIDEO_HEVCProfileMaxEnum: return HEVCProfileType::HEVC_PROFILE_MAX_ENUM;
  default: return HEVCProfileType::HEVC_PROFILE_MAX_ENUM;
  }

  return HEVCProfileType::HEVC_PROFILE_MAX_ENUM;
}

static inline HEVCProfileType ConvertOMXToModuleHEVCHighTierProfile(OMX_ALG_VIDEO_HEVCPROFILETYPE profile)
{
  switch(profile)
  {
  case OMX_ALG_VIDEO_HEVCProfileMain: return HEVCProfileType::HEVC_PROFILE_MAIN_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain10: return HEVCProfileType::HEVC_PROFILE_MAIN_10_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMainStill: return HEVCProfileType::HEVC_PROFILE_MAIN_STILL_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMonochrome: return HEVCProfileType::HEVC_PROFILE_MONOCHROME_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMonochrome10: return HEVCProfileType::HEVC_PROFILE_MONOCHROME_10_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMonochrome12: return HEVCProfileType::HEVC_PROFILE_MONOCHROME_12_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMonochrome16: return HEVCProfileType::HEVC_PROFILE_MONOCHROME_16_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain12: return HEVCProfileType::HEVC_PROFILE_MAIN_12_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain422: return HEVCProfileType::HEVC_PROFILE_MAIN_422_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain422_10: return HEVCProfileType::HEVC_PROFILE_MAIN_422_10_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain422_12: return HEVCProfileType::HEVC_PROFILE_MAIN_422_12_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444: return HEVCProfileType::HEVC_PROFILE_MAIN_444_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444_10: return HEVCProfileType::HEVC_PROFILE_MAIN_444_10_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444_12: return HEVCProfileType::HEVC_PROFILE_MAIN_444_12_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain10_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain12_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_12_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain422_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain422_10_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain422_12_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_422_12_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_444_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444_10_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_444_10_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444_12_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_444_12_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444_16_Intra: return HEVCProfileType::HEVC_PROFILE_MAIN_444_16_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444_Still: return HEVCProfileType::HEVC_PROFILE_MAIN_444_STILL_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain444_16_Still: return HEVCProfileType::HEVC_PROFILE_MAIN_444_16_STILL_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileHighThroughtPut444_16_Intra: return HEVCProfileType::HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMaxEnum: return HEVCProfileType::HEVC_PROFILE_MAX_ENUM;
  default: return HEVCProfileType::HEVC_PROFILE_MAX_ENUM;
  }

  return HEVCProfileType::HEVC_PROFILE_MAX_ENUM;
}

static inline int ConvertOMXToModuleHEVCLevel(OMX_ALG_VIDEO_HEVCLEVELTYPE level)
{
  switch(level)
  {
  case OMX_ALG_VIDEO_HEVCMainTierLevel1: return 10;
  case OMX_ALG_VIDEO_HEVCMainTierLevel2: return 20;
  case OMX_ALG_VIDEO_HEVCMainTierLevel21: return 21;
  case OMX_ALG_VIDEO_HEVCMainTierLevel3: return 30;
  case OMX_ALG_VIDEO_HEVCMainTierLevel31: return 31;
  case OMX_ALG_VIDEO_HEVCMainTierLevel4:
  case OMX_ALG_VIDEO_HEVCHighTierLevel4: return 40;
  case OMX_ALG_VIDEO_HEVCMainTierLevel41:
  case OMX_ALG_VIDEO_HEVCHighTierLevel41: return 41;
  case OMX_ALG_VIDEO_HEVCMainTierLevel5:
  case OMX_ALG_VIDEO_HEVCHighTierLevel5: return 50;
  case OMX_ALG_VIDEO_HEVCMainTierLevel51:
  case OMX_ALG_VIDEO_HEVCHighTierLevel51: return 51;
  case OMX_ALG_VIDEO_HEVCMainTierLevel52:
  case OMX_ALG_VIDEO_HEVCHighTierLevel52: return 52;
  case OMX_ALG_VIDEO_HEVCMainTierLevel6:
  case OMX_ALG_VIDEO_HEVCHighTierLevel6: return 60;
  case OMX_ALG_VIDEO_HEVCMainTierLevel61:
  case OMX_ALG_VIDEO_HEVCHighTierLevel61: return 61;
  case OMX_ALG_VIDEO_HEVCMainTierLevel62:
  case OMX_ALG_VIDEO_HEVCHighTierLevel62: return 62;
  default:
    return 0;
  }

  return 0;
}

static inline bool IsMainTier(OMX_ALG_VIDEO_HEVCLEVELTYPE level)
{
  switch(level)
  {
  case OMX_ALG_VIDEO_HEVCHighTierLevel4:
  case OMX_ALG_VIDEO_HEVCHighTierLevel41:
  case OMX_ALG_VIDEO_HEVCHighTierLevel5:
  case OMX_ALG_VIDEO_HEVCHighTierLevel51:
  case OMX_ALG_VIDEO_HEVCHighTierLevel52:
  case OMX_ALG_VIDEO_HEVCHighTierLevel6:
  case OMX_ALG_VIDEO_HEVCHighTierLevel61:
  case OMX_ALG_VIDEO_HEVCHighTierLevel62: return false;
  default: return true;
  }

  return true;
}

ProfileLevelType ConvertOMXToModuleHEVCProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE profile, OMX_ALG_VIDEO_HEVCLEVELTYPE level)
{
  ProfileLevelType pf;
  pf.profile.hevc = IsMainTier(level) ? ConvertOMXToModuleHEVCMainTierProfile(profile) : ConvertOMXToModuleHEVCHighTierProfile(profile);
  pf.level = ConvertOMXToModuleHEVCLevel(level);
  return pf;
}

OMX_ALG_VIDEO_HEVCPROFILETYPE ConvertModuleToOMXHEVCProfile(ProfileLevelType profileLevel)
{
  switch(profileLevel.profile.hevc)
  {
  case HEVCProfileType::HEVC_PROFILE_MAIN:
  case HEVCProfileType::HEVC_PROFILE_MAIN_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain;
  case HEVCProfileType::HEVC_PROFILE_MAIN_10:
  case HEVCProfileType::HEVC_PROFILE_MAIN_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain10;
  case HEVCProfileType::HEVC_PROFILE_MAIN_STILL:
  case HEVCProfileType::HEVC_PROFILE_MAIN_STILL_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMainStill;
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMonochrome;
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_10:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMonochrome10;
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_12:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_12_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMonochrome12;
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_16:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_16_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMonochrome16;
  case HEVCProfileType::HEVC_PROFILE_MAIN_12:
  case HEVCProfileType::HEVC_PROFILE_MAIN_12_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain12;
  case HEVCProfileType::HEVC_PROFILE_MAIN_422:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422;
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_10;
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_12:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_12_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_12;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_10:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_10;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_12:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_12_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_12;
  case HEVCProfileType::HEVC_PROFILE_MAIN_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain10_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_12_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_12_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain12_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_10_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_12_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_12_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_12_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_10_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_10_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_10_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_12_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_12_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_12_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_16_Intra;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_STILL:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_STILL_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_Still;
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16_STILL:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16_STILL_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_Still;
  case HEVCProfileType::HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA:
  case HEVCProfileType::HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileHighThroughtPut444_16_Intra;
  default: return OMX_ALG_VIDEO_HEVCProfileMaxEnum;
  }

  return OMX_ALG_VIDEO_HEVCProfileMaxEnum;
}

static inline OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertModuleToOMXHEVCMainLevel(ProfileLevelType profileLevel)
{
  switch(profileLevel.level)
  {
  case 10: return OMX_ALG_VIDEO_HEVCMainTierLevel1;
  case 20: return OMX_ALG_VIDEO_HEVCMainTierLevel2;
  case 21: return OMX_ALG_VIDEO_HEVCMainTierLevel21;
  case 30: return OMX_ALG_VIDEO_HEVCMainTierLevel3;
  case 31: return OMX_ALG_VIDEO_HEVCMainTierLevel31;
  case 40: return OMX_ALG_VIDEO_HEVCMainTierLevel4;
  case 41: return OMX_ALG_VIDEO_HEVCMainTierLevel41;
  case 50: return OMX_ALG_VIDEO_HEVCMainTierLevel5;
  case 51: return OMX_ALG_VIDEO_HEVCMainTierLevel51;
  case 52: return OMX_ALG_VIDEO_HEVCMainTierLevel52;
  case 60: return OMX_ALG_VIDEO_HEVCMainTierLevel6;
  case 61: return OMX_ALG_VIDEO_HEVCMainTierLevel61;
  case 62: return OMX_ALG_VIDEO_HEVCMainTierLevel62;
  default: return OMX_ALG_VIDEO_HEVCLevelMaxEnum;
  }

  return OMX_ALG_VIDEO_HEVCLevelMaxEnum;
}

static inline OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertModuleToOMXHEVCHighLevel(ProfileLevelType profileLevel)
{
  switch(profileLevel.level)
  {
  case 40: return OMX_ALG_VIDEO_HEVCHighTierLevel4;
  case 41: return OMX_ALG_VIDEO_HEVCHighTierLevel41;
  case 50: return OMX_ALG_VIDEO_HEVCHighTierLevel5;
  case 51: return OMX_ALG_VIDEO_HEVCHighTierLevel51;
  case 52: return OMX_ALG_VIDEO_HEVCHighTierLevel52;
  case 60: return OMX_ALG_VIDEO_HEVCHighTierLevel6;
  case 61: return OMX_ALG_VIDEO_HEVCHighTierLevel61;
  case 62: return OMX_ALG_VIDEO_HEVCHighTierLevel62;
  default: return OMX_ALG_VIDEO_HEVCLevelMaxEnum;
  }

  return OMX_ALG_VIDEO_HEVCLevelMaxEnum;
}

static inline bool IsMainTier(ProfileLevelType profileLevel)
{
  switch(profileLevel.profile.hevc)
  {
  case HEVCProfileType::HEVC_PROFILE_MAIN_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_10_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_STILL_HIGH_TIER: return false;
  default: return true;
  }

  return true;
}

OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertModuleToOMXHEVCLevel(ProfileLevelType profileLevel)
{
  return IsMainTier(profileLevel) ? ConvertModuleToOMXHEVCMainLevel(profileLevel) : ConvertModuleToOMXHEVCHighLevel(profileLevel);
}

BufferModeType ConvertOMXToModuleBufferMode(OMX_ALG_VIDEO_BUFFER_MODE mode)
{
  switch(mode)
  {
  case OMX_ALG_VIDEO_BUFFER_MODE_FRAME: return BufferModeType::BUFFER_MODE_FRAME;
  case OMX_ALG_VIDEO_BUFFER_MODE_FRAME_NO_REORDERING: return BufferModeType::BUFFER_MODE_FRAME_NO_REORDERING;
  case OMX_ALG_VIDEO_BUFFER_MODE_SLICE: return BufferModeType::BUFFER_MODE_SLICE;
  case OMX_ALG_VIDEO_BUFFER_MODE_MAX_ENUM: return BufferModeType::BUFFER_MODE_MAX_ENUM;
  default: return BufferModeType::BUFFER_MODE_MAX_ENUM;
  }

  return BufferModeType::BUFFER_MODE_MAX_ENUM;
}

OMX_ALG_VIDEO_BUFFER_MODE ConvertModuleToOMXBufferMode(BufferModeType mode)
{
  switch(mode)
  {
  case BufferModeType::BUFFER_MODE_FRAME: return OMX_ALG_VIDEO_BUFFER_MODE_FRAME;
  case BufferModeType::BUFFER_MODE_FRAME_NO_REORDERING: return OMX_ALG_VIDEO_BUFFER_MODE_FRAME_NO_REORDERING;
  case BufferModeType::BUFFER_MODE_SLICE: return OMX_ALG_VIDEO_BUFFER_MODE_SLICE;
  case BufferModeType::BUFFER_MODE_MAX_ENUM: return OMX_ALG_VIDEO_BUFFER_MODE_MAX_ENUM;
  default: return OMX_ALG_VIDEO_BUFFER_MODE_MAX_ENUM;
  }

  return OMX_ALG_VIDEO_BUFFER_MODE_MAX_ENUM;
}

QualityType ConvertOMXToModuleQuality(OMX_ALG_ERoiQuality quality)
{
  switch(quality)
  {
  case OMX_ALG_ROI_QUALITY_HIGH: return QualityType::REGION_OF_INTEREST_QUALITY_HIGH;
  case OMX_ALG_ROI_QUALITY_MEDIUM: return QualityType::REGION_OF_INTEREST_QUALITY_MEDIUM;
  case OMX_ALG_ROI_QUALITY_LOW: return QualityType::REGION_OF_INTEREST_QUALITY_LOW;
  case OMX_ALG_ROI_QUALITY_DONT_CARE: return QualityType::REGION_OF_INTEREST_QUALITY_DONT_CARE;
  case OMX_ALG_ROI_QUALITY_MAX_ENUM: return QualityType::REGION_OF_INTEREST_QUALITY_MAX_ENUM;
  default: return QualityType::REGION_OF_INTEREST_QUALITY_MAX_ENUM;
  }

  return QualityType::REGION_OF_INTEREST_QUALITY_MAX_ENUM;
}

OMX_U32 ConvertModuleToOMXInterlaceFlag(VideoModeType mode)
{
  switch(mode)
  {
  case VideoModeType::VIDEO_MODE_PROGRESSIVE: return OMX_InterlaceFrameProgressive;
  case VideoModeType::VIDEO_MODE_ALTERNATE_TOP_BOTTOM_FIELD: return OMX_ALG_InterlaceAlternateTopFieldFirst;
  case VideoModeType::VIDEO_MODE_ALTERNATE_BOTTOM_TOP_FIELD: return OMX_ALG_InterlaceAlternateBottomFieldFirst;
  case VideoModeType::VIDEO_MODE_MAX_ENUM: assert(0);
  default: assert(0);
  }

  assert(0);
  return 0;
}

VideoModeType ConvertOMXToModuleVideoMode(OMX_U32 flag)
{
  switch(flag)
  {
  case OMX_InterlaceFrameProgressive: return VideoModeType::VIDEO_MODE_PROGRESSIVE;
  case OMX_ALG_InterlaceAlternateTopFieldFirst: return VideoModeType::VIDEO_MODE_ALTERNATE_TOP_BOTTOM_FIELD;
  case OMX_ALG_InterlaceAlternateBottomFieldFirst: return VideoModeType::VIDEO_MODE_ALTERNATE_BOTTOM_TOP_FIELD;
  default: assert(0);
    return VideoModeType::VIDEO_MODE_MAX_ENUM;
  }

  assert(0);
  return VideoModeType::VIDEO_MODE_MAX_ENUM;
}

SequencePictureModeType ConvertOMXToModuleSequencePictureMode(OMX_ALG_SEQUENCE_PICTURE_MODE mode)
{
  switch(mode)
  {
  case OMX_ALG_SEQUENCE_PICTURE_UNKNOWN: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_UNKNOWN;
  case OMX_ALG_SEQUENCE_PICTURE_FRAME: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_FRAME;
  case OMX_ALG_SEQUENCE_PICTURE_FIELD: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_FIELD;
  case OMX_ALG_SEQUENCE_PICTURE_FRAME_AND_FIELD: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_FRAME_AND_FIELD;
  case OMX_ALG_SEQUENCE_PICTURE_MAX_ENUM: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_MAX_ENUM;
  default:
    return SequencePictureModeType::SEQUENCE_PICTURE_MODE_MAX_ENUM;
  }

  return SequencePictureModeType::SEQUENCE_PICTURE_MODE_MAX_ENUM;
}

OMX_ALG_SEQUENCE_PICTURE_MODE ConvertModuleToOMXSequencePictureMode(SequencePictureModeType mode)
{
  switch(mode)
  {
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_UNKNOWN: return OMX_ALG_SEQUENCE_PICTURE_UNKNOWN;
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_FRAME: return OMX_ALG_SEQUENCE_PICTURE_FRAME;
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_FIELD: return OMX_ALG_SEQUENCE_PICTURE_FIELD;
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_FRAME_AND_FIELD: return OMX_ALG_SEQUENCE_PICTURE_FRAME_AND_FIELD;
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_MAX_ENUM: return OMX_ALG_SEQUENCE_PICTURE_MAX_ENUM;
  default:
    return OMX_ALG_SEQUENCE_PICTURE_MAX_ENUM;
  }

  return OMX_ALG_SEQUENCE_PICTURE_MAX_ENUM;
}

