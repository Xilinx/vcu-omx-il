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

int CreateMillisecondsLatency(AL_TEncSettings const& settings, BufferCounts const& bufferCounts)
{
  auto const channel = settings.tChParam;
  auto const rateCtrl = channel.tRCParam;
  auto const outputBufferCount = bufferCounts.output;
  auto const realFramerate = (static_cast<double>(rateCtrl.uFrameRate * rateCtrl.uClkRatio) / 1000.0);
  auto timeInMilliseconds = (static_cast<double>(outputBufferCount * 1000.0) / realFramerate);

  if(channel.bSubframeLatency)
  {
    timeInMilliseconds /= channel.uNumSlices;
    timeInMilliseconds += 1.0; // overhead
  }

  return ceil(timeInMilliseconds);
}

BufferSizes CreateBufferSizes(AL_TEncSettings const& settings)
{
  BufferSizes bufferSizes;
  auto const channel = settings.tChParam;
  bufferSizes.input = GetAllocSize_Src({ channel.uWidth, channel.uHeight }, AL_GET_BITDEPTH(channel.ePicFormat), AL_GET_CHROMA_MODE(channel.ePicFormat), channel.eSrcMode);
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

Gop CreateGroupOfPictures(AL_TEncSettings const& settings)
{
  Gop gop;
  auto const gopParam = settings.tChParam.tGopParam;

  gop.b = gopParam.uNumB;
  gop.length = gopParam.uGopLength;
  gop.idrFrequency = gopParam.uFreqIDR;
  gop.mode = ConvertSoftToModuleGopControl(gopParam.eMode);
  gop.gdr = ConvertSoftToModuleGdr(gopParam.eGdrMode);

  return gop;
}

bool UpdateGroupOfPictures(AL_TEncSettings& settings, Gop const& gop)
{
  if(!CheckGroupOfPictures(gop))
    return false;

  auto& gopParam = settings.tChParam.tGopParam;
  gopParam.uNumB = gop.b;
  gopParam.uGopLength = gop.length;
  gopParam.uFreqIDR = gop.idrFrequency;
  gopParam.eMode = ConvertModuleToSoftGopControl(gop.mode);
  gopParam.eGdrMode = ConvertModuleToSoftGdr(gop.gdr);

  return true;
}

Resolution CreateResolution(AL_TEncSettings const& settings, Alignments const& alignments)
{
  Resolution resolution;
  auto const channel = settings.tChParam;
  resolution.width = channel.uWidth;
  resolution.height = channel.uHeight;
  resolution.stride = RoundUp(AL_CalculatePitchValue(channel.uWidth, AL_GET_BITDEPTH(channel.ePicFormat), AL_FB_RASTER), alignments.stride);
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

  auto& channel = settings.tChParam;
  channel.uWidth = resolution.width;
  channel.uHeight = resolution.height;

  return true;
}

Clock CreateClock(AL_TEncSettings const& settings)
{
  Clock clock;
  auto const rateCtrl = settings.tChParam.tRCParam;
  clock.framerate = rateCtrl.uFrameRate;
  clock.clockratio = rateCtrl.uClkRatio;
  return clock;
}

bool UpdateClock(AL_TEncSettings& settings, Clock const& clock)
{
  if(!CheckClock(clock))
    return false;

  auto& rateCtrl = settings.tChParam.tRCParam;

  rateCtrl.uFrameRate = clock.framerate;
  rateCtrl.uClkRatio = clock.clockratio;

  return true;
}

Format CreateFormat(AL_TEncSettings const& settings)
{
  Format format;
  auto const channel = settings.tChParam;

  format.color = ConvertSoftToModuleColor(AL_GET_CHROMA_MODE(channel.ePicFormat));
  format.bitdepth = AL_GET_BITDEPTH(channel.ePicFormat);

  return format;
}

bool UpdateFormat(AL_TEncSettings& settings, Format const& format)
{
  if(!CheckFormat(format))
    return false;

  auto& channel = settings.tChParam;
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
  auto const channel = settings.tChParam;

  if(channel.bSubframeLatency)
    return BUFFER_MODE_SLICE;

  auto const gop = channel.tGopParam;

  if(gop.uNumB == 0 && isSeiEndOfFrameEnabled(settings))
    return BUFFER_MODE_FRAME_NO_REORDERING;

  return BUFFER_MODE_FRAME;
}

