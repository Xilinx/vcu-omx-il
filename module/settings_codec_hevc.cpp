// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "settings_codec_hevc.h"
#include "convert_module_soft_hevc.h"

using namespace std;

bool IsHighTierProfile(HEVCProfileType profile)
{
  switch(profile)
  {
  case HEVCProfileType::HEVC_PROFILE_MAIN_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_10_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_STILL_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_10_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_12_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MONOCHROME_16_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_12_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_12_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_10_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_12_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_12_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_422_12_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_10_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_12_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16_INTRA_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_STILL_HIGH_TIER:
  case HEVCProfileType::HEVC_PROFILE_MAIN_444_16_STILL_HIGH_TIER:
    return true;
  default: return false;
  }

  return false;
}

static bool isCompliant(HEVCProfileType profile, int level)
{
  if(IsHighTierProfile(profile) && level < 40)
    return false;
  return true;
}

vector<ProfileLevel> CreateHEVCProfileLevelSupported(vector<HEVCProfileType> profiles, vector<int> levels)
{
  vector<ProfileLevel> plSupported;

  for(auto profile : profiles)
  {
    for(auto level : levels)
    {
      if(isCompliant(profile, level))
      {
        ProfileLevel pl;
        pl.profile.hevc = profile;
        pl.level = level;
        plSupported.push_back(pl);
      }
    }
  }

  return plSupported;
}

ProfileLevel CreateHEVCMainTierProfileLevel(AL_EProfile profile, int level)
{
  ProfileLevel pl;
  pl.profile.hevc = ConvertSoftToModuleHEVCMainTierProfile(profile);
  pl.level = level;
  return pl;
}

ProfileLevel CreateHEVCHighTierProfileLevel(AL_EProfile profile, int level)
{
  ProfileLevel pl;
  pl.profile.hevc = ConvertSoftToModuleHEVCHighTierProfile(profile);
  pl.level = level;
  return pl;
}
