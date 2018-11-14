/******************************************************************************
*
* Copyright (C) 2018 Allegro DVT2.  All rights reserved.
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

#include "omx_module_enums.h"
#include <string>
#include <vector>

struct BufferHandles
{
  BufferHandleType input;
  BufferHandleType output;
};

struct BufferCounts
{
  int input;
  int output;
};

struct BufferSizes
{
  int input;
  int output;
};

struct BufferBytesAlignments
{
  int input;
  int output;
};

struct BufferContiguities
{
  bool input;
  bool output;
};

struct BufferDefinitions
{
  int bytesAlignment;
  bool contiguous;
};

struct Stride
{
  int widthStride;
  int heightStride;
};

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

    if(resolution.stride.widthStride != stride.widthStride)
      return true;

    if(resolution.stride.heightStride != stride.heightStride)
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

struct Mime
{
  std::string mime;
  CompressionType compression;
};

struct Mimes
{
  Mime input;
  Mime output;
};

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

struct SupportedFormats
{
  std::vector<Format> input;
  std::vector<Format> output;
};

struct ProfileLevelType
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

struct QPs
{
  int initial;
  int deltaIP;
  int deltaPB;
  int min;
  int max;
  QPControlType mode;
};

struct Bitrate
{
  int target; // In kbits
  int max; // In kbits
  int cpb; // CPB in milliseconds
  int ird; // InitialRemovalDelay in milliseconds
  RateControlType mode;
  RateControlOptionType option;
};

struct Slices
{
  int num;
  int size;
  bool dependent;
};

struct Region
{
  int x;
  int y;
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
  int nLookAhead;
};

struct TwoPass
{
  int nPass;
  std::string sLogFile;
};

