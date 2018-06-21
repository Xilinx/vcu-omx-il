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

#include <vector>

struct EncMediatypeHEVC : EncMediatypeInterface
{
  EncMediatypeHEVC();
  ~EncMediatypeHEVC();

  void Reset() override;
  ErrorSettingsType Get(std::string index, void* settings) const override;
  ErrorSettingsType Set(std::string index, void const* settings) override;

  ProfileLevelType ProfileLevel() const;
  bool SetProfileLevel(ProfileLevelType profileLevel);

private:
  std::vector<HEVCProfileType> const profiles
  {
    HEVCProfileType::HEVC_PROFILE_MAIN,
    HEVCProfileType::HEVC_PROFILE_MAIN_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_STILL,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_422,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10,
    HEVCProfileType::HEVC_PROFILE_MAIN_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA,
    HEVCProfileType::HEVC_PROFILE_MAIN_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_STILL_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MONOCHROME_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_10_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_INTRA_HIGH_TIER,
    HEVCProfileType::HEVC_PROFILE_MAIN_422_10_INTRA_HIGH_TIER,
  };

  std::vector<int> const levels
  {
    10,
    20,
    21,
    30,
    31,
    40,
    41,
    50,
    51,
    52,
    60,
    61,
    62,
  };

  std::vector<ColorType> const colors
  {
    ColorType::COLOR_400,
    ColorType::COLOR_420,
    ColorType::COLOR_422,
  };

  std::vector<int> const bitdepths
  {
    8,
    10,
  };

  std::vector<VideoModeType> const videoModes
  {
    VideoModeType::VIDEO_MODE_PROGRESSIVE,
    VideoModeType::VIDEO_MODE_ALTERNATE_TOP_BOTTOM_FIELD,
    VideoModeType::VIDEO_MODE_ALTERNATE_BOTTOM_TOP_FIELD,
  };
};

