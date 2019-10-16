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

#include <cassert>
#include <utility/round.h>
#include "mediatype_dec_itu.h"
#include "mediatype_checks.h"
#include "convert_module_soft.h"
#include "convert_module_soft_dec.h"

extern "C"
{
#include <lib_common/StreamBuffer.h>
}

using namespace std;

Clock CreateClock(AL_TDecSettings settings)
{
  Clock clock;

  clock.framerate = settings.uFrameRate / 1000;
  clock.clockratio = settings.uClkRatio;

  return clock;
}

bool UpdateClock(AL_TDecSettings& settings, Clock clock)
{
  if(!CheckClock(clock))
    return false;

  settings.uFrameRate = clock.framerate * 1000;
  settings.uClkRatio = clock.clockratio;
  settings.bForceFrameRate = settings.uFrameRate && settings.uClkRatio;

  return true;
}

int CreateInternalEntropyBuffer(AL_TDecSettings settings)
{
  return settings.iStackSize;
}

bool UpdateInternalEntropyBuffer(AL_TDecSettings& settings, int internalEntropyBuffer)
{
  if(!CheckInternalEntropyBuffer(internalEntropyBuffer))
    return false;

  settings.iStackSize = internalEntropyBuffer;

  return true;
}

SequencePictureModeType CreateSequenceMode(AL_TDecSettings settings)
{
  auto stream = settings.tStream;
  return ConvertSoftToModuleSequenceMode(stream.eSequenceMode);
}

bool UpdateSequenceMode(AL_TDecSettings& settings, SequencePictureModeType sequenceMode, vector<SequencePictureModeType> sequenceModes)
{
  if(!CheckSequenceMode(sequenceMode, sequenceModes))
    return false;

  auto& stream = settings.tStream;
  stream.eSequenceMode = ConvertModuleToSoftSequenceMode(sequenceMode);
  return true;
}

Format CreateFormat(AL_TDecSettings settings)
{
  Format format;
  auto stream = settings.tStream;

  format.color = ConvertSoftToModuleColor(stream.eChroma);
  format.bitdepth = stream.iBitDepth;

  return format;
}

bool UpdateFormat(AL_TDecSettings& settings, Format format, vector<ColorType> colors, vector<int> bitdepths, int& horizontalStride, StrideAlignments strideAlignments)
{
  if(!CheckFormat(format, colors, bitdepths))
    return false;

  auto& stream = settings.tStream;
  stream.eChroma = ConvertModuleToSoftChroma(format.color);
  stream.iBitDepth = format.bitdepth;

  int minHorizontalStride = RoundUp(static_cast<int>(AL_Decoder_GetMinPitch(stream.tDim.iWidth, stream.iBitDepth, settings.eFBStorageMode)), strideAlignments.horizontal);
  horizontalStride = max(minHorizontalStride, horizontalStride);

  return true;
}

Resolution CreateResolution(AL_TDecSettings settings, int horizontalStride, int verticalStride)
{
  auto streamSettings = settings.tStream;
  Resolution resolution;
  resolution.width = streamSettings.tDim.iWidth;
  resolution.height = streamSettings.tDim.iHeight;
  resolution.stride.horizontal = horizontalStride;
  resolution.stride.vertical = verticalStride;

  return resolution;
}

static int RawAllocationSize(int horizontalStride, int verticalStride, AL_EChromaMode eChromaMode)
{
  int constexpr IP_WIDTH_ALIGNMENT = 64;
  int constexpr IP_HEIGHT_ALIGNMENT = 64;
  assert(horizontalStride % IP_WIDTH_ALIGNMENT == 0); // IP requirements
  assert(verticalStride % IP_HEIGHT_ALIGNMENT == 0); // IP requirements
  auto size = horizontalStride * verticalStride;
  switch(eChromaMode)
  {
  case AL_CHROMA_MONO: return size;
  case AL_CHROMA_4_2_0: return (3 * size) / 2;
  case AL_CHROMA_4_2_2: return 2 * size;
  default: return -1;
  }
}

BufferSizes CreateBufferSizes(AL_TDecSettings settings, int horizontalStride, int verticalStride)
{
  BufferSizes bufferSizes {};
  auto streamSettings = settings.tStream;
  bufferSizes.input = AL_GetMaxNalSize(settings.eCodec, streamSettings.tDim, streamSettings.eChroma, streamSettings.iBitDepth, streamSettings.iLevel, streamSettings.iProfileIdc);
  bufferSizes.output = RawAllocationSize(horizontalStride, verticalStride, streamSettings.eChroma);
  return bufferSizes;
}

DecodedPictureBufferType CreateDecodedPictureBuffer(AL_TDecSettings settings)
{
  return ConvertSoftToModuleDecodedPictureBuffer(settings.eDpbMode);
}

bool UpdateIsEnabledSubframe(AL_TDecSettings& settings, bool isSubframeEnabled)
{
  settings.bLowLat = isSubframeEnabled;
  settings.eDecUnit = isSubframeEnabled ? ConvertModuleToSoftDecodeUnit(DecodeUnitType::DECODE_UNIT_SLICE) : ConvertModuleToSoftDecodeUnit(DecodeUnitType::DECODE_UNIT_FRAME);
  settings.eDpbMode = isSubframeEnabled ? ConvertModuleToSoftDecodedPictureBuffer(DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NO_REORDERING) : ConvertModuleToSoftDecodedPictureBuffer(DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NORMAL);
  return true;
}

bool UpdateDecodedPictureBuffer(AL_TDecSettings& settings, DecodedPictureBufferType decodedPictureBuffer)
{
  if(decodedPictureBuffer == DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM)
    return false;

  settings.eDpbMode = ConvertModuleToSoftDecodedPictureBuffer(decodedPictureBuffer);
  return true;
}

bool UpdateResolution(AL_TDecSettings& settings, int& horizontalStride, int& verticalStride, StrideAlignments strideAlignments, Resolution resolution)
{
  auto& streamSettings = settings.tStream;
  streamSettings.tDim = { resolution.width, resolution.height };

  int minHorizontalStride = RoundUp(static_cast<int>(AL_Decoder_GetMinPitch(streamSettings.tDim.iWidth, streamSettings.iBitDepth, settings.eFBStorageMode)), strideAlignments.horizontal);
  horizontalStride = max(minHorizontalStride, RoundUp(resolution.stride.horizontal, strideAlignments.horizontal));

  int minVerticalStride = RoundUp(static_cast<int>(AL_Decoder_GetMinStrideHeight(streamSettings.tDim.iHeight)), strideAlignments.vertical);
  verticalStride = max(minVerticalStride, RoundUp(resolution.stride.vertical, strideAlignments.vertical));

  return true;
}

