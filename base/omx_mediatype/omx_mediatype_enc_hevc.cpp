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

#include <assert.h>

EncMediatypeHEVC::EncMediatypeHEVC()
{
  Reset();
}

EncMediatypeHEVC::~EncMediatypeHEVC()
{
}

void EncMediatypeHEVC::Reset()
{
  strideAlignment = 32;
  sliceHeightAlignment = 8;
  AL_Settings_SetDefaults(&settings);
  auto& chan = settings.tChParam;
  chan.eProfile = AL_PROFILE_HEVC_MAIN;
  AL_Settings_SetDefaultParam(&settings);
  chan.uLevel = 10;
}

CompressionType EncMediatypeHEVC::Compression() const
{
  return COMPRESSION_HEVC;
}

std::string EncMediatypeHEVC::Mime() const
{
  return "video/x-h265";
}

std::vector<ProfileLevelType> EncMediatypeHEVC::ProfileLevelSupported() const
{
  std::vector<ProfileLevelType> vector;

  for(auto const& profile : profiles)
    for(auto const & level : levels)
    {
      ProfileLevelType tmp;
      tmp.profile.hevc = profile;
      tmp.level = level;
      vector.push_back(tmp);
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
  case HEVC_PROFILE_MAIN10_HIGH_TIER:
  case HEVC_PROFILE_MAIN422_HIGH_TIER:
  case HEVC_PROFILE_MAINSTILL_HIGH_TIER:
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
  case AL_PROFILE_HEVC_MAIN10: return HEVC_PROFILE_MAIN10_HIGH_TIER;
  case AL_PROFILE_HEVC_MAIN_422: return HEVC_PROFILE_MAIN422_HIGH_TIER;
  case AL_PROFILE_HEVC_MAIN_STILL: return HEVC_PROFILE_MAINSTILL_HIGH_TIER;
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
  case AL_PROFILE_HEVC_MAIN10: return HEVC_PROFILE_MAIN10;
  case AL_PROFILE_HEVC_MAIN_422: return HEVC_PROFILE_MAIN422;
  case AL_PROFILE_HEVC_MAIN_STILL: return HEVC_PROFILE_MAINSTILL;
  default:
    return HEVC_PROFILE_MAX;
  }

  return HEVC_PROFILE_MAX;
}

ProfileLevelType EncMediatypeHEVC::ProfileLevel() const
{
  auto const chan = settings.tChParam;
  ProfileLevelType p;
  p.profile.hevc = IsHighTier(chan.uTier) ? ToHighProfile(chan.eProfile) : ToMainProfile(chan.eProfile);
  p.level = chan.uLevel;
  return p;
}

static AL_EProfile ToProfile(HEVCProfileType const& profile)
{
  switch(profile)
  {
  case HEVC_PROFILE_MAIN:
  case HEVC_PROFILE_MAIN_HIGH_TIER: return AL_PROFILE_HEVC_MAIN;
  case HEVC_PROFILE_MAIN10:
  case HEVC_PROFILE_MAIN10_HIGH_TIER: return AL_PROFILE_HEVC_MAIN10;
  case HEVC_PROFILE_MAIN422:
  case HEVC_PROFILE_MAIN422_HIGH_TIER: return AL_PROFILE_HEVC_MAIN_422;
  case HEVC_PROFILE_MAINSTILL:
  case HEVC_PROFILE_MAINSTILL_HIGH_TIER: return AL_PROFILE_HEVC_MAIN_STILL;
  default: return AL_PROFILE_HEVC;
  }

  return AL_PROFILE_HEVC;
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

bool EncMediatypeHEVC::IsInLevelsSupported(int const& level)
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

  auto const profile = ToProfile(profileLevel.profile.hevc);

  if(profile == AL_PROFILE_HEVC)
    return false;

  auto& chan = settings.tChParam;
  chan.eProfile = profile;
  chan.uLevel = profileLevel.level;
  chan.uTier = IsHighTier(profileLevel.profile.hevc) ? 1 : 0;
  return true;
}

EntropyCodingType EncMediatypeHEVC::EntropyCoding() const
{
  auto const chan = settings.tChParam;
  assert(chan.eEntropyMode == AL_MODE_CABAC);
  return ENTROPY_CODING_CABAC;
}

bool EncMediatypeHEVC::SetEntropyCoding(EntropyCodingType const& entropyCoding)
{
  if(entropyCoding != ENTROPY_CODING_CABAC)
    return false;

  auto& chan = settings.tChParam;
  chan.eEntropyMode = AL_MODE_CABAC;
  return true;
}

bool EncMediatypeHEVC::IsConstrainedIntraPrediction() const
{
  auto const chan = settings.tChParam;
  return chan.eOptions & AL_OPT_CONST_INTRA_PRED;
}

bool EncMediatypeHEVC::SetConstrainedIntraPrediction(bool const& constrainedIntraPrediction)
{
  auto& opt = settings.tChParam.eOptions;

  if(constrainedIntraPrediction)
    opt = static_cast<AL_EChEncOption>(opt | AL_OPT_CONST_INTRA_PRED);

  return true;
}

static inline LoopFilterType ToLoopFilter(AL_EChEncOption const& opt)
{
  if(opt & (AL_OPT_LF_X_SLICE | AL_OPT_LF_X_TILE | AL_OPT_LF))
    return LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE;

  if(opt & (AL_OPT_LF_X_TILE | AL_OPT_LF))
    return LOOP_FILTER_ENABLE_CROSS_TILE;

  if(opt & (AL_OPT_LF_X_SLICE | AL_OPT_LF))
    return LOOP_FILTER_ENABLE_CROSS_SLICE;

  if(opt & AL_OPT_LF)
    return LOOP_FILTER_ENABLE;

  return LOOP_FILTER_DISABLE;
}

LoopFilterType EncMediatypeHEVC::LoopFilter() const
{
  auto const chan = settings.tChParam;
  return ToLoopFilter(chan.eOptions);
}

bool EncMediatypeHEVC::SetLoopFilter(LoopFilterType const& loopFilter)
{
  auto& opt = settings.tChParam.eOptions;

  if(loopFilter == LOOP_FILTER_MAX)
    return false;

  if(loopFilter == LOOP_FILTER_DISABLE)
    opt = static_cast<AL_EChEncOption>(opt & ~(AL_OPT_LF | AL_OPT_LF_X_SLICE | AL_OPT_LF_X_TILE));
  else // LoopFilter is enable
  {
    opt = static_cast<AL_EChEncOption>(opt | AL_OPT_LF);
    switch(loopFilter)
    {
    case LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE:
    {
      opt = static_cast<AL_EChEncOption>(opt | AL_OPT_LF | AL_OPT_LF_X_SLICE | AL_OPT_LF_X_TILE);
      break;
    }
    case LOOP_FILTER_ENABLE_CROSS_TILE:
    {
      opt = static_cast<AL_EChEncOption>(opt | AL_OPT_LF | AL_OPT_LF_X_TILE);
      break;
    }
    case LOOP_FILTER_ENABLE_CROSS_SLICE:
    {
      opt = static_cast<AL_EChEncOption>(opt | AL_OPT_LF | AL_OPT_LF_X_SLICE);
      break;
    }
    default:
      opt = static_cast<AL_EChEncOption>(opt | AL_OPT_LF);
    }
  }

  return true;
}

bool EncMediatypeHEVC::IsEnableLowBandwidth() const
{
  auto const chan = settings.tChParam;
  return chan.pMeRange[SLICE_P][1] == 16;
}

bool EncMediatypeHEVC::SetEnableLowBandwidth(bool const& enable)
{
  auto& chan = settings.tChParam;
  auto bw = enable ? 16 : 32;
  chan.pMeRange[SLICE_P][1] = bw;
  return true;
}

