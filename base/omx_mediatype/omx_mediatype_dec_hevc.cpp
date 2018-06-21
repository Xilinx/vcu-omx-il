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

#include <cstring> // memset
#include <cmath>
#include "omx_mediatype_dec_hevc.h"
#include "omx_mediatype_dec_common.h"
#include "omx_mediatype_checks.h"
#include "omx_mediatype_common_hevc.h"
#include "omx_mediatype_common.h"
#include "base/omx_settings/omx_convert_module_soft_hevc.h"
#include "base/omx_utils/round.h"

using namespace std;

DecMediatypeHEVC::DecMediatypeHEVC()
{
  Reset();
}

DecMediatypeHEVC::~DecMediatypeHEVC() = default;

void DecMediatypeHEVC::Reset()
{
  memset(&settings, 0, sizeof(settings));
  settings.iStackSize = 5;
  settings.uFrameRate = 60000;
  settings.uClkRatio = 1000;
  settings.uDDRWidth = 32;
  settings.eDecUnit = AL_AU_UNIT;
  settings.eDpbMode = AL_DPB_NORMAL;
  settings.eFBStorageMode = AL_FB_RASTER;
  settings.eCodec = AL_CODEC_HEVC;

  auto& stream = settings.tStream;
  stream.tDim = { 176, 144 };
  stream.eChroma = CHROMA_4_2_0;
  stream.iBitDepth = 8;
  stream.iLevel = 10;
  stream.iProfileIdc = AL_PROFILE_HEVC_MAIN;
  stream.eSequenceMode = AL_SM_PROGRESSIVE;

  tier = 0;
  stride = RoundUp(AL_Decoder_GetMinPitch(stream.tDim.iWidth, stream.iBitDepth, settings.eFBStorageMode), strideAlignment);
  sliceHeight = RoundUp(AL_Decoder_GetMinStrideHeight(stream.tDim.iHeight), sliceHeightAlignment);
}

static bool IsHighTier(uint8_t tier)
{
  return tier != 0;
}

ProfileLevelType DecMediatypeHEVC::ProfileLevel() const
{
  auto stream = settings.tStream;
  return IsHighTier(tier) ? CreateHEVCHighTierProfileLevel(static_cast<AL_EProfile>(stream.iProfileIdc), stream.iLevel) : CreateHEVCMainTierProfileLevel(static_cast<AL_EProfile>(stream.iProfileIdc), stream.iLevel);
}

bool DecMediatypeHEVC::SetProfileLevel(ProfileLevelType profileLevel)
{
  if(!IsSupported(profileLevel.profile.hevc, profiles))
    return false;

  if(!IsSupported(profileLevel.level, levels))
    return false;

  auto profile = ConvertModuleToSoftHEVCProfile(profileLevel.profile.hevc);

  if(profile == AL_PROFILE_HEVC)
    return false;

  settings.tStream.iProfileIdc = profile;
  settings.tStream.iLevel = profileLevel.level;
  tier = IsHighTierProfile(profileLevel.profile.hevc) ? 1 : 0;

  return true;
}

static Mimes CreateMimes()
{
  Mimes mimes;
  auto& input = mimes.input;

  input.mime = "video/x-h265";
  input.compression = CompressionType::COMPRESSION_HEVC;

  auto& output = mimes.output;

  output.mime = "video/x-raw";
  output.compression = CompressionType::COMPRESSION_UNUSED;

  return mimes;
}

static int CreateLatency(AL_TDecSettings settings)
{
  auto stream = settings.tStream;
  auto buffers = AL_HEVC_GetMinOutputBuffersNeeded(stream, settings.iStackSize, settings.eDpbMode);

  if(settings.eDpbMode == AL_DPB_LOW_REF)
    buffers -= settings.iStackSize;

  if(settings.eDecUnit == AL_VCL_NAL_UNIT)
    buffers = 1;

  auto realFramerate = (static_cast<double>(settings.uFrameRate) / static_cast<double>(settings.uClkRatio));
  auto timeInMilliseconds = (static_cast<double>(buffers * 1000.0) / realFramerate);

  return ceil(timeInMilliseconds);
}

static BufferCounts CreateBufferCounts(AL_TDecSettings settings)
{
  BufferCounts bufferCounts;
  bufferCounts.input = 2;
  auto stream = settings.tStream;
  bufferCounts.output = AL_HEVC_GetMinOutputBuffersNeeded(stream, settings.iStackSize, settings.eDpbMode);
  return bufferCounts;
}

MediatypeInterface::ErrorSettingsType DecMediatypeHEVC::Get(std::string index, void* settings) const
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

  if(index == "SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER")
  {
    *(static_cast<int*>(settings)) = CreateInternalEntropyBuffer(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_LATENCY")
  {
    *(static_cast<int*>(settings)) = CreateLatency(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SEQUENCE_PICTURE_MODE")
  {
    *(static_cast<SequencePictureModeType*>(settings)) = CreateSequenceMode(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SEQUENCE_PICTURE_MODES_SUPPORTED")
  {
    *(static_cast<vector<SequencePictureModeType>*>(settings)) = this->sequenceModes;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_BUFFER_COUNTS")
  {
    *(static_cast<BufferCounts*>(settings)) = CreateBufferCounts(this->settings);
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

  return ERROR_SETTINGS_BAD_INDEX;
}

MediatypeInterface::ErrorSettingsType DecMediatypeHEVC::Set(std::string index, void const* settings)
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

  if(index == "SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER")
  {
    auto internalEntropyBuffer = *(static_cast<int const*>(settings));

    if(!UpdateInternalEntropyBuffer(this->settings, internalEntropyBuffer))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SEQUENCE_PICTURE_MODE")
  {
    auto sequenceMode = *(static_cast<SequencePictureModeType const*>(settings));

    if(!UpdateSequenceMode(this->settings, sequenceMode))
      return ERROR_SETTINGS_BAD_PARAMETER;
    return ERROR_SETTINGS_NONE;
  }

  return ERROR_SETTINGS_BAD_INDEX;
}

