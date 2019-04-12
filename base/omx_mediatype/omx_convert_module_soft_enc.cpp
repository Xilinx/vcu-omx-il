/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

RateControlType ConvertSoftToModuleRateControl(AL_ERateCtrlMode mode)
{
  switch(mode)
  {
  case AL_RC_CONST_QP: return RateControlType::RATE_CONTROL_CONSTANT_QUANTIZATION;
  case AL_RC_CBR: return RateControlType::RATE_CONTROL_CONSTANT_BITRATE;
  case AL_RC_VBR: return RateControlType::RATE_CONTROL_VARIABLE_BITRATE;
  case AL_RC_CAPPED_VBR: return RateControlType::RATE_CONTROL_VARIABLE_CAPPED_BITRATE;
  case AL_RC_LOW_LATENCY: return RateControlType::RATE_CONTROL_LOW_LATENCY;
  case AL_RC_MAX_ENUM: return RateControlType::RATE_CONTROL_MAX_ENUM;
  default: return RateControlType::RATE_CONTROL_MAX_ENUM;
  }

  return RateControlType::RATE_CONTROL_MAX_ENUM;
}

AspectRatioType ConvertSoftToModuleAspectRatio(AL_EAspectRatio aspectRatio)
{
  switch(aspectRatio)
  {
  case AL_ASPECT_RATIO_NONE: return AspectRatioType::ASPECT_RATIO_NONE;
  case AL_ASPECT_RATIO_4_3: return AspectRatioType::ASPECT_RATIO_4_3;
  case AL_ASPECT_RATIO_16_9: return AspectRatioType::ASPECT_RATIO_16_9;
  case AL_ASPECT_RATIO_AUTO: return AspectRatioType::ASPECT_RATIO_AUTO;
  case AL_ASPECT_RATIO_MAX_ENUM: return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
  default: return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
  }

  return AspectRatioType::ASPECT_RATIO_MAX_ENUM;
}

GopControlType ConvertSoftToModuleGopControl(AL_EGopCtrlMode mode)
{
  switch(mode)
  {
  case AL_GOP_MODE_DEFAULT: return GopControlType::GOP_CONTROL_DEFAULT;
  case AL_GOP_MODE_PYRAMIDAL: return GopControlType::GOP_CONTROL_PYRAMIDAL;
  case AL_GOP_MODE_ADAPTIVE: return GopControlType::GOP_CONTROL_ADAPTIVE;
  case AL_GOP_MODE_LOW_DELAY_P: return GopControlType::GOP_CONTROL_LOW_DELAY_P;
  case AL_GOP_MODE_LOW_DELAY_B: return GopControlType::GOP_CONTROL_LOW_DELAY_B;
  case AL_GOP_MODE_MAX_ENUM: return GopControlType::GOP_CONTROL_MAX_ENUM;
  default: return GopControlType::GOP_CONTROL_MAX_ENUM;
  }

  return GopControlType::GOP_CONTROL_MAX_ENUM;
}

GdrType ConvertSoftToModuleGdr(AL_EGdrMode gdr)
{
  switch(gdr)
  {
  case AL_GDR_OFF: return GdrType::GDR_OFF;
  case AL_GDR_VERTICAL: return GdrType::GDR_VERTICAL;
  case AL_GDR_HORIZONTAL: return GdrType::GDR_HORTIZONTAL;
  case AL_GDR_MAX_ENUM: return GdrType::GDR_MAX_ENUM;
  default: return GdrType::GDR_MAX_ENUM;
  }

  return GdrType::GDR_MAX_ENUM;
}

RateControlOptions ConvertSoftToModuleRateControlOption(AL_ERateCtrlOption options)
{
	if(options & AL_RC_OPT_NONE)
		return RateControlOptions {};

	RateControlOptions moduleOptions {};

  if(options & AL_RC_OPT_SCN_CHG_RES)
    moduleOptions.isSceneChangeResilienceEnabled = true;

  if(options & AL_RC_OPT_DELAYED)
    moduleOptions.isDelayEnabled = true;

  if(options & AL_RC_OPT_STATIC_SCENE)
    moduleOptions.isStaticSceneEnabled = true;

  if(options & AL_RC_OPT_ENABLE_SKIP)
    moduleOptions.isSkipEnabled = true;

  return moduleOptions;
}

QPControlType ConvertSoftToModuleQPControl(AL_EQpCtrlMode mode)
{
  switch(mode)
  {
  case UNIFORM_QP: return QPControlType::QP_UNIFORM;
  case ROI_QP: return QPControlType::QP_ROI;
  case AUTO_QP: return QPControlType::QP_AUTO;
  case QP_MAX_ENUM: return QPControlType::QP_MAX_ENUM;
  default: return QPControlType::QP_MAX_ENUM;
  }

  return QPControlType::QP_MAX_ENUM;
}

ScalingListType ConvertSoftToModuleScalingList(AL_EScalingList scalingList)
{
  switch(scalingList)
  {
  case AL_SCL_DEFAULT: return ScalingListType::SCALING_LIST_DEFAULT;
  case AL_SCL_FLAT: return ScalingListType::SCALING_LIST_FLAT;
  case AL_SCL_MAX_ENUM: return ScalingListType::SCALING_LIST_MAX_ENUM;
  default: return ScalingListType::SCALING_LIST_MAX_ENUM;
  }

  return ScalingListType::SCALING_LIST_MAX_ENUM;
}

AL_ERateCtrlMode ConvertModuleToSoftRateControl(RateControlType mode)
{
  switch(mode)
  {
  case RateControlType::RATE_CONTROL_CONSTANT_QUANTIZATION: return AL_RC_CONST_QP;
  case RateControlType::RATE_CONTROL_CONSTANT_BITRATE: return AL_RC_CBR;
  case RateControlType::RATE_CONTROL_VARIABLE_BITRATE: return AL_RC_VBR;
  case RateControlType::RATE_CONTROL_VARIABLE_CAPPED_BITRATE: return AL_RC_CAPPED_VBR;
  case RateControlType::RATE_CONTROL_LOW_LATENCY: return AL_RC_LOW_LATENCY;
  case RateControlType::RATE_CONTROL_MAX_ENUM: return AL_RC_MAX_ENUM;
  default: return AL_RC_MAX_ENUM;
  }

  return AL_RC_MAX_ENUM;
}

AL_EAspectRatio ConvertModuleToSoftAspectRatio(AspectRatioType aspectRatio)
{
  switch(aspectRatio)
  {
  case AspectRatioType::ASPECT_RATIO_NONE: return AL_ASPECT_RATIO_NONE;
  case AspectRatioType::ASPECT_RATIO_4_3: return AL_ASPECT_RATIO_4_3;
  case AspectRatioType::ASPECT_RATIO_16_9: return AL_ASPECT_RATIO_16_9;
  case AspectRatioType::ASPECT_RATIO_AUTO: return AL_ASPECT_RATIO_AUTO;
  case AspectRatioType::ASPECT_RATIO_MAX_ENUM: return AL_ASPECT_RATIO_NONE;
  default: return AL_ASPECT_RATIO_NONE;
  }

  return AL_ASPECT_RATIO_NONE;
}

AL_EGopCtrlMode ConvertModuleToSoftGopControl(GopControlType mode)
{
  switch(mode)
  {
  case GopControlType::GOP_CONTROL_DEFAULT: return AL_GOP_MODE_DEFAULT;
  case GopControlType::GOP_CONTROL_PYRAMIDAL: return AL_GOP_MODE_PYRAMIDAL;
  case GopControlType::GOP_CONTROL_ADAPTIVE: return AL_GOP_MODE_ADAPTIVE;
  case GopControlType::GOP_CONTROL_LOW_DELAY_P: return AL_GOP_MODE_LOW_DELAY_P;
  case GopControlType::GOP_CONTROL_LOW_DELAY_B: return AL_GOP_MODE_LOW_DELAY_B;
  case GopControlType::GOP_CONTROL_MAX_ENUM: return AL_GOP_MODE_MAX_ENUM;
  default: return AL_GOP_MODE_MAX_ENUM;
  }

  return AL_GOP_MODE_MAX_ENUM;
}

AL_EScalingList ConvertModuleToSoftScalingList(ScalingListType scalingList)
{
  switch(scalingList)
  {
  case ScalingListType::SCALING_LIST_DEFAULT: return AL_SCL_DEFAULT;
  case ScalingListType::SCALING_LIST_FLAT: return AL_SCL_FLAT;
  case ScalingListType::SCALING_LIST_MAX_ENUM: return AL_SCL_MAX_ENUM;
  default: return AL_SCL_MAX_ENUM;
  }

  return AL_SCL_MAX_ENUM;
}

AL_EGdrMode ConvertModuleToSoftGdr(GdrType gdr)
{
  switch(gdr)
  {
  case GdrType::GDR_OFF: return AL_GDR_OFF;
  case GdrType::GDR_VERTICAL: return AL_GDR_VERTICAL;
  case GdrType::GDR_HORTIZONTAL: return AL_GDR_HORIZONTAL;
  case GdrType::GDR_MAX_ENUM: return AL_GDR_MAX_ENUM;
  default: return AL_GDR_MAX_ENUM;
  }

  return AL_GDR_MAX_ENUM;
}

AL_ERateCtrlOption ConvertModuleToSoftRateControlOption(RateControlOptions options)
{
  AL_ERateCtrlOption softOptions{};

  if(options.isSceneChangeResilienceEnabled)
    softOptions = static_cast<AL_ERateCtrlOption>(softOptions|AL_RC_OPT_SCN_CHG_RES);

  if(options.isDelayEnabled)
    softOptions = static_cast<AL_ERateCtrlOption>(softOptions|AL_RC_OPT_DELAYED);

  if(options.isStaticSceneEnabled)
    softOptions = static_cast<AL_ERateCtrlOption>(softOptions|AL_RC_OPT_STATIC_SCENE);

  if(options.isSkipEnabled)
    softOptions = static_cast<AL_ERateCtrlOption>(softOptions|AL_RC_OPT_ENABLE_SKIP);

  return softOptions;
}

AL_EQpCtrlMode ConvertModuleToSoftQPControl(QPControlType mode)
{
  switch(mode)
  {
  case QPControlType::QP_UNIFORM: return UNIFORM_QP;
  case QPControlType::QP_ROI: return ROI_QP;
  case QPControlType::QP_AUTO: return AUTO_QP;
  case QPControlType::QP_MAX_ENUM: return QP_MAX_ENUM;
  default: return QP_MAX_ENUM;
  }

  return QP_MAX_ENUM;
}

LoopFilterType ConvertSoftToModuleLoopFilter(AL_EChEncTool tools)
{
  auto loopFilterFlags = AL_OPT_LF | AL_OPT_LF_X_SLICE | AL_OPT_LF_X_TILE;
  auto loopFilterOptions = tools & loopFilterFlags;

  if((loopFilterOptions & loopFilterFlags) == 0)
    return LoopFilterType::LOOP_FILTER_DISABLE;

  if((loopFilterOptions & (AL_OPT_LF | AL_OPT_LF_X_TILE | AL_OPT_LF_X_SLICE)) == (AL_OPT_LF | AL_OPT_LF_X_TILE | AL_OPT_LF_X_SLICE))
    return LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE;

  if((loopFilterOptions & (AL_OPT_LF | AL_OPT_LF_X_SLICE)) == (AL_OPT_LF | AL_OPT_LF_X_SLICE))
    return LoopFilterType::LOOP_FILTER_ENABLE_CROSS_SLICE;

  if((loopFilterOptions & (AL_OPT_LF | AL_OPT_LF_X_TILE)) == (AL_OPT_LF | AL_OPT_LF_X_TILE))
    return LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE;

  if((loopFilterOptions & AL_OPT_LF) == AL_OPT_LF)
    return LoopFilterType::LOOP_FILTER_ENABLE;

  return LoopFilterType::LOOP_FILTER_MAX_ENUM;
}

AL_EChEncTool ConvertModuleToSoftLoopFilter(LoopFilterType loopFilter)
{
  switch(loopFilter)
  {
  case LoopFilterType::LOOP_FILTER_ENABLE: return static_cast<AL_EChEncTool>(AL_OPT_LF);
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_SLICE: return static_cast<AL_EChEncTool>(AL_OPT_LF | AL_OPT_LF_X_SLICE);
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE: return static_cast<AL_EChEncTool>(AL_OPT_LF | AL_OPT_LF_X_TILE);
  case LoopFilterType::LOOP_FILTER_ENABLE_CROSS_TILE_AND_SLICE: return static_cast<AL_EChEncTool>(AL_OPT_LF | AL_OPT_LF_X_TILE | AL_OPT_LF_X_SLICE);
  case LoopFilterType::LOOP_FILTER_DISABLE: return static_cast<AL_EChEncTool>(0);
  case LoopFilterType::LOOP_FILTER_MAX_ENUM: return static_cast<AL_EChEncTool>(0);
  }

  return static_cast<AL_EChEncTool>(0);
}

ColorPrimariesType ConvertSoftToModuleColorPrimaries(AL_EColourDescription colourDescription)
{
  switch(colourDescription)
  {
  case AL_COLOUR_DESC_RESERVED: return ColorPrimariesType::COLOR_PRIMARIES_RESERVED;
  case AL_COLOUR_DESC_UNSPECIFIED: return ColorPrimariesType::COLOR_PRIMARIES_UNSPECIFIED;
  case AL_COLOUR_DESC_BT_470_NTSC: return ColorPrimariesType::COLOR_PRIMARIES_BT_470_NTSC;
  case AL_COLOUR_DESC_BT_601_NTSC: return ColorPrimariesType::COLOR_PRIMARIES_BT_601_NTSC;
  case AL_COLOUR_DESC_BT_601_PAL: return ColorPrimariesType::COLOR_PRIMARIES_BT_601_PAL;
  case AL_COLOUR_DESC_BT_709: return ColorPrimariesType::COLOR_PRIMARIES_BT_709;
  case AL_COLOUR_DESC_BT_2020: return ColorPrimariesType::COLOR_PRIMARIES_BT_2020;
  case AL_COLOUR_DESC_SMPTE_240M: return ColorPrimariesType::COLOR_PRIMARIES_SMPTE_240M;
  case AL_COLOUR_DESC_SMPTE_ST_428: return ColorPrimariesType::COLOR_PRIMARIES_SMPTE_ST_428;
  case AL_COLOUR_DESC_SMPTE_RP_431: return ColorPrimariesType::COLOR_PRIMARIES_SMPTE_RP_431;
  case AL_COLOUR_DESC_SMPTE_EG_432: return ColorPrimariesType::COLOR_PRIMARIES_SMPTE_EG_432;
  case AL_COLOUR_DESC_EBU_3213: return ColorPrimariesType::COLOR_PRIMARIES_EBU_3213;
  case AL_COLOUR_DESC_GENERIC_FILM: return ColorPrimariesType::COLOR_PRIMARIES_GENERIC_FILM;
  case AL_COLOUR_DESC_MAX_ENUM: return ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM;
  default: return ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM;
  }

  return ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM;
}

AL_EColourDescription ConvertModuleToSoftColorPrimaries(ColorPrimariesType colorPrimaries)
{
  switch(colorPrimaries)
  {
  case ColorPrimariesType::COLOR_PRIMARIES_RESERVED: return AL_COLOUR_DESC_RESERVED;
  case ColorPrimariesType::COLOR_PRIMARIES_UNSPECIFIED: return AL_COLOUR_DESC_UNSPECIFIED;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_470_NTSC: return AL_COLOUR_DESC_BT_470_NTSC;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_601_NTSC: return AL_COLOUR_DESC_BT_601_NTSC;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_601_PAL: return AL_COLOUR_DESC_BT_601_PAL;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_709: return AL_COLOUR_DESC_BT_709;
  case ColorPrimariesType::COLOR_PRIMARIES_BT_2020: return AL_COLOUR_DESC_BT_2020;
  case ColorPrimariesType::COLOR_PRIMARIES_SMPTE_240M: return AL_COLOUR_DESC_SMPTE_240M;
  case ColorPrimariesType::COLOR_PRIMARIES_SMPTE_ST_428: return AL_COLOUR_DESC_SMPTE_ST_428;
  case ColorPrimariesType::COLOR_PRIMARIES_SMPTE_RP_431: return AL_COLOUR_DESC_SMPTE_RP_431;
  case ColorPrimariesType::COLOR_PRIMARIES_SMPTE_EG_432: return AL_COLOUR_DESC_SMPTE_EG_432;
  case ColorPrimariesType::COLOR_PRIMARIES_EBU_3213: return AL_COLOUR_DESC_EBU_3213;
  case ColorPrimariesType::COLOR_PRIMARIES_GENERIC_FILM: return AL_COLOUR_DESC_GENERIC_FILM;
  case ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM: return AL_COLOUR_DESC_MAX_ENUM;
  default: return AL_COLOUR_DESC_MAX_ENUM;
  }

  return AL_COLOUR_DESC_MAX_ENUM;
}

