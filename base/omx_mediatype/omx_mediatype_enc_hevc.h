/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
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

#include "omx_mediatype_enc_interface.h"

struct EncMediatypeHEVC : EncMediatypeInterface
{
  EncMediatypeHEVC();
  ~EncMediatypeHEVC();

  void Reset();
  CompressionType Compression() const;
  std::string Mime() const;

  std::vector<ProfileLevelType> ProfileLevelSupported() const;
  ProfileLevelType ProfileLevel() const;
  bool SetProfileLevel(ProfileLevelType const& profileLevel);

  EntropyCodingType EntropyCoding() const;
  bool SetEntropyCoding(EntropyCodingType const& entropyCoding);

  bool IsConstrainedIntraPrediction() const;
  bool SetConstrainedIntraPrediction(bool const& constrainedIntraPrediction);

  bool IsEnableLowBandwidth() const;
  bool SetEnableLowBandwidth(bool const& enableLowBandwidth);

  LoopFilterType LoopFilter() const;
  bool SetLoopFilter(LoopFilterType const& loopFilter);

  int strideAlignment;
  int sliceHeightAlignment;

private:
  std::vector<HEVCProfileType> const profiles
  {
    HEVCProfileType::HEVC_PROFILE_MAIN,
    HEVCProfileType::HEVC_PROFILE_MAIN10,
    HEVCProfileType::HEVC_PROFILE_MAIN422,
    HEVCProfileType::HEVC_PROFILE_MAIN422_10,
    HEVCProfileType::HEVC_PROFILE_MAINSTILL,
    HEVCProfileType::HEVC_PROFILE_MAIN_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN422_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN422_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAINSTILL_HIGH_TIER,
  };
  std::vector<int> const levels
  {
    10,
    20,
    21,
    30,
    31,
    32,
    40,
    41,
    50,
    51,
    52,
    60,
    61,
    62,
  };
  bool IsInProfilesSupported(HEVCProfileType const& profile);
  bool IsInLevelsSupported(int const& level);
};

