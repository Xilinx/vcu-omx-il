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

#include "mediatype_checks.h"

using namespace std;

bool CheckClock(Clock clock)
{
  if(clock.framerate < 0)
    return false;

  if(clock.clockratio <= 0)
    return false;

  return true;
}

bool CheckGroupOfPictures(Gop gop)
{
  if(gop.b < 0)
    return false;

  if(gop.length <= gop.b)
    return false;

  if(gop.idrFrequency < 0)
    return false;

  if(gop.mode == GopControlType::GOP_CONTROL_MAX_ENUM)
    return false;

  if(gop.gdr == GdrType::GDR_MAX_ENUM)
    return false;

  if(gop.ltFrequency < 0)
    return false;

  return true;
}

bool CheckInternalEntropyBuffer(int internalEntropyBuffer)
{
  if(internalEntropyBuffer < 1)
    return false;

  if(internalEntropyBuffer > 16)
    return false;

  return true;
}

bool CheckVideoMode(VideoModeType videoMode)
{
  return videoMode != VideoModeType::VIDEO_MODE_MAX_ENUM;
}

bool CheckSequenceMode(SequencePictureModeType sequenceMode, vector<SequencePictureModeType> sequenceModes)
{
  if(!IsSupported(sequenceMode, sequenceModes))
    return false;
  return true;
}

bool CheckBitrate(Bitrate bitrate, Clock clock)
{
  if(bitrate.target <= 0)
    return false;

  if(bitrate.max < bitrate.target)
    return false;

  if(bitrate.rateControl.mode == RateControlType::RATE_CONTROL_MAX_ENUM)
    return false;

  auto timeFrame = ((1 * 1000) / ((clock.framerate * 1000) / clock.clockratio));
  auto timeHalfFrame = timeFrame / 2;

  if(bitrate.cpb < timeHalfFrame)
    return false;

  if(bitrate.ird > bitrate.cpb)
    return false;

  if(bitrate.quality < 0)
    return false;

  if(bitrate.quality > 20)
    return false;

  return true;
}

bool CheckAspectRatio(AspectRatioType aspectRatio)
{
  return aspectRatio != AspectRatioType::ASPECT_RATIO_MAX_ENUM;
}

bool CheckScalingList(ScalingListType scalingList)
{
  return scalingList != ScalingListType::SCALING_LIST_MAX_ENUM;
}

bool CheckQuantizationParameter(QPs qps)
{
  if(qps.mode.ctrl == QPControlType::QP_CONTROL_MAX_ENUM)
    return false;

  if(qps.mode.table == QPTableType::QP_TABLE_MAX_ENUM)
    return false;

  if(qps.max < qps.min)
    return false;

  if(qps.min < 0)
    return false;

  if(qps.initial < 0)
    return false;

  if(qps.deltaIP < -1) // -1 means default
    return false;

  if(qps.deltaPB < -1) // -1 means default
    return false;

  return true;
}

bool CheckSlicesParameter(Slices slices)
{
  if(slices.num <= 0)
    return false;

  return true;
}

bool CheckFormat(Format format, vector<ColorType> colors, vector<int> bitdepths)
{
  if(!IsSupported(format.color, colors))
    return false;

  if(!IsSupported(format.bitdepth, bitdepths))
    return false;

  return true;
}

bool CheckBufferHandles(BufferHandles bufferHandles)
{
  if(bufferHandles.input == BufferHandleType::BUFFER_HANDLE_MAX_ENUM)
    return false;

  if(bufferHandles.output == BufferHandleType::BUFFER_HANDLE_MAX_ENUM)
    return false;

  return true;
}

bool CheckColorPrimaries(ColorPrimariesType colorPrimaries)
{
  if(colorPrimaries == ColorPrimariesType::COLOR_PRIMARIES_MAX_ENUM)
    return false;

  return true;
}

bool CheckTransferCharacteristics(TransferCharacteristicsType transferCharacteristics)
{
  return transferCharacteristics != TransferCharacteristicsType::TRANSFER_CHARACTERISTICS_MAX_ENUM;
}

bool CheckColourMatrix(ColourMatrixType colourMatrix)
{
  return colourMatrix != ColourMatrixType::COLOUR_MATRIX_MAX_ENUM;
}

bool CheckLookAhead(LookAhead la)
{
  if(la.lookAhead < 0)
    return false;

  return true;
}

bool CheckTwoPass(TwoPass tp)
{
  if(tp.nPass < 0 || tp.nPass > 2)
    return false;

  return true;
}

bool CheckMaxPictureSizes(MaxPicturesSizes sizes)
{
  if(sizes.i < 0 || sizes.p < 0 || sizes.b < 0)
    return false;

  return true;
}

bool CheckLoopFilterBeta(int beta)
{
  (void)beta;
  return true;
}

bool CheckLoopFilterTc(int tc)
{
  (void)tc;
  return true;
}

bool CheckCrop(Region region)
{
  return (region.point.x >= 0) && (region.point.y >= 0) && (region.dimension.horizontal >= 0) && (region.dimension.vertical >= 0);
}

bool CheckLog2CodingUnit(MinMax<int> log2CodingUnit)
{
  return log2CodingUnit.min >= 0 && log2CodingUnit.max >= 0;
}

bool CheckStartCodeBytesAlignment(StartCodeBytesAlignmentType startCodeBytesAlignment)
{
  return startCodeBytesAlignment != StartCodeBytesAlignmentType::START_CODE_BYTES_ALIGNMENT_MAX_ENUM;
}
