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

#include "mediatype_enc_interface.h"

#include <vector>
#include <memory>

extern "C"
{
#include <lib_common/Allocator.h>
}

struct EncMediatypeAVC final : EncMediatypeInterface
{
  EncMediatypeAVC(BufferContiguities bufferContiguities, BufferBytesAlignments bufferBytesAlignments, StrideAlignments strideAlignments, bool isSeparateConfigurationFromDataEnabled, std::shared_ptr<AL_TAllocator> const& allocator);
  ~EncMediatypeAVC() override;

  ErrorType Get(std::string index, void* settings) const override;
  ErrorType Set(std::string index, void const* settings) override;
  void Reset() override;

  bool Check() override;

private:
  BufferContiguities bufferContiguities;
  BufferBytesAlignments bufferBytesAlignments;
  StrideAlignments strideAlignments;
  bool isSeparateConfigurationFromDataEnabled;
  BufferHandles bufferHandles;
  std::string sTwoPassLogFile;
  std::shared_ptr<AL_TAllocator> allocator;

  std::vector<AVCProfileType> const profiles
  {
    AVCProfileType::AVC_PROFILE_BASELINE,
    AVCProfileType::AVC_PROFILE_MAIN,
    AVCProfileType::AVC_PROFILE_HIGH,
    AVCProfileType::AVC_PROFILE_HIGH_10,
    AVCProfileType::AVC_PROFILE_HIGH_422,
    AVCProfileType::AVC_PROFILE_CONSTRAINED_BASELINE,
    AVCProfileType::AVC_PROFILE_PROGRESSIVE_HIGH,
    AVCProfileType::AVC_PROFILE_CONSTRAINED_HIGH,
    AVCProfileType::AVC_PROFILE_HIGH_10_INTRA,
    AVCProfileType::AVC_PROFILE_HIGH_422_INTRA,
    AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_CBG,
    AVCProfileType::XAVC_PROFILE_HIGH10_INTRA_VBR,
    AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_CBG,
    AVCProfileType::XAVC_PROFILE_HIGH_422_INTRA_VBR,
    AVCProfileType::XAVC_PROFILE_LONG_GOP_MAIN_MP4,
    AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MP4,
    AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_MXF,
    AVCProfileType::XAVC_PROFILE_LONG_GOP_HIGH_422_MXF,
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
  };

  std::map<Format, std::vector<Format>> supportedFormatsMap;
};
