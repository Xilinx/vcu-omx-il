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
#include <assert.h>

#include "base/omx_utils/omx_setup.h"
#include "base/omx_checker/omx_checker.h"
#include "omx_hevc_codec.h"

extern "C"
{
#include "lib_common_dec/DecDpbMode.h"
}

static VideoProfileLevelType SupportedHEVCProfileLevels[] =
{
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel1
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel2
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel21
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel3
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel31
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCMainTierLevel62
  },

  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel1
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel2
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel21
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel3
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel31
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCMainTierLevel62
  },

  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel1
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel2
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel21
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel3
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel31
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCMainTierLevel62
  },

  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel1
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel2
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel21
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel3
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel31
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCMainTierLevel62
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCHighTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCHighTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCHighTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCHighTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCHighTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCHighTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCHighTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain, OMX_ALG_VIDEO_HEVCHighTierLevel62
  },

  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCHighTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCHighTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCHighTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCHighTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCHighTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCHighTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCHighTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain10, OMX_ALG_VIDEO_HEVCHighTierLevel62
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCHighTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCHighTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCHighTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCHighTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCHighTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCHighTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCHighTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422, OMX_ALG_VIDEO_HEVCHighTierLevel62
  },

  {
    OMX_ALG_VIDEO_HEVCProfileMain422_10, OMX_ALG_VIDEO_HEVCHighTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422_10, OMX_ALG_VIDEO_HEVCHighTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422_10, OMX_ALG_VIDEO_HEVCHighTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422_10, OMX_ALG_VIDEO_HEVCHighTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422_10, OMX_ALG_VIDEO_HEVCHighTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422_10, OMX_ALG_VIDEO_HEVCHighTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422_10, OMX_ALG_VIDEO_HEVCHighTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMain422_10, OMX_ALG_VIDEO_HEVCHighTierLevel62
  },

  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCHighTierLevel4
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCHighTierLevel41
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCHighTierLevel5
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCHighTierLevel51
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCHighTierLevel52
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCHighTierLevel6
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCHighTierLevel61
  },
  {
    OMX_ALG_VIDEO_HEVCProfileMainStill, OMX_ALG_VIDEO_HEVCHighTierLevel62
  },
};

static OMX_BOOL IsMainTier(OMX_ALG_VIDEO_HEVCLEVELTYPE eLevel)
{
  if(eLevel % 2 == 0)
    return OMX_FALSE;
  return OMX_TRUE;
}

HEVCCodec::HEVCCodec()
{
  OMXChecker::SetHeaderVersion(m_ParameterVideoCodec);
  m_ParameterVideoCodec.eProfile = OMX_ALG_VIDEO_HEVCProfileMain;
  m_ParameterVideoCodec.eLevel = OMX_ALG_VIDEO_HEVCMainTierLevel1;
  m_ParameterVideoCodec.nPFrames = (GOPLENGTH - 1) / (BFRAMES + 1);
  m_ParameterVideoCodec.nBFrames = BFRAMES;
  m_ParameterVideoCodec.bConstIpred = OMX_FALSE;
  m_ParameterVideoCodec.eLoopFilterMode = OMX_ALG_VIDEO_HEVCLoopFilterEnable;

  m_bIsMainTier = OMX_TRUE;
  m_bLowBW = false;
};

HEVCCodec::~HEVCCodec()
{
};

OMX_STRING HEVCCodec::GetType()
{
  return (OMX_STRING)"h265";
};

OMX_STRING HEVCCodec::GetRole()
{
  return (OMX_STRING)"hevc";
};

OMX_STRING HEVCCodec::GetMIME()
{
  return (OMX_STRING)"video/hevc";
};

OMX_U32 HEVCCodec::GetCurProfile()
{
  return m_ParameterVideoCodec.eProfile;
};

OMX_U32 HEVCCodec::GetCurLevel()
{
  return m_ParameterVideoCodec.eLevel;
};

OMX_VIDEO_CODINGTYPE HEVCCodec::GetCompressionFormat()
{
  return (OMX_VIDEO_CODINGTYPE)OMX_ALG_VIDEO_CodingHEVC;
};

OMX_U32 HEVCCodec::GeteProfile(OMX_IN OMX_U32 nProfileIndex)
{
  assert(nProfileIndex < ARRAYSIZE(SupportedHEVCProfileLevels));
  return SupportedHEVCProfileLevels[nProfileIndex].eProfile;
};

OMX_U32 HEVCCodec::GeteLevel(OMX_IN OMX_U32 nProfileIndex)
{
  assert(nProfileIndex < ARRAYSIZE(SupportedHEVCProfileLevels));
  return SupportedHEVCProfileLevels[nProfileIndex].eLevel;
};

OMX_U32 HEVCCodec::GetSupportedProfileLevelSize()
{
  return ARRAYSIZE(SupportedHEVCProfileLevels);
};

OMX_BOOL HEVCCodec::CheckIndexParamVideoCodec(OMX_INDEXTYPE nParamIndex)
{
  return (((OMX_U32)nParamIndex) == OMX_ALG_IndexParamVideoHevc) ? OMX_TRUE : OMX_FALSE;
};

OMX_ERRORTYPE HEVCCodec::GetIndexParamVideoCodec(OMX_PTR pParam, OMX_PARAM_PORTDEFINITIONTYPE PortDef)
{
  try
  {
    auto port = (OMX_ALG_VIDEO_PARAM_HEVCTYPE*)pParam;

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

OMX_ERRORTYPE HEVCCodec::SetIndexParamVideoCodec(OMX_INOUT OMX_PTR pParam, OMX_IN OMX_PARAM_PORTDEFINITIONTYPE& CodecPortDef, OMX_OUT OMX_PARAM_PORTDEFINITIONTYPE& PortDef)
{
  try
  {
    auto port = (OMX_ALG_VIDEO_PARAM_HEVCTYPE*)pParam;

    OMXChecker::CheckHeaderVersion(port->nVersion);

    if(port->nPortIndex == CodecPortDef.nPortIndex)
    {
      m_ParameterVideoCodec = *port;
      m_bIsMainTier = IsMainTier(m_ParameterVideoCodec.eLevel) ? OMX_TRUE : OMX_FALSE;
      auto const iBFrames = (m_ParameterVideoCodec.nBFrames / (m_ParameterVideoCodec.nPFrames + 1));
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

OMX_U32 HEVCCodec::ConvertLevel(int level)
{
  OMX_U32 uLevel;
  switch(level)
  {
  case 10: uLevel = OMX_ALG_VIDEO_HEVCMainTierLevel1;
    break;
  case 20: uLevel = OMX_ALG_VIDEO_HEVCMainTierLevel2;
    break;
  case 21: uLevel = OMX_ALG_VIDEO_HEVCMainTierLevel21;
    break;
  case 30: uLevel = OMX_ALG_VIDEO_HEVCMainTierLevel3;
    break;
  case 31: uLevel = OMX_ALG_VIDEO_HEVCMainTierLevel31;
    break;
  case 40: uLevel = m_bIsMainTier ? OMX_ALG_VIDEO_HEVCMainTierLevel4 : OMX_ALG_VIDEO_HEVCHighTierLevel4;
    break;
  case 41: uLevel = m_bIsMainTier ? OMX_ALG_VIDEO_HEVCMainTierLevel41 : OMX_ALG_VIDEO_HEVCHighTierLevel41;
    break;
  case 50: uLevel = m_bIsMainTier ? OMX_ALG_VIDEO_HEVCMainTierLevel5 : OMX_ALG_VIDEO_HEVCHighTierLevel5;
    break;
  case 51: uLevel = m_bIsMainTier ? OMX_ALG_VIDEO_HEVCMainTierLevel51 : OMX_ALG_VIDEO_HEVCHighTierLevel51;
    break;
  case 52: uLevel = m_bIsMainTier ? OMX_ALG_VIDEO_HEVCMainTierLevel52 : OMX_ALG_VIDEO_HEVCHighTierLevel52;
    break;
  case 60: uLevel = m_bIsMainTier ? OMX_ALG_VIDEO_HEVCMainTierLevel6 : OMX_ALG_VIDEO_HEVCHighTierLevel6;
    break;
  case 61: uLevel = m_bIsMainTier ? OMX_ALG_VIDEO_HEVCMainTierLevel61 : OMX_ALG_VIDEO_HEVCHighTierLevel61;
    break;
  case 62: uLevel = m_bIsMainTier ? OMX_ALG_VIDEO_HEVCMainTierLevel62 : OMX_ALG_VIDEO_HEVCHighTierLevel62;
    break;
  default:
    assert(0);
  }

  return uLevel;
};

int HEVCCodec::ConvertLevel(OMX_U32 level)
{
  switch(level)
  {
  case OMX_ALG_VIDEO_HEVCMainTierLevel1:
    return 10;
  case OMX_ALG_VIDEO_HEVCMainTierLevel2:
    return 20;
  case OMX_ALG_VIDEO_HEVCMainTierLevel21:
    return 21;
  case OMX_ALG_VIDEO_HEVCMainTierLevel3:
    return 30;
  case OMX_ALG_VIDEO_HEVCMainTierLevel31:
    return 31;
  case OMX_ALG_VIDEO_HEVCMainTierLevel4:
    return 40;
  case OMX_ALG_VIDEO_HEVCMainTierLevel41:
  case OMX_ALG_VIDEO_HEVCHighTierLevel41:
    return 41;
  case OMX_ALG_VIDEO_HEVCMainTierLevel5:
  case OMX_ALG_VIDEO_HEVCHighTierLevel5:
    return 50;
  case OMX_ALG_VIDEO_HEVCMainTierLevel51:
  case OMX_ALG_VIDEO_HEVCHighTierLevel51:
    return 51;
  case OMX_ALG_VIDEO_HEVCMainTierLevel52:
  case OMX_ALG_VIDEO_HEVCHighTierLevel52:
    return 52;
  case OMX_ALG_VIDEO_HEVCMainTierLevel6:
  case OMX_ALG_VIDEO_HEVCHighTierLevel6:
    return 60;
  case OMX_ALG_VIDEO_HEVCMainTierLevel61:
  case OMX_ALG_VIDEO_HEVCHighTierLevel61:
    return 61;
  case OMX_ALG_VIDEO_HEVCMainTierLevel62:
  case OMX_ALG_VIDEO_HEVCHighTierLevel62:
    return 62;
  default:
    assert(0);
    return 0;
  }
};

AL_EProfile HEVCCodec::ConvertProfile(OMX_U32 profile)
{
  switch(profile)
  {
  case OMX_ALG_VIDEO_HEVCProfileMain:      return AL_PROFILE_HEVC_MAIN;
  case OMX_ALG_VIDEO_HEVCProfileMain10:    return AL_PROFILE_HEVC_MAIN10;
  case OMX_ALG_VIDEO_HEVCProfileMainStill: return AL_PROFILE_HEVC_MAIN_STILL;
  case OMX_ALG_VIDEO_HEVCProfileMain422:   return AL_PROFILE_HEVC_MAIN_422;
  case OMX_ALG_VIDEO_HEVCProfileMain422_10: return AL_PROFILE_HEVC_MAIN_422_10;
  default:
    assert(0);
    return AL_PROFILE_HEVC_MAIN;
  }
};

OMX_U32 HEVCCodec::ConvertProfile(AL_EProfile profile)
{
  switch(profile)
  {
  case AL_PROFILE_HEVC_MAIN:       return OMX_ALG_VIDEO_HEVCProfileMain;
  case AL_PROFILE_HEVC_MAIN10:     return OMX_ALG_VIDEO_HEVCProfileMain10;
  case AL_PROFILE_HEVC_MAIN_STILL: return OMX_ALG_VIDEO_HEVCProfileMainStill;
  case AL_PROFILE_HEVC_MAIN_422:   return OMX_ALG_VIDEO_HEVCProfileMain422;
  case AL_PROFILE_HEVC_MAIN_422_10: return OMX_ALG_VIDEO_HEVCProfileMain422_10;
  default:
    assert(0);
    return 0;
  }
};

OMX_U32 HEVCCodec::GetCodecPFrames()
{
  return m_ParameterVideoCodec.nPFrames;
};

OMX_U32 HEVCCodec::GetCodecBFrames()
{
  return m_ParameterVideoCodec.nBFrames;
};

OMX_U32 HEVCCodec::GetCodecTier()
{
  return m_bIsMainTier ? 0 : 1;
};

AL_EChEncOption HEVCCodec::GetCodecOptions()
{
  uint32_t opt;

  switch(m_ParameterVideoCodec.eLoopFilterMode)
  {
  case OMX_ALG_VIDEO_HEVCLoopFilterEnable: opt = AL_OPT_LF_X_SLICE | AL_OPT_LF_X_TILE;
    break;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSlice: opt = AL_OPT_LF | AL_OPT_LF_X_TILE;
    break;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossTile: opt = AL_OPT_LF | AL_OPT_LF_X_SLICE;
    break;
  case OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSliceAndTile: opt = AL_OPT_LF;
    break;
  default: // disable
    opt = 0;
  }

  if(m_ParameterVideoCodec.bConstIpred)
    opt |= AL_OPT_CONST_INTRA_PRED;

  return (AL_EChEncOption)opt;
};

void HEVCCodec::SetProfileLevel(VideoProfileLevelType proflevel)
{
  m_ParameterVideoCodec.eProfile = (OMX_ALG_VIDEO_HEVCPROFILETYPE)proflevel.eProfile;
  m_ParameterVideoCodec.eLevel = (OMX_ALG_VIDEO_HEVCLEVELTYPE)proflevel.eLevel;
}

int HEVCCodec::DPBSize(int width, int height)
{
  return AL_HEVC_GetMaxDPBSize(ConvertLevel((OMX_U32)m_ParameterVideoCodec.eLevel), width, height);
}

bool HEVCCodec::IsCAVLC()
{
  return true;
}

void HEVCCodec::EnableLowBandwidth(bool shouldBeEnabled)
{
  m_bLowBW = shouldBeEnabled;
}

int HEVCCodec::GetBandwidth()
{
  return m_bLowBW ? 16 : 32;
}

