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

#include "omx_settings_dec_common.h"
#include "omx_convert_module_soft.h"
#include "omx_convert_module_soft_dec.h"
#include "omx_settings_utils.h"
#include "omx_settings_checks.h"
#include "base/omx_utils/round.h"
#include <cassert>
#include <cmath>

extern "C"
{
#include <lib_common/StreamBuffer.h>
}

static int RawAllocationSize(AL_TStreamSettings const& stream, Alignments const& alignments)
{
  auto IP_WIDTH_ALIGNMENT = 64;
  auto IP_HEIGHT_ALIGNMENT = 64;
  auto widthAlignment = alignments.stride;
  auto heightAlignment = alignments.sliceHeight;
  assert(widthAlignment % IP_WIDTH_ALIGNMENT == 0); // IP requirements
  assert(heightAlignment % IP_HEIGHT_ALIGNMENT == 0); // IP requirements

  auto adjustedWidthAlignment = widthAlignment > IP_WIDTH_ALIGNMENT ? widthAlignment : IP_WIDTH_ALIGNMENT;
  int const adjustedHeightAlignment = heightAlignment > IP_HEIGHT_ALIGNMENT ? heightAlignment : IP_HEIGHT_ALIGNMENT;

  auto width = stream.tDim.iWidth;
  auto height = stream.tDim.iHeight;
  auto bitdepthWidth = stream.iBitDepth == 8 ? width : (width + 2) / 3 * 4;
  auto adjustedWidth = RoundUp(bitdepthWidth, adjustedWidthAlignment);
  auto adjustedHeight = RoundUp(height, adjustedHeightAlignment);

  auto size = adjustedWidth * adjustedHeight;
  switch(stream.eChroma)
  {
  case CHROMA_MONO: return size;
  case CHROMA_4_2_0: return (3 * size) / 2;
  case CHROMA_4_2_2: return 2 * size;
  default: return -1;
  }
}

BufferSizes CreateBufferSizes(AL_TDecSettings const& settings, Alignments const& alignments)
{
  BufferSizes bufferSizes;
  auto stream = settings.tStream;
  bufferSizes.input = AL_GetMaxNalSize(stream.tDim, stream.eChroma, stream.iBitDepth);
  bufferSizes.output = RawAllocationSize(stream, alignments);
  return bufferSizes;
}

Resolution CreateResolution(AL_TDecSettings const& settings, Alignments const& alignments)
{
  Resolution resolution;
  auto stream = settings.tStream;
  resolution.width = stream.tDim.iWidth;
  resolution.height = stream.tDim.iHeight;
  resolution.stride = RoundUp(AL_Decoder_GetMinPitch(stream.tDim.iWidth, stream.iBitDepth, settings.eFBStorageMode), alignments.stride);
  resolution.sliceHeight = RoundUp(AL_Decoder_GetMinStrideHeight(stream.tDim.iHeight), alignments.sliceHeight);
  return resolution;
}

bool UpdateResolution(AL_TDecSettings& settings, Alignments& alignments, Resolution const& resolution)
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

  auto& streamSettings = settings.tStream;
  streamSettings.tDim = { resolution.width, resolution.height };

  return true;
}

Format CreateFormat(AL_TDecSettings const& settings)
{
  Format format;
  auto stream = settings.tStream;

  format.color = ConvertSoftToModuleColor(stream.eChroma);
  format.bitdepth = stream.iBitDepth;

  return format;
}

bool UpdateFormat(AL_TDecSettings& settings, Format const& format)
{
  if(!CheckFormat(format))
    return false;

  auto& streamSettings = settings.tStream;

  streamSettings.iBitDepth = format.bitdepth;
  streamSettings.eChroma = ConvertModuleToSoftChroma(format.color);
  return true;
}

BufferModeType CreateBufferMode(AL_TDecSettings const& settings)
{
  if(ConvertSoftToModuleDecodeUnit(settings.eDecUnit) == DecodeUnitType::DECODE_UNIT_SLICE)
    return BufferModeType::BUFFER_MODE_SLICE;

  if(ConvertSoftToModuleDecodedPictureBuffer(settings.eDpbMode) == DecodedPictureBufferType::DECODED_PICTURE_BUFFER_LOW_REFERENCE)
    return BufferModeType::BUFFER_MODE_FRAME_NO_REORDERING;

  return BufferModeType::BUFFER_MODE_FRAME;
}

