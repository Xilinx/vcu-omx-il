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

#include <cstring> // memset
#include <cmath>
#include <cassert>
#include <utility/round.h>
#include "mediatype_dec_hevc.h"
#include "mediatype_dec_itu.h"
#include "mediatype_codec_hevc.h"
#include "mediatype_codec_itu.h"
#include "mediatype_checks.h"
#include "convert_module_soft_hevc.h"

using namespace std;

DecMediatypeHEVC::DecMediatypeHEVC(BufferContiguities bufferContiguities, BufferBytesAlignments bufferBytesAlignments, StrideAlignments strideAlignments)
{
  this->bufferContiguities.input = bufferContiguities.input;
  this->bufferContiguities.output = bufferContiguities.output;
  this->bufferBytesAlignments.input = bufferBytesAlignments.input;
  this->bufferBytesAlignments.output = bufferBytesAlignments.output;
  this->strideAlignments.horizontal = strideAlignments.horizontal;
  this->strideAlignments.vertical = strideAlignments.vertical;
  CreateFormatsSupportedMap(this->colors, this->bitdepths, this->supportedFormatsMap);
  Reset();
}

DecMediatypeHEVC::~DecMediatypeHEVC() = default;

void DecMediatypeHEVC::Reset()
{
  bufferHandles.input = BufferHandleType::BUFFER_HANDLE_CHAR_PTR;
  bufferHandles.output = BufferHandleType::BUFFER_HANDLE_CHAR_PTR;

  ::memset(&settings, 0, sizeof(settings));
  settings.iStackSize = 5;
  settings.uFrameRate = 60000;
  settings.uClkRatio = 1000;
  settings.uDDRWidth = 64;
  settings.eDecUnit = AL_AU_UNIT;
  settings.eDpbMode = AL_DPB_NORMAL;
  settings.bLowLat = false;
  settings.eFBStorageMode = AL_FB_RASTER;
  settings.eCodec = AL_CODEC_HEVC;
  settings.bUseIFramesAsSyncPoint = true;
  settings.eInputMode = AL_DEC_UNSPLIT_INPUT;

  auto& stream = settings.tStream;
  stream.tDim = { 176, 144 };
  stream.eChroma = AL_CHROMA_4_2_0;
  stream.iBitDepth = 8;
  stream.iLevel = 10;
  stream.iProfileIdc = AL_PROFILE_HEVC_MAIN;
  stream.eSequenceMode = AL_SM_PROGRESSIVE;

  tier = 0;
  this->stride.horizontal = RoundUp(static_cast<int>(AL_Decoder_GetMinPitch(stream.tDim.iWidth, stream.iBitDepth, settings.eFBStorageMode)), strideAlignments.horizontal);
  this->stride.vertical = RoundUp(static_cast<int>(AL_Decoder_GetMinStrideHeight(stream.tDim.iHeight)), strideAlignments.vertical);
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
  double buffers = static_cast<double>(AL_HEVC_GetMinOutputBuffersNeeded(&stream, settings.iStackSize));

  if(settings.eDpbMode == AL_DPB_NO_REORDERING)
    buffers = 3;

  if(settings.eDecUnit == AL_VCL_NAL_UNIT)
    buffers = 1;

  if(settings.bUseEarlyCallback)
  {
    assert(settings.eDecUnit == AL_VCL_NAL_UNIT);
    assert(settings.eDpbMode == AL_DPB_NO_REORDERING);
    buffers = 0.5;
  }

  double realFramerate = (static_cast<double>(settings.uFrameRate) / static_cast<double>(settings.uClkRatio));
  double timeInMilliseconds = (static_cast<double>(buffers * 1000.0) / realFramerate);

  return ceil(timeInMilliseconds);
}

static BufferCounts CreateBufferCounts(AL_TDecSettings settings)
{
  BufferCounts bufferCounts;
  bufferCounts.input = 2;
  auto stream = settings.tStream;
  bufferCounts.output = AL_HEVC_GetMinOutputBuffersNeeded(&stream, settings.iStackSize);
  return bufferCounts;
}

static ProfileLevel CreateProfileLevel(AL_TDecSettings settings, int tier)
{
  auto stream = settings.tStream;
  return IsHighTier(tier) ? CreateHEVCHighTierProfileLevel(static_cast<AL_EProfile>(stream.iProfileIdc), stream.iLevel) : CreateHEVCMainTierProfileLevel(static_cast<AL_EProfile>(stream.iProfileIdc), stream.iLevel);
}

MediatypeInterface::ErrorType DecMediatypeHEVC::Get(std::string index, void* settings) const
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

  if(index == "SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER")
  {
    *(static_cast<int*>(settings)) = CreateInternalEntropyBuffer(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LATENCY")
  {
    *(static_cast<int*>(settings)) = CreateLatency(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SEQUENCE_PICTURE_MODE")
  {
    *(static_cast<SequencePictureModeType*>(settings)) = CreateSequenceMode(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SEQUENCE_PICTURE_MODES_SUPPORTED")
  {
    *(static_cast<vector<SequencePictureModeType>*>(settings)) = this->sequenceModes;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFER_HANDLES")
  {
    *(static_cast<BufferHandles*>(settings)) = this->bufferHandles;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_BUFFER_COUNTS")
  {
    *(static_cast<BufferCounts*>(settings)) = CreateBufferCounts(this->settings);
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

  if(index == "SETTINGS_INDEX_PROFILE_LEVEL")
  {
    *(static_cast<ProfileLevel*>(settings)) = CreateProfileLevel(this->settings, tier);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED")
  {
    *(static_cast<vector<ProfileLevel>*>(settings)) = CreateHEVCProfileLevelSupported(profiles, levels);
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

  if(index == "SETTINGS_INDEX_SUBFRAME")
  {
    *(static_cast<bool*>(settings)) = (this->settings.eDecUnit == AL_VCL_NAL_UNIT);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_RESOLUTION")
  {
    *(static_cast<Resolution*>(settings)) = CreateResolution(this->settings, this->stride);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_DECODED_PICTURE_BUFFER")
  {
    *(static_cast<DecodedPictureBufferType*>(settings)) = CreateDecodedPictureBuffer(this->settings);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LLP2_EARLY_CB")
  {
    *(static_cast<bool*>(settings)) = this->settings.bUseEarlyCallback;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_INPUT_PARSED")
  {
    *(static_cast<bool*>(settings)) = (this->settings.eInputMode == AL_DEC_SPLIT_INPUT);
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_OUTPUT_POSITION")
  {
    *(static_cast<Point<int>*>(settings)) = CreateOutputPosition(this->settings);
    return SUCCESS;
  }

  return BAD_INDEX;
}

static bool CheckProfileLevel(ProfileLevel profilelevel, vector<HEVCProfileType> profiles, vector<int> levels)
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

static bool UpdateProfileLevel(AL_TDecSettings& settings, int& tier, ProfileLevel profilelevel, vector<HEVCProfileType> profiles, vector<int> levels)
{
  if(!CheckProfileLevel(profilelevel, profiles, levels))
    return false;

  auto profile = ConvertModuleToSoftHEVCProfile(profilelevel.profile.hevc);

  settings.tStream.iProfileIdc = profile;
  settings.tStream.iLevel = profilelevel.level;
  tier = IsHighTierProfile(profilelevel.profile.hevc) ? 1 : 0;
  return true;
}

MediatypeInterface::ErrorType DecMediatypeHEVC::Set(std::string index, void const* settings)
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

  if(index == "SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER")
  {
    auto internalEntropyBuffer = *(static_cast<int const*>(settings));

    if(!UpdateInternalEntropyBuffer(this->settings, internalEntropyBuffer))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_SEQUENCE_PICTURE_MODE")
  {
    auto sequenceMode = *(static_cast<SequencePictureModeType const*>(settings));

    if(!UpdateSequenceMode(this->settings, sequenceMode, sequenceModes))
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

  if(index == "SETTINGS_INDEX_PROFILE_LEVEL")
  {
    auto profilelevel = *(static_cast<ProfileLevel const*>(settings));

    if(!UpdateProfileLevel(this->settings, tier, profilelevel, profiles, levels))
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

  if(index == "SETTINGS_INDEX_DECODED_PICTURE_BUFFER")
  {
    auto decodedPictureBuffer = *(static_cast<DecodedPictureBufferType const*>(settings));

    if(!UpdateDecodedPictureBuffer(this->settings, decodedPictureBuffer))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_LLP2_EARLY_CB")
  {
    this->settings.bUseEarlyCallback = *(static_cast<bool const*>(settings));
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_INPUT_PARSED")
  {
    this->settings.eInputMode = *(static_cast<bool const*>(settings)) ? AL_DEC_SPLIT_INPUT : AL_DEC_UNSPLIT_INPUT;
    return SUCCESS;
  }

  if(index == "SETTINGS_INDEX_OUTPUT_POSITION")
  {
    auto position = *(static_cast<Point<int> const*>(settings));

    if(!UpdateOutputPosition(this->settings, position))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  return BAD_INDEX;
}

bool DecMediatypeHEVC::Check()
{
  int tmp_height = settings.tStream.tDim.iHeight;

  settings.tStream.tDim.iHeight = RoundUp(settings.tStream.tDim.iHeight, 16);

  if(AL_DecSettings_CheckValidity(&settings, stderr) != 0)
    return false;

  AL_DecSettings_CheckCoherency(&settings, stdout);

  settings.tStream.tDim.iHeight = tmp_height;

  return true;
}
