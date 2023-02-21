// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "convert_module_soft_avc.h"

AVCProfileType ConvertSoftToModuleAVCProfile(AL_EProfile const& profile)
{
  if(!AL_IS_AVC(profile))
    return AVCProfileType::AVC_PROFILE_MAX_ENUM;
  switch(profile)
  {
  case AL_PROFILE_AVC_BASELINE: return AVCProfileType::AVC_PROFILE_BASELINE;
  case AL_PROFILE_AVC_C_BASELINE: return AVCProfileType::AVC_PROFILE_CONSTRAINED_BASELINE;
  case AL_PROFILE_AVC_MAIN: return AVCProfileType::AVC_PROFILE_MAIN;
  case AL_PROFILE_AVC_EXTENDED: return AVCProfileType::AVC_PROFILE_EXTENDED;
  case AL_PROFILE_AVC_HIGH: return AVCProfileType::AVC_PROFILE_HIGH;
  case AL_PROFILE_AVC_HIGH10: return AVCProfileType::AVC_PROFILE_HIGH_10;
  case AL_PROFILE_AVC_HIGH_422: return AVCProfileType::AVC_PROFILE_HIGH_422;
  case AL_PROFILE_AVC_HIGH_444_PRED: return AVCProfileType::AVC_PROFILE_HIGH_444_PREDICTIVE;
  case AL_PROFILE_AVC_PROG_HIGH: return AVCProfileType::AVC_PROFILE_PROGRESSIVE_HIGH;
  case AL_PROFILE_AVC_C_HIGH: return AVCProfileType::AVC_PROFILE_CONSTRAINED_HIGH;
  case AL_PROFILE_AVC_HIGH10_INTRA: return AVCProfileType::AVC_PROFILE_HIGH_10_INTRA;
  case AL_PROFILE_AVC_HIGH_422_INTRA: return AVCProfileType::AVC_PROFILE_HIGH_422_INTRA;
  case AL_PROFILE_AVC_HIGH_444_INTRA: return AVCProfileType::AVC_PROFILE_HIGH_444_INTRA;
  case AL_PROFILE_AVC_CAVLC_444_INTRA: return AVCProfileType::AVC_PROFILE_CAVLC_444_INTRA;
  case AL_PROFILE_XAVC_HIGH10_INTRA_CBG: return AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_CBG;
  case AL_PROFILE_XAVC_HIGH10_INTRA_VBR: return AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_VBR;
  case AL_PROFILE_XAVC_HIGH_422_INTRA_CBG: return AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_CBG;
  case AL_PROFILE_XAVC_HIGH_422_INTRA_VBR: return AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_VBR;
  case AL_PROFILE_XAVC_LONG_GOP_MAIN_MP4: return AVCProfileType::XAVC_PROFILE_LONG_GOP_MAIN_MP4;
  case AL_PROFILE_XAVC_LONG_GOP_HIGH_MP4: return AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MP4;
  case AL_PROFILE_XAVC_LONG_GOP_HIGH_MXF: return AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MXF;
  case AL_PROFILE_XAVC_LONG_GOP_HIGH_422_MXF: return AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_422_MXF;
  default: return AVCProfileType::AVC_PROFILE_MAX_ENUM;
  }

  return AVCProfileType::AVC_PROFILE_MAX_ENUM;
}

AL_EProfile ConvertModuleToSoftAVCProfile(AVCProfileType const& profile)
{
  switch(profile)
  {
  case AVCProfileType::AVC_PROFILE_BASELINE: return AL_PROFILE_AVC_BASELINE;
  case AVCProfileType::AVC_PROFILE_CONSTRAINED_BASELINE: return AL_PROFILE_AVC_C_BASELINE;
  case AVCProfileType::AVC_PROFILE_MAIN: return AL_PROFILE_AVC_MAIN;
  case AVCProfileType::AVC_PROFILE_EXTENDED: return AL_PROFILE_AVC_EXTENDED;
  case AVCProfileType::AVC_PROFILE_HIGH: return AL_PROFILE_AVC_HIGH;
  case AVCProfileType::AVC_PROFILE_PROGRESSIVE_HIGH: return AL_PROFILE_AVC_PROG_HIGH;
  case AVCProfileType::AVC_PROFILE_CONSTRAINED_HIGH: return AL_PROFILE_AVC_C_HIGH;
  case AVCProfileType::AVC_PROFILE_HIGH_10: return AL_PROFILE_AVC_HIGH10;
  case AVCProfileType::AVC_PROFILE_HIGH_422: return AL_PROFILE_AVC_HIGH_422;
  case AVCProfileType::AVC_PROFILE_HIGH_444_PREDICTIVE: return AL_PROFILE_AVC_HIGH_444_PRED;
  case AVCProfileType::AVC_PROFILE_HIGH_10_INTRA: return AL_PROFILE_AVC_HIGH10_INTRA;
  case AVCProfileType::AVC_PROFILE_HIGH_422_INTRA: return AL_PROFILE_AVC_HIGH_422_INTRA;
  case AVCProfileType::AVC_PROFILE_HIGH_444_INTRA: return AL_PROFILE_AVC_HIGH_444_INTRA;
  case AVCProfileType::AVC_PROFILE_CAVLC_444_INTRA: return AL_PROFILE_AVC_CAVLC_444_INTRA;
  case AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_CBG: return AL_PROFILE_XAVC_HIGH10_INTRA_CBG;
  case AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_VBR: return AL_PROFILE_XAVC_HIGH10_INTRA_VBR;
  case AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_CBG: return AL_PROFILE_XAVC_HIGH_422_INTRA_CBG;
  case AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_VBR: return AL_PROFILE_XAVC_HIGH_422_INTRA_VBR;
  case AVCProfileType::XAVC_PROFILE_LONG_GOP_MAIN_MP4: return AL_PROFILE_XAVC_LONG_GOP_MAIN_MP4;
  case AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MP4: return AL_PROFILE_XAVC_LONG_GOP_HIGH_MP4;
  case AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MXF: return AL_PROFILE_XAVC_LONG_GOP_HIGH_MXF;
  case AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_422_MXF: return AL_PROFILE_XAVC_LONG_GOP_HIGH_422_MXF;
  case AVCProfileType::AVC_PROFILE_MAX_ENUM: return AL_PROFILE_AVC;
  default: return AL_PROFILE_AVC;
  }

  return AL_PROFILE_AVC;
}
