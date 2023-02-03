/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#include "omx_convert_omx_media.h"

#include <stdexcept>
#include <cassert>
#include <cmath>

using namespace std;

OMX_BOOL ConvertMediaToOMXBool(bool boolean)
{
  return (!boolean) ? OMX_FALSE : OMX_TRUE;
}

OMX_COLOR_FORMATTYPE ConvertMediaToOMXColor(ColorType color, int bitdepth)
{
  switch(bitdepth)
  {
  case 8:
    switch(color)
    {
    case ColorType::COLOR_400:
      return OMX_COLOR_FormatL8;
    case ColorType::COLOR_420:
      return OMX_COLOR_FormatYUV420SemiPlanar;
    case ColorType::COLOR_422:
      return OMX_COLOR_FormatYUV422SemiPlanar;
    case ColorType::COLOR_444:
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV444Planar8bit);
    case ColorType::COLOR_MAX_ENUM: return OMX_COLOR_FormatMax;
    default:
      throw invalid_argument("color");
    }

    break;

  case 10:
    switch(color)
    {
    case ColorType::COLOR_400:
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL10bitPacked);
    case ColorType::COLOR_420:
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked);
    case ColorType::COLOR_422:
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked);
    case ColorType::COLOR_MAX_ENUM: return OMX_COLOR_FormatMax;
    default:
      throw invalid_argument("color");
    }

    break;

  case 12:
    switch(color)
    {
    case ColorType::COLOR_400:
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL12bit);
    case ColorType::COLOR_420:
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar12bit);
    case ColorType::COLOR_422:
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar12bit);
    case ColorType::COLOR_444:
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV444Planar12bit);
    case ColorType::COLOR_MAX_ENUM: return OMX_COLOR_FormatMax;
    default:
      throw invalid_argument("color");
    }

    break;

  default:
    throw invalid_argument("bitdepth");
    break;
  }
}

OMX_VIDEO_CODINGTYPE ConvertMediaToOMXCompression(CompressionType compression)
{
  switch(compression)
  {
  case CompressionType::COMPRESSION_UNUSED: return OMX_VIDEO_CodingUnused;
  case CompressionType::COMPRESSION_MAX_ENUM: return OMX_VIDEO_CodingMax;
  case CompressionType::COMPRESSION_AVC: return OMX_VIDEO_CodingAVC;
  case CompressionType::COMPRESSION_MJPEG: return OMX_VIDEO_CodingMJPEG;
  case CompressionType::COMPRESSION_VP9: return static_cast<OMX_VIDEO_CODINGTYPE>(OMX_ALG_VIDEO_CodingVP9);
  case CompressionType::COMPRESSION_HEVC: return static_cast<OMX_VIDEO_CODINGTYPE>(OMX_ALG_VIDEO_CodingHEVC);
  default:
    throw invalid_argument("compression");
  }
}

OMX_U32 ConvertMediaToOMXFramerate(Clock clock)
{
  assert(clock.clockratio);
  auto f = ((clock.framerate * 1000.0) / clock.clockratio) * 65536.0;
  return ceil(f);
}

OMX_ALG_BUFFER_MODE ConvertMediaToOMXBufferHandle(BufferHandleType handle)
{
  if(handle == BufferHandleType::BUFFER_HANDLE_FD)
    return OMX_ALG_BUF_DMA;
  else
    return OMX_ALG_BUF_NORMAL;
}

bool ConvertOMXToMediaBool(OMX_BOOL boolean)
{
  if(boolean == OMX_FALSE)
    return false;
  return true;
}

CompressionType ConvertOMXToMediaCompression(OMX_VIDEO_CODINGTYPE coding)
{
  switch(static_cast<OMX_U32>(coding))
  {
  case OMX_VIDEO_CodingUnused: return CompressionType::COMPRESSION_UNUSED;
  case OMX_VIDEO_CodingAVC: return CompressionType::COMPRESSION_AVC;
  case OMX_VIDEO_CodingMJPEG: return CompressionType::COMPRESSION_MJPEG;
  case OMX_ALG_VIDEO_CodingHEVC: return CompressionType::COMPRESSION_HEVC;
  case OMX_ALG_VIDEO_CodingVP9: return CompressionType::COMPRESSION_VP9;
  case OMX_VIDEO_CodingMax: return CompressionType::COMPRESSION_MAX_ENUM;
  default:
    throw invalid_argument("coding");
  }

  throw invalid_argument("coding");
}

ColorType ConvertOMXToMediaColor(OMX_COLOR_FORMATTYPE format)
{
  switch(static_cast<OMX_U32>(format))
  {
  case OMX_COLOR_FormatL8:
  case OMX_ALG_COLOR_FormatL10bitPacked:
  case OMX_ALG_COLOR_FormatL10bit:
  case OMX_ALG_COLOR_FormatL12bit:
    return ColorType::COLOR_400;

  case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bit:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar12bit:
    return ColorType::COLOR_420;

  case OMX_COLOR_FormatYUV422SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bit:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar12bit:
    return ColorType::COLOR_422;

  case OMX_ALG_COLOR_FormatYUV444Planar8bit:
  case OMX_ALG_COLOR_FormatYUV444Planar10bit:
  case OMX_ALG_COLOR_FormatYUV444Planar12bit:
    return ColorType::COLOR_444;

  case OMX_COLOR_FormatMax: return ColorType::COLOR_MAX_ENUM;
  default:
    throw invalid_argument("format");
  }

  throw invalid_argument("format");
}

int ConvertOMXToMediaBitdepth(OMX_COLOR_FORMATTYPE format)
{
  switch(static_cast<OMX_U32>(format))
  {
  case OMX_COLOR_FormatL8:
  case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_COLOR_FormatYUV422SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV444Planar8bit:
    return 8;
  case OMX_ALG_COLOR_FormatL10bitPacked:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked:
  case OMX_ALG_COLOR_FormatL10bit:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bit:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bit:
  case OMX_ALG_COLOR_FormatYUV444Planar10bit:
    return 10;
  case OMX_ALG_COLOR_FormatL12bit:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar12bit:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar12bit:
  case OMX_ALG_COLOR_FormatYUV444Planar12bit:
    return 12;
  case OMX_COLOR_FormatUnused: return 0; // XXX
  default:
    throw invalid_argument("color");
  }

  throw invalid_argument("color");
}

Clock ConvertOMXToMediaClock(OMX_U32 framerateInQ16)
{
  Clock clock {};
  clock.framerate = ceil(framerateInQ16 / 65536.0);
  clock.clockratio = rint((clock.framerate * 1000.0 * 65536.0) / framerateInQ16);

  if(clock.framerate == 0)
    clock.clockratio = 1000;
  return clock;
}

BufferHandleType ConvertOMXToMediaBufferHandle(OMX_ALG_BUFFER_MODE bufferMode)
{
  switch(bufferMode)
  {
  case OMX_ALG_BUF_NORMAL: return BufferHandleType::BUFFER_HANDLE_CHAR_PTR;
  case OMX_ALG_BUF_DMA: return BufferHandleType::BUFFER_HANDLE_FD;
  case OMX_ALG_BUF_MAX_ENUM: // fallthrough
  default:
    throw invalid_argument("bufferMode");
  }

  throw invalid_argument("bufferMode");
}

DecodedPictureBufferType ConvertOMXToMediaDecodedPictureBuffer(OMX_ALG_EDpbMode mode)
{
  switch(mode)
  {
  case OMX_ALG_DPB_NORMAL: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NORMAL;
  case OMX_ALG_DPB_NO_REORDERING: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NO_REORDERING;
  case OMX_ALG_DPB_MAX_ENUM: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
  default: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
  }

  return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
}

OMX_ALG_EDpbMode ConvertMediaToOMXDecodedPictureBuffer(DecodedPictureBufferType mode)
{
  switch(mode)
  {
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NORMAL: return OMX_ALG_DPB_NORMAL;
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NO_REORDERING: return OMX_ALG_DPB_NO_REORDERING;
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM: return OMX_ALG_DPB_MAX_ENUM;
  default: return OMX_ALG_DPB_MAX_ENUM;
  }

  return OMX_ALG_DPB_MAX_ENUM;
}

static inline AVCProfileType ConvertOMXToMediaAVCProfile(OMX_VIDEO_AVCPROFILETYPE profile)
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
  case OMX_ALG_VIDEO_XAVCProfileHigh10_Intra_CBG: return AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_CBG;
  case OMX_ALG_VIDEO_XAVCProfileHigh10_Intra_VBR: return AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_VBR;
  case OMX_ALG_VIDEO_XAVCProfileHigh422_Intra_CBG: return AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_CBG;
  case OMX_ALG_VIDEO_XAVCProfileHigh422_Intra_VBR: return AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_VBR;
  case OMX_ALG_VIDEO_XAVCProfileLongGopMain_MP4: return AVCProfileType::XAVC_PROFILE_LONG_GOP_MAIN_MP4;
  case OMX_ALG_VIDEO_XAVCProfileLongGopHigh_MP4: return AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MP4;
  case OMX_ALG_VIDEO_XAVCProfileLongGopHigh_MXF: return AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MXF;
  case OMX_ALG_VIDEO_XAVCProfileLongGopHigh422_MXF: return AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_422_MXF;
  case OMX_VIDEO_AVCProfileMax: return AVCProfileType::AVC_PROFILE_MAX_ENUM;
  default: return AVCProfileType::AVC_PROFILE_MAX_ENUM;
  }

  return AVCProfileType::AVC_PROFILE_MAX_ENUM;
}

static inline int ConvertOMXToMediaAVCLevel(OMX_VIDEO_AVCLEVELTYPE level)
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

ProfileLevel ConvertOMXToMediaAVCProfileLevel(OMX_VIDEO_AVCPROFILETYPE profile, OMX_VIDEO_AVCLEVELTYPE level)
{
  ProfileLevel pf;
  pf.profile.avc = ConvertOMXToMediaAVCProfile(profile);
  pf.level = ConvertOMXToMediaAVCLevel(level);
  return pf;
}

OMX_VIDEO_AVCPROFILETYPE ConvertMediaToOMXAVCProfile(ProfileLevel profileLevel)
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
  case AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_CBG: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_XAVCProfileHigh10_Intra_CBG);
  case AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_VBR: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_XAVCProfileHigh10_Intra_VBR);
  case AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_CBG: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_XAVCProfileHigh422_Intra_CBG);
  case AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_VBR: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_XAVCProfileHigh422_Intra_VBR);
  case AVCProfileType::XAVC_PROFILE_LONG_GOP_MAIN_MP4: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_XAVCProfileLongGopMain_MP4);
  case AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MP4: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_XAVCProfileLongGopHigh_MP4);
  case AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MXF: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_XAVCProfileLongGopHigh_MXF);
  case AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_422_MXF: return static_cast<OMX_VIDEO_AVCPROFILETYPE>(OMX_ALG_VIDEO_XAVCProfileLongGopHigh422_MXF);
  default: return OMX_VIDEO_AVCProfileMax;
  }

  return OMX_VIDEO_AVCProfileMax;
}

OMX_VIDEO_AVCLEVELTYPE ConvertMediaToOMXAVCLevel(ProfileLevel profileLevel)
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

LoopFilterType ConvertOMXToMediaAVCLoopFilter(OMX_VIDEO_AVCLOOPFILTERTYPE loopFilter)
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

OMX_VIDEO_AVCLOOPFILTERTYPE ConvertMediaToOMXAVCLoopFilter(LoopFilterType loopFilter)
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

int ConvertOMXToMediaBFrames(OMX_U32 bFrames, OMX_U32 pFrames)
{
  if(pFrames == 0)
    return 0;

  return (bFrames + pFrames) / (pFrames + 1);
}

int ConvertOMXToMediaGopLength(OMX_U32 bFrames, OMX_U32 pFrames)
{
  return pFrames + bFrames + 1;
}

EntropyCodingType ConvertOMXToMediaEntropyCoding(OMX_BOOL isCabac)
{
  if(isCabac == OMX_FALSE)
    return EntropyCodingType::ENTROPY_CODING_CAVLC;
  return EntropyCodingType::ENTROPY_CODING_CABAC;
}

int ConvertOMXToMediaQpInitial(OMX_U32 qpI)
{
  return qpI;
}

int ConvertOMXToMediaQpDeltaIP(OMX_U32 qpI, OMX_U32 qpP)
{
  return qpP - qpI;
}

int ConvertOMXToMediaQpDeltaPB(OMX_U32 qpP, OMX_U32 qpB)
{
  return qpB - qpP;
}

int ConvertOMXToMediaQpMin(OMX_S32 qpMin)
{
  return qpMin;
}

int ConvertOMXToMediaQpMax(OMX_S32 qpMax)
{
  return qpMax;
}

QPControlType ConvertOMXToMediaQpCtrlMode(OMX_ALG_EQpCtrlMode mode)
{
  switch(mode)
  {
  case OMX_ALG_QP_CTRL_NONE: return QPControlType::QP_CONTROL_NONE;
  case OMX_ALG_QP_CTRL_AUTO: return QPControlType::QP_CONTROL_AUTO;
  case OMX_ALG_QP_CTRL_ADAPTIVE_AUTO: return QPControlType::QP_CONTROL_ADAPTIVE_AUTO;
  case OMX_ALG_QP_CTRL_MAX_ENUM: return QPControlType::QP_CONTROL_MAX_ENUM;
  default: return QPControlType::QP_CONTROL_MAX_ENUM;
  }

  return QPControlType::QP_CONTROL_MAX_ENUM;
}

RateControlType ConvertOMXToMediaControlRate(OMX_VIDEO_CONTROLRATETYPE mode)
{
  switch(static_cast<OMX_U32>(mode)) // all indexes are in OMX_U32
  {
  case OMX_Video_ControlRateDisable: return RateControlType::RATE_CONTROL_CONSTANT_QUANTIZATION;
  case OMX_Video_ControlRateConstant: return RateControlType::RATE_CONTROL_CONSTANT_BITRATE;
  case OMX_Video_ControlRateVariable: return RateControlType::RATE_CONTROL_VARIABLE_BITRATE;
  case OMX_ALG_Video_ControlRateVariableCapped: return RateControlType::RATE_CONTROL_VARIABLE_CAPPED_BITRATE;
  case OMX_ALG_Video_ControlRateLowLatency: return RateControlType::RATE_CONTROL_LOW_LATENCY;
  case OMX_ALG_Video_ControlRatePlugin: return RateControlType::RATE_CONTROL_PLUGIN;
  case OMX_ALG_Video_ControlRateMaxEnum: return RateControlType::RATE_CONTROL_MAX_ENUM;
  default: return RateControlType::RATE_CONTROL_MAX_ENUM;
  }

  return RateControlType::RATE_CONTROL_MAX_ENUM;
}

AspectRatioType ConvertOMXToMediaAspectRatio(OMX_ALG_EAspectRatio aspectRatio)
{
  switch(aspectRatio)
  {
  case OMX_ALG_ASPECT_RATIO_NONE: return AspectRatioType::ASPECT_RATIO_NONE;
  case OMX_ALG_ASPECT_RATIO_1_1: return AspectRatioType::ASPECT_RATIO_1_1;
  case OMX_ALG_ASPECT_RATIO_4_3: return AspectRatioType::ASPECT_RATIO_4_3;
  case OMX_ALG_ASPECT_RATIO_16_9: return AspectRatioType::ASPECT_RATIO_16_9;
  case OMX_ALG_ASPECT_RATIO_AUTO: return AspectRatioType::ASPECT_RATIO_AUTO;
  case OMX_ALG_ASPECT_RATIO_MAX_ENUM: return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
  default: return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
  }

  return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
}

GopControlType ConvertOMXToMediaGopControl(OMX_ALG_EGopCtrlMode mode)
{
  switch(mode)
  {
  case OMX_ALG_GOP_MODE_DEFAULT: return GopControlType::GOP_CONTROL_DEFAULT;
  case OMX_ALG_GOP_MODE_DEFAULT_B: return GopControlType::GOP_CONTROL_DEFAULT_B;
  case OMX_ALG_GOP_MODE_PYRAMIDAL: return GopControlType::GOP_CONTROL_PYRAMIDAL;
  case OMX_ALG_GOP_MODE_PYRAMIDAL_B: return GopControlType::GOP_CONTROL_PYRAMIDAL_B;
  case OMX_ALG_GOP_MODE_ADAPTIVE: return GopControlType::GOP_CONTROL_ADAPTIVE;
  case OMX_ALG_GOP_MODE_LOW_DELAY_P: return GopControlType::GOP_CONTROL_LOW_DELAY_P;
  case OMX_ALG_GOP_MODE_LOW_DELAY_B: return GopControlType::GOP_CONTROL_LOW_DELAY_B;
  case OMX_ALG_GOP_MODE_MAX_ENUM: return GopControlType::GOP_CONTROL_MAX_ENUM;
  default: return GopControlType::GOP_CONTROL_MAX_ENUM;
  }

  return GopControlType::GOP_CONTROL_MAX_ENUM;
}

GdrType ConvertOMXToMediaGdr(OMX_ALG_EGdrMode gdr)
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

ScalingListType ConvertOMXToMediaScalingList(OMX_ALG_EScalingList scalingListMode)
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

OMX_BOOL ConvertMediaToOMXEntropyCoding(EntropyCodingType mode)
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

OMX_U32 ConvertMediaToOMXBFrames(Gop gop)
{
  return gop.length - ConvertMediaToOMXPFrames(gop) - 1;
}

OMX_U32 ConvertMediaToOMXPFrames(Gop gop)
{
  return (gop.length - 1) / (gop.b + 1);
}

OMX_U32 ConvertMediaToOMXQpI(QPs qps)
{
  return qps.initial;
}

OMX_U32 ConvertMediaToOMXQpP(QPs qps)
{
  return qps.deltaIP + qps.initial;
}

OMX_U32 ConvertMediaToOMXQpB(QPs qps)
{
  return qps.deltaIP + qps.deltaPB + qps.initial;
}

OMX_S32 ConvertMediaToOMXQpMin(QPs qps)
{
  return qps.range[0].min;
}

OMX_S32 ConvertMediaToOMXQpMax(QPs qps)
{
  return qps.range[0].max;
}

OMX_ALG_EQpCtrlMode ConvertMediaToOMXQpCtrlMode(QPControlType mode)
{
  switch(mode)
  {
  case QPControlType::QP_CONTROL_NONE: return OMX_ALG_QP_CTRL_NONE;
  case QPControlType::QP_CONTROL_AUTO: return OMX_ALG_QP_CTRL_AUTO;
  case QPControlType::QP_CONTROL_ADAPTIVE_AUTO: return OMX_ALG_QP_CTRL_ADAPTIVE_AUTO;
  case QPControlType::QP_CONTROL_MAX_ENUM: return OMX_ALG_QP_CTRL_MAX_ENUM;
  default: return OMX_ALG_QP_CTRL_MAX_ENUM;
  }

  return OMX_ALG_QP_CTRL_MAX_ENUM;
}

OMX_VIDEO_CONTROLRATETYPE ConvertMediaToOMXControlRate(RateControlType mode)
{
  switch(mode)
  {
  case RateControlType::RATE_CONTROL_CONSTANT_QUANTIZATION: return OMX_Video_ControlRateDisable;
  case RateControlType::RATE_CONTROL_CONSTANT_BITRATE: return OMX_Video_ControlRateConstant;
  case RateControlType::RATE_CONTROL_VARIABLE_BITRATE: return OMX_Video_ControlRateVariable;
  case RateControlType::RATE_CONTROL_VARIABLE_CAPPED_BITRATE: return static_cast<OMX_VIDEO_CONTROLRATETYPE>(OMX_ALG_Video_ControlRateVariableCapped);
  case RateControlType::RATE_CONTROL_LOW_LATENCY: return static_cast<OMX_VIDEO_CONTROLRATETYPE>(OMX_ALG_Video_ControlRateLowLatency);
  case RateControlType::RATE_CONTROL_PLUGIN: return static_cast<OMX_VIDEO_CONTROLRATETYPE>(OMX_ALG_Video_ControlRatePlugin);
  case RateControlType::RATE_CONTROL_MAX_ENUM: return OMX_Video_ControlRateMax;
  default: return OMX_Video_ControlRateMax;
  }

  return OMX_Video_ControlRateMax;
}

OMX_ALG_EAspectRatio ConvertMediaToOMXAspectRatio(AspectRatioType aspectRatio)
{
  switch(aspectRatio)
  {
  case AspectRatioType::ASPECT_RATIO_NONE: return OMX_ALG_ASPECT_RATIO_NONE;
  case AspectRatioType::ASPECT_RATIO_1_1: return OMX_ALG_ASPECT_RATIO_1_1;
  case AspectRatioType::ASPECT_RATIO_4_3: return OMX_ALG_ASPECT_RATIO_4_3;
  case AspectRatioType::ASPECT_RATIO_16_9: return OMX_ALG_ASPECT_RATIO_16_9;
  case AspectRatioType::ASPECT_RATIO_AUTO: return OMX_ALG_ASPECT_RATIO_AUTO;
  case AspectRatioType::ASPECT_RATIO_MAX_ENUM: return OMX_ALG_ASPECT_RATIO_MAX_ENUM;
  default: return OMX_ALG_ASPECT_RATIO_MAX_ENUM;
  }

  return OMX_ALG_ASPECT_RATIO_MAX_ENUM;
}

OMX_ALG_EGopCtrlMode ConvertMediaToOMXGopControl(GopControlType mode)
{
  switch(mode)
  {
  case GopControlType::GOP_CONTROL_DEFAULT: return OMX_ALG_GOP_MODE_DEFAULT;
  case GopControlType::GOP_CONTROL_DEFAULT_B: return OMX_ALG_GOP_MODE_DEFAULT_B;
  case GopControlType::GOP_CONTROL_PYRAMIDAL: return OMX_ALG_GOP_MODE_PYRAMIDAL;
  case GopControlType::GOP_CONTROL_PYRAMIDAL_B: return OMX_ALG_GOP_MODE_PYRAMIDAL_B;
  case GopControlType::GOP_CONTROL_ADAPTIVE: return OMX_ALG_GOP_MODE_ADAPTIVE;
  case GopControlType::GOP_CONTROL_LOW_DELAY_P: return OMX_ALG_GOP_MODE_LOW_DELAY_P;
  case GopControlType::GOP_CONTROL_LOW_DELAY_B: return OMX_ALG_GOP_MODE_LOW_DELAY_B;
  default: return OMX_ALG_GOP_MODE_MAX_ENUM;
  }

  return OMX_ALG_GOP_MODE_MAX_ENUM;
}

OMX_ALG_EGdrMode ConvertMediaToOMXGdr(GdrType gdr)
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

OMX_ALG_EScalingList ConvertMediaToOMXScalingList(ScalingListType scalingLisgt)
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

LoopFilterType ConvertOMXToMediaHEVCLoopFilter(OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE loopFilter)
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

OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE ConvertMediaToOMXHEVCLoopFilter(LoopFilterType loopFilter)
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

static inline HEVCProfileType ConvertOMXToMediaHEVCMainTierProfile(OMX_ALG_VIDEO_HEVCPROFILETYPE profile)
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

static inline HEVCProfileType ConvertOMXToMediaHEVCHighTierProfile(OMX_ALG_VIDEO_HEVCPROFILETYPE profile)
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

static inline int ConvertOMXToMediaHEVCLevel(OMX_ALG_VIDEO_HEVCLEVELTYPE level)
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
  case OMX_ALG_VIDEO_HEVCMainTierLevel1:
  case OMX_ALG_VIDEO_HEVCMainTierLevel2:
  case OMX_ALG_VIDEO_HEVCMainTierLevel21:
  case OMX_ALG_VIDEO_HEVCMainTierLevel3:
  case OMX_ALG_VIDEO_HEVCMainTierLevel31:
  case OMX_ALG_VIDEO_HEVCMainTierLevel4:
  case OMX_ALG_VIDEO_HEVCMainTierLevel41:
  case OMX_ALG_VIDEO_HEVCMainTierLevel5:
  case OMX_ALG_VIDEO_HEVCMainTierLevel51:
  case OMX_ALG_VIDEO_HEVCMainTierLevel52:
  case OMX_ALG_VIDEO_HEVCMainTierLevel6:
  case OMX_ALG_VIDEO_HEVCMainTierLevel61:
  case OMX_ALG_VIDEO_HEVCMainTierLevel62:
    return true;
  default: return false;
  }

  return false;
}

ProfileLevel ConvertOMXToMediaHEVCProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE profile, OMX_ALG_VIDEO_HEVCLEVELTYPE level)
{
  ProfileLevel pf;
  pf.profile.hevc = IsMainTier(level) ? ConvertOMXToMediaHEVCMainTierProfile(profile) : ConvertOMXToMediaHEVCHighTierProfile(profile);
  pf.level = ConvertOMXToMediaHEVCLevel(level);
  return pf;
}

OMX_ALG_VIDEO_HEVCPROFILETYPE ConvertMediaToOMXHEVCProfile(ProfileLevel profileLevel)
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

static inline OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertMediaToOMXHEVCMainLevel(ProfileLevel profileLevel)
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

static inline OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertMediaToOMXHEVCHighLevel(ProfileLevel profileLevel)
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

static inline bool IsMainTier(ProfileLevel profileLevel)
{
  switch(profileLevel.profile.hevc)
  {
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_10:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_12:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_16:
  case HEVCProfileType::HEVC_PROFILE_MAIN:
  case HEVCProfileType::HEVC_PROFILE_MAIN_10:
  case HEVCProfileType::HEVC_PROFILE_MAIN_12:
  case HEVCProfileType::HEVC_PROFILE_MAIN_16:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_12:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_16:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_10:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_12:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16:
  case HEVCProfileType::HEVC_PROFILE_MAIN_STILL:
  case HEVCProfileType::HEVC_PROFILE_MAIN_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_12_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_16_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_12_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_16_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_10_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_12_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16_INTRA:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_STILL:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16_STILL:
  case HEVCProfileType::HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA:
    return true;
  default: return false;
  }

  return false;
}

OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertMediaToOMXHEVCLevel(ProfileLevel profileLevel)
{
  return IsMainTier(profileLevel) ? ConvertMediaToOMXHEVCMainLevel(profileLevel) : ConvertMediaToOMXHEVCHighLevel(profileLevel);
}

BufferModeType ConvertOMXToMediaBufferMode(OMX_ALG_VIDEO_BUFFER_MODE mode)
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

OMX_ALG_VIDEO_BUFFER_MODE ConvertMediaToOMXBufferMode(BufferModeType mode)
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

QualityType ConvertOMXToMediaQualityPreset(OMX_ALG_ERoiQuality quality)
{
  switch(quality)
  {
  case OMX_ALG_ROI_QUALITY_HIGH: return QualityType::REGION_OF_INTEREST_QUALITY_HIGH;
  case OMX_ALG_ROI_QUALITY_MEDIUM: return QualityType::REGION_OF_INTEREST_QUALITY_MEDIUM;
  case OMX_ALG_ROI_QUALITY_LOW: return QualityType::REGION_OF_INTEREST_QUALITY_LOW;
  case OMX_ALG_ROI_QUALITY_DONT_CARE: return QualityType::REGION_OF_INTEREST_QUALITY_DONT_CARE;
  case OMX_ALG_ROI_QUALITY_INTRA: return QualityType::REGION_OF_INTEREST_QUALITY_INTRA;
  case OMX_ALG_ROI_QUALITY_MAX_ENUM: return QualityType::REGION_OF_INTEREST_QUALITY_MAX_ENUM;
  default: return QualityType::REGION_OF_INTEREST_QUALITY_MAX_ENUM;
  }

  return QualityType::REGION_OF_INTEREST_QUALITY_MAX_ENUM;
}

int ConvertOMXToMediaQualityValue(OMX_S32 quality)
{
  return static_cast<int>(quality);
}

OMX_U32 ConvertMediaToOMXInterlaceFlag(VideoModeType mode)
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

VideoModeType ConvertOMXToMediaVideoMode(OMX_U32 flag)
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

SequencePictureModeType ConvertOMXToMediaSequencePictureMode(OMX_ALG_SEQUENCE_PICTURE_MODE mode)
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

OMX_ALG_SEQUENCE_PICTURE_MODE ConvertMediaToOMXSequencePictureMode(SequencePictureModeType mode)
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

OMX_ALG_VIDEO_COLOR_PRIMARIESTYPE ConvertMediaToOMXColorPrimaries(ColorPrimariesType colorPrimaries)
{
  switch(colorPrimaries)
  {
  case ColorPrimariesType::COLOR_PRIMARIES_UNSPECIFIED: return OMX_ALG_VIDEO_COLOR_PRIMARIES_UNSPECIFIED;
  case ColorPrimariesType::COLOR_PRIMARIES_RESERVED: return OMX_ALG_VIDEO_COLOR_PRIMARIES_RESERVED;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_470_NTSC: return OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_470_NTSC;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_601_NTSC: return OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_601_NTSC;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_601_PAL: return OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_601_PAL;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_709: return OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_709;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_2020: return OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_2020;
  case ColorPrimariesType::COLOR_PRIMARIES_SMPTE_240M: return OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_240M;
  case ColorPrimariesType::COLOR_PRIMARIES_SMPTE_ST_428: return OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_ST_428;
  case ColorPrimariesType::COLOR_PRIMARIES_SMPTE_RP_431: return OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_RP_431;
  case ColorPrimariesType::COLOR_PRIMARIES_SMPTE_EG_432: return OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_EG_432;
  case ColorPrimariesType::COLOR_PRIMARIES_EBU_3213: return OMX_ALG_VIDEO_COLOR_PRIMARIES_EBU_3213;
  case ColorPrimariesType::COLOR_PRIMARIES_GENERIC_FILM: return OMX_ALG_VIDEO_COLOR_PRIMARIES_GENERIC_FILM;
  case ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM: return OMX_ALG_VIDEO_COLOR_PRIMARIES_MAX_ENUM;
  default: return OMX_ALG_VIDEO_COLOR_PRIMARIES_MAX_ENUM;
  }

  return OMX_ALG_VIDEO_COLOR_PRIMARIES_MAX_ENUM;
}

ColorPrimariesType ConvertOMXToMediaColorPrimaries(OMX_ALG_VIDEO_COLOR_PRIMARIESTYPE colorPrimaries)
{
  switch(colorPrimaries)
  {
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_UNSPECIFIED: return ColorPrimariesType::COLOR_PRIMARIES_UNSPECIFIED;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_RESERVED: return ColorPrimariesType::COLOR_PRIMARIES_RESERVED;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_470_NTSC: return ColorPrimariesType::COLOR_PRIMARIES_BT_470_NTSC;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_601_NTSC: return ColorPrimariesType::COLOR_PRIMARIES_BT_601_NTSC;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_601_PAL: return ColorPrimariesType::COLOR_PRIMARIES_BT_601_PAL;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_709: return ColorPrimariesType::COLOR_PRIMARIES_BT_709;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_2020: return ColorPrimariesType::COLOR_PRIMARIES_BT_2020;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_240M: return ColorPrimariesType::COLOR_PRIMARIES_SMPTE_240M;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_ST_428: return ColorPrimariesType::COLOR_PRIMARIES_SMPTE_ST_428;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_RP_431: return ColorPrimariesType::COLOR_PRIMARIES_SMPTE_RP_431;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_EG_432: return ColorPrimariesType::COLOR_PRIMARIES_SMPTE_EG_432;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_EBU_3213: return ColorPrimariesType::COLOR_PRIMARIES_EBU_3213;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_GENERIC_FILM: return ColorPrimariesType::COLOR_PRIMARIES_GENERIC_FILM;
  case OMX_ALG_VIDEO_COLOR_PRIMARIES_MAX_ENUM: return ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM;
  default: return ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM;
  }

  return ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM;
}

OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS ConvertMediaToOMXTransferCharacteristics(TransferCharacteristicsType transferCharac)
{
  switch(transferCharac)
  {
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_709: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_709;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_UNSPECIFIED: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_UNSPECIFIED;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_M: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_M;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_B: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_B;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_601: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_601;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_SMPTE_240M: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_SMPTE_240M;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LINEAR: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_LINEAR;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LOG: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_LOG;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LOG_EXTENDED: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_LOG_EXTENDED;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_IEC_61966_2_4: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_IEC_61966_2_4;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_1361: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_1361;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_IEC_61966_2_1: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_IEC_61966_2_1;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2020_10B: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_2020_10B;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2020_12B: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_2020_12B;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_PQ: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_2100_PQ;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_SMPTE_428: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_SMPTE_428;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_HLG: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_2100_HLG;
  default: return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_MAX_ENUM;
  }

  return OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_MAX_ENUM;
}

TransferCharacteristicsType ConvertOMXToMediaTransferCharacteristics(OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS transferCharac)
{
  switch(transferCharac)
  {
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_709: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_709;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_UNSPECIFIED: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_UNSPECIFIED;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_M: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_M;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_B: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_B;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_601: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_601;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_SMPTE_240M: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_SMPTE_240M;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_LINEAR: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LINEAR;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_LOG: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LOG;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_LOG_EXTENDED: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LOG_EXTENDED;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_IEC_61966_2_4: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_IEC_61966_2_4;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_1361: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_1361;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_IEC_61966_2_1: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_IEC_61966_2_1;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_2020_10B: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2020_10B;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_2020_12B: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2020_12B;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_2100_PQ: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_PQ;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_SMPTE_428: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_SMPTE_428;
  case OMX_ALG_VIDEO_TRANSFER_CHARACTERISTICS_BT_2100_HLG: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_HLG;
  default: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_MAX_ENUM;
  }

  return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_MAX_ENUM;
}

OMX_ALG_VIDEO_COLOR_MATRIX ConvertMediaToOMXColourMatrix(ColourMatrixType colourMatrix)
{
  switch(colourMatrix)
  {
  case ColourMatrixType::COLOUR_MATRIX_GBR: return OMX_ALG_VIDEO_COLOR_MATRIX_GBR;
  case ColourMatrixType::COLOUR_MATRIX_BT_709: return OMX_ALG_VIDEO_COLOR_MATRIX_BT_709;
  case ColourMatrixType::COLOUR_MATRIX_UNSPECIFIED: return OMX_ALG_VIDEO_COLOR_MATRIX_UNSPECIFIED;
  case ColourMatrixType::COLOUR_MATRIX_USFCC_CFR: return OMX_ALG_VIDEO_COLOR_MATRIX_USFCC_CFR;
  case ColourMatrixType::COLOUR_MATRIX_BT_601_625: return OMX_ALG_VIDEO_COLOR_MATRIX_BT_601_625;
  case ColourMatrixType::COLOUR_MATRIX_BT_601_525: return OMX_ALG_VIDEO_COLOR_MATRIX_BT_601_525;
  case ColourMatrixType::COLOUR_MATRIX_BT_SMPTE_240M: return OMX_ALG_VIDEO_COLOR_MATRIX_BT_SMPTE_240M;
  case ColourMatrixType::COLOUR_MATRIX_BT_YCGCO: return OMX_ALG_VIDEO_COLOR_MATRIX_BT_YCGCO;
  case ColourMatrixType::COLOUR_MATRIX_BT_2100_YCBCR: return OMX_ALG_VIDEO_COLOR_MATRIX_BT_2100_YCBCR;
  case ColourMatrixType::COLOUR_MATRIX_BT_2020_CLS: return OMX_ALG_VIDEO_COLOR_MATRIX_BT_2020_CLS;
  case ColourMatrixType::COLOUR_MATRIX_SMPTE_2085: return OMX_ALG_VIDEO_COLOR_MATRIX_SMPTE_2085;
  case ColourMatrixType::COLOUR_MATRIX_CHROMA_DERIVED_NCLS: return OMX_ALG_VIDEO_COLOR_MATRIX_CHROMA_DERIVED_NCLS;
  case ColourMatrixType::COLOUR_MATRIX_CHROMA_DERIVED_CLS: return OMX_ALG_VIDEO_COLOR_MATRIX_CHROMA_DERIVED_CLS;
  case ColourMatrixType::COLOUR_MATRIX_BT_2100_ICTCP: return OMX_ALG_VIDEO_COLOR_MATRIX_BT_2100_ICTCP;
  default: return OMX_ALG_VIDEO_COLOR_MATRIX_MAX_ENUM;
  }

  return OMX_ALG_VIDEO_COLOR_MATRIX_MAX_ENUM;
}

ColourMatrixType ConvertOMXToMediaColourMatrix(OMX_ALG_VIDEO_COLOR_MATRIX colourMatrix)
{
  switch(colourMatrix)
  {
  case OMX_ALG_VIDEO_COLOR_MATRIX_GBR: return ColourMatrixType::COLOUR_MATRIX_GBR;
  case OMX_ALG_VIDEO_COLOR_MATRIX_BT_709: return ColourMatrixType::COLOUR_MATRIX_BT_709;
  case OMX_ALG_VIDEO_COLOR_MATRIX_UNSPECIFIED: return ColourMatrixType::COLOUR_MATRIX_UNSPECIFIED;
  case OMX_ALG_VIDEO_COLOR_MATRIX_USFCC_CFR: return ColourMatrixType::COLOUR_MATRIX_USFCC_CFR;
  case OMX_ALG_VIDEO_COLOR_MATRIX_BT_601_625: return ColourMatrixType::COLOUR_MATRIX_BT_601_625;
  case OMX_ALG_VIDEO_COLOR_MATRIX_BT_601_525: return ColourMatrixType::COLOUR_MATRIX_BT_601_525;
  case OMX_ALG_VIDEO_COLOR_MATRIX_BT_SMPTE_240M: return ColourMatrixType::COLOUR_MATRIX_BT_SMPTE_240M;
  case OMX_ALG_VIDEO_COLOR_MATRIX_BT_YCGCO: return ColourMatrixType::COLOUR_MATRIX_BT_YCGCO;
  case OMX_ALG_VIDEO_COLOR_MATRIX_BT_2100_YCBCR: return ColourMatrixType::COLOUR_MATRIX_BT_2100_YCBCR;
  case OMX_ALG_VIDEO_COLOR_MATRIX_BT_2020_CLS: return ColourMatrixType::COLOUR_MATRIX_BT_2020_CLS;
  case OMX_ALG_VIDEO_COLOR_MATRIX_SMPTE_2085: return ColourMatrixType::COLOUR_MATRIX_SMPTE_2085;
  case OMX_ALG_VIDEO_COLOR_MATRIX_CHROMA_DERIVED_NCLS: return ColourMatrixType::COLOUR_MATRIX_CHROMA_DERIVED_NCLS;
  case OMX_ALG_VIDEO_COLOR_MATRIX_CHROMA_DERIVED_CLS: return ColourMatrixType::COLOUR_MATRIX_CHROMA_DERIVED_CLS;
  case OMX_ALG_VIDEO_COLOR_MATRIX_BT_2100_ICTCP: return ColourMatrixType::COLOUR_MATRIX_BT_2100_ICTCP;
  default: return ColourMatrixType::COLOUR_MATRIX_MAX_ENUM;
  }

  return ColourMatrixType::COLOUR_MATRIX_MAX_ENUM;
}

void ConvertMediaToOMX_DPL_ST2094_40(OMX_ALG_DISPLAY_PEAK_LUMINANCE_ST2094_40& dst, DisplayPeakLuminance_ST2094_40 const& src)
{
  dst.bActualPeakLuminanceFlag = ConvertMediaToOMXBool(src.actualPeakLuminanceFlag);

  if(src.actualPeakLuminanceFlag)
  {
    dst.nNumRowsActualPeakLuminance = src.numRowsActualPeakLuminance;
    dst.nNumColsActualPeakLuminance = src.numColsActualPeakLuminance;

    for(int i = 0; i < src.numRowsActualPeakLuminance; i++)
      for(int j = 0; j < src.numColsActualPeakLuminance; j++)
        dst.nActualPeakLuminance[i][j] = src.actualPeakLuminance[i][j];
  }
}

OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI ConvertMediaToOMXHDRSEI(HighDynamicRangeSeis const& hdrSEIs)
{
  OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI omxHDRSEIs;

  omxHDRSEIs.bHasMDCV = ConvertMediaToOMXBool(hdrSEIs.mdcv.enabled);

  if(hdrSEIs.mdcv.enabled)
  {
    for(int i = 0; i < 3; i++)
    {
      omxHDRSEIs.masteringDisplayColourVolume.displayPrimaries[i].nX = hdrSEIs.mdcv.feature.displayPrimaries[i].x;
      omxHDRSEIs.masteringDisplayColourVolume.displayPrimaries[i].nY = hdrSEIs.mdcv.feature.displayPrimaries[i].y;
    }

    omxHDRSEIs.masteringDisplayColourVolume.whitePoint.nX = hdrSEIs.mdcv.feature.whitePoint.x;
    omxHDRSEIs.masteringDisplayColourVolume.whitePoint.nY = hdrSEIs.mdcv.feature.whitePoint.y;
    omxHDRSEIs.masteringDisplayColourVolume.nMaxDisplayMasteringLuminance = hdrSEIs.mdcv.feature.maxDisplayMasteringLuminance;
    omxHDRSEIs.masteringDisplayColourVolume.nMinDisplayMasteringLuminance = hdrSEIs.mdcv.feature.minDisplayMasteringLuminance;
  }

  omxHDRSEIs.bHasCLL = ConvertMediaToOMXBool(hdrSEIs.cll.enabled);

  if(hdrSEIs.cll.enabled)
  {
    omxHDRSEIs.contentLightLevel.nMaxContentLightLevel = hdrSEIs.cll.feature.maxContentLightLevel;
    omxHDRSEIs.contentLightLevel.nMaxPicAverageLightLevel = hdrSEIs.cll.feature.maxPicAverageLightLevel;
  }

  omxHDRSEIs.bHasATC = ConvertMediaToOMXBool(hdrSEIs.atc.enabled);

  if(hdrSEIs.atc.enabled)
  {
    omxHDRSEIs.alternativeTransferCharacteristics.preferredTransferCharacteristics =
      ConvertMediaToOMXTransferCharacteristics(hdrSEIs.atc.feature.preferredTransferCharacteristics);
  }

  omxHDRSEIs.bHasST2094_10 = ConvertMediaToOMXBool(hdrSEIs.st2094_10.enabled);

  if(hdrSEIs.st2094_10.enabled)
  {
    omxHDRSEIs.st2094_10.nApplicationVersion = hdrSEIs.st2094_10.feature.applicationVersion;
    omxHDRSEIs.st2094_10.bProcessingWindowFlag = ConvertMediaToOMXBool(hdrSEIs.st2094_10.feature.processingWindowFlag);

    if(hdrSEIs.st2094_10.feature.processingWindowFlag)
    {
      omxHDRSEIs.st2094_10.processingWindow.nActiveAreaLeftOffset = hdrSEIs.st2094_10.feature.processingWindow.activeAreaLeftOffset;
      omxHDRSEIs.st2094_10.processingWindow.nActiveAreaRightOffset = hdrSEIs.st2094_10.feature.processingWindow.activeAreaRightOffset;
      omxHDRSEIs.st2094_10.processingWindow.nActiveAreaTopOffset = hdrSEIs.st2094_10.feature.processingWindow.activeAreaTopOffset;
      omxHDRSEIs.st2094_10.processingWindow.nActiveAreaBottomOffset = hdrSEIs.st2094_10.feature.processingWindow.activeAreaBottomOffset;
    }

    omxHDRSEIs.st2094_10.imageCharacteristics.nMinPQ = hdrSEIs.st2094_10.feature.imageCharacteristics.minPQ;
    omxHDRSEIs.st2094_10.imageCharacteristics.nMaxPQ = hdrSEIs.st2094_10.feature.imageCharacteristics.maxPQ;
    omxHDRSEIs.st2094_10.imageCharacteristics.nAvgPQ = hdrSEIs.st2094_10.feature.imageCharacteristics.avgPQ;

    omxHDRSEIs.st2094_10.nNumManualAdjustments = hdrSEIs.st2094_10.feature.numManualAdjustments;

    for(int i = 0; i < hdrSEIs.st2094_10.feature.numManualAdjustments; i++)
    {
      omxHDRSEIs.st2094_10.manualAdjustments[i].nTargetMaxPQ = hdrSEIs.st2094_10.feature.manualAdjustments[i].targetMaxPQ;
      omxHDRSEIs.st2094_10.manualAdjustments[i].nTrimSlope = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimSlope;
      omxHDRSEIs.st2094_10.manualAdjustments[i].nTrimOffset = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimOffset;
      omxHDRSEIs.st2094_10.manualAdjustments[i].nTrimPower = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimPower;
      omxHDRSEIs.st2094_10.manualAdjustments[i].nTrimChromaWeight = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimChromaWeight;
      omxHDRSEIs.st2094_10.manualAdjustments[i].nTrimSaturationGain = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimSaturationGain;
      omxHDRSEIs.st2094_10.manualAdjustments[i].nMSWeight = hdrSEIs.st2094_10.feature.manualAdjustments[i].msWeight;
    }
  }

  omxHDRSEIs.bHasST2094_40 = ConvertMediaToOMXBool(hdrSEIs.st2094_40.enabled);

  if(hdrSEIs.st2094_40.enabled)
  {
    omxHDRSEIs.st2094_40.nApplicationVersion = hdrSEIs.st2094_40.feature.applicationVersion;
    omxHDRSEIs.st2094_40.nNumWindows = hdrSEIs.st2094_40.feature.numWindows;

    for(int i = 0; i < hdrSEIs.st2094_40.feature.numWindows - 1; i++)
    {
      OMX_ALG_PROCESSING_WINDOW_ST2094_40* pOmxPW = &omxHDRSEIs.st2094_40.processingWindows[i];
      const ProcessingWindow_ST2094_40* pPW = &hdrSEIs.st2094_40.feature.processingWindows[i];

      pOmxPW->baseProcessingWindow.nUpperLeftCornerX = pPW->baseProcessingWindow.upperLeftCornerX;
      pOmxPW->baseProcessingWindow.nUpperLeftCornerY = pPW->baseProcessingWindow.upperLeftCornerY;
      pOmxPW->baseProcessingWindow.nLowerRightCornerX = pPW->baseProcessingWindow.lowerRightCornerX;
      pOmxPW->baseProcessingWindow.nLowerRightCornerY = pPW->baseProcessingWindow.lowerRightCornerY;

      pOmxPW->nCenterOfEllipseX = pPW->centerOfEllipseX;
      pOmxPW->nCenterOfEllipseY = pPW->centerOfEllipseY;
      pOmxPW->nRotationAngle = pPW->rotationAngle;
      pOmxPW->nSemimajorAxisInternalEllipse = pPW->semimajorAxisInternalEllipse;
      pOmxPW->nSemimajorAxisExternalEllipse = pPW->semimajorAxisExternalEllipse;
      pOmxPW->nSemiminorAxisExternalEllipse = pPW->semiminorAxisExternalEllipse;
      pOmxPW->nOverlapProcessOption = pPW->overlapProcessOption;
    }

    omxHDRSEIs.st2094_40.targetedSystemDisplay.nMaximumLuminance = hdrSEIs.st2094_40.feature.targetedSystemDisplay.maximumLuminance;
    ConvertMediaToOMX_DPL_ST2094_40(omxHDRSEIs.st2094_40.targetedSystemDisplay.peakLuminance, hdrSEIs.st2094_40.feature.targetedSystemDisplay.peakLuminance);

    ConvertMediaToOMX_DPL_ST2094_40(omxHDRSEIs.st2094_40.masteringDisplayPeakLuminance, hdrSEIs.st2094_40.feature.masteringDisplayPeakLuminance);

    for(int i = 0; i < hdrSEIs.st2094_40.feature.numWindows; i++)
    {
      OMX_ALG_PROCESSING_WINDOW_TRANSFORM_ST2094_40* pOmxPWT = &omxHDRSEIs.st2094_40.processingWindowTransforms[i];
      const ProcessingWindowTransform_ST2094_40* pPWT = &hdrSEIs.st2094_40.feature.processingWindowTransforms[i];

      for(int j = 0; j < 3; j++)
        pOmxPWT->nMaxScl[j] = pPWT->maxscl[j];

      pOmxPWT->nAverageMaxrgb = pPWT->averageMaxrgb;
      pOmxPWT->nNumDistributionMaxrgbPercentiles = pPWT->numDistributionMaxrgbPercentiles;

      for(int j = 0; j < pPWT->numDistributionMaxrgbPercentiles; j++)
      {
        pOmxPWT->nDistributionMaxrgbPercentages[j] = pPWT->distributionMaxrgbPercentages[j];
        pOmxPWT->nDistributionMaxrgbPercentiles[j] = pPWT->distributionMaxrgbPercentiles[j];
      }

      pOmxPWT->nFractionBrightPixels = pPWT->fractionBrightPixels;

      pOmxPWT->toneMapping.bToneMappingFlag = ConvertMediaToOMXBool(pPWT->toneMapping.toneMappingFlag);

      if(pPWT->toneMapping.toneMappingFlag)
      {
        pOmxPWT->toneMapping.bKneePointX = pPWT->toneMapping.kneePointX;
        pOmxPWT->toneMapping.bKneePointY = pPWT->toneMapping.kneePointY;
        pOmxPWT->toneMapping.nNumBezierCurveAnchors = pPWT->toneMapping.numBezierCurveAnchors;

        for(int j = 0; j < pPWT->toneMapping.numBezierCurveAnchors; j++)
          pOmxPWT->toneMapping.nBezierCurveAnchors[j] = pPWT->toneMapping.bezierCurveAnchors[j];
      }

      pOmxPWT->bColorSaturationMappingFlag = ConvertMediaToOMXBool(pPWT->colorSaturationMappingFlag);

      if(pPWT->colorSaturationMappingFlag)
        pOmxPWT->nColorSaturationWeight = pPWT->colorSaturationWeight;
    }
  }

  return omxHDRSEIs;
}

void ConvertOMXToMedia_DPL_ST2094_40(DisplayPeakLuminance_ST2094_40& dst, OMX_ALG_DISPLAY_PEAK_LUMINANCE_ST2094_40 const& src)
{
  dst.actualPeakLuminanceFlag = ConvertOMXToMediaBool(src.bActualPeakLuminanceFlag);

  if(dst.actualPeakLuminanceFlag)
  {
    dst.numRowsActualPeakLuminance = src.nNumRowsActualPeakLuminance;
    dst.numColsActualPeakLuminance = src.nNumColsActualPeakLuminance;

    for(int i = 0; i < dst.numRowsActualPeakLuminance; i++)
      for(int j = 0; j < dst.numColsActualPeakLuminance; j++)
        dst.actualPeakLuminance[i][j] = src.nActualPeakLuminance[i][j];
  }
}

HighDynamicRangeSeis ConvertOMXToMediaHDRSEI(OMX_ALG_VIDEO_CONFIG_HIGH_DYNAMIC_RANGE_SEI const& hdrSEIs)
{
  HighDynamicRangeSeis modHDRSEIs;

  modHDRSEIs.mdcv.enabled = ConvertOMXToMediaBool(hdrSEIs.bHasMDCV);

  if(modHDRSEIs.mdcv.enabled)
  {
    for(int i = 0; i < 3; i++)
    {
      modHDRSEIs.mdcv.feature.displayPrimaries[i].x = hdrSEIs.masteringDisplayColourVolume.displayPrimaries[i].nX;
      modHDRSEIs.mdcv.feature.displayPrimaries[i].y = hdrSEIs.masteringDisplayColourVolume.displayPrimaries[i].nY;
    }

    modHDRSEIs.mdcv.feature.whitePoint.x = hdrSEIs.masteringDisplayColourVolume.whitePoint.nX;
    modHDRSEIs.mdcv.feature.whitePoint.y = hdrSEIs.masteringDisplayColourVolume.whitePoint.nY;
    modHDRSEIs.mdcv.feature.maxDisplayMasteringLuminance = hdrSEIs.masteringDisplayColourVolume.nMaxDisplayMasteringLuminance;
    modHDRSEIs.mdcv.feature.minDisplayMasteringLuminance = hdrSEIs.masteringDisplayColourVolume.nMinDisplayMasteringLuminance;
  }

  modHDRSEIs.cll.enabled = ConvertOMXToMediaBool(hdrSEIs.bHasCLL);

  if(modHDRSEIs.cll.enabled)
  {
    modHDRSEIs.cll.feature.maxContentLightLevel = hdrSEIs.contentLightLevel.nMaxContentLightLevel;
    modHDRSEIs.cll.feature.maxPicAverageLightLevel = hdrSEIs.contentLightLevel.nMaxPicAverageLightLevel;
  }

  modHDRSEIs.atc.enabled = ConvertOMXToMediaBool(hdrSEIs.bHasATC);

  if(modHDRSEIs.atc.enabled)
  {
    modHDRSEIs.atc.feature.preferredTransferCharacteristics =
      ConvertOMXToMediaTransferCharacteristics(hdrSEIs.alternativeTransferCharacteristics.preferredTransferCharacteristics);
  }

  modHDRSEIs.st2094_10.enabled = ConvertOMXToMediaBool(hdrSEIs.bHasST2094_10);

  if(modHDRSEIs.st2094_10.enabled)
  {
    modHDRSEIs.st2094_10.feature.applicationVersion = hdrSEIs.st2094_10.nApplicationVersion;

    modHDRSEIs.st2094_10.feature.processingWindowFlag = ConvertOMXToMediaBool(hdrSEIs.st2094_10.bProcessingWindowFlag);

    if(modHDRSEIs.st2094_10.feature.processingWindowFlag)
    {
      modHDRSEIs.st2094_10.feature.processingWindow.activeAreaLeftOffset = hdrSEIs.st2094_10.processingWindow.nActiveAreaLeftOffset;
      modHDRSEIs.st2094_10.feature.processingWindow.activeAreaRightOffset = hdrSEIs.st2094_10.processingWindow.nActiveAreaRightOffset;
      modHDRSEIs.st2094_10.feature.processingWindow.activeAreaTopOffset = hdrSEIs.st2094_10.processingWindow.nActiveAreaTopOffset;
      modHDRSEIs.st2094_10.feature.processingWindow.activeAreaBottomOffset = hdrSEIs.st2094_10.processingWindow.nActiveAreaBottomOffset;
    }

    modHDRSEIs.st2094_10.feature.imageCharacteristics.minPQ = hdrSEIs.st2094_10.imageCharacteristics.nMinPQ;
    modHDRSEIs.st2094_10.feature.imageCharacteristics.maxPQ = hdrSEIs.st2094_10.imageCharacteristics.nMaxPQ;
    modHDRSEIs.st2094_10.feature.imageCharacteristics.avgPQ = hdrSEIs.st2094_10.imageCharacteristics.nAvgPQ;

    modHDRSEIs.st2094_10.feature.numManualAdjustments = hdrSEIs.st2094_10.nNumManualAdjustments;

    for(int i = 0; i < modHDRSEIs.st2094_10.feature.numManualAdjustments; i++)
    {
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].targetMaxPQ = hdrSEIs.st2094_10.manualAdjustments[i].nTargetMaxPQ;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimSlope = hdrSEIs.st2094_10.manualAdjustments[i].nTrimSlope;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimOffset = hdrSEIs.st2094_10.manualAdjustments[i].nTrimOffset;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimPower = hdrSEIs.st2094_10.manualAdjustments[i].nTrimPower;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimChromaWeight = hdrSEIs.st2094_10.manualAdjustments[i].nTrimChromaWeight;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimSaturationGain = hdrSEIs.st2094_10.manualAdjustments[i].nTrimSaturationGain;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].msWeight = hdrSEIs.st2094_10.manualAdjustments[i].nMSWeight;
    }
  }

  modHDRSEIs.st2094_40.enabled = ConvertOMXToMediaBool(hdrSEIs.bHasST2094_40);

  if(modHDRSEIs.st2094_40.enabled)
  {
    modHDRSEIs.st2094_40.feature.applicationVersion = hdrSEIs.st2094_40.nApplicationVersion;
    modHDRSEIs.st2094_40.feature.numWindows = hdrSEIs.st2094_40.nNumWindows;

    for(int i = 0; i < modHDRSEIs.st2094_40.feature.numWindows - 1; i++)
    {
      const OMX_ALG_PROCESSING_WINDOW_ST2094_40* pOmxPW = &hdrSEIs.st2094_40.processingWindows[i];
      ProcessingWindow_ST2094_40* pPW = &modHDRSEIs.st2094_40.feature.processingWindows[i];

      pPW->baseProcessingWindow.upperLeftCornerX = pOmxPW->baseProcessingWindow.nUpperLeftCornerX;
      pPW->baseProcessingWindow.upperLeftCornerY = pOmxPW->baseProcessingWindow.nUpperLeftCornerY;
      pPW->baseProcessingWindow.lowerRightCornerX = pOmxPW->baseProcessingWindow.nLowerRightCornerX;
      pPW->baseProcessingWindow.lowerRightCornerY = pOmxPW->baseProcessingWindow.nLowerRightCornerY;

      pPW->centerOfEllipseX = pOmxPW->nCenterOfEllipseX;
      pPW->centerOfEllipseY = pOmxPW->nCenterOfEllipseY;
      pPW->rotationAngle = pOmxPW->nRotationAngle;
      pPW->semimajorAxisInternalEllipse = pOmxPW->nSemimajorAxisInternalEllipse;
      pPW->semimajorAxisExternalEllipse = pOmxPW->nSemimajorAxisExternalEllipse;
      pPW->semiminorAxisExternalEllipse = pOmxPW->nSemiminorAxisExternalEllipse;
      pPW->overlapProcessOption = pOmxPW->nOverlapProcessOption;
    }

    modHDRSEIs.st2094_40.feature.targetedSystemDisplay.maximumLuminance = hdrSEIs.st2094_40.targetedSystemDisplay.nMaximumLuminance;
    ConvertOMXToMedia_DPL_ST2094_40(modHDRSEIs.st2094_40.feature.targetedSystemDisplay.peakLuminance, hdrSEIs.st2094_40.targetedSystemDisplay.peakLuminance);

    ConvertOMXToMedia_DPL_ST2094_40(modHDRSEIs.st2094_40.feature.masteringDisplayPeakLuminance, hdrSEIs.st2094_40.masteringDisplayPeakLuminance);

    for(int i = 0; i < modHDRSEIs.st2094_40.feature.numWindows; i++)
    {
      const OMX_ALG_PROCESSING_WINDOW_TRANSFORM_ST2094_40* pOmxPWT = &hdrSEIs.st2094_40.processingWindowTransforms[i];
      ProcessingWindowTransform_ST2094_40* pPWT = &modHDRSEIs.st2094_40.feature.processingWindowTransforms[i];

      for(int j = 0; j < 3; j++)
        pPWT->maxscl[j] = pOmxPWT->nMaxScl[j];

      pPWT->averageMaxrgb = pOmxPWT->nAverageMaxrgb;
      pPWT->numDistributionMaxrgbPercentiles = pOmxPWT->nNumDistributionMaxrgbPercentiles;

      for(int j = 0; j < pPWT->numDistributionMaxrgbPercentiles; j++)
      {
        pPWT->distributionMaxrgbPercentages[j] = pOmxPWT->nDistributionMaxrgbPercentages[j];
        pPWT->distributionMaxrgbPercentiles[j] = pOmxPWT->nDistributionMaxrgbPercentiles[j];
      }

      pPWT->fractionBrightPixels = pOmxPWT->nFractionBrightPixels;

      pPWT->toneMapping.toneMappingFlag = ConvertOMXToMediaBool(pOmxPWT->toneMapping.bToneMappingFlag);

      if(pPWT->toneMapping.toneMappingFlag)
      {
        pPWT->toneMapping.kneePointX = pOmxPWT->toneMapping.bKneePointX;
        pPWT->toneMapping.kneePointY = pOmxPWT->toneMapping.bKneePointY;
        pPWT->toneMapping.numBezierCurveAnchors = pOmxPWT->toneMapping.nNumBezierCurveAnchors;

        for(int j = 0; j < pOmxPWT->toneMapping.nNumBezierCurveAnchors; j++)
          pPWT->toneMapping.bezierCurveAnchors[j] = pOmxPWT->toneMapping.nBezierCurveAnchors[j];
      }

      pPWT->colorSaturationMappingFlag = ConvertOMXToMediaBool(pOmxPWT->bColorSaturationMappingFlag);

      if(pPWT->colorSaturationMappingFlag)
        pPWT->colorSaturationWeight = pOmxPWT->nColorSaturationWeight;
    }
  }

  return modHDRSEIs;
}

OMX_ALG_EQpTableMode ConvertMediaToOMXQpTable(QPTableType mode)
{
  switch(mode)
  {
  case QPTableType::QP_TABLE_NONE: return OMX_ALG_QP_TABLE_NONE;
  case QPTableType::QP_TABLE_ABSOLUTE: return OMX_ALG_QP_TABLE_ABSOLUTE;
  case QPTableType::QP_TABLE_RELATIVE: return OMX_ALG_QP_TABLE_RELATIVE;
  case QPTableType::QP_TABLE_MAX_ENUM: return OMX_ALG_QP_TABLE_MAX_ENUM;
  default: return OMX_ALG_QP_TABLE_MAX_ENUM;
  }

  return OMX_ALG_QP_TABLE_MAX_ENUM;
}

QPTableType ConvertOMXToMediaQpTable(OMX_ALG_EQpTableMode mode)
{
  switch(mode)
  {
  case OMX_ALG_QP_TABLE_NONE: return QPTableType::QP_TABLE_NONE;
  case OMX_ALG_QP_TABLE_ABSOLUTE: return QPTableType::QP_TABLE_ABSOLUTE;
  case OMX_ALG_QP_TABLE_RELATIVE: return QPTableType::QP_TABLE_RELATIVE;
  case OMX_ALG_QP_TABLE_MAX_ENUM: return QPTableType::QP_TABLE_MAX_ENUM;
  default: return QPTableType::QP_TABLE_MAX_ENUM;
  }

  return QPTableType::QP_TABLE_MAX_ENUM;
}

OMX_ALG_EStartCodeBytesAligment ConvertMediaToOMXStartCodeBytesAlignment(StartCodeBytesAlignmentType mode)
{
  switch(mode)
  {
  case StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_AUTO: return OMX_ALG_START_CODE_BYTES_ALIGNMENT_AUTO;
  case StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_3_BYTES: return OMX_ALG_START_CODE_BYTES_ALIGNMENT_3_BYTES;
  case StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_4_BYTES: return OMX_ALG_START_CODE_BYTES_ALIGNMENT_4_BYTES;
  case StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM: return OMX_ALG_START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
  default: return OMX_ALG_START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
  }

  return OMX_ALG_START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
}

StartCodeBytesAlignmentType ConvertOMXToMediaStartCodeBytesAlignment(OMX_ALG_EStartCodeBytesAligment mode)
{
  switch(mode)
  {
  case OMX_ALG_START_CODE_BYTES_ALIGNMENT_AUTO: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_AUTO;
  case OMX_ALG_START_CODE_BYTES_ALIGNMENT_3_BYTES: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_3_BYTES;
  case OMX_ALG_START_CODE_BYTES_ALIGNMENT_4_BYTES: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_4_BYTES;
  case OMX_ALG_START_CODE_BYTES_ALIGNMENT_MAX_ENUM: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
  default: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
  }

  return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
}
