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

#include "omx_settings_enc_common.h"
#include "omx_convert_module_soft_enc.h"
#include "omx_convert_module_soft.h"
#include "omx_settings_utils.h"
#include "omx_settings_checks.h"
#include "base/omx_utils/roundup.h"

#include <cmath>

extern "C"
{
#include <lib_common_enc/EncBuffers.h>
#include <lib_common/StreamBuffer.h>
}

static int RawAllocationSize(AL_TEncChanParam const& channel, Alignments const& alignments)
{
  auto const IP_WIDTH_ALIGNMENT = 32;
  auto const IP_HEIGHT_ALIGNMENT = 8;
  auto const widthAlignment = alignments.stride;
  auto const heightAlignment = alignments.sliceHeight;
  assert(widthAlignment % IP_WIDTH_ALIGNMENT == 0); // IP requirements
  assert(heightAlignment % IP_HEIGHT_ALIGNMENT == 0); // IP requirements

  auto const adjustedWidthAlignment = widthAlignment > IP_WIDTH_ALIGNMENT ? widthAlignment : IP_WIDTH_ALIGNMENT;
  int const adjustedHeightAlignment = heightAlignment > IP_HEIGHT_ALIGNMENT ? heightAlignment : IP_HEIGHT_ALIGNMENT;

  auto const width = channel.uWidth;
  auto const height = channel.uHeight;
  auto const bitdepthWidth = AL_GET_BITDEPTH(channel.ePicFormat) == 8 ? width : (width + 2) / 3 * 4;
  auto const adjustedWidth = RoundUp(bitdepthWidth, adjustedWidthAlignment);
  auto const adjustedHeight = RoundUp(height, adjustedHeightAlignment);

  auto size = adjustedWidth * adjustedHeight;
  switch(AL_GET_CHROMA_MODE(channel.ePicFormat))
  {
  case CHROMA_MONO: return size;
  case CHROMA_4_2_0: return (3 * size) / 2;
  case CHROMA_4_2_2: return 2 * size;
  default: return -1;
  }
}

BufferSizes CreateBufferSizes(AL_TEncSettings const& settings, Alignments const& alignments)
{
  BufferSizes bufferSizes;
  auto const channel = settings.tChParam[0];
  bufferSizes.input = RawAllocationSize(channel, alignments);
  bufferSizes.output = AL_GetMaxNalSize({ channel.uWidth, channel.uHeight }, AL_GET_CHROMA_MODE(channel.ePicFormat), AL_GET_BITDEPTH(channel.ePicFormat));

  if(channel.bSubframeLatency)
  {
    auto const numSlices = channel.uNumSlices;
    auto& size = bufferSizes.output;
    size /= numSlices;
    size += 4095 * 2; /* we need space for the headers on each slice */
    size = RoundUp(size, 32);
  }

  return bufferSizes;
}

Resolution CreateResolution(AL_TEncSettings const& settings, Alignments const& alignments)
{
  Resolution resolution;
  auto const channel = settings.tChParam[0];
  resolution.width = channel.uWidth;
  resolution.height = channel.uHeight;
  resolution.stride = RoundUp(AL_EncGetMinPitch(channel.uWidth, AL_GET_BITDEPTH(channel.ePicFormat), AL_FB_RASTER), alignments.stride);
  resolution.sliceHeight = RoundUp(channel.uHeight, alignments.sliceHeight);

  return resolution;
}

bool UpdateResolution(AL_TEncSettings& settings, Alignments& alignments, Resolution const& resolution)
{
  if(!CheckResolution(resolution))
    return false;

  if(resolution.stride != 0)
  {
    int const align = GetPow2MaxAlignment(8, resolution.stride);

    if(align == 0)
      return false;

    if(align > alignments.stride)
      alignments.stride = align;
  }

  if(resolution.sliceHeight != 0)
  {
    int const align = GetPow2MaxAlignment(8, resolution.sliceHeight);

    if(align == 0)
      return false;

    if(align > alignments.sliceHeight)
      alignments.sliceHeight = align;
  }

  auto& channel = settings.tChParam[0];
  channel.uWidth = resolution.width;
  channel.uHeight = resolution.height;

  return true;
}

Format CreateFormat(AL_TEncSettings const& settings)
{
  Format format;
  auto const channel = settings.tChParam[0];

  format.color = ConvertSoftToModuleColor(AL_GET_CHROMA_MODE(channel.ePicFormat));
  format.bitdepth = AL_GET_BITDEPTH(channel.ePicFormat);

  return format;
}

bool UpdateFormat(AL_TEncSettings& settings, Format const& format)
{
  if(!CheckFormat(format))
    return false;

  auto& channel = settings.tChParam[0];
  AL_SET_BITDEPTH(channel.ePicFormat, format.bitdepth);
  AL_SET_CHROMA_MODE(channel.ePicFormat, ConvertModuleToSoftChroma(format.color));

  return true;
}

static inline bool isSeiEndOfFrameEnabled(AL_TEncSettings const& settings)
{
  return settings.uEnableSEI & SEI_EOF;
}

BufferModeType CreateBufferMode(AL_TEncSettings const& settings)
{
  auto const channel = settings.tChParam[0];

  if(channel.bSubframeLatency)
    return BufferModeType::BUFFER_MODE_SLICE;

  auto const gop = channel.tGopParam;

  if(gop.uNumB == 0 && isSeiEndOfFrameEnabled(settings))
    return BufferModeType::BUFFER_MODE_FRAME_NO_REORDERING;

  return BufferModeType::BUFFER_MODE_FRAME;
}

