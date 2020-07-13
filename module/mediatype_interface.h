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

#include <string>

static std::string const SETTINGS_INDEX_MIMES {
  "SETTINGS_INDEX_MIMES"
};
static std::string const SETTINGS_INDEX_CLOCK {
  "SETTINGS_INDEX_CLOCK"
};
static std::string const SETTINGS_INDEX_STRIDE_ALIGNMENTS {
  "SETTINGS_INDEX_STRIDE_ALIGNMENTS"
};
static std::string const SETTINGS_INDEX_GROUP_OF_PICTURES {
  "SETTINGS_INDEX_GROUP_OF_PICTURES"
};
static std::string const SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER {
  "SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER"
};
static std::string const SETTINGS_INDEX_LATENCY {
  "SETTINGS_INDEX_LATENCY"
};
static std::string const SETTINGS_INDEX_LOW_BANDWIDTH {
  "SETTINGS_INDEX_LOW_BANDWIDTH"
};
static std::string const SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION {
  "SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION"
};
static std::string const SETTINGS_INDEX_ENTROPY_CODING {
  "SETTINGS_INDEX_ENTROPY_CODING"
};
static std::string const SETTINGS_INDEX_VIDEO_MODE {
  "SETTINGS_INDEX_VIDEO_MODE"
};
static std::string const SETTINGS_INDEX_VIDEO_MODES_SUPPORTED {
  "SETTINGS_INDEX_VIDEO_MODES_SUPPORTED"
};
static std::string const SETTINGS_INDEX_SEQUENCE_PICTURE_MODE {
  "SETTINGS_INDEX_SEQUENCE_PICTURE_MODE"
};
static std::string const SETTINGS_INDEX_SEQUENCE_PICTURE_MODES_SUPPORTED {
  "SETTINGS_INDEX_SEQUENCE_PICTURE_MODES_SUPPORTED"
};
static std::string const SETTINGS_INDEX_BITRATE {
  "SETTINGS_INDEX_BITRATE"
};
static std::string const SETTINGS_INDEX_CACHE_LEVEL2 {
  "SETTINGS_INDEX_CACHE_LEVEL2"
};
static std::string const SETTINGS_INDEX_BUFFER_HANDLES {
  "SETTINGS_INDEX_BUFFER_HANDLES"
};
static std::string const SETTINGS_INDEX_BUFFER_COUNTS {
  "SETTINGS_INDEX_BUFFER_COUNTS"
};
static std::string const SETTINGS_INDEX_BUFFER_SIZES {
  "SETTINGS_INDEX_BUFFER_SIZES"
};
static std::string const SETTINGS_INDEX_BUFFER_BYTES_ALIGNMENTS {
  "SETTINGS_INDEX_BUFFER_BYTES_ALIGNMENTS"
};
static std::string const SETTINGS_INDEX_BUFFER_CONTIGUITIES {
  "SETTINGS_INDEX_BUFFER_CONTIGUITIES"
};
static std::string const SETTINGS_INDEX_FILLER_DATA {
  "SETTINGS_INDEX_FILLER_DATA"
};
static std::string const SETTINGS_INDEX_ASPECT_RATIO {
  "SETTINGS_INDEX_ASPECT_RATIO"
};
static std::string const SETTINGS_INDEX_SCALING_LIST {
  "SETTINGS_INDEX_SCALING_LIST"
};
static std::string const SETTINGS_INDEX_QUANTIZATION_PARAMETER {
  "SETTINGS_INDEX_QUANTIZATION_PARAMETER"
};
static std::string const SETTINGS_INDEX_LOOP_FILTER {
  "SETTINGS_INDEX_LOOP_FILTER"
};
static std::string const SETTINGS_INDEX_PROFILE_LEVEL {
  "SETTINGS_INDEX_PROFILE_LEVEL"
};
static std::string const SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED {
  "SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED"
};
static std::string const SETTINGS_INDEX_FORMAT {
  "SETTINGS_INDEX_FORMAT"
};
static std::string const SETTINGS_INDEX_FORMATS_SUPPORTED {
  "SETTINGS_INDEX_FORMATS_SUPPORTED"
};
static std::string const SETTINGS_INDEX_SLICE_PARAMETER {
  "SETTINGS_INDEX_SLICE_PARAMETER"
};
static std::string const SETTINGS_INDEX_SUBFRAME {
  "SETTINGS_INDEX_SUBFRAME"
};
static std::string const SETTINGS_INDEX_RESOLUTION {
  "SETTINGS_INDEX_RESOLUTION"
};
static std::string const SETTINGS_INDEX_DECODED_PICTURE_BUFFER {
  "SETTINGS_INDEX_DECODED_PICTURE_BUFFER"
};
static std::string const SETTINGS_INDEX_COLOR_PRIMARIES {
  "SETTINGS_INDEX_COLOR_PRIMARIES"
};
static std::string const SETTINGS_INDEX_TRANSFER_CHARACTERISTICS {
  "SETTINGS_INDEX_TRANSFER_CHARACTERISTICS"
};
static std::string const SETTINGS_INDEX_COLOUR_MATRIX {
  "SETTINGS_INDEX_COLOUR_MATRIX"
};
static std::string const SETTINGS_INDEX_INPUT_PARSED {
  "SETTINGS_INDEX_INPUT_PARSED"
};
static std::string const SETTINGS_INDEX_LOOKAHEAD {
  "SETTINGS_INDEX_LOOKAHEAD"
};
static std::string const SETTINGS_INDEX_TWOPASS {
  "SETTINGS_INDEX_TWOPASS"
};
static std::string const SETTINGS_INDEX_LLP2_EARLY_CB {
  "SETTINGS_INDEX_LLP2_EARLY_CB"
};
static std::string const SETTINGS_INDEX_SEPARATE_CONFIGURATION_FROM_DATA {
  "SETTINGS_INDEX_SEPARATE_CONFIGURATION_FROM_DATA"
};
static std::string const SETTINGS_INDEX_MAX_PICTURE_SIZES {
  "SETTINGS_INDEX_MAX_PICTURE_SIZES"
};
static std::string const SETTINGS_INDEX_LOOP_FILTER_BETA {
  "SETTINGS_INDEX_LOOP_FILTER_BETA"
};
static std::string const SETTINGS_INDEX_LOOP_FILTER_TC {
  "SETTINGS_INDEX_LOOP_FILTER_TC"
};
static std::string const SETTINGS_INDEX_ACCESS_UNIT_DELIMITER {
  "SETTINGS_INDEX_ACCESS_UNIT_DELIMITER"
};
static std::string const SETTINGS_INDEX_BUFFERING_PERIOD_SEI {
  "SETTINGS_INDEX_BUFFERING_PERIOD_SEI"
};
static std::string const SETTINGS_INDEX_PICTURE_TIMING_SEI {
  "SETTINGS_INDEX_PICTURE_TIMING_SEI"
};
static std::string const SETTINGS_INDEX_RECOVERY_POINT_SEI {
  "SETTINGS_INDEX_RECOVERY_POINT_SEI"
};
static std::string const SETTINGS_INDEX_MASTERING_DISPLAY_COLOUR_VOLUME_SEI {
  "SETTINGS_INDEX_MASTERING_DISPLAY_COLOUR_VOLUME_SEI"
};
static std::string const SETTINGS_INDEX_CONTENT_LIGHT_LEVEL_SEI {
  "SETTINGS_INDEX_CONTENT_LIGHT_LEVEL_SEI"
};
static std::string const SETTINGS_INDEX_ST2094_10_SEI {
  "SETTINGS_INDEX_ST2094_10_SEI"
};
static std::string const SETTINGS_INDEX_ST2094_40_SEI {
  "SETTINGS_INDEX_ST2094_40_SEI"
};
static std::string const SETTINGS_INDEX_RATE_CONTROL_PLUGIN {
  "SETTINGS_INDEX_RATE_CONTROL_PLUGIN"
};
static std::string const SETTINGS_INDEX_CROP {
  "SETTINGS_INDEX_CROP"
};

struct MediatypeInterface
{
  enum ErrorType
  {
    SUCCESS,
    BAD_INDEX,
    BAD_PARAMETER,
    NOT_IMPLEMENTED,
    UNDEFINED,
    MAX,
  };

  virtual ~MediatypeInterface() = 0;
  virtual ErrorType Get(std::string index, void* settings) const = 0;
  virtual ErrorType Set(std::string index, void const* settings) = 0;
  virtual void Reset() = 0;
  virtual bool Check() = 0;
};

#include <map>
static std::map<MediatypeInterface::ErrorType, std::string> ToStringErrorSettings
{
  {
    MediatypeInterface::SUCCESS, "SUCCESS"
  },
  {
    MediatypeInterface::BAD_INDEX, "BAD_INDEX"
  },
  {
    MediatypeInterface::BAD_PARAMETER, "BAD_PARAMETER"
  },
  {
    MediatypeInterface::NOT_IMPLEMENTED, "NOT_IMPLEMENTED"
  },
  {
    MediatypeInterface::UNDEFINED, "UNDEFINED"
  },
  {
    MediatypeInterface::MAX, "MAX"
  },
};
