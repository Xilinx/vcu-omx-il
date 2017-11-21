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

#include "omx_convert_omx_to_module_enc.h"

int ConvertToModuleBFrames(OMX_U32 const& bFrames, OMX_U32 const& pFrames)
{
  return bFrames / (pFrames + 1);
}

int ConvertToModuleGopLength(OMX_U32 const& bFrames, OMX_U32 const& pFrames)
{
  return pFrames + bFrames + 1;
}

EntropyCodingType ConvertToModuleEntropyCoding(OMX_BOOL const& isCabac)
{
  if(isCabac == OMX_FALSE)
    return ENTROPY_CODING_CAVLC;
  return ENTROPY_CODING_CABAC;
}

int ConvertToModuleQPInitial(OMX_U32 const& qpI)
{
  return qpI;
}

int ConvertToModuleQPDeltaIP(OMX_U32 const& qpI, OMX_U32 const& qpP)
{
  return qpP - qpI;
}

int ConvertToModuleQPDeltaPB(OMX_U32 const& qpP, OMX_U32 const& qpB)
{
  return qpB - qpP;
}

int ConvertToModuleQPMin(OMX_S32 const& qpMin)
{
  return qpMin;
}

int ConvertToModuleQPMax(OMX_S32 const& qpMax)
{
  return qpMax;
}

QPControlType ConvertToModuleQPControl(OMX_ALG_EQpCtrlMode const& mode)
{
  switch(mode)
  {
  case OMX_ALG_UNIFORM_QP: return QP_UNIFORM;
  case OMX_ALG_AUTO_QP: return QP_AUTO;
  case OMX_ALG_MAX_QP: return QP_MAX;
  default: return QP_MAX;
  }

  return QP_MAX;
}

RateControlType ConvertToModuleControlRate(OMX_VIDEO_CONTROLRATETYPE const& mode)
{
  switch(static_cast<OMX_U32>(mode)) // all indexes are in OMX_U32
  {
  case OMX_Video_ControlRateDisable: return RATE_CONTROL_CONSTANT_QUANTIZATION;
  case OMX_Video_ControlRateConstant: return RATE_CONTROL_CONSTANT_BITRATE;
  case OMX_Video_ControlRateVariable: return RATE_CONTROL_VARIABLE_BITRATE;
  case OMX_ALG_Video_ControlRateLowLatency: return RATE_CONTROL_LOW_LATENCY;
  default: return RATE_CONTROL_MAX;
  }

  return RATE_CONTROL_MAX;
}

AspectRatioType ConvertToModuleAspectRatio(OMX_ALG_EAspectRatio const& aspectRatio)
{
  switch(aspectRatio)
  {
  case OMX_ALG_ASPECT_RATIO_NONE: return ASPECT_RATIO_NONE;
  case OMX_ALG_ASPECT_RATIO_4_3: return ASPECT_RATIO_4_3;
  case OMX_ALG_ASPECT_RATIO_16_9: return ASPECT_RATIO_16_9;
  case OMX_ALG_ASPECT_RATIO_AUTO: return ASPECT_RATIO_AUTO;
  case OMX_ALG_ASPECT_RATIO_MAX: return ASPECT_RATIO_MAX;
  default: return ASPECT_RATIO_MAX;
  }

  return ASPECT_RATIO_MAX;
}

GopControlType ConvertToModuleGopControl(OMX_ALG_EGopCtrlMode const& mode)
{
  switch(mode)
  {
  case OMX_ALG_GOP_MODE_DEFAULT: return GOP_CONTROL_DEFAULT;
  case OMX_ALG_GOP_MODE_PYRAMIDAL: return GOP_CONTROL_PYRAMIDAL;
  case OMX_ALG_GOP_MODE_LOW_DELAY_P: return GOP_CONTROL_LOW_DELAY_P;
  case OMX_ALG_GOP_MODE_LOW_DELAY_B: return GOP_CONTROL_LOW_DELAY_B;
  case OMX_ALG_GOP_MODE_MAX: return GOP_CONTROL_MAX;
  default: return GOP_CONTROL_MAX;
  }

  return GOP_CONTROL_MAX;
}

GdrType ConvertToModuleGdr(OMX_ALG_EGdrMode const& gdr)
{
  switch(gdr)
  {
  case OMX_ALG_GDR_OFF: return GDR_OFF;
  case OMX_ALG_GDR_HORIZONTAL: return GDR_HORTIZONTAL;
  case OMX_ALG_GDR_VERTICAL: return GDR_VERTICAL;
  case OMX_ALG_GDR_MAX: return GDR_MAX;
  default: return GDR_MAX;
  }

  return GDR_MAX;
}

RateControlOptionType ConvertToModuleDisableSceneChangeResilience(OMX_BOOL const& disable)
{
  return (disable == OMX_TRUE) ? RATE_CONTROL_OPTION_NONE : RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE;
}

ScalingListType ConvertToModuleScalingList(OMX_ALG_EScalingList const& scalingListMode)
{
  switch(scalingListMode)
  {
  case OMX_ALG_SCL_DEFAULT: return SCALING_LIST_DEFAULT;
  case OMX_ALG_SCL_FLAT: return SCALING_LIST_FLAT;
  case OMX_ALG_SCL_MAX: return SCALING_LIST_MAX;
  default: return SCALING_LIST_MAX;
  }

  return SCALING_LIST_MAX;
}

