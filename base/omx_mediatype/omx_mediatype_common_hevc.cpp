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

#include "omx_mediatype_common_hevc.h"
#include "omx_convert_module_soft_hevc.h"

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

vector<ProfileLevelType> CreateHEVCProfileLevelSupported(vector<HEVCProfileType> profiles, vector<int> levels)
{
  vector<ProfileLevelType> plSupported;

  for(auto profile : profiles)
  {
    for(auto level : levels)
    {
      if(isCompliant(profile, level))
      {
        ProfileLevelType pl;
        pl.profile.hevc = profile;
        pl.level = level;
        plSupported.push_back(pl);
      }
    }
  }

  return plSupported;
}

ProfileLevelType CreateHEVCMainTierProfileLevel(AL_EProfile profile, int level)
{
  ProfileLevelType pl;
  pl.profile.hevc = ConvertSoftToModuleHEVCMainTierProfile(profile);
  pl.level = level;
  return pl;
}

ProfileLevelType CreateHEVCHighTierProfileLevel(AL_EProfile profile, int level)
{
  ProfileLevelType pl;
  pl.profile.hevc = ConvertSoftToModuleHEVCHighTierProfile(profile);
  pl.level = level;
  return pl;
}

