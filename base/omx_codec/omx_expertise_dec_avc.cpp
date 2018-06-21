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

#include "omx_expertise_dec_avc.h"
#include "omx_convert_omx_module.h"

static bool SetModuleProfileLevel(OMX_VIDEO_AVCPROFILETYPE const& profile, OMX_VIDEO_AVCLEVELTYPE const& level, DecModule& module)
{
  ProfileLevelType p;
  p.profile.avc = ConvertOMXToModuleAVCProfileLevel(profile, level).profile.avc;
  p.level = ConvertOMXToModuleAVCProfileLevel(profile, level).level;
  return module.SetProfileLevel(p);
}

bool DecExpertiseAVC::GetProfileLevelSupported(OMX_PTR param, DecModule const& module)
{
  auto supported = module.GetProfileLevelSupported();
  auto& pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;

  if(pl.nProfileIndex >= supported.size())
    return false;

  pl.eProfile = ConvertModuleToOMXAVCProfile(supported[pl.nProfileIndex]);
  pl.eLevel = ConvertModuleToOMXAVCLevel(supported[pl.nProfileIndex]);

  return true;
}

void DecExpertiseAVC::GetProfileLevel(OMX_PTR param, Port const& port, DecModule const& module)
{
  auto& pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;
  pl.nPortIndex = port.index;
  pl.eProfile = ConvertModuleToOMXAVCProfile(module.GetProfileLevel());
  pl.eLevel = ConvertModuleToOMXAVCLevel(module.GetProfileLevel());
}

bool DecExpertiseAVC::SetProfileLevel(OMX_PTR param, Port const& port, DecModule& module)
{
  OMX_VIDEO_PARAM_PROFILELEVELTYPE rollback;
  GetProfileLevel(&rollback, port, module);
  auto pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;
  auto profile = static_cast<OMX_VIDEO_AVCPROFILETYPE>(pl.eProfile);
  auto level = static_cast<OMX_VIDEO_AVCLEVELTYPE>(pl.eLevel);

  if(!SetModuleProfileLevel(profile, level, module))
  {
    SetProfileLevel(&rollback, port, module);
    return false;
  }

  return true;
}

void DecExpertiseAVC::GetExpertise(OMX_PTR param, Port const& port, DecModule const& module)
{
  auto& avc = *(OMX_VIDEO_PARAM_AVCTYPE*)param;
  avc.nPortIndex = port.index;
  avc.nBFrames = 0; // XXX
  avc.nPFrames = 0; // XXX
  avc.bUseHadamard = OMX_TRUE; // XXX
  avc.nRefFrames = 1; // XXX
  avc.nRefIdx10ActiveMinus1 = 0; // XXX
  avc.nRefIdx11ActiveMinus1 = 0; // XXX
  avc.bEnableUEP = OMX_FALSE; // XXX
  avc.bEnableFMO = OMX_FALSE; // XXX
  avc.bEnableASO = OMX_FALSE; // XXX
  avc.bEnableRS = OMX_FALSE; // XXX
  avc.eProfile = ConvertModuleToOMXAVCProfile(module.GetProfileLevel());
  avc.eLevel = ConvertModuleToOMXAVCLevel(module.GetProfileLevel());
  avc.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP | OMX_VIDEO_PictureTypeB; // XXX
  avc.bFrameMBsOnly = OMX_TRUE; // XXX
  avc.bMBAFF = OMX_FALSE; // XXX
  avc.bEntropyCodingCABAC = OMX_FALSE; // XXX
  avc.bWeightedPPrediction = OMX_FALSE; // XXX
  avc.nWeightedBipredicitonMode = OMX_FALSE; // XXX
  avc.bconstIpred = OMX_FALSE; // XXX
  avc.bDirect8x8Inference = OMX_TRUE; // XXX
  avc.bDirectSpatialTemporal = OMX_TRUE; // XXX
  avc.nCabacInitIdc = 0; // XXX
  avc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterMax; // XXX
}

bool DecExpertiseAVC::SetExpertise(OMX_PTR param, Port const& port, DecModule& module)
{
  OMX_VIDEO_PARAM_AVCTYPE rollback;
  GetExpertise(&rollback, port, module);
  auto avc = *(OMX_VIDEO_PARAM_AVCTYPE*)param;

  if(!SetModuleProfileLevel(avc.eProfile, avc.eLevel, module))
  {
    SetExpertise(&rollback, port, module);
    return false;
  }

  return true;
}

