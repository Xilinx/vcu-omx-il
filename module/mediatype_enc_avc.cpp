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

#include "mediatype_enc_avc.h"
#include "mediatype_enc_itu.h"
#include "mediatype_codec_avc.h"
#include "mediatype_codec_itu.h"
#include "convert_module_soft_avc.h"
#include "convert_module_soft_enc.h"
#include "convert_module_soft.h"
#include "mediatype_checks.h"
#include <utility/round.h>
#include <cmath>
#include <cstring> // memset
#include <algorithm> // max

extern "C"
{
#include <lib_common/SEI.h>
#include <lib_common/Profiles.h>
#include <lib_common_enc/EncBuffers.h>
#include <lib_common_enc/IpEncFourCC.h>
#include <lib_fpga/DmaAllocLinux.h>
}

using namespace std;

EncMediatypeAVC::EncMediatypeAVC(BufferContiguities bufferContiguities, BufferBytesAlignments bufferBytesAlignments, StrideAlignments strideAlignments, bool isSeparateConfigurationFromDataEnabled, std::shared_ptr<AL_TAllocator> const& allocator)
{
  this->bufferContiguities.input = bufferContiguities.input;
  this->bufferContiguities.output = bufferContiguities.output;
  this->bufferBytesAlignments.input = bufferBytesAlignments.input;
  this->bufferBytesAlignments.output = bufferBytesAlignments.output;
  this->strideAlignments.horizontal = strideAlignments.horizontal;
  this->strideAlignments.vertical = strideAlignments.vertical;
  this->isSeparateConfigurationFromDataEnabled = isSeparateConfigurationFromDataEnabled;
  this->allocator = allocator;
  CreateFormatsSupportedMap(this->colors, this->bitdepths, this->supportedFormatsMap);
  Reset();
}

EncMediatypeAVC::~EncMediatypeAVC()
{
  ResetRcPluginContext(this->allocator.get(), &this->settings);
}

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
  channel.uEncWidth = 176;
  channel.uEncHeight = 144;
  channel.ePicFormat = AL_420_8BITS;
  channel.uSrcWidth = 176;
  channel.uSrcHeight = 144;
  channel.uSrcBitDepth = 8;
  channel.eEncTools = static_cast<AL_EChEncTool>(channel.eEncTools & ~(AL_OPT_LF_X_TILE));
  auto& rateControl = channel.tRCParam;
  rateControl.eRCMode = AL_RC_CBR;
  rateControl.iInitialQP = 30;
  rateControl.eOptions = static_cast<AL_ERateCtrlOption>(rateControl.eOptions | AL_RC_OPT_SCN_CHG_RES);
  rateControl.uMaxBitRate = rateControl.uTargetBitRate = 64000;
  rateControl.uFrameRate = 15;
  auto& gopParam = channel.tGopParam;
  gopParam.bEnableLT = false;
  settings.eEnableFillerData = AL_FILLER_APP;
  settings.bEnableAUD = false;
  settings.iPrefetchLevel2 = 0;
  settings.LookAhead = 0;
  settings.TwoPass = 0;
  settings.uEnableSEI = AL_SEI_NONE;

  stride.horizontal = RoundUp(AL_EncGetMinPitch(channel.uEncWidth, AL_GET_BITDEPTH(channel.ePicFormat), AL_FB_RASTER), strideAlignments.horizontal);
  stride.vertical = RoundUp(static_cast<int>(channel.uEncHeight), strideAlignments.vertical);

  ResetRcPluginContext(this->allocator.get(), &this->settings);
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
    timeInMilliseconds *= 2.0;
  }

  auto overhead = 1.0;
  timeInMilliseconds += overhead;

  return ceil(timeInMilliseconds);
}

static bool CreateLowBandwidth(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return channel.pMeRange[AL_SLICE_P][1] == 8;
}

static EntropyCodingType CreateEntropyCoding(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return ConvertSoftToModuleEntropyCoding(channel.eEntropyMode);
}

static BufferCounts CreateBufferCounts(AL_TEncSettings settings, bool isSeparateConfigurationFromDataEnabled)
{
  BufferCounts bufferCounts;
  auto channel = settings.tChParam[0];
  auto gopParam = channel.tGopParam;

  auto intermediate = 1;
  auto buffer = 1;
  auto buffers = buffer + intermediate + gopParam.uNumB;

  bufferCounts.input = bufferCounts.output = buffers;

  if(settings.LookAhead)
    bufferCounts.input += settings.LookAhead;

  if(channel.bSubframeLatency)
  {
    auto numSlices = channel.uNumSlices;
    bufferCounts.output *= numSlices;
  }

  if(isSeparateConfigurationFromDataEnabled)
    bufferCounts.output += 1;

  return bufferCounts;
}

static LoopFilterType CreateLoopFilter(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return ConvertSoftToModuleLoopFilter(channel.eEncTools);
}

static ProfileLevel CreateProfileLevel(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return CreateAVCProfileLevel(channel.eProfile, channel.uLevel);
}

MediatypeInterface::ErrorType EncMediatypeAVC::Get(std::string index, void* settings) const
{
  if(!settings)
    return BAD_PARAMETER;

  if(index == "SETTINGS_INDEX_MIMES")
  {
    *(static_cast<Mimes*>(settings)) = CreateMimes();
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_CLOCK")
  {
    *(static_cast<Clock*>(settings)) = CreateClock(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_STRIDE_ALIGNMENTS")
  {
    *(static_cast<StrideAlignments*>(settings)) = this->strideAlignments;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_GROUP_OF_PICTURES")
  {
    *(static_cast<Gop*>(settings)) = CreateGroupOfPictures(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LATENCY")
  {
    *(static_cast<int*>(settings)) = CreateLatency(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOW_BANDWIDTH")
  {
    *(static_cast<bool*>(settings)) = CreateLowBandwidth(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION")
  {
    *(static_cast<bool*>(settings)) = CreateConstrainedIntraPrediction(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_ENTROPY_CODING")
  {
    *(static_cast<EntropyCodingType*>(settings)) = CreateEntropyCoding(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_VIDEO_MODE")
  {
    *(static_cast<VideoModeType*>(settings)) = CreateVideoMode(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_VIDEO_MODES_SUPPORTED")
  {
    *(static_cast<vector<VideoModeType>*>(settings)) = this->videoModes;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BITRATE")
  {
    *(static_cast<Bitrate*>(settings)) = CreateBitrate(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_CACHE_LEVEL2")
  {
    *(static_cast<bool*>(settings)) = CreateCacheLevel2(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFER_HANDLES")
  {
    *(static_cast<BufferHandles*>(settings)) = this->bufferHandles;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFER_COUNTS")
  {
    *(static_cast<BufferCounts*>(settings)) = CreateBufferCounts(this->settings, this->isSeparateConfigurationFromDataEnabled);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFER_SIZES")
  {
    *(static_cast<BufferSizes*>(settings)) = CreateBufferSizes(this->settings, this->stride);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFER_CONTIGUITIES")
  {
    *(static_cast<BufferContiguities*>(settings)) = this->bufferContiguities;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFER_BYTES_ALIGNMENTS")
  {
    *(static_cast<BufferBytesAlignments*>(settings)) = this->bufferBytesAlignments;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_FILLER_DATA")
  {
    *(static_cast<bool*>(settings)) = CreateFillerData(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_ASPECT_RATIO")
  {
    *(static_cast<AspectRatioType*>(settings)) = CreateAspectRatio(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SCALING_LIST")
  {
    *(static_cast<ScalingListType*>(settings)) = CreateScalingList(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_QUANTIZATION_PARAMETER")
  {
    *(static_cast<QPs*>(settings)) = CreateQuantizationParameter(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER")
  {
    *(static_cast<LoopFilterType*>(settings)) = CreateLoopFilter(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_PROFILE_LEVEL")
  {
    *(static_cast<ProfileLevel*>(settings)) = CreateProfileLevel(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED")
  {
    *(static_cast<vector<ProfileLevel>*>(settings)) = CreateAVCProfileLevelSupported(profiles, levels);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_FORMAT")
  {
    *(static_cast<Format*>(settings)) = CreateFormat(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_FORMATS_SUPPORTED")
  {
    SupportedFormats supported {};
    supported.input = CreateFormatsSupported(this->colors, this->bitdepths);
    supported.output = CreateFormatsSupportedByCurrent(CreateFormat(this->settings), this->supportedFormatsMap);
    *(static_cast<SupportedFormats*>(settings)) = supported;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SLICE_PARAMETER")
  {
    *(static_cast<Slices*>(settings)) = CreateSlicesParameter(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SUBFRAME")
  {
    *(static_cast<bool*>(settings)) = (this->settings.tChParam[0].bSubframeLatency);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_RESOLUTION")
  {
    *(static_cast<Resolution*>(settings)) = CreateResolution(this->settings, this->stride);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_COLOR_PRIMARIES")
  {
    *(static_cast<ColorPrimariesType*>(settings)) = CreateColorPrimaries(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_TRANSFER_CHARACTERISTICS")
  {
    *(static_cast<TransferCharacteristicsType*>(settings)) = CreateTransferCharacteristics(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_COLOUR_MATRIX")
  {
    *(static_cast<ColourMatrixType*>(settings)) = CreateColourMatrix(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOOKAHEAD")
  {
    *(static_cast<LookAhead*>(settings)) = CreateLookAhead(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_TWOPASS")
  {
    *(static_cast<TwoPass*>(settings)) = CreateTwoPass(this->settings, this->sTwoPassLogFile);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SEPARATE_CONFIGURATION_FROM_DATA")
  {
    *(static_cast<bool*>(settings)) = this->isSeparateConfigurationFromDataEnabled;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_MAX_PICTURE_SIZES")
  {
    *static_cast<MaxPicturesSizes*>(settings) = CreateMaxPictureSizes(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER_BETA")
  {
    *static_cast<int*>(settings) = CreateLoopFilterBeta(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER_TC")
  {
    *static_cast<int*>(settings) = CreateLoopFilterTc(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_ACCESS_UNIT_DELIMITER")
  {
    *static_cast<bool*>(settings) = CreateAccessUnitDelimiter(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFERING_PERIOD_SEI")
  {
    *static_cast<bool*>(settings) = CreateBufferingPeriodSEI(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_PICTURE_TIMING_SEI")
  {
    *static_cast<bool*>(settings) = CreatePictureTimingSEI(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_RECOVERY_POINT_SEI")
  {
    *static_cast<bool*>(settings) = CreateRecoveryPointSEI(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_MASTERING_DISPLAY_COLOUR_VOLUME_SEI")
  {
    *static_cast<bool*>(settings) = CreateMasteringDisplayColourVolumeSEI(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_CONTENT_LIGHT_LEVEL_SEI")
  {
    *static_cast<bool*>(settings) = CreateContentLightLevelSEI(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_RATE_CONTROL_PLUGIN")
  {
    *(static_cast<RateControlPlugin*>(settings)) = CreateRateControlPlugin(this->allocator.get(), this->settings);
    return SUCCESS;
  }

  return BAD_INDEX;
}

static bool UpdateLowBandwidth(AL_TEncSettings& settings, bool isLowBandwidthEnabled)
{
  auto& channel = settings.tChParam[0];
  channel.pMeRange[AL_SLICE_P][1] = isLowBandwidthEnabled ? 8 : 16;
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

  auto& options = settings.tChParam[0].eEncTools;
  options = static_cast<AL_EChEncTool>(options | ConvertModuleToSoftLoopFilter(loopFilter));

  return true;
}

static bool CheckProfileLevel(ProfileLevel profilelevel, vector<AVCProfileType> profiles, vector<int> levels)
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

static bool UpdateProfileLevel(AL_TEncSettings& settings, ProfileLevel profilelevel, vector<AVCProfileType> profiles, vector<int> levels)
{
  if(!CheckProfileLevel(profilelevel, profiles, levels))
    return false;

  auto& channel = settings.tChParam[0];
  auto profile = ConvertModuleToSoftAVCProfile(profilelevel.profile.avc);
  channel.eProfile = profile;
  channel.uLevel = profilelevel.level;
  return true;
}

MediatypeInterface::ErrorType EncMediatypeAVC::Set(std::string index, void const* settings)
{
  if(!settings)
    return BAD_PARAMETER;

  if(index == "SETTINGS_INDEX_CLOCK")
  {
    auto clock = *(static_cast<Clock const*>(settings));

    if(!UpdateClock(this->settings, clock))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_GROUP_OF_PICTURES")
  {
    auto gop = *(static_cast<Gop const*>(settings));

    if(!UpdateGroupOfPictures(this->settings, gop))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOW_BANDWIDTH")
  {
    auto isLowBandwidthEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateLowBandwidth(this->settings, isLowBandwidthEnabled))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION")
  {
    auto isConstrainedIntraPredictionEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateConstrainedIntraPrediction(this->settings, isConstrainedIntraPredictionEnabled))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_ENTROPY_CODING")
  {
    auto entropyCoding = *(static_cast<EntropyCodingType const*>(settings));

    if(!UpdateEntropyCoding(this->settings, entropyCoding))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_VIDEO_MODE")
  {
    auto videoMode = *(static_cast<VideoModeType const*>(settings));

    if(!UpdateVideoMode(this->settings, videoMode))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BITRATE")
  {
    auto bitrate = *(static_cast<Bitrate const*>(settings));

    if(!UpdateBitrate(this->settings, bitrate))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_CACHE_LEVEL2")
  {
    auto isCacheLevel2Enabled = *(static_cast<bool const*>(settings));

    if(!UpdateCacheLevel2(this->settings, isCacheLevel2Enabled))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_FILLER_DATA")
  {
    auto isFillerDataEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateFillerData(this->settings, isFillerDataEnabled))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_ASPECT_RATIO")
  {
    auto aspectRatio = *(static_cast<AspectRatioType const*>(settings));

    if(!UpdateAspectRatio(this->settings, aspectRatio))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SCALING_LIST")
  {
    auto scalingList = *(static_cast<ScalingListType const*>(settings));

    if(!UpdateScalingList(this->settings, scalingList))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_QUANTIZATION_PARAMETER")
  {
    auto qps = *(static_cast<QPs const*>(settings));

    if(!UpdateQuantizationParameter(this->settings, qps))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER")
  {
    auto loopFilter = *(static_cast<LoopFilterType const*>(settings));

    if(!UpdateLoopFilter(this->settings, loopFilter))
      return BAD_PARAMETER;

    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_PROFILE_LEVEL")
  {
    auto profilelevel = *(static_cast<ProfileLevel const*>(settings));

    if(!UpdateProfileLevel(this->settings, profilelevel, profiles, levels))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_FORMAT")
  {
    auto format = *(static_cast<Format const*>(settings));

    if(!UpdateFormat(this->settings, format, this->colors, this->bitdepths, this->stride, this->strideAlignments))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SLICE_PARAMETER")
  {
    auto slices = *(static_cast<Slices const*>(settings));

    if(!UpdateSlicesParameter(this->settings, slices))
      return BAD_PARAMETER;

    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFER_HANDLES")
  {
    auto bufferHandles = *(static_cast<BufferHandles const*>(settings));

    if(!UpdateBufferHandles(this->bufferHandles, bufferHandles))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SUBFRAME")
  {
    auto isSubframeEnabled = *(static_cast<bool const*>(settings));

    if(!UpdateIsEnabledSubframe(this->settings, isSubframeEnabled))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_RESOLUTION")
  {
    auto resolution = *(static_cast<Resolution const*>(settings));

    if(!UpdateResolution(this->settings, this->stride, this->strideAlignments, resolution))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_COLOR_PRIMARIES")
  {
    auto colorimerty = *(static_cast<ColorPrimariesType const*>(settings));

    if(!UpdateColorPrimaries(this->settings, colorimerty))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_TRANSFER_CHARACTERISTICS")
  {
    auto transferCharac = *(static_cast<TransferCharacteristicsType const*>(settings));

    if(!UpdateTransferCharacteristics(this->settings, transferCharac))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_COLOUR_MATRIX")
  {
    auto colourMatrix = *(static_cast<ColourMatrixType const*>(settings));

    if(!UpdateColourMatrix(this->settings, colourMatrix))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOOKAHEAD")
  {
    auto la = *(static_cast<LookAhead const*>(settings));

    if(!UpdateLookAhead(this->settings, la))
      return BAD_PARAMETER;

    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_TWOPASS")
  {
    auto tp = *(static_cast<TwoPass const*>(settings));

    if(!UpdateTwoPass(this->settings, this->sTwoPassLogFile, tp))
      return BAD_PARAMETER;

    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_MAX_PICTURE_SIZES")
  {
    auto sizes = *(static_cast<MaxPicturesSizes const*>(settings));

    if(!UpdateMaxPictureSizes(this->settings, sizes))
      return BAD_PARAMETER;

    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER_BETA")
  {
    auto beta = *(static_cast<int const*>(settings));

    if(!UpdateLoopFilterBeta(this->settings, beta))
      return BAD_PARAMETER;

    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LOOP_FILTER_TC")
  {
    auto tc = *(static_cast<int const*>(settings));

    if(!UpdateLoopFilterTc(this->settings, tc))
      return BAD_PARAMETER;

    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_ACCESS_UNIT_DELIMITER")
  {
    auto aud = *(static_cast<bool const*>(settings));

    if(!UpdateAccessUnitDelimiter(this->settings, aud))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFERING_PERIOD_SEI")
  {
    auto bp = *(static_cast<bool const*>(settings));

    if(!UpdateBufferingPeriodSEI(this->settings, bp))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_PICTURE_TIMING_SEI")
  {
    auto pt = *(static_cast<bool const*>(settings));

    if(!UpdatePictureTimingSEI(this->settings, pt))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_RECOVERY_POINT_SEI")
  {
    auto rp = *(static_cast<bool const*>(settings));

    if(!UpdateRecoveryPointSEI(this->settings, rp))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_MASTERING_DISPLAY_COLOUR_VOLUME_SEI")
  {
    auto mdcv = *(static_cast<bool const*>(settings));

    if(!UpdateMasteringDisplayColourVolumeSEI(this->settings, mdcv))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_CONTENT_LIGHT_LEVEL_SEI")
  {
    auto cll = *(static_cast<bool const*>(settings));

    if(!UpdateContentLightLevelSEI(this->settings, cll))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_RATE_CONTROL_PLUGIN")
  {
    auto rcp = *(static_cast<RateControlPlugin const*>(settings));

    if(rcp.dmaBuf == -1)
      return BAD_PARAMETER;

    if(!SetRcPluginContext(this->allocator.get(), &this->settings, rcp))
      return BAD_PARAMETER;

    return SUCCESS;
  }

  return BAD_INDEX;
}

bool EncMediatypeAVC::Check()
{
  if(AL_Settings_CheckValidity(&settings, &settings.tChParam[0], stderr) != 0)
    return false;

  auto& channel = settings.tChParam[0];
  auto picFormat = AL_EncGetSrcPicFormat(AL_GET_CHROMA_MODE(channel.ePicFormat), AL_GET_BITDEPTH(channel.ePicFormat), AL_FB_RASTER, false);
  auto fourCC = AL_EncGetSrcFourCC(picFormat);
  assert(AL_GET_BITDEPTH(channel.ePicFormat) == channel.uSrcBitDepth);
  AL_Settings_CheckCoherency(&settings, &channel, fourCC, stdout);

  stride.horizontal = max(stride.horizontal, RoundUp(AL_EncGetMinPitch(channel.uEncWidth, AL_GET_BITDEPTH(channel.ePicFormat), AL_FB_RASTER), strideAlignments.horizontal));
  stride.vertical = max(stride.vertical, RoundUp(static_cast<int>(channel.uEncHeight), strideAlignments.vertical));

  return true;
}
