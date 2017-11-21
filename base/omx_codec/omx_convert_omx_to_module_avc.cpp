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

#include "omx_convert_omx_to_module_avc.h"

static inline AVCProfileType ConvertToModuleAVCProfile(OMX_VIDEO_AVCPROFILETYPE const& profile)
{
  switch(profile)
  {
  case OMX_VIDEO_AVCProfileBaseline: return AVC_PROFILE_BASELINE;
  case OMX_VIDEO_AVCProfileMain: return AVC_PROFILE_MAIN;
  case OMX_VIDEO_AVCProfileHigh: return AVC_PROFILE_HIGH;
  case OMX_VIDEO_AVCProfileHigh10: return AVC_PROFILE_HIGH10;
  case OMX_VIDEO_AVCProfileHigh422: return AVC_PROFILE_HIGH422;
  case OMX_VIDEO_AVCProfileMax:
  default: return AVC_PROFILE_MAX;
  }

  return AVC_PROFILE_MAX;
}

static inline int ConvertToModuleAVCLevel(OMX_VIDEO_AVCLEVELTYPE const& level)
{
  switch(level)
  {
  case OMX_VIDEO_AVCLevel1b: return 9;
  case OMX_VIDEO_AVCLevel1: return 10;
  case OMX_VIDEO_AVCLevel11: return 11;
  case OMX_VIDEO_AVCLevel12: return 12;
  case OMX_VIDEO_AVCLevel13: return 13;
  case OMX_VIDEO_AVCLevel2: return 20;
  case OMX_VIDEO_AVCLevel21: return 21;
  case OMX_VIDEO_AVCLevel22: return 22;
  case OMX_VIDEO_AVCLevel3: return 30;
  case OMX_VIDEO_AVCLevel31: return 31;
  case OMX_VIDEO_AVCLevel32: return 32;
  case OMX_VIDEO_AVCLevel4: return 40;
  case OMX_VIDEO_AVCLevel41: return 41;
  case OMX_VIDEO_AVCLevel42: return 42;
  case OMX_VIDEO_AVCLevel5: return 50;
  case OMX_VIDEO_AVCLevel51: return 51;
  default:

    if(level == static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel52))
      return 52;

    if(level == static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel60))
      return 60;

    if(level == static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel61))
      return 61;

    if(level == static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel62))
      return 62;
    return 0;
  }

  return 0;
}

ProfileLevelType ConvertToModuleAVCProfileLevel(OMX_VIDEO_AVCPROFILETYPE const& profile, OMX_VIDEO_AVCLEVELTYPE const& level)
{
  ProfileLevelType pf;
  pf.profile.avc = ConvertToModuleAVCProfile(profile);
  pf.level = ConvertToModuleAVCLevel(level);
  return pf;
}

