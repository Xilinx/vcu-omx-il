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

#include <cstring> // memset
#include <cmath>
#include "omx_mediatype_dec_hevc.h"
#include "omx_mediatype_dec_common.h"
#include "omx_mediatype_checks.h"
#include "omx_mediatype_common_hevc.h"
#include "omx_mediatype_common.h"
#include "omx_convert_module_soft_hevc.h"
#include "base/omx_utils/round.h"

using namespace std;

DecMediatypeHEVC::DecMediatypeHEVC()
{
  strideAlignment.widthStride = 64;
  strideAlignment.heightStride = 64;
  Reset();
}

DecMediatypeHEVC::~DecMediatypeHEVC() = default;

void DecMediatypeHEVC::Reset()
{
  bufferHandles.input = BufferHandleType::BUFFER_HANDLE_CHAR_PTR;
  bufferHandles.output = BufferHandleType::BUFFER_HANDLE_CHAR_PTR;

  memset(&settings, 0, sizeof(settings));
  settings.iStackSize = 5;
  settings.uFrameRate = 60000;
  settings.uClkRatio = 1000;
  settings.uDDRWidth = 64;
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
  stride = RoundUp(AL_Decoder_GetMinPitch(stream.tDim.iWidth, stream.iBitDepth, settings.eFBStorageMode), strideAlignment.widthStride);
  sliceHeight = RoundUp(AL_Decoder_GetMinStrideHeight(stream.tDim.iHeight), strideAlignment.heightStride);
}

static bool IsHighTier(uint8_t tier)
{
  return tier != 0;
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
  auto buffers = AL_HEVC_GetMinOutputBuffersNeeded(stream, settings.iStackSize);

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
  bufferCounts.output = AL_HEVC_GetMinOutputBuffersNeeded(stream, settings.iStackSize);
  return bufferCounts;
}

static ProfileLevelType CreateProfileLevel(AL_TDecSettings settings, int tier)
{
  auto stream = settings.tStream;
  return IsHighTier(tier) ? CreateHEVCHighTierProfileLevel(static_cast<AL_EProfile>(stream.iProfileIdc), stream.iLevel) : CreateHEVCMainTierProfileLevel(static_cast<AL_EProfile>(stream.iProfileIdc), stream.iLevel);
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

  if(index == "SETTINGS_INDEX_STRIDE_ALIGNMENT")
  {
    *(static_cast<Stride*>(settings)) = this->strideAlignment;
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

  if(index == "SETTINGS_INDEX_PROFILE_LEVEL")
  {
    *(static_cast<ProfileLevelType*>(settings)) = CreateProfileLevel(this->settings, tier);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED")
  {
    *(static_cast<vector<ProfileLevelType>*>(settings)) = CreateHEVCProfileLevelSupported(profiles, levels);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FORMAT")
  {
    *(static_cast<Format*>(settings)) = CreateFormat(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_FORMATS_SUPPORTED")
  {
    *(static_cast<vector<Format>*>(settings)) = CreateFormatsSupported(colors, bitdepths);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_SUBFRAME")
  {
    *(static_cast<bool*>(settings)) = (this->settings.eDecUnit == AL_VCL_NAL_UNIT);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_RESOLUTION")
  {
    *(static_cast<Resolution*>(settings)) = CreateResolution(this->settings, stride, sliceHeight);
    return ERROR_SETTINGS_NONE;
  }

  if(index == "SETTINGS_INDEX_DECODED_PICTURE_BUFFER")
  {
    *(static_cast<DecodedPictureBufferType*>(settings)) = CreateDecodedPictureBuffer(this->settings);
    return ERROR_SETTINGS_NONE;
  }

  return ERROR_SETTINGS_BAD_INDEX;
}

static bool CheckProfileLevel(ProfileLevelType profilelevel, vector<HEVCProfileType> profiles, vector<int> levels)
{
  if(!IsSupported(profilelevel.profile.hevc, profiles))
    return false;

  if(!IsSupported(profilelevel.level, levels))
    return false;

  auto profile = ConvertModuleToSoftHEVCProfile(profilelevel.profile.hevc);

  if(!AL_IS_HEVC(profile))
    return false;

  return true;
}

static bool UpdateProfileLevel(AL_TDecSettings& settings, int& tier, ProfileLevelType profilelevel, vector<HEVCProfileType> profiles, vector<int> levels)
{
  if(!CheckProfileLevel(profilelevel, profiles, levels))
    return false;

  auto profile = ConvertModuleToSoftHEVCProfile(profilelevel.profile.hevc);

  settings.tStream.iProfileIdc = profile;
  settings.tStream.iLevel = profilelevel.level;
  tier = IsHighTierProfile(profilelevel.profile.hevc) ? 1 : 0;
  return true;
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

    if(!UpdateSequenceMode(this->settings, sequenceMode, sequenceModes))
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

  if(index == "SETTINGS_INDEX_PROFILE_LEVEL")
  {
    auto profilelevel = *(static_cast<ProfileLevelType const*>(settings));

    if(!UpdateProfileLevel(this->settings, tier, profilelevel, profiles, levels))
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

  if(index == "SETTINGS_INDEX_DECODED_PICTURE_BUFFER")
  {
    auto decodedPictureBuffer = *(static_cast<DecodedPictureBufferType const*>(settings));

    if(!UpdateDecodedPictureBuffer(this->settings, decodedPictureBuffer))
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

  return ERROR_SETTINGS_BAD_INDEX;
}

