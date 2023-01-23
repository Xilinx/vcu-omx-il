/******************************************************************************
*
* Copyright (C) 2015-2022 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
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
  case AL_TRANSFER_CHARAC_BT_709: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_709;
  case AL_TRANSFER_CHARAC_UNSPECIFIED: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_UNSPECIFIED;
  case AL_TRANSFER_CHARAC_BT_470_SYSTEM_M: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_M;
  case AL_TRANSFER_CHARAC_BT_470_SYSTEM_B: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_B;
  case AL_TRANSFER_CHARAC_BT_601: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_601;
  case AL_TRANSFER_CHARAC_SMPTE_240M: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_SMPTE_240M;
  case AL_TRANSFER_CHARAC_LINEAR: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LINEAR;
  case AL_TRANSFER_CHARAC_LOG: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LOG;
  case AL_TRANSFER_CHARAC_LOG_EXTENDED: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LOG_EXTENDED;
  case AL_TRANSFER_CHARAC_IEC_61966_2_4: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_IEC_61966_2_4;
  case AL_TRANSFER_CHARAC_BT_1361: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_1361;
  case AL_TRANSFER_CHARAC_IEC_61966_2_1: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_IEC_61966_2_1;
  case AL_TRANSFER_CHARAC_BT_2020_10B: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2020_10B;
  case AL_TRANSFER_CHARAC_BT_2020_12B: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2020_12B;
  case AL_TRANSFER_CHARAC_BT_2100_PQ: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_PQ;
  case AL_TRANSFER_CHARAC_SMPTE_428: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_SMPTE_428;
  case AL_TRANSFER_CHARAC_BT_2100_HLG: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_HLG;
  default: return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_MAX_ENUM;
  }

  return TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_MAX_ENUM;
}

AL_ETransferCharacteristics ConvertModuleToSoftTransferCharacteristics(TransferCharacteristicsType transferCharac)
{
  switch(transferCharac)
  {
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_709: return AL_TRANSFER_CHARAC_BT_709;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_UNSPECIFIED: return AL_TRANSFER_CHARAC_UNSPECIFIED;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_M: return AL_TRANSFER_CHARAC_BT_470_SYSTEM_M;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_470_SYSTEM_B: return AL_TRANSFER_CHARAC_BT_470_SYSTEM_B;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_601: return AL_TRANSFER_CHARAC_BT_601;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_SMPTE_240M: return AL_TRANSFER_CHARAC_SMPTE_240M;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LINEAR: return AL_TRANSFER_CHARAC_LINEAR;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LOG: return AL_TRANSFER_CHARAC_LOG;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_LOG_EXTENDED: return AL_TRANSFER_CHARAC_LOG_EXTENDED;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_IEC_61966_2_4: return AL_TRANSFER_CHARAC_IEC_61966_2_4;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_1361: return AL_TRANSFER_CHARAC_BT_1361;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_IEC_61966_2_1: return AL_TRANSFER_CHARAC_IEC_61966_2_1;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2020_10B: return AL_TRANSFER_CHARAC_BT_2020_10B;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2020_12B: return AL_TRANSFER_CHARAC_BT_2020_12B;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_PQ: return AL_TRANSFER_CHARAC_BT_2100_PQ;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_SMPTE_428: return AL_TRANSFER_CHARAC_SMPTE_428;
  case TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_BT_2100_HLG: return AL_TRANSFER_CHARAC_BT_2100_HLG;
  default: return AL_TRANSFER_CHARAC_MAX_ENUM;
  }

  return AL_TRANSFER_CHARAC_MAX_ENUM;
}

ColourMatrixType ConvertSoftToModuleColourMatrix(AL_EColourMatrixCoefficients colourMatrix)
{
  switch(colourMatrix)
  {
  case AL_COLOUR_MAT_COEFF_GBR: return ColourMatrixType::COLOUR_MATRIX_GBR;
  case AL_COLOUR_MAT_COEFF_BT_709: return ColourMatrixType::COLOUR_MATRIX_BT_709;
  case AL_COLOUR_MAT_COEFF_UNSPECIFIED: return ColourMatrixType::COLOUR_MATRIX_UNSPECIFIED;
  case AL_COLOUR_MAT_COEFF_USFCC_CFR: return ColourMatrixType::COLOUR_MATRIX_USFCC_CFR;
  case AL_COLOUR_MAT_COEFF_BT_601_625: return ColourMatrixType::COLOUR_MATRIX_BT_601_625;
  case AL_COLOUR_MAT_COEFF_BT_601_525: return ColourMatrixType::COLOUR_MATRIX_BT_601_525;
  case AL_COLOUR_MAT_COEFF_BT_SMPTE_240M: return ColourMatrixType::COLOUR_MATRIX_BT_SMPTE_240M;
  case AL_COLOUR_MAT_COEFF_BT_YCGCO: return ColourMatrixType::COLOUR_MATRIX_BT_YCGCO;
  case AL_COLOUR_MAT_COEFF_BT_2100_YCBCR: return ColourMatrixType::COLOUR_MATRIX_BT_2100_YCBCR;
  case AL_COLOUR_MAT_COEFF_BT_2020_CLS: return ColourMatrixType::COLOUR_MATRIX_BT_2020_CLS;
  case AL_COLOUR_MAT_COEFF_SMPTE_2085: return ColourMatrixType::COLOUR_MATRIX_SMPTE_2085;
  case AL_COLOUR_MAT_COEFF_CHROMA_DERIVED_NCLS: return ColourMatrixType::COLOUR_MATRIX_CHROMA_DERIVED_NCLS;
  case AL_COLOUR_MAT_COEFF_CHROMA_DERIVED_CLS: return ColourMatrixType::COLOUR_MATRIX_CHROMA_DERIVED_CLS;
  case AL_COLOUR_MAT_COEFF_BT_2100_ICTCP: return ColourMatrixType::COLOUR_MATRIX_BT_2100_ICTCP;
  default: return ColourMatrixType::COLOUR_MATRIX_MAX_ENUM;
  }

  return ColourMatrixType::COLOUR_MATRIX_MAX_ENUM;
}

AL_EColourMatrixCoefficients ConvertModuleToSoftColourMatrix(ColourMatrixType colourMatrix)
{
  switch(colourMatrix)
  {
  case ColourMatrixType::COLOUR_MATRIX_GBR: return AL_COLOUR_MAT_COEFF_GBR;
  case ColourMatrixType::COLOUR_MATRIX_BT_709: return AL_COLOUR_MAT_COEFF_BT_709;
  case ColourMatrixType::COLOUR_MATRIX_UNSPECIFIED: return AL_COLOUR_MAT_COEFF_UNSPECIFIED;
  case ColourMatrixType::COLOUR_MATRIX_USFCC_CFR: return AL_COLOUR_MAT_COEFF_USFCC_CFR;
  case ColourMatrixType::COLOUR_MATRIX_BT_601_625: return AL_COLOUR_MAT_COEFF_BT_601_625;
  case ColourMatrixType::COLOUR_MATRIX_BT_601_525: return AL_COLOUR_MAT_COEFF_BT_601_525;
  case ColourMatrixType::COLOUR_MATRIX_BT_SMPTE_240M: return AL_COLOUR_MAT_COEFF_BT_SMPTE_240M;
  case ColourMatrixType::COLOUR_MATRIX_BT_YCGCO: return AL_COLOUR_MAT_COEFF_BT_YCGCO;
  case ColourMatrixType::COLOUR_MATRIX_BT_2100_YCBCR: return AL_COLOUR_MAT_COEFF_BT_2100_YCBCR;
  case ColourMatrixType::COLOUR_MATRIX_BT_2020_CLS: return AL_COLOUR_MAT_COEFF_BT_2020_CLS;
  case ColourMatrixType::COLOUR_MATRIX_SMPTE_2085: return AL_COLOUR_MAT_COEFF_SMPTE_2085;
  case ColourMatrixType::COLOUR_MATRIX_CHROMA_DERIVED_NCLS: return AL_COLOUR_MAT_COEFF_CHROMA_DERIVED_NCLS;
  case ColourMatrixType::COLOUR_MATRIX_CHROMA_DERIVED_CLS: return AL_COLOUR_MAT_COEFF_CHROMA_DERIVED_CLS;
  case ColourMatrixType::COLOUR_MATRIX_BT_2100_ICTCP: return AL_COLOUR_MAT_COEFF_BT_2100_ICTCP;
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

HighDynamicRangeSeis ConvertSoftToModuleHDRSEIs(AL_THDRSEIs const& hdrSEIs)
{
  HighDynamicRangeSeis modHDRSEIs;

  modHDRSEIs.mdcv.enabled = hdrSEIs.bHasMDCV;

  if(modHDRSEIs.mdcv.enabled)
  {
    for(int i = 0; i < 3; i++)
    {
      modHDRSEIs.mdcv.feature.displayPrimaries[i].x = hdrSEIs.tMDCV.display_primaries[i].x;
      modHDRSEIs.mdcv.feature.displayPrimaries[i].y = hdrSEIs.tMDCV.display_primaries[i].y;
    }

    modHDRSEIs.mdcv.feature.whitePoint.x = hdrSEIs.tMDCV.white_point.x;
    modHDRSEIs.mdcv.feature.whitePoint.y = hdrSEIs.tMDCV.white_point.y;
    modHDRSEIs.mdcv.feature.maxDisplayMasteringLuminance = hdrSEIs.tMDCV.max_display_mastering_luminance;
    modHDRSEIs.mdcv.feature.minDisplayMasteringLuminance = hdrSEIs.tMDCV.min_display_mastering_luminance;
  }

  modHDRSEIs.cll.enabled = hdrSEIs.bHasCLL;

  if(modHDRSEIs.cll.enabled)
  {
    modHDRSEIs.cll.feature.maxContentLightLevel = hdrSEIs.tCLL.max_content_light_level;
    modHDRSEIs.cll.feature.maxPicAverageLightLevel = hdrSEIs.tCLL.max_pic_average_light_level;
  }

  modHDRSEIs.atc.enabled = hdrSEIs.bHasATC;

  if(modHDRSEIs.atc.enabled)
  {
    modHDRSEIs.atc.feature.preferredTransferCharacteristics =
      ConvertSoftToModuleTransferCharacteristics(hdrSEIs.tATC.preferred_transfer_characteristics);
  }

  modHDRSEIs.st2094_10.enabled = hdrSEIs.bHasST2094_10;

  if(modHDRSEIs.st2094_10.enabled)
  {
    modHDRSEIs.st2094_10.feature.applicationVersion = hdrSEIs.tST2094_10.application_version;

    modHDRSEIs.st2094_10.feature.processingWindowFlag = hdrSEIs.tST2094_10.processing_window_flag;

    if(modHDRSEIs.st2094_10.feature.processingWindowFlag)
    {
      modHDRSEIs.st2094_10.feature.processingWindow.activeAreaLeftOffset = hdrSEIs.tST2094_10.processing_window.active_area_left_offset;
      modHDRSEIs.st2094_10.feature.processingWindow.activeAreaRightOffset = hdrSEIs.tST2094_10.processing_window.active_area_right_offset;
      modHDRSEIs.st2094_10.feature.processingWindow.activeAreaTopOffset = hdrSEIs.tST2094_10.processing_window.active_area_top_offset;
      modHDRSEIs.st2094_10.feature.processingWindow.activeAreaBottomOffset = hdrSEIs.tST2094_10.processing_window.active_area_bottom_offset;
    }

    modHDRSEIs.st2094_10.feature.imageCharacteristics.minPQ = hdrSEIs.tST2094_10.image_characteristics.min_pq;
    modHDRSEIs.st2094_10.feature.imageCharacteristics.maxPQ = hdrSEIs.tST2094_10.image_characteristics.max_pq;
    modHDRSEIs.st2094_10.feature.imageCharacteristics.avgPQ = hdrSEIs.tST2094_10.image_characteristics.avg_pq;

    modHDRSEIs.st2094_10.feature.numManualAdjustments = hdrSEIs.tST2094_10.num_manual_adjustments;

    for(int i = 0; i < modHDRSEIs.st2094_10.feature.numManualAdjustments; i++)
    {
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].targetMaxPQ = hdrSEIs.tST2094_10.manual_adjustments[i].target_max_pq;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimSlope = hdrSEIs.tST2094_10.manual_adjustments[i].trim_slope;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimOffset = hdrSEIs.tST2094_10.manual_adjustments[i].trim_offset;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimPower = hdrSEIs.tST2094_10.manual_adjustments[i].trim_power;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimChromaWeight = hdrSEIs.tST2094_10.manual_adjustments[i].trim_chroma_weight;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].trimSaturationGain = hdrSEIs.tST2094_10.manual_adjustments[i].trim_saturation_gain;
      modHDRSEIs.st2094_10.feature.manualAdjustments[i].msWeight = hdrSEIs.tST2094_10.manual_adjustments[i].ms_weight;
    }
  }

  modHDRSEIs.st2094_40.enabled = hdrSEIs.bHasST2094_40;

  if(modHDRSEIs.st2094_40.enabled)
  {
    modHDRSEIs.st2094_40.feature.applicationVersion = hdrSEIs.tST2094_40.application_version;
    modHDRSEIs.st2094_40.feature.numWindows = hdrSEIs.tST2094_40.num_windows;

    for(int i = 0; i < modHDRSEIs.st2094_40.feature.numWindows - 1; i++)
    {
      const AL_TProcessingWindow_ST2094_40* pALPW = &hdrSEIs.tST2094_40.processing_windows[i];
      ProcessingWindow_ST2094_40* pPW = &modHDRSEIs.st2094_40.feature.processingWindows[i];

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

    modHDRSEIs.st2094_40.feature.targetedSystemDisplay.maximumLuminance = hdrSEIs.tST2094_40.targeted_system_display.maximum_luminance;
    ConvertSoftToModule_DPL_ST2094_40(modHDRSEIs.st2094_40.feature.targetedSystemDisplay.peakLuminance, hdrSEIs.tST2094_40.targeted_system_display.peak_luminance);

    ConvertSoftToModule_DPL_ST2094_40(modHDRSEIs.st2094_40.feature.masteringDisplayPeakLuminance, hdrSEIs.tST2094_40.mastering_display_peak_luminance);

    for(int i = 0; i < modHDRSEIs.st2094_40.feature.numWindows; i++)
    {
      const AL_TProcessingWindowTransform_ST2094_40* pALPWT = &hdrSEIs.tST2094_40.processing_window_transforms[i];
      ProcessingWindowTransform_ST2094_40* pPWT = &modHDRSEIs.st2094_40.feature.processingWindowTransforms[i];

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

AL_THDRSEIs ConvertModuleToSoftHDRSEIs(HighDynamicRangeSeis const& hdrSEIs)
{
  AL_THDRSEIs alHDRSEIs;

  alHDRSEIs.bHasMDCV = hdrSEIs.mdcv.enabled;

  if(hdrSEIs.mdcv.enabled)
  {
    for(int i = 0; i < 3; i++)
    {
      alHDRSEIs.tMDCV.display_primaries[i].x = hdrSEIs.mdcv.feature.displayPrimaries[i].x;
      alHDRSEIs.tMDCV.display_primaries[i].y = hdrSEIs.mdcv.feature.displayPrimaries[i].y;
    }

    alHDRSEIs.tMDCV.white_point.x = hdrSEIs.mdcv.feature.whitePoint.x;
    alHDRSEIs.tMDCV.white_point.y = hdrSEIs.mdcv.feature.whitePoint.y;
    alHDRSEIs.tMDCV.max_display_mastering_luminance = hdrSEIs.mdcv.feature.maxDisplayMasteringLuminance;
    alHDRSEIs.tMDCV.min_display_mastering_luminance = hdrSEIs.mdcv.feature.minDisplayMasteringLuminance;
  }

  alHDRSEIs.bHasCLL = hdrSEIs.cll.enabled;

  if(hdrSEIs.cll.enabled)
  {
    alHDRSEIs.tCLL.max_content_light_level = hdrSEIs.cll.feature.maxContentLightLevel;
    alHDRSEIs.tCLL.max_pic_average_light_level = hdrSEIs.cll.feature.maxPicAverageLightLevel;
  }

  alHDRSEIs.bHasATC = hdrSEIs.atc.enabled;

  if(hdrSEIs.atc.enabled)
  {
    alHDRSEIs.tATC.preferred_transfer_characteristics =
      ConvertModuleToSoftTransferCharacteristics(hdrSEIs.atc.feature.preferredTransferCharacteristics);
  }

  alHDRSEIs.bHasST2094_10 = hdrSEIs.st2094_10.enabled;

  if(hdrSEIs.st2094_10.enabled)
  {
    alHDRSEIs.tST2094_10.application_version = hdrSEIs.st2094_10.feature.applicationVersion;

    alHDRSEIs.tST2094_10.processing_window_flag = hdrSEIs.st2094_10.feature.processingWindowFlag;

    if(hdrSEIs.st2094_10.feature.processingWindowFlag)
    {
      alHDRSEIs.tST2094_10.processing_window.active_area_left_offset = hdrSEIs.st2094_10.feature.processingWindow.activeAreaLeftOffset;
      alHDRSEIs.tST2094_10.processing_window.active_area_right_offset = hdrSEIs.st2094_10.feature.processingWindow.activeAreaRightOffset;
      alHDRSEIs.tST2094_10.processing_window.active_area_top_offset = hdrSEIs.st2094_10.feature.processingWindow.activeAreaTopOffset;
      alHDRSEIs.tST2094_10.processing_window.active_area_bottom_offset = hdrSEIs.st2094_10.feature.processingWindow.activeAreaBottomOffset;
    }

    alHDRSEIs.tST2094_10.image_characteristics.min_pq = hdrSEIs.st2094_10.feature.imageCharacteristics.minPQ;
    alHDRSEIs.tST2094_10.image_characteristics.max_pq = hdrSEIs.st2094_10.feature.imageCharacteristics.maxPQ;
    alHDRSEIs.tST2094_10.image_characteristics.avg_pq = hdrSEIs.st2094_10.feature.imageCharacteristics.avgPQ;

    alHDRSEIs.tST2094_10.num_manual_adjustments = hdrSEIs.st2094_10.feature.numManualAdjustments;

    for(int i = 0; i < hdrSEIs.st2094_10.feature.numManualAdjustments; i++)
    {
      alHDRSEIs.tST2094_10.manual_adjustments[i].target_max_pq = hdrSEIs.st2094_10.feature.manualAdjustments[i].targetMaxPQ;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_slope = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimSlope;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_offset = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimOffset;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_power = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimPower;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_chroma_weight = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimChromaWeight;
      alHDRSEIs.tST2094_10.manual_adjustments[i].trim_saturation_gain = hdrSEIs.st2094_10.feature.manualAdjustments[i].trimSaturationGain;
      alHDRSEIs.tST2094_10.manual_adjustments[i].ms_weight = hdrSEIs.st2094_10.feature.manualAdjustments[i].msWeight;
    }
  }

  alHDRSEIs.bHasST2094_40 = hdrSEIs.st2094_40.enabled;

  if(hdrSEIs.st2094_40.enabled)
  {
    alHDRSEIs.tST2094_40.application_version = hdrSEIs.st2094_40.feature.applicationVersion;
    alHDRSEIs.tST2094_40.num_windows = hdrSEIs.st2094_40.feature.numWindows;

    for(int i = 0; i < hdrSEIs.st2094_40.feature.numWindows - 1; i++)
    {
      AL_TProcessingWindow_ST2094_40* pALPW = &alHDRSEIs.tST2094_40.processing_windows[i];
      const ProcessingWindow_ST2094_40* pPW = &hdrSEIs.st2094_40.feature.processingWindows[i];

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

    alHDRSEIs.tST2094_40.targeted_system_display.maximum_luminance = hdrSEIs.st2094_40.feature.targetedSystemDisplay.maximumLuminance;
    ConvertModuleToSoft_DPL_ST2094_40(alHDRSEIs.tST2094_40.targeted_system_display.peak_luminance, hdrSEIs.st2094_40.feature.targetedSystemDisplay.peakLuminance);

    ConvertModuleToSoft_DPL_ST2094_40(alHDRSEIs.tST2094_40.mastering_display_peak_luminance, hdrSEIs.st2094_40.feature.masteringDisplayPeakLuminance);

    for(int i = 0; i < hdrSEIs.st2094_40.feature.numWindows; i++)
    {
      AL_TProcessingWindowTransform_ST2094_40* pALPWT = &alHDRSEIs.tST2094_40.processing_window_transforms[i];
      const ProcessingWindowTransform_ST2094_40* pPWT = &hdrSEIs.st2094_40.feature.processingWindowTransforms[i];

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

AL_EStartCodeBytesAlignedMode ConvertModuleToSoftStartCodeBytesAlignment(StartCodeBytesAlignmentType alignment)
{
  switch(alignment)
  {
  case StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_AUTO: return AL_START_CODE_AUTO;
  case StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_3_BYTES: return AL_START_CODE_3_BYTES;
  case StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_4_BYTES: return AL_START_CODE_4_BYTES;
  case StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM: return AL_START_CODE_MAX_ENUM;
  default: return AL_START_CODE_MAX_ENUM;
  }

  return AL_START_CODE_MAX_ENUM;
}

StartCodeBytesAlignmentType ConvertSoftToModuleStartCodeBytesAlignment(AL_EStartCodeBytesAlignedMode mode)
{
  switch(mode)
  {
  case AL_START_CODE_AUTO: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_AUTO;
  case AL_START_CODE_3_BYTES: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_3_BYTES;
  case AL_START_CODE_4_BYTES: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_4_BYTES;
  case AL_START_CODE_MAX_ENUM: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
  default: return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
  }

  return StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
}
