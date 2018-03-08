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

#include <OMX_VideoExt.h>
#include <stdexcept>
#include <cassert>
#include <cmath>

using namespace std;

OMX_BOOL ConvertToOMXBool(bool const& boolean)
{
  return (!boolean) ? OMX_FALSE : OMX_TRUE;
}

OMX_COLOR_FORMATTYPE ConvertToOMXColor(ColorType const& color, int const& bitdepth)
{
  switch(color)
  {
  case COLOR_MONO:
  {
    if(bitdepth == 8)
      return OMX_COLOR_FormatL8;

    if(bitdepth == 10)
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL10bitPacked);

    throw invalid_argument("bitdepth");
  }
  case COLOR_420:
  {
    if(bitdepth == 8)
      return OMX_COLOR_FormatYUV420SemiPlanar;

    if(bitdepth == 10)
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked);

    throw invalid_argument("bitdepth");
  }
  case COLOR_422:
  {
    if(bitdepth == 8)
      return OMX_COLOR_FormatYUV422SemiPlanar;

    if(bitdepth == 10)
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked);

    throw invalid_argument("bitdepth");
  }
  case COLOR_MAX: return OMX_COLOR_FormatMax;
  default:
    throw invalid_argument("color");
  }
}

OMX_VIDEO_CODINGTYPE ConvertToOMXCompression(CompressionType const& compression)
{
  switch(compression)
  {
  case COMPRESSION_UNUSED: return OMX_VIDEO_CodingUnused;
  case COMPRESSION_MAX: return OMX_VIDEO_CodingMax;
  case COMPRESSION_AVC: return OMX_VIDEO_CodingAVC;
  case COMPRESSION_VP9: return static_cast<OMX_VIDEO_CODINGTYPE>(OMX_ALG_VIDEO_CodingVP9);
  case COMPRESSION_HEVC: return static_cast<OMX_VIDEO_CODINGTYPE>(OMX_ALG_VIDEO_CodingHEVC);
  default:
    throw invalid_argument("compression");
  }
}

OMX_U32 ConvertToOMXFramerate(Clock const& clock)
{
  auto const f = ((clock.framerate * 1000.0) / clock.clockratio) * 65536.0;
  return ceil(f);
}

OMX_ALG_BUFFER_MODE ConvertToOMXBufferMode(bool const& useFd)
{
  if(useFd)
    return OMX_ALG_BUF_DMA;
  else
    return OMX_ALG_BUF_NORMAL;
}

bool ConvertToModuleBool(OMX_BOOL const& boolean)
{
  if(boolean == OMX_FALSE)
    return false;
  return true;
}

CompressionType ConvertToModuleCompression(OMX_VIDEO_CODINGTYPE const& coding)
{
  switch(static_cast<OMX_U32>(coding))
  {
  case OMX_VIDEO_CodingUnused: return COMPRESSION_UNUSED;
  case OMX_VIDEO_CodingAVC: return COMPRESSION_AVC;
  case OMX_ALG_VIDEO_CodingHEVC: return COMPRESSION_HEVC;
  case OMX_ALG_VIDEO_CodingVP9: return COMPRESSION_VP9;
  case OMX_VIDEO_CodingMax: return COMPRESSION_MAX;
  default:
    throw invalid_argument("coding");
  }

  throw invalid_argument("coding");
}

ColorType ConvertToModuleColor(OMX_COLOR_FORMATTYPE const& format)
{
  switch(static_cast<OMX_U32>(format))
  {
  case OMX_COLOR_FormatL8:
  case OMX_ALG_COLOR_FormatL10bitPacked:
    return COLOR_MONO;
  case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked:
    return COLOR_420;
  case OMX_COLOR_FormatYUV422SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked:
    return COLOR_422;
  case OMX_COLOR_FormatMax: return COLOR_MAX;
  default:
    throw invalid_argument("format");
  }

  throw invalid_argument("format");
}

int ConvertToModuleBitdepth(OMX_COLOR_FORMATTYPE const& format)
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

Clock ConvertToModuleClock(OMX_U32 const& framerateInQ16)
{
  Clock clock;
  clock.framerate = ceil(framerateInQ16 / 65536.0);
  clock.clockratio = rint((clock.framerate * 1000.0 * 65536.0) / framerateInQ16);
  return clock;
}

bool ConvertToModuleFileDescriptor(OMX_ALG_BUFFER_MODE const& bufferMode)
{
  switch(bufferMode)
  {
  case OMX_ALG_BUF_NORMAL: return false;
  case OMX_ALG_BUF_DMA: return true;
  case OMX_ALG_BUF_MAX: // fallthrough
  default:
    throw invalid_argument("bufferMode");
  }

  throw invalid_argument("bufferMode");
}

DecodedPictureBufferType ConvertToModuleDecodedPictureBuffer(OMX_ALG_EDpbMode const& mode)
{
  switch(mode)
  {
  case OMX_ALG_DPB_NORMAL: return DECODED_PICTURE_BUFFER_NORMAL;
  case OMX_ALG_DPB_LOW_REF: return DECODED_PICTURE_BUFFER_LOW_REFERENCE;
  case OMX_ALG_DPB_MAX: // fallthrough
  default: return DECODED_PICTURE_BUFFER_MAX;
  }

  return DECODED_PICTURE_BUFFER_MAX;
}

OMX_ALG_EDpbMode ConvertToOMXDecodedPictureBuffer(DecodedPictureBufferType const& mode)
{
  switch(mode)
  {
  case DECODED_PICTURE_BUFFER_NORMAL: return OMX_ALG_DPB_NORMAL;
  case DECODED_PICTURE_BUFFER_LOW_REFERENCE: return OMX_ALG_DPB_LOW_REF;
  case DECODED_PICTURE_BUFFER_MAX: // fallthrough
  default: return OMX_ALG_DPB_MAX;
  }

  return OMX_ALG_DPB_MAX;
}

static inline AVCProfileType ConvertToModuleAVCProfile(OMX_VIDEO_AVCPROFILETYPE const& profile)
{
  switch(static_cast<OMX_U32>(profile)) // all indexes are in OMX_U32
  {
  case OMX_VIDEO_AVCProfileBaseline: return AVC_PROFILE_BASELINE;
  case OMX_VIDEO_AVCProfileMain: return AVC_PROFILE_MAIN;
  case OMX_VIDEO_AVCProfileExtended: return AVC_PROFILE_EXTENDED;
  case OMX_VIDEO_AVCProfileHigh: return AVC_PROFILE_HIGH;
  case OMX_VIDEO_AVCProfileHigh10: return AVC_PROFILE_HIGH_10;
  case OMX_VIDEO_AVCProfileHigh422: return AVC_PROFILE_HIGH_422;
  case OMX_VIDEO_AVCProfileHigh444: return AVC_PROFILE_HIGH_444_PREDICTIVE;
  case OMX_ALG_VIDEO_AVCProfileConstrainedBaseline: return AVC_PROFILE_CONSTRAINED_BASELINE;
  case OMX_ALG_VIDEO_AVCProfileProgressiveHigh: return AVC_PROFILE_PROGRESSIVE_HIGH;
  case OMX_ALG_VIDEO_AVCProfileConstrainedHigh: return AVC_PROFILE_CONSTRAINED_HIGH;
  case OMX_ALG_VIDEO_AVCProfileHigh10_Intra: return AVC_PROFILE_HIGH_10_INTRA;
  case OMX_ALG_VIDEO_AVCProfileHigh422_Intra: return AVC_PROFILE_HIGH_422_INTRA;
  case OMX_ALG_VIDEO_AVCProfileHigh444_Intra: return AVC_PROFILE_HIGH_444_INTRA;
  case OMX_ALG_VIDEO_AVCProfileCAVLC444_Intra: return AVC_PROFILE_CAVLC_444_INTRA;
  case OMX_VIDEO_AVCProfileMax: return AVC_PROFILE_MAX;
  default: return AVC_PROFILE_MAX;
  }

  return AVC_PROFILE_MAX;
}

static inline int ConvertToModuleAVCLevel(OMX_VIDEO_AVCLEVELTYPE const& level)
{
  switch(level)
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
  default:

    if(level == static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel52))
      return 52;

    if(level == static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel60))
      return 60;

    if(level == static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel61))
      return 61;

    if(level == static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel62))
      return 62;
    return 0;
  }

  return 0;
}

ProfileLevelType ConvertToModuleAVCProfileLevel(OMX_VIDEO_AVCPROFILETYPE const& profile, OMX_VIDEO_AVCLEVELTYPE const& level)
{
  ProfileLevelType pf;
  pf.profile.avc = ConvertToModuleAVCProfile(profile);
  pf.level = ConvertToModuleAVCLevel(level);
  return pf;
}

OMX_VIDEO_AVCPROFILETYPE ConvertToOMXAVCProfile(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.profile.avc)
  {
  case AVC_PROFILE_BASELINE: return OMX_VIDEO_AVCProfileBaseline;
  case AVC_PROFILE_MAIN: return OMX_VIDEO_AVCProfileMain;
  case AVC_PROFILE_EXTENDED: return OMX_VIDEO_AVCProfileExtended;
  case AVC_PROFILE_HIGH: return OMX_VIDEO_AVCProfileHigh;
  case AVC_PROFILE_HIGH_10: return OMX_VIDEO_AVCProfileHigh10;
  case AVC_PROFILE_HIGH_422: return OMX_VIDEO_AVCProfileHigh422;
  case AVC_PROFILE_HIGH_444_PREDICTIVE: return OMX_VIDEO_AVCProfileHigh444;
  case AVC_PROFILE_CONSTRAINED_BASELINE: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileConstrainedBaseline);
  case AVC_PROFILE_PROGRESSIVE_HIGH: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileProgressiveHigh);
  case AVC_PROFILE_CONSTRAINED_HIGH: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileConstrainedHigh);
  case AVC_PROFILE_HIGH_10_INTRA: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileHigh10_Intra);
  case AVC_PROFILE_HIGH_422_INTRA: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileHigh422_Intra);
  case AVC_PROFILE_HIGH_444_INTRA: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileHigh444_Intra);
  case AVC_PROFILE_CAVLC_444_INTRA: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_AVCProfileCAVLC444_Intra);
  default: return OMX_VIDEO_AVCProfileMax;
  }

  return OMX_VIDEO_AVCProfileMax;
}

OMX_VIDEO_AVCLEVELTYPE ConvertToOMXAVCLevel(ProfileLevelType const& profileLevel)
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
};

LoopFilterType ConvertToModuleAVCLoopFilter(OMX_VIDEO_AVCLOOPFILTERTYPE const& loopFilter)
{
  switch(loopFilter)
  {
  case OMX_VIDEO_AVCLoopFilterDisable: return LOOP_FILTER_DISABLE;
  case OMX_VIDEO_AVCLoopFilterDisableSliceBoundary: return LOOP_FILTER_ENABLE;
  case OMX_VIDEO_AVCLoopFilterEnable: return LOOP_FILTER_ENABLE_CROSS_SLICE;
  case OMX_VIDEO_AVCLoopFilterMax:
  default: return LOOP_FILTER_MAX;
  }

  return LOOP_FILTER_MAX;
}

OMX_VIDEO_AVCLOOPFILTERTYPE ConvertToOMXAVCLoopFilter(LoopFilterType const& loopFilter)
{
  switch(loopFilter)
  {
  case LOOP_FILTER_DISABLE: return OMX_VIDEO_AVCLoopFilterDisable;
  case LOOP_FILTER_ENABLE: return OMX_VIDEO_AVCLoopFilterDisableSliceBoundary;
  case LOOP_FILTER_ENABLE_CROSS_SLICE: return OMX_VIDEO_AVCLoopFilterEnable;
  case LOOP_FILTER_ENABLE_CROSS_TILE: // Don't exist in AVC
  case LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE: // Don't exist in AVC
  case LOOP_FILTER_MAX: return OMX_VIDEO_AVCLoopFilterMax;
  default: return OMX_VIDEO_AVCLoopFilterMax;
  }

  return OMX_VIDEO_AVCLoopFilterMax;
}

int ConvertToModuleBFrames(OMX_U32 const& bFrames, OMX_U32 const& pFrames)
{
  return bFrames / (pFrames + 1);
}

int ConvertToModuleGopLength(OMX_U32 const& bFrames, OMX_U32 const& pFrames)
{
  return pFrames + bFrames + 1;
}

EntropyCodingType ConvertToModuleEntropyCoding(OMX_BOOL const& isCabac)
{
  if(isCabac == OMX_FALSE)
    return ENTROPY_CODING_CAVLC;
  return ENTROPY_CODING_CABAC;
}

int ConvertToModuleQPInitial(OMX_U32 const& qpI)
{
  return qpI;
}

int ConvertToModuleQPDeltaIP(OMX_U32 const& qpI, OMX_U32 const& qpP)
{
  return qpP - qpI;
}

int ConvertToModuleQPDeltaPB(OMX_U32 const& qpP, OMX_U32 const& qpB)
{
  return qpB - qpP;
}

int ConvertToModuleQPMin(OMX_S32 const& qpMin)
{
  return qpMin;
}

int ConvertToModuleQPMax(OMX_S32 const& qpMax)
{
  return qpMax;
}

QPControlType ConvertToModuleQPControl(OMX_ALG_EQpCtrlMode const& mode)
{
  switch(mode)
  {
  case OMX_ALG_UNIFORM_QP: return QP_UNIFORM;
  case OMX_ALG_ROI_QP: return QP_ROI;
  case OMX_ALG_AUTO_QP: return QP_AUTO;
  case OMX_ALG_MAX_QP: return QP_MAX;
  default: return QP_MAX;
  }

  return QP_MAX;
}

RateControlType ConvertToModuleControlRate(OMX_VIDEO_CONTROLRATETYPE const& mode)
{
  switch(static_cast<OMX_U32>(mode)) // all indexes are in OMX_U32
  {
  case OMX_Video_ControlRateDisable: return RATE_CONTROL_CONSTANT_QUANTIZATION;
  case OMX_Video_ControlRateConstant: return RATE_CONTROL_CONSTANT_BITRATE;
  case OMX_Video_ControlRateVariable: return RATE_CONTROL_VARIABLE_BITRATE;
  case OMX_ALG_Video_ControlRateLowLatency: return RATE_CONTROL_LOW_LATENCY;
  default: return RATE_CONTROL_MAX;
  }

  return RATE_CONTROL_MAX;
}

AspectRatioType ConvertToModuleAspectRatio(OMX_ALG_EAspectRatio const& aspectRatio)
{
  switch(aspectRatio)
  {
  case OMX_ALG_ASPECT_RATIO_NONE: return ASPECT_RATIO_NONE;
  case OMX_ALG_ASPECT_RATIO_4_3: return ASPECT_RATIO_4_3;
  case OMX_ALG_ASPECT_RATIO_16_9: return ASPECT_RATIO_16_9;
  case OMX_ALG_ASPECT_RATIO_AUTO: return ASPECT_RATIO_AUTO;
  case OMX_ALG_ASPECT_RATIO_MAX: return ASPECT_RATIO_MAX;
  default: return ASPECT_RATIO_MAX;
  }

  return ASPECT_RATIO_MAX;
}

GopControlType ConvertToModuleGopControl(OMX_ALG_EGopCtrlMode const& mode)
{
  switch(mode)
  {
  case OMX_ALG_GOP_MODE_DEFAULT: return GOP_CONTROL_DEFAULT;
  case OMX_ALG_GOP_MODE_PYRAMIDAL: return GOP_CONTROL_PYRAMIDAL;
  case OMX_ALG_GOP_MODE_LOW_DELAY_P: return GOP_CONTROL_LOW_DELAY_P;
  case OMX_ALG_GOP_MODE_LOW_DELAY_B: return GOP_CONTROL_LOW_DELAY_B;
  case OMX_ALG_GOP_MODE_MAX: return GOP_CONTROL_MAX;
  default: return GOP_CONTROL_MAX;
  }

  return GOP_CONTROL_MAX;
}

GdrType ConvertToModuleGdr(OMX_ALG_EGdrMode const& gdr)
{
  switch(gdr)
  {
  case OMX_ALG_GDR_OFF: return GDR_OFF;
  case OMX_ALG_GDR_HORIZONTAL: return GDR_HORTIZONTAL;
  case OMX_ALG_GDR_VERTICAL: return GDR_VERTICAL;
  case OMX_ALG_GDR_MAX: return GDR_MAX;
  default: return GDR_MAX;
  }

  return GDR_MAX;
}

RateControlOptionType ConvertToModuleDisableSceneChangeResilience(OMX_BOOL const& disable)
{
  return (disable == OMX_TRUE) ? RATE_CONTROL_OPTION_NONE : RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE;
}

ScalingListType ConvertToModuleScalingList(OMX_ALG_EScalingList const& scalingListMode)
{
  switch(scalingListMode)
  {
  case OMX_ALG_SCL_DEFAULT: return SCALING_LIST_DEFAULT;
  case OMX_ALG_SCL_FLAT: return SCALING_LIST_FLAT;
  case OMX_ALG_SCL_MAX: return SCALING_LIST_MAX;
  default: return SCALING_LIST_MAX;
  }

  return SCALING_LIST_MAX;
}

OMX_BOOL ConvertToOMXEntropyCoding(EntropyCodingType const& mode)
{
  switch(mode)
  {
  case ENTROPY_CODING_CABAC: return OMX_TRUE;
  case ENTROPY_CODING_CAVLC: return OMX_FALSE;
  case ENTROPY_CODING_MAX: return OMX_FALSE;
  default: return OMX_FALSE;
  }

  return OMX_FALSE;
}

OMX_U32 ConvertToOMXBFrames(Gop const& gop)
{
  assert(gop.b >= 0);
  return (gop.b * gop.length) / (1 + gop.b);
}

OMX_U32 ConvertToOMXPFrames(Gop const& gop)
{
  assert(gop.b >= 0);
  return (gop.b - gop.length + 1) / (-gop.b - 1);
}

OMX_U32 ConvertToOMXQpI(QPs const& qps)
{
  return qps.initial;
}

OMX_U32 ConvertToOMXQpP(QPs const& qps)
{
  return qps.deltaIP + qps.initial;
}

OMX_U32 ConvertToOMXQpB(QPs const& qps)
{
  return qps.deltaIP + qps.deltaPB + qps.initial;
}

OMX_S32 ConvertToOMXQpMin(QPs const& qps)
{
  return qps.min;
}

OMX_S32 ConvertToOMXQpMax(QPs const& qps)
{
  return qps.max;
}

OMX_ALG_EQpCtrlMode ConvertToOMXQpControl(QPs const& qps)
{
  switch(qps.mode)
  {
  case QP_UNIFORM: return OMX_ALG_UNIFORM_QP;
  case QP_AUTO: return OMX_ALG_AUTO_QP;
  case QP_MAX: return OMX_ALG_MAX_QP;
  default: return OMX_ALG_MAX_QP;
  }

  return OMX_ALG_MAX_QP;
}

OMX_VIDEO_CONTROLRATETYPE ConvertToOMXControlRate(RateControlType const& mode)
{
  switch(mode)
  {
  case RATE_CONTROL_CONSTANT_QUANTIZATION: return OMX_Video_ControlRateDisable;
  case RATE_CONTROL_CONSTANT_BITRATE: return OMX_Video_ControlRateConstant;
  case RATE_CONTROL_VARIABLE_BITRATE: return OMX_Video_ControlRateVariable;
  case RATE_CONTROL_LOW_LATENCY: return static_cast<OMX_VIDEO_CONTROLRATETYPE>(OMX_ALG_Video_ControlRateLowLatency);
  default: return OMX_Video_ControlRateMax;
  }

  return OMX_Video_ControlRateMax;
}

OMX_ALG_EAspectRatio ConvertToOMXAspectRatio(AspectRatioType const& aspectRatio)
{
  switch(aspectRatio)
  {
  case ASPECT_RATIO_NONE: return OMX_ALG_ASPECT_RATIO_NONE;
  case ASPECT_RATIO_4_3: return OMX_ALG_ASPECT_RATIO_4_3;
  case ASPECT_RATIO_16_9: return OMX_ALG_ASPECT_RATIO_16_9;
  case ASPECT_RATIO_AUTO: return OMX_ALG_ASPECT_RATIO_AUTO;
  case ASPECT_RATIO_MAX: return OMX_ALG_ASPECT_RATIO_MAX;
  default: return OMX_ALG_ASPECT_RATIO_MAX;
  }

  return OMX_ALG_ASPECT_RATIO_MAX;
}

OMX_ALG_EGopCtrlMode ConvertToOMXGopControl(GopControlType const& mode)
{
  switch(mode)
  {
  case GOP_CONTROL_DEFAULT: return OMX_ALG_GOP_MODE_DEFAULT;
  case GOP_CONTROL_PYRAMIDAL: return OMX_ALG_GOP_MODE_PYRAMIDAL;
  case GOP_CONTROL_LOW_DELAY_P: return OMX_ALG_GOP_MODE_LOW_DELAY_P;
  case GOP_CONTROL_LOW_DELAY_B: return OMX_ALG_GOP_MODE_LOW_DELAY_B;
  default: return OMX_ALG_GOP_MODE_MAX;
  }

  return OMX_ALG_GOP_MODE_MAX;
}

OMX_ALG_EGdrMode ConvertToOMXGdr(GdrType const& gdr)
{
  switch(gdr)
  {
  case GDR_OFF: return OMX_ALG_GDR_OFF;
  case GDR_VERTICAL: return OMX_ALG_GDR_VERTICAL;
  case GDR_HORTIZONTAL: return OMX_ALG_GDR_HORIZONTAL;
  default: return OMX_ALG_GDR_MAX;
  }

  return OMX_ALG_GDR_MAX;
}

OMX_BOOL ConvertToOMXDisableSceneChangeResilience(RateControlOptionType const& option)
{
  if(option == RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE)
    return OMX_FALSE; // Because it's bDisableSceneChangeResilience

  return OMX_TRUE;
}

OMX_ALG_EScalingList ConvertToOMXScalingList(ScalingListType const& scalingLisgt)
{
  switch(scalingLisgt)
  {
  case SCALING_LIST_DEFAULT: return OMX_ALG_SCL_DEFAULT;
  case SCALING_LIST_FLAT: return OMX_ALG_SCL_FLAT;
  case SCALING_LIST_MAX: return OMX_ALG_SCL_MAX;
  default: return OMX_ALG_SCL_MAX;
  }

  return OMX_ALG_SCL_MAX;
}

LoopFilterType ConvertToModuleHEVCLoopFilter(OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE const& loopFilter)
{
  switch(loopFilter)
  {
  case OMX_ALG_VIDEO_HEVCLoopFilterDisable: return LOOP_FILTER_DISABLE;
  case OMX_ALG_VIDEO_HEVCLoopFilterEnable: return LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSlice: return LOOP_FILTER_ENABLE_CROSS_TILE;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossTile: return LOOP_FILTER_ENABLE_CROSS_SLICE;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSliceAndTile: return LOOP_FILTER_ENABLE;
  case OMX_ALG_VIDEO_HEVCLoopFilterMax: return LOOP_FILTER_MAX;
  default: return LOOP_FILTER_MAX;
  }

  return LOOP_FILTER_MAX;
}

OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE ConvertToOMXHEVCLoopFilter(LoopFilterType const& loopFilter)
{
  switch(loopFilter)
  {
  case LOOP_FILTER_DISABLE: return OMX_ALG_VIDEO_HEVCLoopFilterDisable;
  case LOOP_FILTER_ENABLE: return OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSliceAndTile;
  case LOOP_FILTER_ENABLE_CROSS_SLICE: return OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossTile;
  case LOOP_FILTER_ENABLE_CROSS_TILE: return OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSlice;
  case LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE: return OMX_ALG_VIDEO_HEVCLoopFilterEnable;
  case LOOP_FILTER_MAX: return OMX_ALG_VIDEO_HEVCLoopFilterMax;
  default: return OMX_ALG_VIDEO_HEVCLoopFilterMax;
  }

  return OMX_ALG_VIDEO_HEVCLoopFilterMax;
}

static inline HEVCProfileType ConvertToModuleHEVCMainProfile(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile)
{
  switch(profile)
  {
    case OMX_ALG_VIDEO_HEVCProfileMain: return HEVC_PROFILE_MAIN;
    case OMX_ALG_VIDEO_HEVCProfileMain10: return HEVC_PROFILE_MAIN_10;
    case OMX_ALG_VIDEO_HEVCProfileMainStill: return HEVC_PROFILE_MAIN_STILL;
    case OMX_ALG_VIDEO_HEVCProfileMonochrome: return HEVC_PROFILE_MONOCHROME;
    case OMX_ALG_VIDEO_HEVCProfileMonochrome10: return HEVC_PROFILE_MONOCHROME_10;
    case OMX_ALG_VIDEO_HEVCProfileMonochrome12: return HEVC_PROFILE_MONOCHROME_12;
    case OMX_ALG_VIDEO_HEVCProfileMonochrome16: return HEVC_PROFILE_MONOCHROME_16;
    case OMX_ALG_VIDEO_HEVCProfileMain12: return HEVC_PROFILE_MAIN_12;
    case OMX_ALG_VIDEO_HEVCProfileMain422: return HEVC_PROFILE_MAIN_422;
    case OMX_ALG_VIDEO_HEVCProfileMain422_10: return HEVC_PROFILE_MAIN_422_10;
    case OMX_ALG_VIDEO_HEVCProfileMain422_12: return HEVC_PROFILE_MAIN_422_12;
    case OMX_ALG_VIDEO_HEVCProfileMain444: return HEVC_PROFILE_MAIN_444;
    case OMX_ALG_VIDEO_HEVCProfileMain444_10: return HEVC_PROFILE_MAIN_444_10;
    case OMX_ALG_VIDEO_HEVCProfileMain444_12: return HEVC_PROFILE_MAIN_444_12;
    case OMX_ALG_VIDEO_HEVCProfileMain_Intra: return HEVC_PROFILE_MAIN_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain10_Intra: return HEVC_PROFILE_MAIN_10_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain12_Intra: return HEVC_PROFILE_MAIN_12_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain422_Intra: return HEVC_PROFILE_MAIN_422_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain422_10_Intra: return HEVC_PROFILE_MAIN_422_10_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain422_12_Intra: return HEVC_PROFILE_MAIN_422_12_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain444_Intra: return HEVC_PROFILE_MAIN_444_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain444_10_Intra: return HEVC_PROFILE_MAIN_444_10_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain444_12_Intra: return HEVC_PROFILE_MAIN_444_12_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain444_16_Intra: return HEVC_PROFILE_MAIN_444_16_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMain444_Still: return HEVC_PROFILE_MAIN_444_STILL;
    case OMX_ALG_VIDEO_HEVCProfileMain444_16_Still: return HEVC_PROFILE_MAIN_444_16_STILL;
    case OMX_ALG_VIDEO_HEVCProfileHighThroughtPut444_16_Intra: return HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA;
    case OMX_ALG_VIDEO_HEVCProfileMax: return HEVC_PROFILE_MAX;
    default: return HEVC_PROFILE_MAX;
  }

  return HEVC_PROFILE_MAX;
}

static inline HEVCProfileType ConvertToModuleHEVCHighTierProfile(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile)
{
  switch(profile)
  {
    case OMX_ALG_VIDEO_HEVCProfileMain: return HEVC_PROFILE_MAIN_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain10: return HEVC_PROFILE_MAIN_10_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMainStill: return HEVC_PROFILE_MAIN_STILL_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMonochrome: return HEVC_PROFILE_MONOCHROME_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMonochrome10: return HEVC_PROFILE_MONOCHROME_10_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMonochrome12: return HEVC_PROFILE_MONOCHROME_12_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMonochrome16: return HEVC_PROFILE_MONOCHROME_16_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain12: return HEVC_PROFILE_MAIN_12_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain422: return HEVC_PROFILE_MAIN_422_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain422_10: return HEVC_PROFILE_MAIN_422_10_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain422_12: return HEVC_PROFILE_MAIN_422_12_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444: return HEVC_PROFILE_MAIN_444_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444_10: return HEVC_PROFILE_MAIN_444_10_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444_12: return HEVC_PROFILE_MAIN_444_12_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain_Intra: return HEVC_PROFILE_MAIN_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain10_Intra: return HEVC_PROFILE_MAIN_10_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain12_Intra: return HEVC_PROFILE_MAIN_12_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain422_Intra: return HEVC_PROFILE_MAIN_422_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain422_10_Intra: return HEVC_PROFILE_MAIN_422_10_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain422_12_Intra: return HEVC_PROFILE_MAIN_422_12_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444_Intra: return HEVC_PROFILE_MAIN_444_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444_10_Intra: return HEVC_PROFILE_MAIN_444_10_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444_12_Intra: return HEVC_PROFILE_MAIN_444_12_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444_16_Intra: return HEVC_PROFILE_MAIN_444_16_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444_Still: return HEVC_PROFILE_MAIN_444_STILL_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMain444_16_Still: return HEVC_PROFILE_MAIN_444_16_STILL_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileHighThroughtPut444_16_Intra: return HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA_HIGH_TIER;
    case OMX_ALG_VIDEO_HEVCProfileMax: return HEVC_PROFILE_MAX;
    default: return HEVC_PROFILE_MAX;
  }


  return HEVC_PROFILE_MAX;
}

static inline int ConvertToModuleHEVCLevel(OMX_ALG_VIDEO_HEVCLEVELTYPE const& level)
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

static inline bool IsMainTier(OMX_ALG_VIDEO_HEVCLEVELTYPE const& level)
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

ProfileLevelType ConvertToModuleHEVCProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile, OMX_ALG_VIDEO_HEVCLEVELTYPE const& level)
{
  ProfileLevelType pf;
  pf.profile.hevc = IsMainTier(level) ? ConvertToModuleHEVCMainProfile(profile) : ConvertToModuleHEVCHighTierProfile(profile);
  pf.level = ConvertToModuleHEVCLevel(level);
  return pf;
}

OMX_ALG_VIDEO_HEVCPROFILETYPE ConvertToOMXHEVCProfile(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.profile.hevc)
  {
  case HEVC_PROFILE_MAIN:
  case HEVC_PROFILE_MAIN_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain;
  case HEVC_PROFILE_MAIN_10:
  case HEVC_PROFILE_MAIN_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain10;
  case HEVC_PROFILE_MAIN_STILL:
  case HEVC_PROFILE_MAIN_STILL_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMainStill;
  case HEVC_PROFILE_MONOCHROME:
  case HEVC_PROFILE_MONOCHROME_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMonochrome;
  case HEVC_PROFILE_MONOCHROME_10:
  case HEVC_PROFILE_MONOCHROME_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMonochrome10;
  case HEVC_PROFILE_MONOCHROME_12:
  case HEVC_PROFILE_MONOCHROME_12_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMonochrome12;
  case HEVC_PROFILE_MONOCHROME_16:
  case HEVC_PROFILE_MONOCHROME_16_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMonochrome16;
  case HEVC_PROFILE_MAIN_12:
  case HEVC_PROFILE_MAIN_12_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain12;
  case HEVC_PROFILE_MAIN_422:
  case HEVC_PROFILE_MAIN_422_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422;
  case HEVC_PROFILE_MAIN_422_10:
  case HEVC_PROFILE_MAIN_422_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_10;
  case HEVC_PROFILE_MAIN_422_12:
  case HEVC_PROFILE_MAIN_422_12_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_12;
  case HEVC_PROFILE_MAIN_444:
  case HEVC_PROFILE_MAIN_444_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444;
  case HEVC_PROFILE_MAIN_444_10:
  case HEVC_PROFILE_MAIN_444_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_10;
  case HEVC_PROFILE_MAIN_444_12:
  case HEVC_PROFILE_MAIN_444_12_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_12;
  case HEVC_PROFILE_MAIN_INTRA:
  case HEVC_PROFILE_MAIN_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain_Intra;
  case HEVC_PROFILE_MAIN_10_INTRA:
  case HEVC_PROFILE_MAIN_10_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain10_Intra;
  case HEVC_PROFILE_MAIN_12_INTRA:
  case HEVC_PROFILE_MAIN_12_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain12_Intra;
  case HEVC_PROFILE_MAIN_422_INTRA:
  case HEVC_PROFILE_MAIN_422_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_Intra;
  case HEVC_PROFILE_MAIN_422_10_INTRA:
  case HEVC_PROFILE_MAIN_422_10_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_10_Intra;
  case HEVC_PROFILE_MAIN_422_12_INTRA:
  case HEVC_PROFILE_MAIN_422_12_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_12_Intra;
  case HEVC_PROFILE_MAIN_444_INTRA:
  case HEVC_PROFILE_MAIN_444_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_Intra;
  case HEVC_PROFILE_MAIN_444_10_INTRA:
  case HEVC_PROFILE_MAIN_444_10_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_10_Intra;
  case HEVC_PROFILE_MAIN_444_12_INTRA:
  case HEVC_PROFILE_MAIN_444_12_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_12_Intra;
  case HEVC_PROFILE_MAIN_444_16_INTRA:
  case HEVC_PROFILE_MAIN_444_16_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_16_Intra;
  case HEVC_PROFILE_MAIN_444_STILL:
  case HEVC_PROFILE_MAIN_444_STILL_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_Still;
  case HEVC_PROFILE_MAIN_444_16_STILL:
  case HEVC_PROFILE_MAIN_444_16_STILL_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain444_Still;
  case HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA:
  case HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileHighThroughtPut444_16_Intra;
  default: return OMX_ALG_VIDEO_HEVCProfileMax;
  }

  return OMX_ALG_VIDEO_HEVCProfileMax;
}

static inline OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertToOMXHEVCMainLevel(ProfileLevelType const& profileLevel)
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
  default: return OMX_ALG_VIDEO_HEVCLevelMax;
  }

  return OMX_ALG_VIDEO_HEVCLevelMax;
}

static inline OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertToOMXHEVCHighLevel(ProfileLevelType const& profileLevel)
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
  default: return OMX_ALG_VIDEO_HEVCLevelMax;
  }

  return OMX_ALG_VIDEO_HEVCLevelMax;
};

static inline bool IsMainTier(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.profile.hevc)
  {
  case HEVC_PROFILE_MAIN_HIGH_TIER:
  case HEVC_PROFILE_MAIN_10_HIGH_TIER:
  case HEVC_PROFILE_MAIN_422_HIGH_TIER:
  case HEVC_PROFILE_MAIN_422_10_HIGH_TIER:
  case HEVC_PROFILE_MAIN_STILL_HIGH_TIER: return false;
  default: return true;
  }

  return true;
}

OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertToOMXHEVCLevel(ProfileLevelType const& profileLevel)
{
  return IsMainTier(profileLevel) ? ConvertToOMXHEVCMainLevel(profileLevel) : ConvertToOMXHEVCHighLevel(profileLevel);
}

BufferModeType ConvertToModuleBufferMode(OMX_ALG_VIDEO_BUFFER_MODE const& mode)
{
  switch(mode)
  {
  case OMX_ALG_VIDEO_BUFFER_MODE_FRAME: return BUFFER_MODE_FRAME;
  case OMX_ALG_VIDEO_BUFFER_MODE_FRAME_NO_REORDERING: return BUFFER_MODE_FRAME_NO_REORDERING;
  case OMX_ALG_VIDEO_BUFFER_MODE_SLICE: return BUFFER_MODE_SLICE;
  case OMX_ALG_VIDEO_BUFFER_MODE_MAX: // fallthrough;
  default: return BUFFER_MODE_MAX;
  }

  return BUFFER_MODE_MAX;
}

OMX_ALG_VIDEO_BUFFER_MODE ConvertToOMXBufferMode(BufferModeType const& mode)
{
  switch(mode)
  {
  case BUFFER_MODE_FRAME: return OMX_ALG_VIDEO_BUFFER_MODE_FRAME;
  case BUFFER_MODE_FRAME_NO_REORDERING: return OMX_ALG_VIDEO_BUFFER_MODE_FRAME_NO_REORDERING;
  case BUFFER_MODE_SLICE: return OMX_ALG_VIDEO_BUFFER_MODE_SLICE;
  case BUFFER_MODE_MAX: // fallthrough
  default: return OMX_ALG_VIDEO_BUFFER_MODE_MAX;
  }

  return OMX_ALG_VIDEO_BUFFER_MODE_MAX;
}

QualityType ConvertToModuleQuality(OMX_ALG_ERoiQuality const& quality)
{
  switch(quality)
  {
  case OMX_ALG_ROI_QUALITY_HIGH: return REGION_OF_INTEREST_QUALITY_HIGH;
  case OMX_ALG_ROI_QUALITY_MEDIUM: return REGION_OF_INTEREST_QUALITY_MEDIUM;
  case OMX_ALG_ROI_QUALITY_LOW: return REGION_OF_INTEREST_QUALITY_LOW;
  case OMX_ALG_ROI_QUALITY_DONT_CARE: return REGION_OF_INTEREST_QUALITY_DONT_CARE;
  case OMX_ALG_ROI_QUALITY_MAX_ENUM: return REGION_OF_INTEREST_QUALITY_MAX_ENUM;
  default: return REGION_OF_INTEREST_QUALITY_MAX_ENUM;
  }

  return REGION_OF_INTEREST_QUALITY_MAX_ENUM;
}

