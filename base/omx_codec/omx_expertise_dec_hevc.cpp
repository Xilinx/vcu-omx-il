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

#include "omx_expertise_dec_hevc.h"
#include "omx_convert_omx_module.h"

static bool SetModuleProfileLevel(OMX_ALG_VIDEO_HEVCPROFILETYPE const& profile, OMX_ALG_VIDEO_HEVCLEVELTYPE const& level, DecModule& module)
{
  ProfileLevelType p;
  p.profile.hevc = ConvertToModuleHEVCProfileLevel(profile, level).profile.hevc;
  p.level = ConvertToModuleHEVCProfileLevel(profile, level).level;
  return module.SetProfileLevel(p);
}

bool DecExpertiseHEVC::GetProfileLevelSupported(OMX_PTR param, DecModule const& module)
{
  auto supported = module.GetProfileLevelSupported();
  auto& pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;

  if(pl.nProfileIndex >= supported.size())
    return false;

  pl.eProfile = ConvertToOMXHEVCProfile(supported[pl.nProfileIndex]);
  pl.eLevel = ConvertToOMXHEVCLevel(supported[pl.nProfileIndex]);

  return true;
}

void DecExpertiseHEVC::GetProfileLevel(OMX_PTR param, Port const& port, DecModule const& module)
{
  auto& pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;
  pl.nPortIndex = port.index;
  pl.eProfile = ConvertToOMXHEVCProfile(module.GetProfileLevel());
  pl.eLevel = ConvertToOMXHEVCLevel(module.GetProfileLevel());
}

bool DecExpertiseHEVC::SetProfileLevel(OMX_PTR param, Port const& port, DecModule& module)
{
  OMX_VIDEO_PARAM_PROFILELEVELTYPE rollback;
  GetProfileLevel(&rollback, port, module);
  auto const pl = *(OMX_VIDEO_PARAM_PROFILELEVELTYPE*)param;
  auto const profile = static_cast<OMX_ALG_VIDEO_HEVCPROFILETYPE>(pl.eProfile);
  auto const level = static_cast<OMX_ALG_VIDEO_HEVCLEVELTYPE>(pl.eLevel);

  if(!SetModuleProfileLevel(profile, level, module))
  {
    SetProfileLevel(&rollback, port, module);
    return false;
  }

  return true;
}

void DecExpertiseHEVC::GetExpertise(OMX_PTR param, Port const& port, DecModule const& module)
{
  auto& hevc = *(OMX_ALG_VIDEO_PARAM_HEVCTYPE*)param;
  hevc.nPortIndex = port.index;
  hevc.nBFrames = 0; // XXX
  hevc.nPFrames = 0; // XXX
  hevc.eProfile = ConvertToOMXHEVCProfile(module.GetProfileLevel());
  hevc.eLevel = ConvertToOMXHEVCLevel(module.GetProfileLevel());
  hevc.bConstIpred = OMX_FALSE; // XXX
  hevc.eLoopFilterMode = OMX_ALG_VIDEO_HEVCLoopFilterMax; // XXX
}

bool DecExpertiseHEVC::SetExpertise(OMX_PTR param, Port const& port, DecModule& module)
{
  OMX_ALG_VIDEO_PARAM_HEVCTYPE rollback;
  GetExpertise(&rollback, port, module);
  auto const hevc = *(OMX_ALG_VIDEO_PARAM_HEVCTYPE*)param;

  if(!SetModuleProfileLevel(hevc.eProfile, hevc.eLevel, module))
  {
    SetExpertise(&rollback, port, module);
    return false;
  }

  return true;
}

