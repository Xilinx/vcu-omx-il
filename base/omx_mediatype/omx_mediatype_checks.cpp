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

#include "omx_mediatype_checks.h"

bool CheckClock(Clock const& clock)
{
  if(clock.framerate <= 0)
    return false;

  if(clock.clockratio <= 0)
    return false;

  return true;
}

bool CheckGroupOfPictures(Gop const& gop)
{
  if(gop.b < 0)
    return false;

  if(gop.length <= gop.b)
    return false;

  if(gop.mode == GopControlType::GOP_CONTROL_MAX_ENUM)
    return false;

  if(gop.gdr == GdrType::GDR_MAX_ENUM)
    return false;

  return true;
}

bool CheckInternalEntropyBuffer(int const& internalEntropyBuffer)
{
  if(internalEntropyBuffer < 1)
    return false;

  if(internalEntropyBuffer > 16)
    return false;

  return true;
}

bool CheckVideoMode(VideoModeType const& videoMode)
{
  return videoMode != VideoModeType::VIDEO_MODE_MAX_ENUM;
}

bool CheckBitrate(Bitrate const& bitrate, Clock const& clock)
{
  if(bitrate.target <= 0)
    return false;

  if(bitrate.max < bitrate.target)
    return false;

  if(bitrate.mode == RateControlType::RATE_CONTROL_MAX_ENUM)
    return false;

  if(bitrate.option == RateControlOptionType::RATE_CONTROL_OPTION_MAX_ENUM)
    return false;

  auto timeFrame = ((1 * 1000) / ((clock.framerate * 1000) / clock.clockratio));
  auto timeHalfFrame = timeFrame / 2;

  if(bitrate.cpb < timeHalfFrame)
    return false;

  if(bitrate.ird > bitrate.cpb)
    return false;

  return true;
}

bool CheckAspectRatio(AspectRatioType const& aspectRatio)
{
  return aspectRatio != AspectRatioType::ASPECT_RATIO_MAX_ENUM;
}

bool CheckScalingList(ScalingListType const& scalingList)
{
  return scalingList != ScalingListType::SCALING_LIST_MAX_ENUM;
}

bool CheckQuantizationParameter(QPs const& qps)
{
  if(qps.mode == QPControlType::QP_MAX_ENUM)
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

bool CheckSlicesParameter(Slices const& slices)
{
  if(slices.num <= 0)
    return false;

  return true;
}

