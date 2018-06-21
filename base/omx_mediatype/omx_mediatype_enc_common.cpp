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

#include "omx_mediatype_enc_common.h"
#include "omx_mediatype_checks.h"
#include "base/omx_settings/omx_convert_module_soft.h"
#include "base/omx_settings/omx_convert_module_soft_enc.h"

Clock CreateClock(AL_TEncSettings settings)
{
  Clock clock;
  auto rateCtrl = settings.tChParam[0].tRCParam;
  clock.framerate = rateCtrl.uFrameRate;
  clock.clockratio = rateCtrl.uClkRatio;
  return clock;
}

bool UpdateClock(AL_TEncSettings& settings, Clock clock)
{
  if(!CheckClock(clock))
    return false;

  auto& rateCtrl = settings.tChParam[0].tRCParam;

  rateCtrl.uFrameRate = clock.framerate;
  rateCtrl.uClkRatio = clock.clockratio;

  return true;
}

Gop CreateGroupOfPictures(AL_TEncSettings settings)
{
  Gop gop;
  auto gopParam = settings.tChParam[0].tGopParam;

  gop.b = gopParam.uNumB;
  gop.length = gopParam.uGopLength;
  gop.idrFrequency = gopParam.uFreqIDR;
  gop.mode = ConvertSoftToModuleGopControl(gopParam.eMode);
  gop.gdr = ConvertSoftToModuleGdr(gopParam.eGdrMode);
  gop.isLongTermEnabled = gopParam.bEnableLT;

  return gop;
}

bool UpdateGroupOfPictures(AL_TEncSettings& settings, Gop gop)
{
  if(!CheckGroupOfPictures(gop))
    return false;

  auto& gopParam = settings.tChParam[0].tGopParam;
  gopParam.uNumB = gop.b;
  gopParam.uGopLength = gop.length;
  gopParam.uFreqIDR = gop.idrFrequency;
  gopParam.eMode = ConvertModuleToSoftGopControl(gop.mode);
  gopParam.eGdrMode = ConvertModuleToSoftGdr(gop.gdr);
  gopParam.bEnableLT = gop.isLongTermEnabled;

  return true;
}

bool CreateConstrainedIntraPrediction(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return channel.eOptions & AL_OPT_CONST_INTRA_PRED;
}

bool UpdateConstrainedIntraPrediction(AL_TEncSettings& settings, bool isConstrainedIntraPredictionEnabled)
{
  auto& opt = settings.tChParam[0].eOptions;

  if(isConstrainedIntraPredictionEnabled)
    opt = static_cast<AL_EChEncOption>(opt | AL_OPT_CONST_INTRA_PRED);

  return true;
}

VideoModeType CreateVideoMode(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return ConvertSoftToModuleVideoMode(channel.eVideoMode);
}

bool UpdateVideoMode(AL_TEncSettings& settings, VideoModeType videoMode)
{
  if(!CheckVideoMode(videoMode))
    return false;

  auto& channel = settings.tChParam[0];
  channel.eVideoMode = ConvertModuleToSoftVideoMode(videoMode);
  return true;
}

Bitrate CreateBitrate(AL_TEncSettings settings)
{
  Bitrate bitrate;
  auto rateCtrl = settings.tChParam[0].tRCParam;

  bitrate.target = rateCtrl.uTargetBitRate / 1000;
  bitrate.max = rateCtrl.uMaxBitRate / 1000;
  bitrate.cpb = rateCtrl.uCPBSize / 90;
  bitrate.ird = rateCtrl.uInitialRemDelay / 90;
  bitrate.mode = ConvertSoftToModuleRateControl(rateCtrl.eRCMode);
  bitrate.option = ConvertSoftToModuleRateControlOption(rateCtrl.eOptions);
  return bitrate;
}

bool UpdateBitrate(AL_TEncSettings& settings, Bitrate bitrate)
{
  auto clock = CreateClock(settings);

  if(!CheckBitrate(bitrate, clock))
    return false;

  auto& rateCtrl = settings.tChParam[0].tRCParam;

  rateCtrl.uTargetBitRate = bitrate.target * 1000;
  rateCtrl.uMaxBitRate = bitrate.max * 1000;
  rateCtrl.uCPBSize = bitrate.cpb * 90;
  rateCtrl.uInitialRemDelay = bitrate.ird * 90;
  rateCtrl.eRCMode = ConvertModuleToSoftRateControl(bitrate.mode);
  rateCtrl.eOptions = ConvertModuleToSoftRateControlOption(bitrate.option);
  return true;
}

bool CreateCacheLevel2(AL_TEncSettings settings)
{
  return settings.iPrefetchLevel2 != 0;
}

bool UpdateCacheLevel2(AL_TEncSettings& settings, bool isCacheLevel2Enabled)
{
  settings.iPrefetchLevel2 = isCacheLevel2Enabled ? !0 : 0;
  return true;
}

bool CreateFillerData(AL_TEncSettings settings)
{
  return settings.bEnableFillerData;
}

bool UpdateFillerData(AL_TEncSettings& settings, bool isFillerDataEnabled)
{
  settings.bEnableFillerData = isFillerDataEnabled;
  return true;
}

AspectRatioType CreateAspectRatio(AL_TEncSettings settings)
{
  return ConvertSoftToModuleAspectRatio(settings.eAspectRatio);
}

bool UpdateAspectRatio(AL_TEncSettings& settings, AspectRatioType aspectRatio)
{
  if(!CheckAspectRatio(aspectRatio))
    return false;

  settings.eAspectRatio = ConvertModuleToSoftAspectRatio(aspectRatio);
  return true;
}

ScalingListType CreateScalingList(AL_TEncSettings settings)
{
  return ConvertSoftToModuleScalingList(settings.eScalingList);
}

bool UpdateScalingList(AL_TEncSettings& settings, ScalingListType scalingList)
{
  if(!CheckScalingList(scalingList))
    return false;

  settings.eScalingList = ConvertModuleToSoftScalingList(scalingList);
  return true;
}

QPs CreateQuantizationParameter(AL_TEncSettings settings)
{
  QPs qps;
  qps.mode = ConvertSoftToModuleQPControl(settings.eQpCtrlMode);
  auto rateCtrl = settings.tChParam[0].tRCParam;
  qps.initial = rateCtrl.iInitialQP;
  qps.deltaIP = rateCtrl.uIPDelta;
  qps.deltaPB = rateCtrl.uPBDelta;
  qps.min = rateCtrl.iMinQP;
  qps.max = rateCtrl.iMaxQP;
  return qps;
}

bool UpdateQuantizationParameter(AL_TEncSettings& settings, QPs qps)
{
  if(qps.deltaIP < 0)
    qps.deltaIP = -1;

  if(qps.deltaPB < 0)
    qps.deltaPB = -1;

  if(!CheckQuantizationParameter(qps))
    return false;

  settings.eQpCtrlMode = ConvertModuleToSoftQPControl(qps.mode);
  auto& rateCtrl = settings.tChParam[0].tRCParam;
  rateCtrl.iInitialQP = qps.initial;
  rateCtrl.uIPDelta = qps.deltaIP;
  rateCtrl.uPBDelta = qps.deltaPB;
  rateCtrl.iMinQP = qps.min;
  rateCtrl.iMaxQP = qps.max;

  return true;
}

Slices CreateSlicesParameter(AL_TEncSettings settings)
{
  Slices slices;
  slices.dependent = settings.bDependentSlice;
  auto channel = settings.tChParam[0];
  slices.num = channel.uNumSlices;
  slices.size = channel.uSliceSize;
  return slices;
}

bool UpdateSlicesParameter(AL_TEncSettings& settings, Slices slices)
{
  if(!CheckSlicesParameter(slices))
    return false;

  settings.bDependentSlice = slices.dependent;
  auto& channel = settings.tChParam[0];
  channel.uNumSlices = slices.num;
  channel.uSliceSize = slices.size;

  return true;
}

