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

  return modHDRSEIs;
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

  return alHDRSEIs;
}
