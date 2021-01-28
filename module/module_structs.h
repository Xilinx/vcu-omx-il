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

#pragma once

#include "module_enums.h"
#include <string>
#include <vector>

template<typename T>
struct InputOutput
{
  T input;
  T output;
};

typedef InputOutput<BufferHandleType> BufferHandles;
typedef InputOutput<int> BufferCounts;
typedef InputOutput<int> BufferSizes;
typedef InputOutput<int> BufferBytesAlignments;
typedef InputOutput<bool> BufferContiguities;

struct Mime
{
  std::string mime;
  CompressionType compression;
};

typedef InputOutput<Mime> Mimes;

struct Format
{
  ColorType color;
  int bitdepth;

  bool operator == (const Format& o) const
  {
    return color == o.color && bitdepth == o.bitdepth;
  }

  bool operator < (const Format& o) const
  {
    return color < o.color || (color == o.color && bitdepth < o.bitdepth);
  }
};

typedef InputOutput<std::vector<Format>> SupportedFormats;

template<typename T>
struct Dimension
{
  T horizontal;
  T vertical;
};

typedef Dimension<int> Stride;
typedef Dimension<int> StrideAlignments;

struct Resolution
{
  Dimension<int> dimension;
  Stride stride;
};

template<typename T>
struct MinMax
{
  T min;
  T max;
};

struct Clock
{
  int framerate;
  int clockratio;

  bool operator != (Clock const& clock) const
  {
    if(clock.framerate != framerate)
      return true;

    if(clock.clockratio != clockratio)
      return true;
    return false;
  }

  bool operator == (Clock const& clock) const
  {
    if(clock.framerate != framerate)
      return false;

    if(clock.clockratio != clockratio)
      return false;
    return true;
  }
};

struct ProfileLevel
{
  ProfileType profile;
  int level;
};

struct Gop
{
  int b;
  int length;
  int idrFrequency;
  GopControlType mode;
  GdrType gdr;
  bool isLongTermEnabled;
  int ltFrequency;
};

struct QPMode
{
  QPControlType ctrl;
  QPTableType table;
};

struct QPs
{
  int initial;
  int deltaIP;
  int deltaPB;
  int min;
  int max;
  QPMode mode;
};

struct RateControlOptions
{
  bool isSceneChangeResilienceEnabled;
  bool isDelayEnabled;
  bool isStaticSceneEnabled;
  bool isSkipEnabled;
};

struct MaxPicturesSizes
{
  int i;
  int p;
  int b;
};

struct RateControl
{
  RateControlType mode;
  RateControlOptions options;
  MaxPicturesSizes sizes;
};

struct Bitrate
{
  int target; // In kbits
  int max; // In kbits
  int cpb; // CPB in milliseconds
  int ird; // InitialRemovalDelay in milliseconds
  int quality;
  uint32_t maxConsecutiveSkipFrame;
  RateControl rateControl;
};

struct Slices
{
  int num;
  int size;
  bool dependent;
};

template<typename T>
struct Point
{
  T x;
  T y;
};

struct Region
{
  Point<int> point;
  Dimension<int> dimension;
};

struct RegionQuality
{
  Region region;
  union
  {
    QualityType byPreset;
    int byValue;
  } quality;
};

struct LookAhead
{
  int lookAhead;
  bool isFirstPassSceneChangeDetectionEnabled;
};

struct TwoPass
{
  int nPass;
  std::string sLogFile;
};

struct Sei
{
  int type;
  uint8_t* data;
  int payload;
};

struct DisplayPictureInfo
{
  int type;
  bool concealed;
};

struct Flags
{
  bool isConfig = false;
  bool isSync = false;
  bool isEndOfSlice = false;
  bool isEndOfFrame = false;
};

typedef Point<uint16_t> ChromaCoord;

struct MasteringDisplayColourVolume
{
  ChromaCoord displayPrimaries[3];
  ChromaCoord whitePoint;
  uint32_t maxDisplayMasteringLuminance;
  uint32_t minDisplayMasteringLuminance;
};

struct ContentLightLevel
{
  uint16_t maxContentLightLevel;
  uint16_t maxPicAverageLightLevel;
};

struct AlternativeTransferCharacteristics
{
  TransferCharacteristicsType preferredTransferCharacteristics;
};

/*  --------- ST2094_10 --------- */
#define MAX_MANUAL_ADJUSTMENT_ST2094_10 16

struct ProcessingWindow_ST2094_10
{
  uint16_t activeAreaLeftOffset;
  uint16_t activeAreaRightOffset;
  uint16_t activeAreaTopOffset;
  uint16_t activeAreaBottomOffset;
};

struct ImageCharacteristics_ST2094_10
{
  uint16_t minPQ;
  uint16_t maxPQ;
  uint16_t avgPQ;
};

struct ManualAdjustment_ST2094_10
{
  uint16_t targetMaxPQ;
  uint16_t trimSlope;
  uint16_t trimOffset;
  uint16_t trimPower;
  uint16_t trimChromaWeight;
  uint16_t trimSaturationGain;
  int16_t msWeight;
};

struct DynamicMeta_ST2094_10
{
  uint8_t applicationVersion;
  bool processingWindowFlag;
  ProcessingWindow_ST2094_10 processingWindow;
  ImageCharacteristics_ST2094_10 imageCharacteristics;
  uint8_t numManualAdjustments;
  ManualAdjustment_ST2094_10 manualAdjustments[MAX_MANUAL_ADJUSTMENT_ST2094_10];
};

/*  --------- ST2094_40 --------- */
#define MIN_WINDOW_ST2094_40 1
#define MAX_WINDOW_ST2094_40 3
#define MAX_MAXRGB_PERCENTILES_ST2094_40 15
#define MAX_BEZIER_CURVE_ANCHORS_ST2094_40 15
#define MAX_ROW_ACTUAL_PEAK_LUMINANCE_ST2094_40 25
#define MAX_COL_ACTUAL_PEAK_LUMINANCE_ST2094_40 25

struct ProcessingWindow_ST2094_1
{
  uint16_t upperLeftCornerX;
  uint16_t upperLeftCornerY;
  uint16_t lowerRightCornerX;
  uint16_t lowerRightCornerY;
};

struct ProcessingWindow_ST2094_40
{
  ProcessingWindow_ST2094_1 baseProcessingWindow;
  uint16_t centerOfEllipseX;
  uint16_t centerOfEllipseY;
  uint8_t rotationAngle;
  uint16_t semimajorAxisInternalEllipse;
  uint16_t semimajorAxisExternalEllipse;
  uint16_t semiminorAxisExternalEllipse;
  uint8_t overlapProcessOption;
};

struct DisplayPeakLuminance_ST2094_40
{
  bool actualPeakLuminanceFlag;
  uint8_t numRowsActualPeakLuminance;
  uint8_t numColsActualPeakLuminance;
  uint8_t actualPeakLuminance[MAX_ROW_ACTUAL_PEAK_LUMINANCE_ST2094_40][MAX_COL_ACTUAL_PEAK_LUMINANCE_ST2094_40];
};

struct TargetedSystemDisplay_ST2094_40
{
  uint32_t maximumLuminance;
  DisplayPeakLuminance_ST2094_40 peakLuminance;
};

struct ToneMapping_ST2094_40
{
  bool toneMappingFlag;
  uint16_t kneePointX;
  uint16_t kneePointY;
  uint8_t numBezierCurveAnchors;
  uint16_t bezierCurveAnchors[MAX_BEZIER_CURVE_ANCHORS_ST2094_40];
};

struct ProcessingWindowTransform_ST2094_40
{
  uint32_t maxscl[3];
  uint32_t averageMaxrgb;
  uint8_t numDistributionMaxrgbPercentiles;
  uint8_t distributionMaxrgbPercentages[MAX_MAXRGB_PERCENTILES_ST2094_40];
  uint32_t distributionMaxrgbPercentiles[MAX_MAXRGB_PERCENTILES_ST2094_40];
  uint8_t fractionBrightPixels;
  ToneMapping_ST2094_40 toneMapping;
  bool colorSaturationMappingFlag;
  uint8_t colorSaturationWeight;
};

struct DynamicMeta_ST2094_40
{
  uint8_t applicationVersion;
  uint8_t numWindows;
  ProcessingWindow_ST2094_40 processingWindows[MAX_WINDOW_ST2094_40 - 1];
  TargetedSystemDisplay_ST2094_40 targetedSystemDisplay;
  DisplayPeakLuminance_ST2094_40 masteringDisplayPeakLuminance;
  ProcessingWindowTransform_ST2094_40 processingWindowTransforms[MAX_WINDOW_ST2094_40];
};

struct HighDynamicRangeSeis
{
  bool hasMDCV;
  MasteringDisplayColourVolume masteringDisplayColourVolume;
  bool hasCLL;
  ContentLightLevel contentLightLevel;
  bool hasATC;
  AlternativeTransferCharacteristics alternativeTransferCharacteristics;
  bool hasST2094_10;
  DynamicMeta_ST2094_10 st2094_10;
  bool hasST2094_40;
  DynamicMeta_ST2094_40 st2094_40;
};

struct RateControlPlugin
{
  int dmaBuf;
  uint32_t dmaSize;
};
