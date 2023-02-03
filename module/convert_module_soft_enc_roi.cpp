/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
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
