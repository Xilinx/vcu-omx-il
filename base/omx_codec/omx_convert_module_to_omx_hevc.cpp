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

#include "omx_convert_module_to_omx_hevc.h"

OMX_ALG_VIDEO_HEVCPROFILETYPE ConvertToOMXHEVCProfile(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.profile.hevc)
  {
  case HEVC_PROFILE_MAIN:
  case HEVC_PROFILE_MAIN_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain;
  case HEVC_PROFILE_MAIN10:
  case HEVC_PROFILE_MAIN10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain10;
  case HEVC_PROFILE_MAIN422:
  case HEVC_PROFILE_MAIN422_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422;
  case HEVC_PROFILE_MAIN422_10:
  case HEVC_PROFILE_MAIN422_10_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMain422_10;
  case HEVC_PROFILE_MAINSTILL:
  case HEVC_PROFILE_MAINSTILL_HIGH_TIER: return OMX_ALG_VIDEO_HEVCProfileMainStill;
  default: return OMX_ALG_VIDEO_HEVCProfileMax;
  }

  return OMX_ALG_VIDEO_HEVCProfileMax;
}

static inline OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertToOMXHEVCMainLevel(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.level)
  {
  case 10: return OMX_ALG_VIDEO_HEVCMainTierLevel1;
  case 20: return OMX_ALG_VIDEO_HEVCMainTierLevel2;
  case 21: return OMX_ALG_VIDEO_HEVCMainTierLevel21;
  case 30: return OMX_ALG_VIDEO_HEVCMainTierLevel3;
  case 31: return OMX_ALG_VIDEO_HEVCMainTierLevel31;
  case 40: return OMX_ALG_VIDEO_HEVCMainTierLevel4;
  case 41: return OMX_ALG_VIDEO_HEVCMainTierLevel41;
  case 50: return OMX_ALG_VIDEO_HEVCMainTierLevel5;
  case 51: return OMX_ALG_VIDEO_HEVCMainTierLevel51;
  case 52: return OMX_ALG_VIDEO_HEVCMainTierLevel52;
  case 60: return OMX_ALG_VIDEO_HEVCMainTierLevel6;
  case 61: return OMX_ALG_VIDEO_HEVCMainTierLevel61;
  case 62: return OMX_ALG_VIDEO_HEVCMainTierLevel62;
  default: return OMX_ALG_VIDEO_HEVCLevelMax;
  }

  return OMX_ALG_VIDEO_HEVCLevelMax;
}

static inline OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertToOMXHEVCHighLevel(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.level)
  {
  case 40: return OMX_ALG_VIDEO_HEVCHighTierLevel4;
  case 41: return OMX_ALG_VIDEO_HEVCHighTierLevel41;
  case 50: return OMX_ALG_VIDEO_HEVCHighTierLevel5;
  case 51: return OMX_ALG_VIDEO_HEVCHighTierLevel51;
  case 52: return OMX_ALG_VIDEO_HEVCHighTierLevel52;
  case 60: return OMX_ALG_VIDEO_HEVCHighTierLevel6;
  case 61: return OMX_ALG_VIDEO_HEVCHighTierLevel61;
  case 62: return OMX_ALG_VIDEO_HEVCHighTierLevel62;
  default: return OMX_ALG_VIDEO_HEVCLevelMax;
  }

  return OMX_ALG_VIDEO_HEVCLevelMax;
};

static inline bool IsMainTier(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.profile.hevc)
  {
  case HEVC_PROFILE_MAIN_HIGH_TIER:
  case HEVC_PROFILE_MAIN10_HIGH_TIER:
  case HEVC_PROFILE_MAIN422_HIGH_TIER:
  case HEVC_PROFILE_MAIN422_10_HIGH_TIER:
  case HEVC_PROFILE_MAINSTILL_HIGH_TIER: return false;
  default: return true;
  }

  return true;
}

OMX_ALG_VIDEO_HEVCLEVELTYPE ConvertToOMXHEVCLevel(ProfileLevelType const& profileLevel)
{
  return IsMainTier(profileLevel) ? ConvertToOMXHEVCMainLevel(profileLevel) : ConvertToOMXHEVCHighLevel(profileLevel);
}

