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

#include "omx_convert_module_to_omx_avc.h"

OMX_VIDEO_AVCPROFILETYPE ConvertToOMXAVCProfile(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.profile.avc)
  {
  case AVC_PROFILE_BASELINE: return OMX_VIDEO_AVCProfileBaseline;
  case AVC_PROFILE_MAIN: return OMX_VIDEO_AVCProfileMain;
  case AVC_PROFILE_HIGH: return OMX_VIDEO_AVCProfileHigh;
  case AVC_PROFILE_HIGH10: return OMX_VIDEO_AVCProfileHigh10;
  case AVC_PROFILE_HIGH422: return OMX_VIDEO_AVCProfileHigh422;
  default: return OMX_VIDEO_AVCProfileMax;
  }

  return OMX_VIDEO_AVCProfileMax;
}

OMX_VIDEO_AVCLEVELTYPE ConvertToOMXAVCLevel(ProfileLevelType const& profileLevel)
{
  switch(profileLevel.level)
  {
  case 9: return OMX_VIDEO_AVCLevel1b;
  case 10: return OMX_VIDEO_AVCLevel1;
  case 11: return OMX_VIDEO_AVCLevel11;
  case 12: return OMX_VIDEO_AVCLevel12;
  case 13: return OMX_VIDEO_AVCLevel13;
  case 20: return OMX_VIDEO_AVCLevel2;
  case 21: return OMX_VIDEO_AVCLevel21;
  case 22: return OMX_VIDEO_AVCLevel22;
  case 30: return OMX_VIDEO_AVCLevel3;
  case 31: return OMX_VIDEO_AVCLevel31;
  case 32: return OMX_VIDEO_AVCLevel32;
  case 40: return OMX_VIDEO_AVCLevel4;
  case 41: return OMX_VIDEO_AVCLevel41;
  case 42: return OMX_VIDEO_AVCLevel42;
  case 50: return OMX_VIDEO_AVCLevel5;
  case 51: return OMX_VIDEO_AVCLevel51;
  case 52: return static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel52);
  case 60: return static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel60);
  case 61: return static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel61);
  case 62: return static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel62);
  default: return OMX_VIDEO_AVCLevelMax;
  }

  return OMX_VIDEO_AVCLevelMax;
};

