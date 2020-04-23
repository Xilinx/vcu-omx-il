#include "convert_module_soft_enc_roi.h"

AL_ERoiQuality ConvertModuleToSoftQualityByPreset(QualityType const& quality)
{
  switch(quality)
  {
  case QualityType::REGION_OF_INTEREST_QUALITY_HIGH: return AL_ROI_QUALITY_HIGH;
  case QualityType::REGION_OF_INTEREST_QUALITY_MEDIUM: return AL_ROI_QUALITY_MEDIUM;
  case QualityType::REGION_OF_INTEREST_QUALITY_LOW: return AL_ROI_QUALITY_LOW;
  case QualityType::REGION_OF_INTEREST_QUALITY_DONT_CARE: return AL_ROI_QUALITY_DONT_CARE;
  case QualityType::REGION_OF_INTEREST_QUALITY_INTRA: return AL_ROI_QUALITY_INTRA;
  case QualityType::REGION_OF_INTEREST_QUALITY_MAX_ENUM: return AL_ROI_QUALITY_MAX_ENUM;
  default: return AL_ROI_QUALITY_MAX_ENUM;
  }

  return AL_ROI_QUALITY_MAX_ENUM;
}
