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

#include "omx_mediatype_enc_avc.h"
#include "omx_mediatype_enc_common.h"
#include "omx_mediatype_common.h"
#include "omx_mediatype_common_avc.h"
#include "base/omx_utils/round.h"
#include "omx_convert_module_soft_avc.h"
#include "omx_convert_module_soft_enc.h"
#include "omx_convert_module_soft.h"
#include "omx_mediatype_checks.h"
#include <cmath>
#include <cstring> // memset

extern "C"
{
#include <lib_common_enc/EncBuffers.h>
}

using namespace std;

EncMediatypeAVC::EncMediatypeAVC()
{
  strideAlignment.widthStride = 32;
  strideAlignment.heightStride = 8;
  CreateFormatsSupportedMap(colors, bitdepths, supportedFormatsMap);
  Reset();
}

EncMediatypeAVC::~EncMediatypeAVC() = default;

void EncMediatypeAVC::Reset()
{
  bufferHandles.input = BufferHandleType::BUFFER_HANDLE_CHAR_PTR;
  bufferHandles.output = BufferHandleType::BUFFER_HANDLE_CHAR_PTR;

  memset(&settings, 0, sizeof(settings));
  AL_Settings_SetDefaults(&settings);
  auto& channel = settings.tChParam[0];
  channel.eProfile = AL_PROFILE_AVC_C_BASELINE;
  AL_Settings_SetDefaultParam(&settings);
  channel.uLevel = 10;
  channel.uWidth = 176;
  channel.uHeight = 144;
  channel.ePicFormat = AL_420_8BITS;
  channel.uSrcBitDepth = 8;
  channel.eOptions = static_cast<AL_EChEncOption>(channel.eOptions & ~(AL_OPT_LF_X_TILE));
  auto& rateControl = channel.tRCParam;
  rateControl.eRCMode = AL_RC_CBR;
  rateControl.eOptions = AL_RC_OPT_SCN_CHG_RES;
  rateControl.uMaxBitRate = rateControl.uTargetBitRate = 64000;
  rateControl.uFrameRate = 15;
  auto& gopParam = channel.tGopParam;
  gopParam.bEnableLT = false;
  settings.bEnableFillerData = true;
  settings.bEnableAUD = false;
  settings.iPrefetchLevel2 = 0;
#if AL_ENABLE_TWOPASS
  settings.LookAhead = 0;
#endif

  stride = RoundUp(AL_EncGetMinPitch(channel.uWidth, AL_GET_BITDEPTH(channel.ePicFormat), AL_FB_RASTER), strideAlignment.widthStride);
  sliceHeight = RoundUp(channel.uHeight, strideAlignment.heightStride);
}

static Mimes CreateMimes()
{
  Mimes mimes;
  auto& input = mimes.input;

  input.mime = "video/x-raw";
  input.compression = CompressionType::COMPRESSION_UNUSED;

  auto& output = mimes.output;

  output.mime = "video/x-h264";
  output.compression = CompressionType::COMPRESSION_AVC;

  return mimes;
}

static int CreateLatency(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  auto rateControl = channel.tRCParam;
  auto gopParam = channel.tGopParam;

  auto intermediate = 1;
  auto buffer = 1;
  auto buffers = buffer + intermediate + gopParam.uNumB;

  auto realFramerate = (static_cast<double>(rateControl.uFrameRate * rateControl.uClkRatio) / 1000.0);
  auto timeInMilliseconds = (static_cast<double>(buffers * 1000.0) / realFramerate);

  if(channel.bSubframeLatency)
  {
    timeInMilliseconds /= channel.uNumSlices;
    timeInMilliseconds += 1.0; // overhead
  }

  return ceil(timeInMilliseconds);
}

static bool CreateLowBandwidth(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return channel.pMeRange[SLICE_P][1] == 8;
}

static EntropyCodingType CreateEntropyCoding(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return ConvertSoftToModuleEntropyCoding(channel.eEntropyMode);
}

static BufferCounts CreateBufferCounts(AL_TEncSettings settings)
{
  BufferCounts bufferCounts;
  auto channel = settings.tChParam[0];
  auto gopParam = channel.tGopParam;

  auto intermediate = 1;
  auto buffer = 1;
  auto buffers = buffer + intermediate + gopParam.uNumB;

  bufferCounts.input = bufferCounts.output = buffers;

  if(channel.bSubframeLatency)
  {
    auto numSlices = channel.uNumSlices;
    bufferCounts.output *= numSlices;
  }

#if AL_ENABLE_TWOPASS

  if(settings.LookAhead)
    bufferCounts.input += settings.LookAhead;
#endif

  return bufferCounts;
}

static LoopFilterType CreateLoopFilter(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return ConvertSoftToModuleLoopFilter(channel.eOptions);
}

static ProfileLevelType CreateProfileLevel(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return CreateAVCProfileLevel(channel.eProfile, channel.uLevel);
}

MediatypeInterface::ErrorSettingsType EncMediatypeAVC::Get(std::string index, void* settings) const
{
  if(!settings)
    return ERROR_SETTINGS_BAD_PARAMETER;

  if(index == "SETTINGS_INDEX_MIMES")
  {
    *(static_cast<Mimes*>(settings)) = CreateMimes();
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_CLOCK")
  {
    *(static_cast<Clock*>(settings)) = CreateClock(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_STRIDE_ALIGNMENT")
  {
    *(static_cast<Stride*>(settings)) = this->strideAlignment;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_GROUP_OF_PICTURES")
  {
    *(static_cast<Gop*>(settings)) = CreateGroupOfPictures(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_LATENCY")
  {
    *(static_cast<int*>(settings)) = CreateLatency(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_LOW_BANDWIDTH")
  {
    *(static_cast<bool*>(settings)) = CreateLowBandwidth(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION")
  {
    *(static_cast<bool*>(settings)) = CreateConstrainedIntraPrediction(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_ENTROPY_CODING")
  {
    *(static_cast<EntropyCodingType*>(settings)) = CreateEntropyCoding(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_VIDEO_MODE")
  {
    *(static_cast<VideoModeType*>(settings)) = CreateVideoMode(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_VIDEO_MODES_SUPPORTED")
  {
    *(static_cast<vector<VideoModeType>*>(settings)) = this->videoModes;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_BITRATE")
  {
    *(static_cast<Bitrate*>(settings)) = CreateBitrate(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_CACHE_LEVEL2")
  {
    *(static_cast<bool*>(settings)) = CreateCacheLevel2(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_BUFFER_HANDLES")
  {
    *(static_cast<BufferHandles*>(settings)) = this->bufferHandles;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_BUFFER_COUNTS")
  {
    *(static_cast<BufferCounts*>(settings)) = CreateBufferCounts(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FILLER_DATA")
  {
    *(static_cast<bool*>(settings)) = CreateFillerData(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_ASPECT_RATIO")
  {
    *(static_cast<AspectRatioType*>(settings)) = CreateAspectRatio(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SCALING_LIST")
  {
    *(static_cast<ScalingListType*>(settings)) = CreateScalingList(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_QUANTIZATION_PARAMETER")
  {
    *(static_cast<QPs*>(settings)) = CreateQuantizationParameter(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER")
  {
    *(static_cast<LoopFilterType*>(settings)) = CreateLoopFilter(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_PROFILE_LEVEL")
  {
    *(static_cast<ProfileLevelType*>(settings)) = CreateProfileLevel(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED")
  {
    *(static_cast<vector<ProfileLevelType>*>(settings)) = CreateAVCProfileLevelSupported(profiles, levels);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FORMAT")
  {
    *(static_cast<Format*>(settings)) = CreateFormat(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FORMATS_SUPPORTED")
  {
    SupportedFormats supported;
    supported.input = CreateFormatsSupported(colors, bitdepths);
    supported.output = CreateFormatsSupportedByCurrent(CreateFormat(this->settings), supportedFormatsMap);
    *(static_cast<SupportedFormats*>(settings)) = supported;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SLICE_PARAMETER")
  {
    *(static_cast<Slices*>(settings)) = CreateSlicesParameter(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SUBFRAME")
  {
    *(static_cast<bool*>(settings)) = (this->settings.tChParam[0].bSubframeLatency);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_RESOLUTION")
  {
    *(static_cast<Resolution*>(settings)) = CreateResolution(this->settings, stride, sliceHeight);
    return ERROR_SETTINGS_NONE;
  }

#if AL_ENABLE_TWOPASS

  if(index == "SETTINGS_INDEX_LOOKAHEAD")
  {
    *(static_cast<LookAhead*>(settings)) = CreateLookAhead(this->settings);
    return ERROR_SETTINGS_NONE;
  }
#endif

  return ERROR_SETTINGS_BAD_INDEX;
}

static bool UpdateLowBandwidth(AL_TEncSettings& settings, bool isLowBandwidthEnabled)
{
  auto& channel = settings.tChParam[0];
  channel.pMeRange[SLICE_P][1] = isLowBandwidthEnabled ? 8 : 16;
  return true;
}

static bool CheckEntropyCoding(EntropyCodingType entropyCoding)
{
  return entropyCoding != EntropyCodingType::ENTROPY_CODING_MAX_ENUM;
}

static bool UpdateEntropyCoding(AL_TEncSettings& settings, EntropyCodingType entropyCoding)
{
  if(!CheckEntropyCoding(entropyCoding))
    return false;

  auto& channel = settings.tChParam[0];
  channel.eEntropyMode = ConvertModuleToSoftEntropyCoding(entropyCoding);
  return true;
}

static bool CheckLoopFilter(LoopFilterType loopFilter)
{
  if(loopFilter == LoopFilterType::LOOP_FILTER_MAX_ENUM)
    return false;

  if(loopFilter == LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE)
    return false;

  if(loopFilter == LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE)
    return false;

  return true;
}

static bool UpdateLoopFilter(AL_TEncSettings& settings, LoopFilterType loopFilter)
{
  if(!CheckLoopFilter(loopFilter))
    return false;

  auto& options = settings.tChParam[0].eOptions;
  options = static_cast<AL_EChEncOption>(options | ConvertModuleToSoftLoopFilter(loopFilter));

  return true;
}

static bool CheckProfileLevel(ProfileLevelType profilelevel, vector<AVCProfileType> profiles, vector<int> levels)
{
  if(!IsSupported(profilelevel.profile.avc, profiles))
    return false;

  if(!IsSupported(profilelevel.level, levels))
    return false;

  auto profile = ConvertModuleToSoftAVCProfile(profilelevel.profile.avc);

  if(!AL_IS_AVC(profile))
    return false;

  return true;
}

static bool UpdateProfileLevel(AL_TEncSettings& settings, ProfileLevelType profilelevel, vector<AVCProfileType> profiles, vector<int> levels)
{
  if(!CheckProfileLevel(profilelevel, profiles, levels))
    return false;

  auto& channel = settings.tChParam[0];
  auto profile = ConvertModuleToSoftAVCProfile(profilelevel.profile.avc);
  channel.eProfile = profile;
  channel.uLevel = profilelevel.level;
  return true;
}

MediatypeInterface::ErrorSettingsType EncMediatypeAVC::Set(std::string index, void const* settings)
{
  if(!settings)
    return ERROR_SETTINGS_BAD_PARAMETER;

  if(index == "SETTINGS_INDEX_CLOCK")
  {
    auto clock = *(static_cast<Clock const*>(settings));

    if(!UpdateClock(this->settings, clock))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_GROUP_OF_PICTURES")
  {
    auto gop = *(static_cast<Gop const*>(settings));

    if(!UpdateGroupOfPictures(this->settings, gop))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_LOW_BANDWIDTH")
  {
    auto isLowBandwidthEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateLowBandwidth(this->settings, isLowBandwidthEnabled))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION")
  {
    auto isConstrainedIntraPredictionEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateConstrainedIntraPrediction(this->settings, isConstrainedIntraPredictionEnabled))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_ENTROPY_CODING")
  {
    auto entropyCoding = *(static_cast<EntropyCodingType const*>(settings));

    if(!UpdateEntropyCoding(this->settings, entropyCoding))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_VIDEO_MODE")
  {
    auto videoMode = *(static_cast<VideoModeType const*>(settings));

    if(!UpdateVideoMode(this->settings, videoMode))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_BITRATE")
  {
    auto bitrate = *(static_cast<Bitrate const*>(settings));

    if(!UpdateBitrate(this->settings, bitrate))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_CACHE_LEVEL2")
  {
    auto isCacheLevel2Enabled = *(static_cast<bool const*>(settings));

    if(!UpdateCacheLevel2(this->settings, isCacheLevel2Enabled))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FILLER_DATA")
  {
    auto isFillerDataEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateFillerData(this->settings, isFillerDataEnabled))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_ASPECT_RATIO")
  {
    auto aspectRatio = *(static_cast<AspectRatioType const*>(settings));

    if(!UpdateAspectRatio(this->settings, aspectRatio))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SCALING_LIST")
  {
    auto scalingList = *(static_cast<ScalingListType const*>(settings));

    if(!UpdateScalingList(this->settings, scalingList))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_QUANTIZATION_PARAMETER")
  {
    auto qps = *(static_cast<QPs const*>(settings));

    if(!UpdateQuantizationParameter(this->settings, qps))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER")
  {
    auto loopFilter = *(static_cast<LoopFilterType const*>(settings));

    if(!UpdateLoopFilter(this->settings, loopFilter))
      return ERROR_SETTINGS_BAD_PARAMETER;

    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_PROFILE_LEVEL")
  {
    auto profilelevel = *(static_cast<ProfileLevelType const*>(settings));

    if(!UpdateProfileLevel(this->settings, profilelevel, profiles, levels))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FORMAT")
  {
    auto format = *(static_cast<Format const*>(settings));

    if(!UpdateFormat(this->settings, format, colors, bitdepths, this->stride, this->strideAlignment))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SLICE_PARAMETER")
  {
    auto slices = *(static_cast<Slices const*>(settings));

    if(!UpdateSlicesParameter(this->settings, slices))
      return ERROR_SETTINGS_BAD_PARAMETER;

    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_BUFFER_HANDLES")
  {
    auto bufferHandles = *(static_cast<BufferHandles const*>(settings));

    if(!UpdateBufferHandles(this->bufferHandles, bufferHandles))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SUBFRAME")
  {
    auto isEnabledSubFrame = *(static_cast<bool const*>(settings));

    if(!UpdateIsEnabledSubFrame(this->settings, isEnabledSubFrame))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_RESOLUTION")
  {
    auto resolution = *(static_cast<Resolution const*>(settings));

    if(!UpdateResolution(this->settings, this->stride, this->sliceHeight, this->strideAlignment, resolution))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

#if AL_ENABLE_TWOPASS

  if(index == "SETTINGS_INDEX_LOOKAHEAD")
  {
    auto la = *(static_cast<LookAhead const*>(settings));

    if(!UpdateLookAhead(this->settings, la))
      return ERROR_SETTINGS_BAD_PARAMETER;

    return ERROR_SETTINGS_NONE;
  }
#endif

  return ERROR_SETTINGS_BAD_INDEX;
}

