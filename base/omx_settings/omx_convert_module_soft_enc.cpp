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

#include "omx_convert_module_soft_enc.h"

RateControlType ConvertSoftToModuleRateControl(AL_ERateCtrlMode const& mode)
{
  switch(mode)
  {
  case AL_RC_CONST_QP: return RATE_CONTROL_CONSTANT_QUANTIZATION;
  case AL_RC_CBR: return RATE_CONTROL_CONSTANT_BITRATE;
  case AL_RC_VBR: return RATE_CONTROL_VARIABLE_BITRATE;
  case AL_RC_LOW_LATENCY: return RATE_CONTROL_LOW_LATENCY;
  case AL_RC_MAX_ENUM: // fallthrough
  default: return RATE_CONTROL_MAX;
  }

  return RATE_CONTROL_MAX;
}

AspectRatioType ConvertSoftToModuleAspectRatio(AL_EAspectRatio const& aspectRatio)
{
  switch(aspectRatio)
  {
  case AL_ASPECT_RATIO_NONE: return ASPECT_RATIO_NONE;
  case AL_ASPECT_RATIO_4_3: return ASPECT_RATIO_4_3;
  case AL_ASPECT_RATIO_16_9: return ASPECT_RATIO_16_9;
  case AL_ASPECT_RATIO_AUTO: return ASPECT_RATIO_AUTO;
  case AL_ASPECT_RATIO_MAX_ENUM: // fallthrough
  default: return ASPECT_RATIO_MAX;
  }

  return ASPECT_RATIO_MAX;
}

GopControlType ConvertSoftToModuleGopControl(AL_EGopCtrlMode const& mode)
{
  switch(mode)
  {
  case AL_GOP_MODE_DEFAULT: return GOP_CONTROL_DEFAULT;
  case AL_GOP_MODE_PYRAMIDAL: return GOP_CONTROL_PYRAMIDAL;
  case AL_GOP_MODE_LOW_DELAY_P: return GOP_CONTROL_LOW_DELAY_P;
  case AL_GOP_MODE_LOW_DELAY_B: return GOP_CONTROL_LOW_DELAY_B;
  case AL_GOP_MODE_MAX_ENUM: // fallthrough
  default: return GOP_CONTROL_MAX;
  }

  return GOP_CONTROL_MAX;
}

GdrType ConvertSoftToModuleGdr(AL_EGdrMode const& gdr)
{
  switch(gdr)
  {
  case AL_GDR_OFF: return GDR_OFF;
  case AL_GDR_VERTICAL: return GDR_VERTICAL;
  case AL_GDR_HORIZONTAL: return GDR_HORTIZONTAL;
  case AL_GDR_MAX_ENUM: // fallthrough
  default: return GDR_MAX;
  }

  return GDR_MAX;
}

RateControlOptionType ConvertSoftToModuleRateControlOption(AL_ERateCtrlOption const& option)
{
  switch(option)
  {
  case AL_RC_OPT_NONE: return RATE_CONTROL_OPTION_NONE;
  case AL_RC_OPT_SCN_CHG_RES: return RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE;
  case AL_RC_OPT_MAX_ENUM: // fallthrough
  default: return RATE_CONTROL_OPTION_MAX;
  }

  return RATE_CONTROL_OPTION_MAX;
}

QPControlType ConvertSoftToModuleQPControl(AL_EQpCtrlMode const& mode)
{
  switch(mode)
  {
  case UNIFORM_QP: return QP_UNIFORM;
  case ROI_QP: return QP_ROI;
  case AUTO_QP: return QP_AUTO;
  case QP_MAX_ENUM: // fallthrough
  default: return QP_MAX;
  }

  return QP_MAX;
}

ScalingListType ConvertSoftToModuleScalingList(AL_EScalingList const& scalingList)
{
  switch(scalingList)
  {
  case AL_SCL_DEFAULT: return SCALING_LIST_DEFAULT;
  case AL_SCL_FLAT: return SCALING_LIST_FLAT;
  case AL_SCL_MAX_ENUM: // fallthrough
  default: return SCALING_LIST_MAX;
  }

  return SCALING_LIST_MAX;
}

AL_ERateCtrlMode ConvertModuleToSoftRateControl(RateControlType const& mode)
{
  switch(mode)
  {
  case RATE_CONTROL_CONSTANT_QUANTIZATION: return AL_RC_CONST_QP;
  case RATE_CONTROL_CONSTANT_BITRATE: return AL_RC_CBR;
  case RATE_CONTROL_VARIABLE_BITRATE: return AL_RC_VBR;
  case RATE_CONTROL_LOW_LATENCY: return AL_RC_LOW_LATENCY;
  case RATE_CONTROL_MAX: // fallthrough
  default: return AL_RC_MAX_ENUM;
  }

  return AL_RC_MAX_ENUM;
}

AL_EAspectRatio ConvertModuleToSoftAspectRatio(AspectRatioType const& aspectRatio)
{
  switch(aspectRatio)
  {
  case ASPECT_RATIO_NONE: return AL_ASPECT_RATIO_NONE;
  case ASPECT_RATIO_4_3: return AL_ASPECT_RATIO_4_3;
  case ASPECT_RATIO_16_9: return AL_ASPECT_RATIO_16_9;
  case ASPECT_RATIO_AUTO: return AL_ASPECT_RATIO_AUTO;
  case ASPECT_RATIO_MAX: // fallthrough
  default: return AL_ASPECT_RATIO_NONE;
  }

  return AL_ASPECT_RATIO_NONE;
}

AL_EGopCtrlMode ConvertModuleToSoftGopControl(GopControlType const& mode)
{
  switch(mode)
  {
  case GOP_CONTROL_DEFAULT: return AL_GOP_MODE_DEFAULT;
  case GOP_CONTROL_PYRAMIDAL: return AL_GOP_MODE_PYRAMIDAL;
  case GOP_CONTROL_LOW_DELAY_P: return AL_GOP_MODE_LOW_DELAY_P;
  case GOP_CONTROL_LOW_DELAY_B: return AL_GOP_MODE_LOW_DELAY_B;
  case GOP_CONTROL_MAX: // fallthrough
  default: return AL_GOP_MODE_MAX_ENUM;
  }

  return AL_GOP_MODE_MAX_ENUM;
}

AL_EScalingList ConvertModuleToSoftScalingList(ScalingListType const& scalingList)
{
  switch(scalingList)
  {
  case SCALING_LIST_DEFAULT: return AL_SCL_DEFAULT;
  case SCALING_LIST_FLAT: return AL_SCL_FLAT;
  default: return AL_SCL_MAX_ENUM;
  }

  return AL_SCL_MAX_ENUM;
}

AL_EGdrMode ConvertModuleToSoftGdr(GdrType const& gdr)
{
  switch(gdr)
  {
  case GDR_OFF: return AL_GDR_OFF;
  case GDR_VERTICAL: return AL_GDR_VERTICAL;
  case GDR_HORTIZONTAL: return AL_GDR_HORIZONTAL;
  case GDR_MAX: // fallthrough
  default: return AL_GDR_MAX_ENUM;
  }

  return AL_GDR_MAX_ENUM;
}

AL_ERateCtrlOption ConvertModuleToSoftRateControlOption(RateControlOptionType const& option)
{
  switch(option)
  {
  case RATE_CONTROL_OPTION_NONE: return AL_RC_OPT_NONE;
  case RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE: return AL_RC_OPT_SCN_CHG_RES;
  default: return AL_RC_OPT_MAX_ENUM;
  }

  return AL_RC_OPT_MAX_ENUM;
}

AL_EQpCtrlMode ConvertModuleToSoftQPControl(QPControlType const& mode)
{
  switch(mode)
  {
  case QP_UNIFORM: return UNIFORM_QP;
  case QP_ROI: return ROI_QP;
  case QP_AUTO: return AUTO_QP;
  default: return QP_MAX_ENUM;
  }

  return QP_MAX_ENUM;
}

