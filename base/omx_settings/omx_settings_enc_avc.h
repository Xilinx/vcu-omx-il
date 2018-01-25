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

#include "omx_settings_structs.h"
#include "omx_settings_enc_interface.h"
#include <vector>

struct EncSettingsAVC : EncSettingsInterface
{
  ~EncSettingsAVC();
  EncSettingsAVC();

  ErrorSettingsType Reset();
  ErrorSettingsType Set(std::string index, void const* settings);
  ErrorSettingsType Get(std::string index, void* settings);

private:
  Alignments alignments;
  BufferHandles bufferHandles;
  BufferBytesAlignments bufferBytesAlignments;
  BufferContiguities bufferContiguities;

  std::vector<ColorType> const colors
  {
    COLOR_MONO,
    COLOR_420,
    COLOR_422,
  };

  std::vector<int> const bitdepths
  {
    8,
    10,
  };

  std::vector<AVCProfileType> const profiles
  {
    AVCProfileType::AVC_PROFILE_BASELINE,
    AVCProfileType::AVC_PROFILE_MAIN,
    AVCProfileType::AVC_PROFILE_HIGH,
    AVCProfileType::AVC_PROFILE_HIGH10,
    AVCProfileType::AVC_PROFILE_HIGH422,
  };

  std::vector<int> const levels
  {
    9,
    10,
    11,
    12,
    13,
    20,
    21,
    22,
    30,
    31,
    32,
    40,
    41,
    42,
    50,
    51,
    52,
    60,
    61,
    62,
  };
};

