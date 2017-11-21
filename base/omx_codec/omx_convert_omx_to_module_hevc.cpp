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

#include "omx_convert_omx_to_module_hevc.h"

static inline HEVCProfileType ConvertToModuleHEVCMainProfile(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile)
{
  switch(profile)
  {
  case OMX_ALG_VIDEO_HEVCProfileMain: return HEVC_PROFILE_MAIN;
  case OMX_ALG_VIDEO_HEVCProfileMain10: return HEVC_PROFILE_MAIN10;
  case OMX_ALG_VIDEO_HEVCProfileMainStill: return HEVC_PROFILE_MAINSTILL;
  case OMX_ALG_VIDEO_HEVCProfileMain422: return HEVC_PROFILE_MAIN422;
  case OMX_ALG_VIDEO_HEVCProfileMain422_10: return HEVC_PROFILE_MAIN422_10;
  case OMX_ALG_VIDEO_HEVCProfileMax:
  default: return HEVC_PROFILE_MAX;
  }

  return HEVC_PROFILE_MAX;
}

static inline HEVCProfileType ConvertToModuleHEVCHighProfile(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile)
{
  switch(profile)
  {
  case OMX_ALG_VIDEO_HEVCProfileMain: return HEVC_PROFILE_MAIN_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain10: return HEVC_PROFILE_MAIN10_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain422: return HEVC_PROFILE_MAIN422_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMain422_10: return HEVC_PROFILE_MAIN422_10_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMainStill: return HEVC_PROFILE_MAINSTILL_HIGH_TIER;
  case OMX_ALG_VIDEO_HEVCProfileMax:
  default: return HEVC_PROFILE_MAX;
  }

  return HEVC_PROFILE_MAX;
}

static inline int ConvertToModuleHEVCLevel(OMX_ALG_VIDEO_HEVCLEVELTYPE const& level)
{
  switch(level)
  {
  case OMX_ALG_VIDEO_HEVCMainTierLevel1: return 10;
  case OMX_ALG_VIDEO_HEVCMainTierLevel2: return 20;
  case OMX_ALG_VIDEO_HEVCMainTierLevel21: return 21;
  case OMX_ALG_VIDEO_HEVCMainTierLevel3: return 30;
  case OMX_ALG_VIDEO_HEVCMainTierLevel31: return 31;
  case OMX_ALG_VIDEO_HEVCMainTierLevel4:
  case OMX_ALG_VIDEO_HEVCHighTierLevel4: return 40;
  case OMX_ALG_VIDEO_HEVCMainTierLevel41:
  case OMX_ALG_VIDEO_HEVCHighTierLevel41: return 41;
  case OMX_ALG_VIDEO_HEVCMainTierLevel5:
  case OMX_ALG_VIDEO_HEVCHighTierLevel5: return 50;
  case OMX_ALG_VIDEO_HEVCMainTierLevel51:
  case OMX_ALG_VIDEO_HEVCHighTierLevel51: return 51;
  case OMX_ALG_VIDEO_HEVCMainTierLevel52:
  case OMX_ALG_VIDEO_HEVCHighTierLevel52: return 52;
  case OMX_ALG_VIDEO_HEVCMainTierLevel6:
  case OMX_ALG_VIDEO_HEVCHighTierLevel6: return 60;
  case OMX_ALG_VIDEO_HEVCMainTierLevel61:
  case OMX_ALG_VIDEO_HEVCHighTierLevel61: return 61;
  case OMX_ALG_VIDEO_HEVCMainTierLevel62:
  case OMX_ALG_VIDEO_HEVCHighTierLevel62: return 62;
  default:
    return 0;
  }

  return 0;
}

static inline bool IsMainTier(OMX_ALG_VIDEO_HEVCLEVELTYPE const& level)
{
  switch(level)
  {
  case OMX_ALG_VIDEO_HEVCHighTierLevel4:
  case OMX_ALG_VIDEO_HEVCHighTierLevel41:
  case OMX_ALG_VIDEO_HEVCHighTierLevel5:
  case OMX_ALG_VIDEO_HEVCHighTierLevel51:
  case OMX_ALG_VIDEO_HEVCHighTierLevel52:
  case OMX_ALG_VIDEO_HEVCHighTierLevel6:
  case OMX_ALG_VIDEO_HEVCHighTierLevel61:
  case OMX_ALG_VIDEO_HEVCHighTierLevel62: return false;
  default: return true;
  }

  return true;
}

ProfileLevelType ConvertToModuleHEVCProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile, OMX_ALG_VIDEO_HEVCLEVELTYPE const& level)
{
  ProfileLevelType pf;
  pf.profile.hevc = IsMainTier(level) ? ConvertToModuleHEVCMainProfile(profile) : ConvertToModuleHEVCHighProfile(profile);
  pf.level = ConvertToModuleHEVCLevel(level);
  return pf;
}

