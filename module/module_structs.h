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

struct HighDynamicRangeSeis
{
  bool hasMDCV;
  MasteringDisplayColourVolume masteringDisplayColourVolume;
  bool hasCLL;
  ContentLightLevel contentLightLevel;
};
