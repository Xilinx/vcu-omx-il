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

#include "omx_convert_module_to_soft_enc.h"

AL_ERateCtrlMode ConvertToSoftRateControl(RateControlType const& mode)
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

AL_EAspectRatio ConvertToSoftAspectRatio(AspectRatioType const& aspectRatio)
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

AL_EGopCtrlMode ConvertToSoftGopControl(GopControlType const& mode)
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

AL_EScalingList ConvertToSoftScalingList(ScalingListType const& scalingList)
{
  switch(scalingList)
  {
  case SCALING_LIST_DEFAULT: return AL_SCL_DEFAULT;
  case SCALING_LIST_FLAT: return AL_SCL_FLAT;
  default: return AL_SCL_MAX_ENUM;
  }

  return AL_SCL_MAX_ENUM;
}

AL_EGdrMode ConvertToSoftGdr(GdrType const& gdr)
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

AL_ERateCtrlOption ConvertToSoftRateControlOption(RateControlOptionType const& option)
{
  switch(option)
  {
  case RATE_CONTROL_OPTION_NONE: return AL_RC_OPT_NONE;
  case RATE_CONTROL_OPTION_SCENE_CHANGE_RESILIENCE: return AL_RC_OPT_SCN_CHG_RES;
  default: return AL_RC_OPT_MAX_ENUM;
  }

  return AL_RC_OPT_MAX_ENUM;
}

AL_EQpCtrlMode ConvertToSoftQPControl(QPControlType const& mode)
{
  switch(mode)
  {
  case QP_UNIFORM: return UNIFORM_QP;
  case QP_AUTO: return AUTO_QP;
  default: return QP_MAX_ENUM;
  }

  return QP_MAX_ENUM;
}

