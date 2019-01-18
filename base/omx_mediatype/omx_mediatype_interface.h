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

#include <string>

#define SETTINGS_INDEX_MIMES "SETTINGS_INDEX_MIMES"
#define SETTINGS_INDEX_CLOCK "SETTINGS_INDEX_CLOCK"
#define SETTINGS_INDEX_STRIDE_ALIGNMENT "SETTINGS_INDEX_STRIDE_ALIGNMENT"
#define SETTINGS_INDEX_GROUP_OF_PICTURES "SETTINGS_INDEX_GROUP_OF_PICTURES"
#define SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER "SETTINGS_INDEX_INTERNAL_ENTROPY_BUFFER"
#define SETTINGS_INDEX_LATENCY "SETTINGS_INDEX_LATENCY"
#define SETTINGS_INDEX_LOW_BANDWIDTH "SETTINGS_INDEX_LOW_BANDWIDTH"
#define SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION "SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION"
#define SETTINGS_INDEX_ENTROPY_CODING "SETTINGS_INDEX_ENTROPY_CODING"
#define SETTINGS_INDEX_VIDEO_MODE "SETTINGS_INDEX_VIDEO_MODE"
#define SETTINGS_INDEX_VIDEO_MODES_SUPPORTED "SETTINGS_INDEX_VIDEO_MODES_SUPPORTED"
#define SETTINGS_INDEX_SEQUENCE_PICTURE_MODE "SETTINGS_INDEX_SEQUENCE_PICTURE_MODE"
#define SETTINGS_INDEX_SEQUENCE_PICTURE_MODES_SUPPORTED "SETTINGS_INDEX_SEQUENCE_PICTURE_MODES_SUPPORTED"
#define SETTINGS_INDEX_BITRATE "SETTINGS_INDEX_BITRATE"
#define SETTINGS_INDEX_CACHE_LEVEL2 "SETTINGS_INDEX_CACHE_LEVEL2"
#define SETTINGS_INDEX_BUFFER_HANDLES "SETTINGS_INDEX_BUFFER_HANDLES"
#define SETTINGS_INDEX_BUFFER_COUNTS "SETTINGS_INDEX_BUFFER_COUNTS"
#define SETTINGS_INDEX_BUFFER_SIZES "SETTINGS_INDEX_BUFFER_SIZES"
#define SETTINGS_INDEX_BUFFER_BYTES_ALIGNMENTS "SETTINGS_INDEX_BUFFER_BYTES_ALIGNMENTS"
#define SETTINGS_INDEX_BUFFER_CONTIGUITIES "SETTINGS_INDEX_BUFFER_CONTIGUITIES"
#define SETTINGS_INDEX_FILLER_DATA "SETTINGS_INDEX_FILLER_DATA"
#define SETTINGS_INDEX_ASPECT_RATIO "SETTINGS_INDEX_ASPECT_RATIO"
#define SETTINGS_INDEX_SCALING_LIST "SETTINGS_INDEX_SCALING_LIST"
#define SETTINGS_INDEX_QUANTIZATION_PARAMETER "SETTINGS_INDEX_QUANTIZATION_PARAMETER"
#define SETTINGS_INDEX_LOOP_FILTER "SETTINGS_INDEX_LOOP_FILTER"
#define SETTINGS_INDEX_PROFILE_LEVEL "SETTINGS_INDEX_PROFILE_LEVEL"
#define SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED "SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED"
#define SETTINGS_INDEX_FORMAT "SETTINGS_INDEX_FORMAT"
#define SETTINGS_INDEX_FORMATS_SUPPORTED "SETTINGS_INDEX_FORMATS_SUPPORTED"
#define SETTINGS_INDEX_SLICE_PARAMETER "SETTINGS_INDEX_SLICE_PARAMETER"
#define SETTINGS_INDEX_SUBFRAME "SETTINGS_INDEX_SUBFRAME"
#define SETTINGS_INDEX_RESOLUTION "SETTINGS_INDEX_RESOLUTION"
#define SETTINGS_INDEX_DECODED_PICTURE_BUFFER "SETTINGS_INDEX_DECODED_PICTURE_BUFFER"
#define SETTINGS_INDEX_LOOKAHEAD "SETTINGS_INDEX_LOOKAHEAD"
#define SETTINGS_INDEX_TWOPASS "SETTINGS_INDEX_TWOPASS"
#define SETTINGS_INDEX_LLP2_EARLY_CB "SETTINGS_INDEX_LLP2_EARLY_CB"

struct MediatypeInterface
{
  enum ErrorSettingsType
  {
    ERROR_SETTINGS_NONE,
    ERROR_SETTINGS_BAD_INDEX,
    ERROR_SETTINGS_BAD_PARAMETER,
    ERROR_SETTINGS_NOT_IMPLEMENTED,
    ERROR_SETTINGS_UNDEFINED,
    ERROR_SETTINGS_MAX,
  };

  virtual ~MediatypeInterface() = 0;
  virtual ErrorSettingsType Get(std::string index, void* settings) const = 0;
  virtual ErrorSettingsType Set(std::string index, void const* settings) = 0;
  virtual void Reset() = 0;

  virtual bool Check() = 0;
};

#include <map>
static std::map<MediatypeInterface::ErrorSettingsType, std::string> ToStringErrorSettings
{
  {
    MediatypeInterface::ERROR_SETTINGS_NONE, "ERROR_SETTINGS_NONE"
  },
  {
    MediatypeInterface::ERROR_SETTINGS_BAD_INDEX, "ERROR_SETTINGS_BAD_INDEX"
  },
  {
    MediatypeInterface::ERROR_SETTINGS_BAD_PARAMETER, "ERROR_SETTINGS_BAD_PARAMETER"
  },
  {
    MediatypeInterface::ERROR_SETTINGS_NOT_IMPLEMENTED, "ERROR_SETTINGS_NOT_IMPLEMENTED"
  },
  {
    MediatypeInterface::ERROR_SETTINGS_UNDEFINED, "ERROR_SETTINGS_UNDEFINED"
  },
  {
    MediatypeInterface::ERROR_SETTINGS_MAX, "ERROR_SETTINGS_MAX"
  },
};

