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
  int width;
  int height;

  Stride stride;

  bool operator != (Resolution const& resolution) const
  {
    if(resolution.width != width)
      return true;

    if(resolution.height != height)
      return true;

    if(resolution.stride.horizontal != stride.horizontal)
      return true;

    if(resolution.stride.vertical != stride.vertical)
      return true;

    return false;
  }
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
  int width;
  int height;
};

struct RegionQuality
{
  Region region;
  QualityType quality;
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

struct HighDynamicRangeSeis
{
  bool hasMDCV;
  MasteringDisplayColourVolume masteringDisplayColourVolume;
  bool hasCLL;
  ContentLightLevel contentLightLevel;
};

