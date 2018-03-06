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

#include "omx_mediatype_dec_hevc.h"
#include "base/omx_settings/omx_convert_module_soft.h"
#include <string.h> // memset

using namespace std;

DecMediatypeHEVC::DecMediatypeHEVC()
{
  Reset();
}

void DecMediatypeHEVC::Reset()
{
  strideAlignment = 64;
  sliceHeightAlignment = 64;
  memset(&settings, 0, sizeof(settings));
  settings.iStackSize = 5;
  settings.uFrameRate = 60000;
  settings.uClkRatio = 1000;
  settings.uDDRWidth = 32;
  settings.eDecUnit = AL_AU_UNIT;
  settings.eDpbMode = AL_DPB_NORMAL;
  settings.eFBStorageMode = AL_FB_RASTER;
  settings.bIsAvc = false;

  settings.tStream.tDim = { 176, 144 };
  settings.tStream.eChroma = CHROMA_4_2_0;
  settings.tStream.iBitDepth = 8;
  settings.tStream.iLevel = 10;
  settings.tStream.iProfileIdc = AL_PROFILE_HEVC_MAIN;
  tier = 0;
}

CompressionType DecMediatypeHEVC::Compression() const
{
  return COMPRESSION_HEVC;
}

string DecMediatypeHEVC::Mime() const
{
  return "video/x-h265";
}

static bool isHighTierProfile(HEVCProfileType const& profile)
{
  switch(profile)
  {
  case HEVC_PROFILE_MAIN_HIGH_TIER:
  case HEVC_PROFILE_MAIN_10_HIGH_TIER:
  case HEVC_PROFILE_MAIN_422_HIGH_TIER:
  case HEVC_PROFILE_MAIN_422_10_HIGH_TIER:
  case HEVC_PROFILE_MAIN_STILL_HIGH_TIER:
    return true;
  default: return false;
  }

  return false;
}

static bool isCompliant(HEVCProfileType const& profile, int const& level)
{
  if(isHighTierProfile(profile) && level < 40)
    return false;
  return true;
}

vector<ProfileLevelType> DecMediatypeHEVC::ProfileLevelSupported() const
{
  vector<ProfileLevelType> vector;

  for(auto const& profile : profiles)
    for(auto const & level : levels)
    {
      if(isCompliant(profile, level))
      {
        ProfileLevelType tmp;
        tmp.profile.hevc = profile;
        tmp.level = level;
        vector.push_back(tmp);
      }
    }

  return vector;
}

static bool IsHighTier(uint8_t const& tier)
{
  return tier != 0;
}

static bool IsHighTier(HEVCProfileType const& profile)
{
  switch(profile)
  {
  case HEVC_PROFILE_MAIN_HIGH_TIER:
  case HEVC_PROFILE_MAIN_10_HIGH_TIER:
  case HEVC_PROFILE_MAIN_422_HIGH_TIER:
  case HEVC_PROFILE_MAIN_STILL_HIGH_TIER:
    return true;
  default: return false;
  }

  return false;
}

static HEVCProfileType ToHighProfile(AL_EProfile const& profile)
{
  if(!AL_IS_HEVC(profile))
    return HEVC_PROFILE_MAX;
  switch(profile)
  {
  case AL_PROFILE_HEVC_MAIN: return HEVC_PROFILE_MAIN_HIGH_TIER;
  case AL_PROFILE_HEVC_MAIN10: return HEVC_PROFILE_MAIN_10_HIGH_TIER;
  case AL_PROFILE_HEVC_MAIN_422: return HEVC_PROFILE_MAIN_422_HIGH_TIER;
  case AL_PROFILE_HEVC_MAIN_422_10: return HEVC_PROFILE_MAIN_422_10_HIGH_TIER;
  case AL_PROFILE_HEVC_MAIN_STILL: return HEVC_PROFILE_MAIN_STILL_HIGH_TIER;
  default:
    return HEVC_PROFILE_MAX;
  }

  return HEVC_PROFILE_MAX;
}

static HEVCProfileType ToMainProfile(AL_EProfile const& profile)
{
  if(!AL_IS_HEVC(profile))
    return HEVC_PROFILE_MAX;
  switch(profile)
  {
  case AL_PROFILE_HEVC_MAIN: return HEVC_PROFILE_MAIN;
  case AL_PROFILE_HEVC_MAIN10: return HEVC_PROFILE_MAIN_10;
  case AL_PROFILE_HEVC_MAIN_422: return HEVC_PROFILE_MAIN_422;
  case AL_PROFILE_HEVC_MAIN_422_10: return HEVC_PROFILE_MAIN_422_10;
  case AL_PROFILE_HEVC_MAIN_STILL: return HEVC_PROFILE_MAIN_STILL;
  default:
    return HEVC_PROFILE_MAX;
  }

  return HEVC_PROFILE_MAX;
}

ProfileLevelType DecMediatypeHEVC::ProfileLevel() const
{
  ProfileLevelType p;
  p.profile.hevc = IsHighTier(tier) ? ToHighProfile(static_cast<AL_EProfile>(settings.tStream.iProfileIdc)) : ToMainProfile(static_cast<AL_EProfile>(settings.tStream.iProfileIdc));
  p.level = settings.tStream.iLevel;
  return p;
}

static AL_EProfile ToProfile(HEVCProfileType const& profile)
{
  switch(profile)
  {
  case HEVC_PROFILE_MAIN:
  case HEVC_PROFILE_MAIN_HIGH_TIER: return AL_PROFILE_HEVC_MAIN;
  case HEVC_PROFILE_MAIN_10:
  case HEVC_PROFILE_MAIN_10_HIGH_TIER: return AL_PROFILE_HEVC_MAIN10;
  case HEVC_PROFILE_MAIN_422:
  case HEVC_PROFILE_MAIN_422_HIGH_TIER: return AL_PROFILE_HEVC_MAIN_422;
  case HEVC_PROFILE_MAIN_422_10:
  case HEVC_PROFILE_MAIN_422_10_HIGH_TIER: return AL_PROFILE_HEVC_MAIN_422_10;
  case HEVC_PROFILE_MAIN_STILL:
  case HEVC_PROFILE_MAIN_STILL_HIGH_TIER: return AL_PROFILE_HEVC_MAIN_STILL;
  default: return AL_PROFILE_HEVC;
  }

  return AL_PROFILE_HEVC;
}

bool DecMediatypeHEVC::IsInProfilesSupported(HEVCProfileType const& profile)
{
  for(auto const& p : profiles)
  {
    if(p == profile)
      return true;
  }

  return false;
}

bool DecMediatypeHEVC::IsInLevelsSupported(int const& level)
{
  for(auto const& l : levels)
  {
    if(l == level)
      return true;
  }

  return false;
}

bool DecMediatypeHEVC::SetProfileLevel(ProfileLevelType const& profileLevel)
{
  if(!IsInProfilesSupported(profileLevel.profile.hevc))
    return false;

  if(!IsInLevelsSupported(profileLevel.level))
    return false;

  auto const profile = ToProfile(profileLevel.profile.hevc);

  if(profile == AL_PROFILE_HEVC)
    return false;

  settings.tStream.iProfileIdc = profile;
  settings.tStream.iLevel = profileLevel.level;
  tier = IsHighTier(profileLevel.profile.hevc) ? 1 : 0;

  return true;
}

int DecMediatypeHEVC::GetRequiredOutputBuffers() const
{
  return AL_HEVC_GetMinOutputBuffersNeeded(settings.tStream, settings.iStackSize, settings.eDpbMode);
}

Format DecMediatypeHEVC::GetFormat() const
{
  Format format;
  format.color = ConvertSoftToModuleColor(settings.tStream.eChroma);
  format.bitdepth = settings.tStream.iBitDepth;
  return format;
}

vector<Format> DecMediatypeHEVC::FormatsSupported() const
{
  vector<Format> formatsSupported;
  formatsSupported.push_back(GetFormat());
  return formatsSupported;
}

