// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "settings_codec_avc.h"
#include "convert_module_soft_avc.h"

using namespace std;

vector<ProfileLevel> CreateAVCProfileLevelSupported(vector<AVCProfileType> profiles, vector<int> levels)
{
  vector<ProfileLevel> plSupported;

  for(auto profile : profiles)
  {
    for(auto level : levels)
    {
      ProfileLevel pl;
      pl.profile.avc = profile;
      pl.level = level;
      plSupported.push_back(pl);
    }
  }

  return plSupported;
}

ProfileLevel CreateAVCProfileLevel(AL_EProfile profile, int level)
{
  ProfileLevel pl;
  pl.profile.avc = ConvertSoftToModuleAVCProfile(profile);
  pl.level = level;
  return pl;
}
