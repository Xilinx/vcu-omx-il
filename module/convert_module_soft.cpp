/******************************************************************************
*
* Copyright (C) 2016-2020 Allegro DVT2.  All rights reserved.
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

#include "convert_module_soft.h"

AL_EChromaMode ConvertModuleToSoftChroma(ColorType color)
{
  switch(color)
  {
  case ColorType::COLOR_400: return AL_CHROMA_4_0_0;
  case ColorType::COLOR_420: return AL_CHROMA_4_2_0;
  case ColorType::COLOR_422: return AL_CHROMA_4_2_2;
  case ColorType::COLOR_444: return AL_CHROMA_4_4_4;
  case ColorType::COLOR_MAX_ENUM: return AL_CHROMA_MAX_ENUM;
  default: return AL_CHROMA_MAX_ENUM;
  }

  return AL_CHROMA_MAX_ENUM;
}

ColorType ConvertSoftToModuleColor(AL_EChromaMode chroma)
{
  switch(chroma)
  {
  case AL_CHROMA_4_0_0: return ColorType::COLOR_400;
  case AL_CHROMA_4_2_0: return ColorType::COLOR_420;
  case AL_CHROMA_4_2_2: return ColorType::COLOR_422;
  case AL_CHROMA_4_4_4: return ColorType::COLOR_444;
  case AL_CHROMA_MAX_ENUM: return ColorType::COLOR_MAX_ENUM;
  default: return ColorType::COLOR_MAX_ENUM;
  }

  return ColorType::COLOR_MAX_ENUM;
}

AL_EEntropyMode ConvertModuleToSoftEntropyCoding(EntropyCodingType entropy)
{
  switch(entropy)
  {
  case EntropyCodingType::ENTROPY_CODING_CABAC: return AL_MODE_CABAC;
  case EntropyCodingType::ENTROPY_CODING_CAVLC: return AL_MODE_CAVLC;
  case EntropyCodingType::ENTROPY_CODING_MAX_ENUM: return AL_MODE_MAX_ENUM;
  default: return AL_MODE_MAX_ENUM;
  }

  return AL_MODE_MAX_ENUM;
}

EntropyCodingType ConvertSoftToModuleEntropyCoding(AL_EEntropyMode entropy)
{
  switch(entropy)
  {
  case AL_MODE_CABAC: return EntropyCodingType::ENTROPY_CODING_CABAC;
  case AL_MODE_CAVLC: return EntropyCodingType::ENTROPY_CODING_CAVLC;
  case AL_MODE_MAX_ENUM: return EntropyCodingType::ENTROPY_CODING_MAX_ENUM;
  default: return EntropyCodingType::ENTROPY_CODING_MAX_ENUM;
  }

  return EntropyCodingType::ENTROPY_CODING_MAX_ENUM;
}

VideoModeType ConvertSoftToModuleVideoMode(AL_EVideoMode videoMode)
{
  switch(videoMode)
  {
  case AL_VM_PROGRESSIVE: return VideoModeType::VIDEO_MODE_PROGRESSIVE;
  case AL_VM_INTERLACED_TOP: return VideoModeType::VIDEO_MODE_ALTERNATE_TOP_BOTTOM_FIELD;
  case AL_VM_INTERLACED_BOTTOM: return VideoModeType::VIDEO_MODE_ALTERNATE_BOTTOM_TOP_FIELD;
  case AL_VM_MAX_ENUM: return VideoModeType::VIDEO_MODE_MAX_ENUM;
  default: return VideoModeType::VIDEO_MODE_MAX_ENUM;
  }

  return VideoModeType::VIDEO_MODE_MAX_ENUM;
}

AL_EVideoMode ConvertModuleToSoftVideoMode(VideoModeType videoMode)
{
  switch(videoMode)
  {
  case VideoModeType::VIDEO_MODE_PROGRESSIVE: return AL_VM_PROGRESSIVE;
  case VideoModeType::VIDEO_MODE_ALTERNATE_TOP_BOTTOM_FIELD: return AL_VM_INTERLACED_TOP;
  case VideoModeType::VIDEO_MODE_ALTERNATE_BOTTOM_TOP_FIELD: return AL_VM_INTERLACED_BOTTOM;
  case VideoModeType::VIDEO_MODE_MAX_ENUM: return AL_VM_MAX_ENUM;
  default: return AL_VM_MAX_ENUM;
  }

  return AL_VM_MAX_ENUM;
}

SequencePictureModeType ConvertSoftToModuleSequenceMode(AL_ESequenceMode sequenceMode)
{
  switch(sequenceMode)
  {
  case AL_SM_UNKNOWN: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_UNKNOWN;
  case AL_SM_PROGRESSIVE: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_FRAME;
  case AL_SM_INTERLACED: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_FIELD;
  case AL_SM_MAX_ENUM: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_MAX_ENUM;
  default: return SequencePictureModeType::SEQUENCE_PICTURE_MODE_MAX_ENUM;
  }

  return SequencePictureModeType::SEQUENCE_PICTURE_MODE_MAX_ENUM;
}

AL_ESequenceMode ConvertModuleToSoftSequenceMode(SequencePictureModeType sequenceMode)
{
  switch(sequenceMode)
  {
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_UNKNOWN: return AL_SM_UNKNOWN;
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_FRAME: return AL_SM_PROGRESSIVE;
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_FIELD: return AL_SM_INTERLACED;
  case SequencePictureModeType::SEQUENCE_PICTURE_MODE_MAX_ENUM: return AL_SM_MAX_ENUM;
  default: return AL_SM_MAX_ENUM;
  }

  return AL_SM_MAX_ENUM;
}

TransferCharacteristicsType ConvertSoftToModuleTransferCharacteristics(AL_ETransferCharacteristics transferCharac)
{
  switch(transferCharac)
  {
  case AL_TRANSFER_CHARAC_UNSPECIFIED: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_UNSPECIFIED;
  case AL_TRANSFER_CHARAC_BT_2100_PQ: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_PQ;
  case AL_TRANSFER_CHARAC_BT_2100_HLG: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_HLG;
  default: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_MAX_ENUM;
  }

  return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_MAX_ENUM;
}

AL_ETransferCharacteristics ConvertModuleToSoftTransferCharacteristics(TransferCharacteristicsType transferCharac)
{
  switch(transferCharac)
  {
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_UNSPECIFIED: return AL_TRANSFER_CHARAC_UNSPECIFIED;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_PQ: return AL_TRANSFER_CHARAC_BT_2100_PQ;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_HLG: return AL_TRANSFER_CHARAC_BT_2100_HLG;
  default: return AL_TRANSFER_CHARAC_MAX_ENUM;
  }

  return AL_TRANSFER_CHARAC_MAX_ENUM;
}

ColourMatrixType ConvertSoftToModuleColourMatrix(AL_EColourMatrixCoefficients colourMatrix)
{
  switch(colourMatrix)
  {
  case AL_COLOUR_MAT_COEFF_UNSPECIFIED: return ColourMatrixType::COLOUR_MATRIX_UNSPECIFIED;
  case AL_COLOUR_MAT_COEFF_BT_2100_YCBCR: return ColourMatrixType::COLOUR_MATRIX_BT_2100_YCBCR;
  default: return ColourMatrixType::COLOUR_MATRIX_MAX_ENUM;
  }

  return ColourMatrixType::COLOUR_MATRIX_MAX_ENUM;
}

AL_EColourMatrixCoefficients ConvertModuleToSoftColourMatrix(ColourMatrixType colourMatrix)
{
  switch(colourMatrix)
  {
  case ColourMatrixType::COLOUR_MATRIX_UNSPECIFIED: return AL_COLOUR_MAT_COEFF_UNSPECIFIED;
  case ColourMatrixType::COLOUR_MATRIX_BT_2100_YCBCR: return AL_COLOUR_MAT_COEFF_BT_2100_YCBCR;
  default: return AL_COLOUR_MAT_COEFF_MAX_ENUM;
  }

  return AL_COLOUR_MAT_COEFF_MAX_ENUM;
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

void ConvertSoftToModule_DPL_ST2094_40(DisplayPeakLuminance_ST2094_40& dst, AL_TDisplayPeakLuminance_ST2094_40 const& src)
{
  dst.actualPeakLuminanceFlag = src.actual_peak_luminance_flag;

  if(src.actual_peak_luminance_flag)
  {
    dst.numRowsActualPeakLuminance = src.num_rows_actual_peak_luminance;
    dst.numColsActualPeakLuminance = src.num_cols_actual_peak_luminance;

    for(int i = 0; i < src.num_rows_actual_peak_luminance; i++)
      for(int j = 0; j < src.num_cols_actual_peak_luminance; j++)
        dst.actualPeakLuminance[i][j] = src.actual_peak_luminance[i][j];
  }
}

HighDynamicRangeSeis ConvertSoftToModuleHDRSEIs(const AL_THDRSEIs& hdrSEIs)
{
  HighDynamicRangeSeis modHDRSEIs;

  modHDRSEIs.hasMDCV = hdrSEIs.bHasMDCV;

  if(modHDRSEIs.hasMDCV)
  {
    for(int i = 0; i < 3; i++)
    {
      modHDRSEIs.masteringDisplayColourVolume.displayPrimaries[i].x = hdrSEIs.tMDCV.display_primaries[i].x;
      modHDRSEIs.masteringDisplayColourVolume.displayPrimaries[i].y = hdrSEIs.tMDCV.display_primaries[i].y;
    }

    modHDRSEIs.masteringDisplayColourVolume.whitePoint.x = hdrSEIs.tMDCV.white_point.x;
    modHDRSEIs.masteringDisplayColourVolume.whitePoint.y = hdrSEIs.tMDCV.white_point.y;
    modHDRSEIs.masteringDisplayColourVolume.maxDisplayMasteringLuminance = hdrSEIs.tMDCV.max_display_mastering_luminance;
    modHDRSEIs.masteringDisplayColourVolume.minDisplayMasteringLuminance = hdrSEIs.tMDCV.min_display_mastering_luminance;
  }

  modHDRSEIs.hasCLL = hdrSEIs.bHasCLL;

  if(modHDRSEIs.hasCLL)
  {
    modHDRSEIs.contentLightLevel.maxContentLightLevel = hdrSEIs.tCLL.max_content_light_level;
    modHDRSEIs.contentLightLevel.maxPicAverageLightLevel = hdrSEIs.tCLL.max_pic_average_light_level;
  }

  modHDRSEIs.hasST2094_10 = hdrSEIs.bHasST2094_10;

  if(modHDRSEIs.hasST2094_10)
  {
    modHDRSEIs.st2094_10.applicationVersion = hdrSEIs.tST2094_10.application_version;

    modHDRSEIs.st2094_10.processingWindowFlag = hdrSEIs.tST2094_10.processing_window_flag;

    if(modHDRSEIs.st2094_10.processingWindowFlag)
    {
      modHDRSEIs.st2094_10.processingWindow.activeAreaLeftOffset = hdrSEIs.tST2094_10.processing_window.active_area_left_offset;
      modHDRSEIs.st2094_10.processingWindow.activeAreaRightOffset = hdrSEIs.tST2094_10.processing_window.active_area_right_offset;
      modHDRSEIs.st2094_10.processingWindow.activeAreaTopOffset = hdrSEIs.tST2094_10.processing_window.active_area_top_offset;
      modHDRSEIs.st2094_10.processingWindow.activeAreaBottomOffset = hdrSEIs.tST2094_10.processing_window.active_area_bottom_offset;
    }

    modHDRSEIs.st2094_10.imageCharacteristics.minPQ = hdrSEIs.tST2094_10.image_characteristics.min_pq;
    modHDRSEIs.st2094_10.imageCharacteristics.maxPQ = hdrSEIs.tST2094_10.image_characteristics.max_pq;
    modHDRSEIs.st2094_10.imageCharacteristics.avgPQ = hdrSEIs.tST2094_10.image_characteristics.avg_pq;

    modHDRSEIs.st2094_10.numManualAdjustments = hdrSEIs.tST2094_10.num_manual_adjustments;

    for(int i = 0; i < modHDRSEIs.st2094_10.numManualAdjustments; i++)
    {
      modHDRSEIs.st2094_10.manualAdjustments[i].targetMaxPQ = hdrSEIs.tST2094_10.manual_adjustments[i].target_max_pq;
      modHDRSEIs.st2094_10.manualAdjustments[i].trimSlope = hdrSEIs.tST2094_10.manual_adjustments[i].trim_slope;
      modHDRSEIs.st2094_10.manualAdjustments[i].trimOffset = hdrSEIs.tST2094_10.manual_adjustments[i].trim_offset;
      modHDRSEIs.st2094_10.manualAdjustments[i].trimPower = hdrSEIs.tST2094_10.manual_adjustments[i].trim_power;
      modHDRSEIs.st2094_10.manualAdjustments[i].trimChromaWeight = hdrSEIs.tST2094_10.manual_adjustments[i].trim_chroma_weight;
      modHDRSEIs.st2094_10.manualAdjustments[i].trimSaturationGain = hdrSEIs.tST2094_10.manual_adjustments[i].trim_saturation_gain;
      modHDRSEIs.st2094_10.manualAdjustments[i].msWeight = hdrSEIs.tST2094_10.manual_adjustments[i].ms_weight;
    }
  }

  modHDRSEIs.hasST2094_40 = hdrSEIs.bHasST2094_40;

  if(modHDRSEIs.hasST2094_40)
  {
    modHDRSEIs.st2094_40.applicationVersion = hdrSEIs.tST2094_40.application_version;
    modHDRSEIs.st2094_40.numWindows = hdrSEIs.tST2094_40.num_windows;

    for(int i = 0; i < modHDRSEIs.st2094_40.numWindows - 1; i++)
    {
      const AL_TProcessingWindow_ST2094_40* pALPW = &hdrSEIs.tST2094_40.processing_windows[i];
      ProcessingWindow_ST2094_40* pPW = &modHDRSEIs.st2094_40.processingWindows[i];

      pPW->baseProcessingWindow.upperLeftCornerX = pALPW->base_processing_window.upper_left_corner_x;
      pPW->baseProcessingWindow.upperLeftCornerY = pALPW->base_processing_window.upper_left_corner_y;
      pPW->baseProcessingWindow.lowerRightCornerX = pALPW->base_processing_window.lower_right_corner_x;
      pPW->baseProcessingWindow.lowerRightCornerY = pALPW->base_processing_window.lower_right_corner_y;

      pPW->centerOfEllipseX = pALPW->center_of_ellipse_x;
      pPW->centerOfEllipseY = pALPW->center_of_ellipse_y;
      pPW->rotationAngle = pALPW->rotation_angle;
      pPW->semimajorAxisInternalEllipse = pALPW->semimajor_axis_internal_ellipse;
      pPW->semimajorAxisExternalEllipse = pALPW->semimajor_axis_external_ellipse;
      pPW->semiminorAxisExternalEllipse = pALPW->semiminor_axis_external_ellipse;
      pPW->overlapProcessOption = pALPW->overlap_process_option;
    }

    modHDRSEIs.st2094_40.targetedSystemDisplay.maximumLuminance = hdrSEIs.tST2094_40.targeted_system_display.maximum_luminance;
    ConvertSoftToModule_DPL_ST2094_40(modHDRSEIs.st2094_40.targetedSystemDisplay.peakLuminance, hdrSEIs.tST2094_40.targeted_system_display.peak_luminance);

    ConvertSoftToModule_DPL_ST2094_40(modHDRSEIs.st2094_40.masteringDisplayPeakLuminance, hdrSEIs.tST2094_40.mastering_display_peak_luminance);

    for(int i = 0; i < modHDRSEIs.st2094_40.numWindows; i++)
    {
      const AL_TProcessingWindowTransform_ST2094_40* pALPWT = &hdrSEIs.tST2094_40.processing_window_transforms[i];
      ProcessingWindowTransform_ST2094_40* pPWT = &modHDRSEIs.st2094_40.processingWindowTransforms[i];

      for(int j = 0; j < 3; j++)
        pPWT->maxscl[j] = pALPWT->maxscl[j];

      pPWT->averageMaxrgb = pALPWT->average_maxrgb;
      pPWT->numDistributionMaxrgbPercentiles = pALPWT->num_distribution_maxrgb_percentiles;

      for(int j = 0; j < pPWT->numDistributionMaxrgbPercentiles; j++)
      {
        pPWT->distributionMaxrgbPercentages[j] = pALPWT->distribution_maxrgb_percentages[j];
        pPWT->distributionMaxrgbPercentiles[j] = pALPWT->distribution_maxrgb_percentiles[j];
      }

      pPWT->fractionBrightPixels = pALPWT->fraction_bright_pixels;

      pPWT->toneMapping.toneMappingFlag = pALPWT->tone_mapping.tone_mapping_flag;

      if(pPWT->toneMapping.toneMappingFlag)
      {
        pPWT->toneMapping.kneePointX = pALPWT->tone_mapping.knee_point_x;
        pPWT->toneMapping.kneePointY = pALPWT->tone_mapping.knee_point_y;
        pPWT->toneMapping.numBezierCurveAnchors = pALPWT->tone_mapping.num_bezier_curve_anchors;

        for(int j = 0; j < pALPWT->tone_mapping.num_bezier_curve_anchors; j++)
          pPWT->toneMapping.bezierCurveAnchors[j] = pALPWT->tone_mapping.bezier_curve_anchors[j];
      }

      pPWT->colorSaturationMappingFlag = pALPWT->color_saturation_mapping_flag;

      if(pPWT->colorSaturationMappingFlag)
        pPWT->colorSaturationWeight = pALPWT->color_saturation_weight;
    }
  }

  return modHDRSEIs;
}

void ConvertModuleToSoft_DPL_ST2094_40(AL_TDisplayPeakLuminance_ST2094_40& dst, DisplayPeakLuminance_ST2094_40 const& src)
{
  dst.actual_peak_luminance_flag = src.actualPeakLuminanceFlag;

  if(src.actualPeakLuminanceFlag)
  {
    dst.num_rows_actual_peak_luminance = src.numRowsActualPeakLuminance;
    dst.num_cols_actual_peak_luminance = src.numColsActualPeakLuminance;

    for(int i = 0; i < src.numRowsActualPeakLuminance; i++)
      for(int j = 0; j < src.numColsActualPeakLuminance; j++)
        dst.actual_peak_luminance[i][j] = src.actualPeakLuminance[i][j];
  }
}

AL_THDRSEIs ConvertModuleToSoftHDRSEIs(const HighDynamicRangeSeis& hdrSEIs)
{
  AL_THDRSEIs alHDRSEIs;

  alHDRSEIs.bHasMDCV = hdrSEIs.hasMDCV;

  if(hdrSEIs.hasMDCV)
  {
    for(int i = 0; i < 3; i++)
    {
      alHDRSEIs.tMDCV.display_primaries[i].x = hdrSEIs.masteringDisplayColourVolume.displayPrimaries[i].x;
      alHDRSEIs.tMDCV.display_primaries[i].y = hdrSEIs.masteringDisplayColourVolume.displayPrimaries[i].y;
    }

    alHDRSEIs.tMDCV.white_point.x = hdrSEIs.masteringDisplayColourVolume.whitePoint.x;
    alHDRSEIs.tMDCV.white_point.y = hdrSEIs.masteringDisplayColourVolume.whitePoint.y;
    alHDRSEIs.tMDCV.max_display_mastering_luminance = hdrSEIs.masteringDisplayColourVolume.maxDisplayMasteringLuminance;
    alHDRSEIs.tMDCV.min_display_mastering_luminance = hdrSEIs.masteringDisplayColourVolume.minDisplayMasteringLuminance;
  }

  alHDRSEIs.bHasCLL = hdrSEIs.hasCLL;

  if(hdrSEIs.hasCLL)
  {
    alHDRSEIs.tCLL.max_content_light_level = hdrSEIs.contentLightLevel.maxContentLightLevel;
    alHDRSEIs.tCLL.max_pic_average_light_level = hdrSEIs.contentLightLevel.maxPicAverageLightLevel;
  }

  alHDRSEIs.bHasST2094_10 = hdrSEIs.hasST2094_10;

  if(hdrSEIs.hasST2094_10)
  {
    alHDRSEIs.tST2094_10.application_version = hdrSEIs.st2094_10.applicationVersion;

    alHDRSEIs.tST2094_10.processing_window_flag = hdrSEIs.st2094_10.processingWindowFlag;

    if(hdrSEIs.st2094_10.processingWindowFlag)
    {
      alHDRSEIs.tST2094_10.processing_window.active_area_left_offset = hdrSEIs.st2094_10.processingWindow.activeAreaLeftOffset;
      alHDRSEIs.tST2094_10.processing_window.active_area_right_offset = hdrSEIs.st2094_10.processingWindow.activeAreaRightOffset;
      alHDRSEIs.tST2094_10.processing_window.active_area_top_offset = hdrSEIs.st2094_10.processingWindow.activeAreaTopOffset;
      alHDRSEIs.tST2094_10.processing_window.active_area_bottom_offset = hdrSEIs.st2094_10.processingWindow.activeAreaBottomOffset;
    }

    alHDRSEIs.tST2094_10.image_characteristics.min_pq = hdrSEIs.st2094_10.imageCharacteristics.minPQ;
    alHDRSEIs.tST2094_10.image_characteristics.max_pq = hdrSEIs.st2094_10.imageCharacteristics.maxPQ;
    alHDRSEIs.tST2094_10.image_characteristics.avg_pq = hdrSEIs.st2094_10.imageCharacteristics.avgPQ;

    alHDRSEIs.tST2094_10.num_manual_adjustments = hdrSEIs.st2094_10.numManualAdjustments;

    for(int i = 0; i < hdrSEIs.st2094_10.numManualAdjustments; i++)
    {
      alHDRSEIs.tST2094_10.manual_adjustments[i].target_max_pq = hdrSEIs.st2094_10.manualAdjustments[i].targetMaxPQ;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_slope = hdrSEIs.st2094_10.manualAdjustments[i].trimSlope;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_offset = hdrSEIs.st2094_10.manualAdjustments[i].trimOffset;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_power = hdrSEIs.st2094_10.manualAdjustments[i].trimPower;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_chroma_weight = hdrSEIs.st2094_10.manualAdjustments[i].trimChromaWeight;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_saturation_gain = hdrSEIs.st2094_10.manualAdjustments[i].trimSaturationGain;
      alHDRSEIs.tST2094_10.manual_adjustments[i].ms_weight = hdrSEIs.st2094_10.manualAdjustments[i].msWeight;
    }
  }

  alHDRSEIs.bHasST2094_40 = hdrSEIs.hasST2094_40;

  if(hdrSEIs.hasST2094_40)
  {
    alHDRSEIs.tST2094_40.application_version = hdrSEIs.st2094_40.applicationVersion;
    alHDRSEIs.tST2094_40.num_windows = hdrSEIs.st2094_40.numWindows;

    for(int i = 0; i < hdrSEIs.st2094_40.numWindows - 1; i++)
    {
      AL_TProcessingWindow_ST2094_40* pALPW = &alHDRSEIs.tST2094_40.processing_windows[i];
      const ProcessingWindow_ST2094_40* pPW = &hdrSEIs.st2094_40.processingWindows[i];

      pALPW->base_processing_window.upper_left_corner_x = pPW->baseProcessingWindow.upperLeftCornerX;
      pALPW->base_processing_window.upper_left_corner_y = pPW->baseProcessingWindow.upperLeftCornerY;
      pALPW->base_processing_window.lower_right_corner_x = pPW->baseProcessingWindow.lowerRightCornerX;
      pALPW->base_processing_window.lower_right_corner_y = pPW->baseProcessingWindow.lowerRightCornerY;

      pALPW->center_of_ellipse_x = pPW->centerOfEllipseX;
      pALPW->center_of_ellipse_y = pPW->centerOfEllipseY;
      pALPW->rotation_angle = pPW->rotationAngle;
      pALPW->semimajor_axis_internal_ellipse = pPW->semimajorAxisInternalEllipse;
      pALPW->semimajor_axis_external_ellipse = pPW->semimajorAxisExternalEllipse;
      pALPW->semiminor_axis_external_ellipse = pPW->semiminorAxisExternalEllipse;
      pALPW->overlap_process_option = pPW->overlapProcessOption;
    }

    alHDRSEIs.tST2094_40.targeted_system_display.maximum_luminance = hdrSEIs.st2094_40.targetedSystemDisplay.maximumLuminance;
    ConvertModuleToSoft_DPL_ST2094_40(alHDRSEIs.tST2094_40.targeted_system_display.peak_luminance, hdrSEIs.st2094_40.targetedSystemDisplay.peakLuminance);

    ConvertModuleToSoft_DPL_ST2094_40(alHDRSEIs.tST2094_40.mastering_display_peak_luminance, hdrSEIs.st2094_40.masteringDisplayPeakLuminance);

    for(int i = 0; i < hdrSEIs.st2094_40.numWindows; i++)
    {
      AL_TProcessingWindowTransform_ST2094_40* pALPWT = &alHDRSEIs.tST2094_40.processing_window_transforms[i];
      const ProcessingWindowTransform_ST2094_40* pPWT = &hdrSEIs.st2094_40.processingWindowTransforms[i];

      for(int j = 0; j < 3; j++)
        pALPWT->maxscl[j] = pPWT->maxscl[j];

      pALPWT->average_maxrgb = pPWT->averageMaxrgb;
      pALPWT->num_distribution_maxrgb_percentiles = pPWT->numDistributionMaxrgbPercentiles;

      for(int j = 0; j < pPWT->numDistributionMaxrgbPercentiles; j++)
      {
        pALPWT->distribution_maxrgb_percentages[j] = pPWT->distributionMaxrgbPercentages[j];
        pALPWT->distribution_maxrgb_percentiles[j] = pPWT->distributionMaxrgbPercentiles[j];
      }

      pALPWT->fraction_bright_pixels = pPWT->fractionBrightPixels;

      pALPWT->tone_mapping.tone_mapping_flag = pPWT->toneMapping.toneMappingFlag;

      if(pPWT->toneMapping.toneMappingFlag)
      {
        pALPWT->tone_mapping.knee_point_x = pPWT->toneMapping.kneePointX;
        pALPWT->tone_mapping.knee_point_y = pPWT->toneMapping.kneePointY;
        pALPWT->tone_mapping.num_bezier_curve_anchors = pPWT->toneMapping.numBezierCurveAnchors;

        for(int j = 0; j < pPWT->toneMapping.numBezierCurveAnchors; j++)
          pALPWT->tone_mapping.bezier_curve_anchors[j] = pPWT->toneMapping.bezierCurveAnchors[j];
      }

      pALPWT->color_saturation_mapping_flag = pPWT->colorSaturationMappingFlag;

      if(pPWT->colorSaturationMappingFlag)
        pALPWT->color_saturation_weight = pPWT->colorSaturationWeight;
    }
  }

  return alHDRSEIs;
}
