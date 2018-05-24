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

#include "omx_mediatype_enc_hevc.h"
#include "omx_mediatype_enc_common.h"
#include "omx_mediatype_common_hevc.h"
#include "omx_mediatype_common.h"
#include "base/omx_settings/omx_convert_module_soft_hevc.h"
#include "base/omx_utils/roundup.h"
#include "base/omx_settings/omx_convert_module_soft_enc.h"
#include <cmath>

extern "C"
{
#include <lib_common_enc/EncBuffers.h>
}

using namespace std;

EncMediatypeHEVC::EncMediatypeHEVC()
{
  Reset();
}

EncMediatypeHEVC::~EncMediatypeHEVC() = default;

void EncMediatypeHEVC::Reset()
{
  AL_Settings_SetDefaults(&settings);
  auto& chan = settings.tChParam[0];
  chan.eProfile = AL_PROFILE_HEVC_MAIN;
  AL_Settings_SetDefaultParam(&settings);
  chan.uLevel = 10;
  chan.uWidth = 176;
  chan.uHeight = 144;
  chan.ePicFormat = AL_420_8BITS;
  chan.uEncodingBitDepth = 8;
  auto& rateCtrl = chan.tRCParam;
  rateCtrl.eRCMode = AL_RC_CBR;
  rateCtrl.eOptions = AL_RC_OPT_SCN_CHG_RES;
  rateCtrl.uMaxBitRate = rateCtrl.uTargetBitRate = 64000;
  rateCtrl.uFrameRate = 15;
  auto& gopParam = chan.tGopParam;
  gopParam.bEnableLT = false;
  settings.bEnableFillerData = true;
  settings.bEnableAUD = false;
  settings.iPrefetchLevel2 = 0;

  stride = RoundUp(AL_EncGetMinPitch(chan.uWidth, AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER), strideAlignment);
  sliceHeight = RoundUp(chan.uHeight, sliceHeightAlignment);
}

static bool IsHighTier(uint8_t const& tier)
{
  return tier != 0;
}

ProfileLevelType EncMediatypeHEVC::ProfileLevel() const
{
  auto const chan = settings.tChParam[0];
  return IsHighTier(chan.uTier) ? CreateHEVCHighTierProfileLevel(chan.eProfile, chan.uLevel) : CreateHEVCMainTierProfileLevel(chan.eProfile, chan.uLevel);
}

bool EncMediatypeHEVC::IsInProfilesSupported(HEVCProfileType const& profile)
{
  for(auto const& p : profiles)
  {
    if(p == profile)
      return true;
  }

  return false;
}

bool EncMediatypeHEVC::IsInLevelsSupported(int level)
{
  for(auto const& l : levels)
  {
    if(l == level)
      return true;
  }

  return false;
}

bool EncMediatypeHEVC::SetProfileLevel(ProfileLevelType const& profileLevel)
{
  if(!IsInProfilesSupported(profileLevel.profile.hevc))
    return false;

  if(!IsInLevelsSupported(profileLevel.level))
    return false;

  auto const profile = ConvertModuleToSoftHEVCProfile(profileLevel.profile.hevc);

  if(profile == AL_PROFILE_HEVC)
    return false;

  auto& chan = settings.tChParam[0];
  chan.eProfile = profile;
  chan.uLevel = profileLevel.level;
  chan.uTier = IsHighTierProfile(profileLevel.profile.hevc) ? 1 : 0;
  return true;
}

static Mimes CreateMimes()
{
  Mimes mimes;
  auto& input = mimes.input;

  input.mime = "video/x-raw";
  input.compression = CompressionType::COMPRESSION_UNUSED;

  auto& output = mimes.output;

  output.mime = "video/x-h265";
  output.compression = CompressionType::COMPRESSION_HEVC;

  return mimes;
}

static int CreateLatency(AL_TEncSettings const& settings)
{
  auto const channel = settings.tChParam[0];
  auto const rateCtrl = channel.tRCParam;
  auto const gopParam = channel.tGopParam;

  auto const intermediate = 0;
  auto const buffer = 1;
  auto const buffers = buffer + intermediate + gopParam.uNumB;

  auto const realFramerate = (static_cast<double>(rateCtrl.uFrameRate * rateCtrl.uClkRatio) / 1000.0);
  auto timeInMilliseconds = (static_cast<double>(buffers * 1000.0) / realFramerate);

  if(channel.bSubframeLatency)
  {
    timeInMilliseconds /= channel.uNumSlices;
    timeInMilliseconds += 1.0; // overhead
  }

  return ceil(timeInMilliseconds);
}

static bool CreateLowBandwidth(AL_TEncSettings const& settings)
{
  auto const channel = settings.tChParam[0];
  return channel.pMeRange[SLICE_P][1] == 16;
}

static bool UpdateLowBandwidth(AL_TEncSettings& settings, bool const& isLowBandwidthEnabled)
{
  auto& channel = settings.tChParam[0];
  channel.pMeRange[SLICE_P][1] = isLowBandwidthEnabled ? 16 : 32;
  return true;
}

static BufferCounts CreateBufferCounts(AL_TEncSettings const& settings)
{
  BufferCounts bufferCounts;
  auto const channel = settings.tChParam[0];
  auto const gopParam = channel.tGopParam;

  auto const intermediate = 1;
  auto const buffer = 1;
  auto const buffers = buffer + intermediate + gopParam.uNumB;

  bufferCounts.input = bufferCounts.output = buffers;

  if(channel.bSubframeLatency)
  {
    auto const numSlices = channel.uNumSlices;
    bufferCounts.output *= numSlices;
  }
  return bufferCounts;
}

static LoopFilterType CreateLoopFilter(AL_TEncSettings const& settings)
{
  auto const channel = settings.tChParam[0];
  return ConvertSoftToModuleLoopFilter(channel.eOptions);
}

static bool CheckLoopFilter(LoopFilterType const& loopFilter)
{
  if(loopFilter == LoopFilterType::LOOP_FILTER_MAX_ENUM)
    return false;

  return true;
}

static bool UpdateLoopFilter(AL_TEncSettings& settings, LoopFilterType const& loopFilter)
{
  if(!CheckLoopFilter(loopFilter))
    return false;

  auto& options = settings.tChParam[0].eOptions;
  options = static_cast<AL_EChEncOption>(options | ConvertModuleToSoftLoopFilter(loopFilter));

  return true;
}

MediatypeInterface::ErrorSettingsType EncMediatypeHEVC::Get(std::string index, void* settings) const
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

  if(index == "SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED")
  {
    *(static_cast<vector<ProfileLevelType>*>(settings)) = CreateHEVCProfileLevelSupported(profiles, levels);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FORMATS_SUPPORTED")
  {
    *(static_cast<vector<Format>*>(settings)) = CreateFormatsSupported(colors, bitdepths);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SLICE_PARAMETER")
  {
    *(static_cast<Slices*>(settings)) = CreateSlicesParameter(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  return ERROR_SETTINGS_BAD_INDEX;
}

MediatypeInterface::ErrorSettingsType EncMediatypeHEVC::Set(std::string index, void const* settings)
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
    auto const gop = *(static_cast<Gop const*>(settings));

    if(!UpdateGroupOfPictures(this->settings, gop))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_LOW_BANDWIDTH")
  {
    auto const isLowBandwidthEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateLowBandwidth(this->settings, isLowBandwidthEnabled))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION")
  {
    auto const isConstrainedIntraPredictionEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateConstrainedIntraPrediction(this->settings, isConstrainedIntraPredictionEnabled))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_VIDEO_MODE")
  {
    auto const videoMode = *(static_cast<VideoModeType const*>(settings));

    if(!UpdateVideoMode(this->settings, videoMode))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_BITRATE")
  {
    auto const bitrate = *(static_cast<Bitrate const*>(settings));

    if(!UpdateBitrate(this->settings, bitrate))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_CACHE_LEVEL2")
  {
    auto const isCacheLevel2Enabled = *(static_cast<bool const*>(settings));

    if(!UpdateCacheLevel2(this->settings, isCacheLevel2Enabled))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FILLER_DATA")
  {
    auto const isFillerDataEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateFillerData(this->settings, isFillerDataEnabled))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_ASPECT_RATIO")
  {
    auto const aspectRatio = *(static_cast<AspectRatioType const*>(settings));

    if(!UpdateAspectRatio(this->settings, aspectRatio))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SCALING_LIST")
  {
    auto const scalingList = *(static_cast<ScalingListType const*>(settings));

    if(!UpdateScalingList(this->settings, scalingList))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_QUANTIZATION_PARAMETER")
  {
    auto const qps = *(static_cast<QPs const*>(settings));

    if(!UpdateQuantizationParameter(this->settings, qps))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER")
  {
    auto const loopFilter = *(static_cast<LoopFilterType const*>(settings));

    if(!UpdateLoopFilter(this->settings, loopFilter))
      return ERROR_SETTINGS_BAD_PARAMETER;

    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SLICE_PARAMETER")
  {
    auto const slices = *(static_cast<Slices const*>(settings));

    if(!UpdateSlicesParameter(this->settings, slices))
      return ERROR_SETTINGS_BAD_PARAMETER;

    return ERROR_SETTINGS_NONE;
  }

  return ERROR_SETTINGS_BAD_INDEX;
}

