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

#include "omx_convert_module_soft_avc.h"

AVCProfileType ConvertSoftToModuleAVCProfile(AL_EProfile const& profile)
{
  if(!AL_IS_AVC(profile))
    return AVC_PROFILE_MAX;
  switch(profile)
  {
    case AL_PROFILE_AVC_BASELINE: return AVC_PROFILE_BASELINE;
    case AL_PROFILE_AVC_C_BASELINE: return AVC_PROFILE_CONSTRAINED_BASELINE;
    case AL_PROFILE_AVC_MAIN: return AVC_PROFILE_MAIN;
    case AL_PROFILE_AVC_EXTENDED: return AVC_PROFILE_EXTENDED;
    case AL_PROFILE_AVC_HIGH: return AVC_PROFILE_HIGH;
    case AL_PROFILE_AVC_HIGH10: return AVC_PROFILE_HIGH_10;
    case AL_PROFILE_AVC_HIGH_422: return AVC_PROFILE_HIGH_422;
    case AL_PROFILE_AVC_HIGH_444_PRED: return AVC_PROFILE_HIGH_444_PREDICTIVE;
    case AL_PROFILE_AVC_PROG_HIGH: return AVC_PROFILE_PROGRESSIVE_HIGH;
    case AL_PROFILE_AVC_C_HIGH: return AVC_PROFILE_CONSTRAINED_HIGH;
    case AL_PROFILE_AVC_HIGH10_INTRA: return AVC_PROFILE_HIGH_10_INTRA;
    case AL_PROFILE_AVC_HIGH_422_INTRA: return AVC_PROFILE_HIGH_422_INTRA;
    case AL_PROFILE_AVC_HIGH_444_INTRA: return AVC_PROFILE_HIGH_444_INTRA;
    case AL_PROFILE_AVC_CAVLC_444: return AVC_PROFILE_CAVLC_444_INTRA;
    default: return AVC_PROFILE_MAX;
  }

  return AVC_PROFILE_MAX;
}

AL_EProfile ConvertModuleToSoftAVCProfile(AVCProfileType const& profile)
{
  switch(profile)
  {
    case AVC_PROFILE_BASELINE: return AL_PROFILE_AVC_BASELINE;
    case AVC_PROFILE_CONSTRAINED_BASELINE: return AL_PROFILE_AVC_C_BASELINE;
    case AVC_PROFILE_MAIN: return AL_PROFILE_AVC_MAIN;
    case AVC_PROFILE_EXTENDED: return AL_PROFILE_AVC_EXTENDED;
    case AVC_PROFILE_HIGH: return AL_PROFILE_AVC_HIGH;
    case AVC_PROFILE_PROGRESSIVE_HIGH: return AL_PROFILE_AVC_PROG_HIGH;
    case AVC_PROFILE_CONSTRAINED_HIGH: return AL_PROFILE_AVC_C_HIGH;
    case AVC_PROFILE_HIGH_10: return AL_PROFILE_AVC_HIGH10;
    case AVC_PROFILE_HIGH_422: return AL_PROFILE_AVC_HIGH_422;
    case AVC_PROFILE_HIGH_444_PREDICTIVE: return AL_PROFILE_AVC_HIGH_444_PRED;
    case AVC_PROFILE_HIGH_10_INTRA: return AL_PROFILE_AVC_HIGH10_INTRA;
    case AVC_PROFILE_HIGH_422_INTRA: return AL_PROFILE_AVC_HIGH_422_INTRA;
    case AVC_PROFILE_HIGH_444_INTRA: return AL_PROFILE_AVC_HIGH_444_INTRA;
    case AVC_PROFILE_CAVLC_444_INTRA: return AL_PROFILE_AVC_CAVLC_444;
    case AVC_PROFILE_MAX: return AL_PROFILE_AVC;
    default: return AL_PROFILE_AVC;
  }

  return AL_PROFILE_AVC;
}

