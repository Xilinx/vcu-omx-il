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

#include "omx_convert_module_to_omx_enc.h"
#include <assert.h>

OMX_BOOL ConvertToOMXEntropyCoding(EntropyCodingType const& mode)
{
  switch(mode)
  {
  case ENTROPY_CODING_CABAC: return OMX_TRUE;
  case ENTROPY_CODING_CAVLC: return OMX_FALSE;
  case ENTROPY_CODING_MAX: return OMX_FALSE;
  default: return OMX_FALSE;
  }

  return OMX_FALSE;
}

OMX_U32 ConvertToOMXBFrames(Gop const& gop)
{
  assert(gop.b >= 0);
  return (gop.b * gop.length) / (1 + gop.b);
}

OMX_U32 ConvertToOMXPFrames(Gop const& gop)
{
  assert(gop.b >= 0);
  return (gop.b - gop.length + 1) / (-gop.b - 1);
}

OMX_U32 ConvertToOMXQpI(QPs const& qps)
{
  return qps.initial;
}

OMX_U32 ConvertToOMXQpP(QPs const& qps)
{
  return qps.deltaIP + qps.initial;
}

OMX_U32 ConvertToOMXQpB(QPs const& qps)
{
  return qps.deltaIP + qps.deltaPB + qps.initial;
}

OMX_S32 ConvertToOMXQpMin(QPs const& qps)
{
  return qps.min;
}

OMX_S32 ConvertToOMXQpMax(QPs const& qps)
{
  return qps.max;
}

OMX_ALG_EQpCtrlMode ConvertToOMXQpControl(QPs const& qps)
{
  switch(qps.mode)
  {
  case QP_UNIFORM: return OMX_ALG_UNIFORM_QP;
  case QP_AUTO: return OMX_ALG_AUTO_QP;
  case QP_MAX: return OMX_ALG_MAX_QP;
  default: return OMX_ALG_MAX_QP;
  }

  return OMX_ALG_MAX_QP;
}

OMX_VIDEO_CONTROLRATETYPE ConvertToOMXControlRate(RateControlType const& mode)
{
  switch(mode)
  {
  case RATE_CONTROL_CONSTANT_QUANTIZATION: return OMX_Video_ControlRateDisable;
  case RATE_CONTROL_CONSTANT_BITRATE: return OMX_Video_ControlRateConstant;
  case RATE_CONTROL_VARIABLE_BITRATE: return OMX_Video_ControlRateVariable;
  case RATE_CONTROL_LOW_LATENCY: return static_cast<OMX_VIDEO_CONTROLRATETYPE>(OMX_ALG_Video_ControlRateLowLatency);
  default: return OMX_Video_ControlRateMax;
  }

  return OMX_Video_ControlRateMax;
}

OMX_ALG_EAspectRatio ConvertToOMXAspectRatio(AspectRatioType const& aspectRatio)
{
  switch(aspectRatio)
  {
  case ASPECT_RATIO_NONE: return OMX_ALG_ASPECT_RATIO_NONE;
  case ASPECT_RATIO_4_3: return OMX_ALG_ASPECT_RATIO_4_3;
  case ASPECT_RATIO_16_9: return OMX_ALG_ASPECT_RATIO_16_9;
  case ASPECT_RATIO_AUTO: return OMX_ALG_ASPECT_RATIO_AUTO;
  case ASPECT_RATIO_MAX: return OMX_ALG_ASPECT_RATIO_MAX;
  default: return OMX_ALG_ASPECT_RATIO_MAX;
  }

  return OMX_ALG_ASPECT_RATIO_MAX;
}

OMX_ALG_EGopCtrlMode ConvertToOMXGopControl(GopControlType const& mode)
{
  switch(mode)
  {
  case GOP_CONTROL_DEFAULT: return OMX_ALG_GOP_MODE_DEFAULT;
  case GOP_CONTROL_PYRAMIDAL: return OMX_ALG_GOP_MODE_PYRAMIDAL;
  case GOP_CONTROL_LOW_DELAY_P: return OMX_ALG_GOP_MODE_LOW_DELAY_P;
  case GOP_CONTROL_LOW_DELAY_B: return OMX_ALG_GOP_MODE_LOW_DELAY_B;
  default: return OMX_ALG_GOP_MODE_MAX;
  }

  return OMX_ALG_GOP_MODE_MAX;
}

OMX_ALG_EGdrMode ConvertToOMXGdr(GdrType const& gdr)
{
  switch(gdr)
  {
  case GDR_OFF: return OMX_ALG_GDR_OFF;
  case GDR_VERTICAL: return OMX_ALG_GDR_VERTICAL;
  case GDR_HORTIZONTAL: return OMX_ALG_GDR_HORIZONTAL;
  default: return OMX_ALG_GDR_MAX;
  }

  return OMX_ALG_GDR_MAX;
}

OMX_BOOL ConvertToOMXDisableSceneChangeResilience(RateControlOptionType const& option)
{
  if(option == RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE)
    return OMX_FALSE; // Because it's bDisableSceneChangeResilience

  return OMX_TRUE;
}

OMX_ALG_EScalingList ConvertToOMXScalingList(ScalingListType const& scalingLisgt)
{
  switch(scalingLisgt)
  {
  case SCALING_LIST_DEFAULT: return OMX_ALG_SCL_DEFAULT;
  case SCALING_LIST_FLAT: return OMX_ALG_SCL_FLAT;
  case SCALING_LIST_MAX: return OMX_ALG_SCL_MAX;
  default: return OMX_ALG_SCL_MAX;
  }

  return OMX_ALG_SCL_MAX;
}

