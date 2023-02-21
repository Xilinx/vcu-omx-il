// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "settings_checks.h"

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

  if(gop.rpFrequency < 0)
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

  for(int frame_type = 0; frame_type < QPs::MAX_FRAME_TYPE; frame_type++)
  {
    if(qps.range[frame_type].max < qps.range[frame_type].min)
      return false;

    if(qps.range[frame_type].min < 0)
      return false;
  }

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
