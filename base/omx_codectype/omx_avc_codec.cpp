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

#include <OMX_IndexExt.h>
#include <OMX_VideoExt.h>
#include "omx_avc_codec.h"

#include "base/omx_utils/omx_setup.h"
#include "base/omx_checker/omx_checker.h"

#include <assert.h>

extern "C"
{
#include "lib_common_dec/DecDpbMode.h"
}

static VideoProfileLevelType SupportedAVCProfileLevels[] =
{
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel32
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel4
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel42
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel5
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel51
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_ALG_VIDEO_AVCLevel52
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_ALG_VIDEO_AVCLevel60
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_ALG_VIDEO_AVCLevel61
  },
  {
    OMX_VIDEO_AVCProfileBaseline, OMX_ALG_VIDEO_AVCLevel62
  },

  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel11
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel12
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel13
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel2
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel21
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel22
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel3
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel31
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel32
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel4
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel41
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel42
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel5
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel51
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_ALG_VIDEO_AVCLevel52
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_ALG_VIDEO_AVCLevel60
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_ALG_VIDEO_AVCLevel61
  },
  {
    OMX_VIDEO_AVCProfileMain, OMX_ALG_VIDEO_AVCLevel62
  },

  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel4
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel41
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel42
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel5
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel51
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_ALG_VIDEO_AVCLevel52
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_ALG_VIDEO_AVCLevel60
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_ALG_VIDEO_AVCLevel61
  },
  {
    OMX_VIDEO_AVCProfileHigh, OMX_ALG_VIDEO_AVCLevel62
  },

  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel1
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel11
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel12
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel13
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel2
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel21
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel22
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel3
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel31
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel32
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel4
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel41
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel42
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel5
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_VIDEO_AVCLevel51
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_ALG_VIDEO_AVCLevel52
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_ALG_VIDEO_AVCLevel60
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_ALG_VIDEO_AVCLevel61
  },
  {
    OMX_VIDEO_AVCProfileHigh10, OMX_ALG_VIDEO_AVCLevel62
  },

  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel1
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel11
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel12
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel13
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel2
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel21
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel22
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel3
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel31
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel32
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel4
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel41
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel42
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel5
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_VIDEO_AVCLevel51
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_ALG_VIDEO_AVCLevel52
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_ALG_VIDEO_AVCLevel60
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_ALG_VIDEO_AVCLevel61
  },
  {
    OMX_VIDEO_AVCProfileHigh422, OMX_ALG_VIDEO_AVCLevel62
  },
};

AVCCodec::AVCCodec()
{
  OMXChecker::SetHeaderVersion(m_ParameterVideoCodec);
  m_ParameterVideoCodec.nSliceHeaderSpacing = 0x0; /* Unused value */
  m_ParameterVideoCodec.nPFrames = (GOPLENGTH - 1) / (BFRAMES + 1);
  m_ParameterVideoCodec.nBFrames = BFRAMES;
  m_ParameterVideoCodec.bUseHadamard = OMX_TRUE; /* Specification value */
  m_ParameterVideoCodec.nRefFrames = 1; /* Specification value */
  m_ParameterVideoCodec.nRefIdx10ActiveMinus1 = 0x0; /* Unused value (automatic) */
  m_ParameterVideoCodec.nRefIdx11ActiveMinus1 = 0x0; /* Unused value (automatic) */
  m_ParameterVideoCodec.bEnableUEP = OMX_FALSE;
  m_ParameterVideoCodec.bEnableFMO = OMX_FALSE; /* Specification value */
  m_ParameterVideoCodec.bEnableASO = OMX_FALSE; /* Specification value */
  m_ParameterVideoCodec.bEnableRS = OMX_FALSE;
  m_ParameterVideoCodec.eProfile = OMX_VIDEO_AVCProfileBaseline;
  m_ParameterVideoCodec.eLevel = OMX_VIDEO_AVCLevel1;
  m_ParameterVideoCodec.nAllowedPictureTypes = 0x0; /* Read-only */
  m_ParameterVideoCodec.bFrameMBsOnly = OMX_TRUE;
  m_ParameterVideoCodec.bMBAFF = OMX_FALSE;
  m_ParameterVideoCodec.bEntropyCodingCABAC = OMX_TRUE;
  m_ParameterVideoCodec.bWeightedPPrediction = OMX_FALSE; /* Specification value */
  m_ParameterVideoCodec.nWeightedBipredicitonMode = OMX_FALSE; /* Read-only */
  m_ParameterVideoCodec.bconstIpred = OMX_FALSE; /* Specification value */
  m_ParameterVideoCodec.bDirect8x8Inference = OMX_TRUE; /* Read-only */
  m_ParameterVideoCodec.bDirectSpatialTemporal = OMX_TRUE; /* Read-only */
  m_ParameterVideoCodec.nCabacInitIdc = CABACINITIDC;
  m_ParameterVideoCodec.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;
  m_bLowBW = false;
};

AVCCodec::~AVCCodec()
{
};

OMX_STRING AVCCodec::GetType()
{
  return (OMX_STRING)"h264";
};

OMX_STRING AVCCodec::GetRole()
{
  return (OMX_STRING)"avc";
};

OMX_STRING AVCCodec::GetMIME()
{
  return (OMX_STRING)"video/avc";
};

OMX_U32 AVCCodec::GetCurProfile()
{
  return m_ParameterVideoCodec.eProfile;
};

OMX_U32 AVCCodec::GetCurLevel()
{
  return m_ParameterVideoCodec.eLevel;
};

OMX_VIDEO_CODINGTYPE AVCCodec::GetCompressionFormat()
{
  return OMX_VIDEO_CodingAVC;
};

OMX_U32 AVCCodec::GeteProfile(OMX_IN OMX_U32 nProfileIndex)
{
  assert(nProfileIndex < ARRAYSIZE(SupportedAVCProfileLevels));
  return SupportedAVCProfileLevels[nProfileIndex].eProfile;
};

OMX_U32 AVCCodec::GeteLevel(OMX_IN OMX_U32 nProfileIndex)
{
  assert(nProfileIndex < ARRAYSIZE(SupportedAVCProfileLevels));
  return SupportedAVCProfileLevels[nProfileIndex].eLevel;
};

OMX_U32 AVCCodec::GetSupportedProfileLevelSize()
{
  return ARRAYSIZE(SupportedAVCProfileLevels);
};

OMX_BOOL AVCCodec::CheckIndexParamVideoCodec(OMX_INDEXTYPE nParamIndex)
{
  return ((nParamIndex) == OMX_IndexParamVideoAvc) ? OMX_TRUE : OMX_FALSE;
};

OMX_ERRORTYPE AVCCodec::GetIndexParamVideoCodec(OMX_PTR pParam, OMX_PARAM_PORTDEFINITIONTYPE PortDef)
{
  try
  {
    auto port = (OMX_VIDEO_PARAM_AVCTYPE*)pParam;

    OMXChecker::CheckHeaderVersion(port->nVersion);

    if(port->nPortIndex == PortDef.nPortIndex)
    {
      *port = m_ParameterVideoCodec;
      port->nPortIndex = PortDef.nPortIndex;
    }
    else
      return OMX_ErrorBadPortIndex;
    return OMX_ErrorNone;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
};

OMX_ERRORTYPE AVCCodec::SetIndexParamVideoCodec(OMX_INOUT OMX_PTR pParam, OMX_IN OMX_PARAM_PORTDEFINITIONTYPE& CodecPortDef, OMX_OUT OMX_PARAM_PORTDEFINITIONTYPE& PortDef)
{
  try
  {
    auto port = (OMX_VIDEO_PARAM_AVCTYPE*)pParam;

    OMXChecker::CheckHeaderVersion(port->nVersion);

    if(port->nPortIndex == CodecPortDef.nPortIndex)
    {
      m_ParameterVideoCodec = *port;
      auto iBFrames = (m_ParameterVideoCodec.nBFrames / (m_ParameterVideoCodec.nPFrames + 1));
      PortDef.nBufferCountMin = iBFrames + 2;
      PortDef.nBufferCountActual = PortDef.nBufferCountMin;

      CodecPortDef.nBufferCountMin = iBFrames + 4;
      CodecPortDef.nBufferCountActual = CodecPortDef.nBufferCountMin;
    }
    else
      return OMX_ErrorBadPortIndex;
    return OMX_ErrorNone;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
};

OMX_U32 AVCCodec::ConvertLevel(int level)
{
  switch(level)
  {
  case 10: return OMX_VIDEO_AVCLevel1;
  case 9: return OMX_VIDEO_AVCLevel1b;
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
  case 52: return OMX_ALG_VIDEO_AVCLevel52;
  case 60: return OMX_ALG_VIDEO_AVCLevel60;
  case 61: return OMX_ALG_VIDEO_AVCLevel61;
  case 62: return OMX_ALG_VIDEO_AVCLevel62;
  default:
    assert(0);
    return 0;
  }
};

int AVCCodec::ConvertLevel(OMX_U32 level)
{
  switch(level)
  {
  case OMX_VIDEO_AVCLevel1:  return 10;
  case OMX_VIDEO_AVCLevel1b: return 9;
  case OMX_VIDEO_AVCLevel11: return 11;
  case OMX_VIDEO_AVCLevel12: return 12;
  case OMX_VIDEO_AVCLevel13: return 13;
  case OMX_VIDEO_AVCLevel2:  return 20;
  case OMX_VIDEO_AVCLevel21: return 21;
  case OMX_VIDEO_AVCLevel22: return 22;
  case OMX_VIDEO_AVCLevel3:  return 30;
  case OMX_VIDEO_AVCLevel31: return 31;
  case OMX_VIDEO_AVCLevel32: return 32;
  case OMX_VIDEO_AVCLevel4:  return 40;
  case OMX_VIDEO_AVCLevel41: return 41;
  case OMX_VIDEO_AVCLevel42: return 42;
  case OMX_VIDEO_AVCLevel5:  return 50;
  case OMX_VIDEO_AVCLevel51: return 51;
  case OMX_ALG_VIDEO_AVCLevel52: return 52;
  case OMX_ALG_VIDEO_AVCLevel60: return 60;
  case OMX_ALG_VIDEO_AVCLevel61: return 61;
  case OMX_ALG_VIDEO_AVCLevel62: return 62;
  default:
    assert(0);
    return 0;
  }
};

AL_EProfile AVCCodec::ConvertProfile(OMX_U32 profile)
{
  switch(profile)
  {
  case OMX_VIDEO_AVCProfileBaseline: return AL_PROFILE_AVC_BASELINE;
  case OMX_VIDEO_AVCProfileMain:     return AL_PROFILE_AVC_MAIN;
  case OMX_VIDEO_AVCProfileExtended: return AL_PROFILE_AVC_EXTENDED; // not supported
  case OMX_VIDEO_AVCProfileHigh:     return AL_PROFILE_AVC_HIGH;
  case OMX_VIDEO_AVCProfileHigh10:   return AL_PROFILE_AVC_HIGH10;
  case OMX_VIDEO_AVCProfileHigh422:  return AL_PROFILE_AVC_HIGH_422;
  default:
    assert(0);
    return AL_PROFILE_AVC_BASELINE;
  }
};

OMX_U32 AVCCodec::ConvertProfile(AL_EProfile profile)
{
  switch(profile)
  {
  case AL_PROFILE_AVC_BASELINE: return OMX_VIDEO_AVCProfileBaseline;
  case AL_PROFILE_AVC_MAIN:     return OMX_VIDEO_AVCProfileMain;
  case AL_PROFILE_AVC_EXTENDED: return OMX_VIDEO_AVCProfileExtended; // not supported
  case AL_PROFILE_AVC_HIGH:     return OMX_VIDEO_AVCProfileHigh;
  case AL_PROFILE_AVC_HIGH10:   return OMX_VIDEO_AVCProfileHigh10;
  case AL_PROFILE_AVC_HIGH_422: return OMX_VIDEO_AVCProfileHigh422;
  default:
    assert(0);
    return 0;
  }
};

OMX_U32 AVCCodec::GetCodecPFrames()
{
  return m_ParameterVideoCodec.nPFrames;
};

OMX_U32 AVCCodec::GetCodecBFrames()
{
  return m_ParameterVideoCodec.nBFrames;
};

OMX_U32 AVCCodec::GetCodecTier()
{
  return 0;
};

AL_EChEncOption AVCCodec::GetCodecOptions()
{
  uint32_t opt;
  switch(m_ParameterVideoCodec.eLoopFilterMode)
  {
  case OMX_VIDEO_AVCLoopFilterEnable: opt = AL_OPT_LF | AL_OPT_LF_X_TILE;
    break;
  case OMX_VIDEO_AVCLoopFilterDisableSliceBoundary: opt = AL_OPT_LF;
    break;
  default: // disable
    opt = 0;
  }

  if(m_ParameterVideoCodec.bconstIpred)
    opt |= AL_OPT_CONST_INTRA_PRED;

  return (AL_EChEncOption)opt;
};

void AVCCodec::SetProfileLevel(VideoProfileLevelType proflevel)
{
  m_ParameterVideoCodec.eProfile = (OMX_VIDEO_AVCPROFILETYPE)proflevel.eProfile;
  m_ParameterVideoCodec.eLevel = (OMX_VIDEO_AVCLEVELTYPE)proflevel.eLevel;
}

int AVCCodec::DPBSize(int width, int height)
{
  return AL_AVC_GetMaxDPBSize(ConvertLevel((OMX_U32)m_ParameterVideoCodec.eLevel), width, height);
}

bool AVCCodec::IsCAVLC()
{
  return !m_ParameterVideoCodec.bEntropyCodingCABAC;
}

void AVCCodec::EnableLowBandwidth(bool shouldBeEnabled)
{
  m_bLowBW = shouldBeEnabled;
}

int AVCCodec::GetBandwidth()
{
  return m_bLowBW ? 8 : 16;
}

