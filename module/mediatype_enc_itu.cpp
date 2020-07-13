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

#include "mediatype_enc_itu.h"
#include "mediatype_checks.h"
#include "convert_module_soft.h"
#include "convert_module_soft_enc.h"
#include <utility/round.h>

extern "C"
{
#include <lib_common_enc/EncBuffers.h>
#include <lib_common/StreamBuffer.h>
#include <lib_common/SEI.h>
#include <lib_fpga/DmaAllocLinux.h>
}

using namespace std;

Clock CreateClock(AL_TEncSettings settings)
{
  Clock clock;
  auto rateControl = settings.tChParam[0].tRCParam;
  clock.framerate = rateControl.uFrameRate;
  clock.clockratio = rateControl.uClkRatio;
  return clock;
}

bool UpdateClock(AL_TEncSettings& settings, Clock clock)
{
  if(!CheckClock(clock))
    return false;

  if(clock.framerate == 0)
    return false;

  auto& rateControl = settings.tChParam[0].tRCParam;

  rateControl.uFrameRate = clock.framerate;
  rateControl.uClkRatio = clock.clockratio;

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
  gop.ltFrequency = gopParam.uFreqLT;

  return gop;
}

static bool isGDREnabled(Gop gop)
{
  return gop.gdr == GdrType::GDR_VERTICAL || gop.gdr == GdrType::GDR_HORTIZONTAL;
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

  if(isGDREnabled(gop))
    settings.uEnableSEI |= AL_SEI_RP;
  else
    settings.uEnableSEI &= ~AL_SEI_RP;

  gopParam.bEnableLT = gop.isLongTermEnabled;
  gopParam.uFreqLT = gop.ltFrequency;

  return true;
}

bool CreateConstrainedIntraPrediction(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return channel.eEncTools & AL_OPT_CONST_INTRA_PRED;
}

bool UpdateConstrainedIntraPrediction(AL_TEncSettings& settings, bool isConstrainedIntraPredictionEnabled)
{
  if(!isConstrainedIntraPredictionEnabled)
    return true;

  auto& opt = settings.tChParam[0].eEncTools;
  opt = static_cast<AL_EChEncTool>(opt | AL_OPT_CONST_INTRA_PRED);

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
  Bitrate bitrate {};
  auto rateControl = settings.tChParam[0].tRCParam;

  bitrate.target = rateControl.uTargetBitRate / 1000;
  bitrate.max = rateControl.uMaxBitRate / 1000;
  bitrate.cpb = rateControl.uCPBSize / 90;
  bitrate.ird = rateControl.uInitialRemDelay / 90;
  bitrate.quality = (rateControl.uMaxPSNR / 100) - 28;
  bitrate.maxConsecutiveSkipFrame = rateControl.uMaxConsecSkip;
  bitrate.rateControl.mode = ConvertSoftToModuleRateControl(rateControl.eRCMode);
  bitrate.rateControl.options = ConvertSoftToModuleRateControlOption(rateControl.eOptions);
  return bitrate;
}

bool UpdateBitrate(AL_TEncSettings& settings, Bitrate bitrate)
{
  auto clock = CreateClock(settings);

  if(!CheckBitrate(bitrate, clock))
    return false;

  auto& rateControl = settings.tChParam[0].tRCParam;

  rateControl.uTargetBitRate = bitrate.target * 1000;
  rateControl.uMaxBitRate = bitrate.max * 1000;
  rateControl.uCPBSize = bitrate.cpb * 90;
  rateControl.uInitialRemDelay = bitrate.ird * 90;
  rateControl.uMaxPSNR = (bitrate.quality + 28) * 100;
  rateControl.uMaxConsecSkip = bitrate.maxConsecutiveSkipFrame;
  rateControl.eRCMode = ConvertModuleToSoftRateControl(bitrate.rateControl.mode);
  rateControl.eOptions = ConvertModuleToSoftRateControlOption(bitrate.rateControl.options);
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

static int RawAllocationSize(Stride stride, AL_EChromaMode eChromaMode)
{
  auto IP_WIDTH_ALIGNMENT = 32;
  auto IP_HEIGHT_ALIGNMENT = 8;
  assert(stride.horizontal % IP_WIDTH_ALIGNMENT == 0); // IP requirements
  assert(stride.vertical % IP_HEIGHT_ALIGNMENT == 0); // IP requirements
  auto size = stride.horizontal * stride.vertical;
  switch(eChromaMode)
  {
  case AL_CHROMA_MONO: return size;
  case AL_CHROMA_4_2_0: return (3 * size) / 2;
  case AL_CHROMA_4_2_2: return 2 * size;
  default: return -1;
  }
}

BufferSizes CreateBufferSizes(AL_TEncSettings settings, Stride stride)
{
  BufferSizes bufferSizes {};
  auto channel = settings.tChParam[0];
  bufferSizes.input = RawAllocationSize(stride, AL_GET_CHROMA_MODE(channel.ePicFormat));
  bufferSizes.output = AL_GetMitigatedMaxNalSize({ channel.uEncWidth, channel.uEncHeight }, AL_GET_CHROMA_MODE(channel.ePicFormat), AL_GET_BITDEPTH(channel.ePicFormat));

  if(channel.bSubframeLatency)
  {
    bufferSizes.output /= channel.uNumSlices;
    bufferSizes.output += 4095 * 2; /* we need space for the headers on each slice */
    bufferSizes.output = RoundUp(bufferSizes.output, 32); /* stream size is required to be 32 bits aligned */
  }
  return bufferSizes;
}

bool CreateFillerData(AL_TEncSettings settings)
{
  return settings.eEnableFillerData != AL_FILLER_DISABLE;
}

bool UpdateFillerData(AL_TEncSettings& settings, bool isFillerDataEnabled)
{
  settings.eEnableFillerData = isFillerDataEnabled ? AL_FILLER_APP : AL_FILLER_DISABLE;
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
  qps.mode.ctrl = ConvertSoftToModuleQPControl(settings.eQpCtrlMode);
  qps.mode.table = ConvertSoftToModuleQPTable(settings.eQpTableMode);
  auto rateControl = settings.tChParam[0].tRCParam;
  qps.initial = rateControl.iInitialQP;
  qps.deltaIP = rateControl.uIPDelta;
  qps.deltaPB = rateControl.uPBDelta;
  qps.min = rateControl.iMinQP;
  qps.max = rateControl.iMaxQP;
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

  settings.eQpCtrlMode = ConvertModuleToSoftQPControl(qps.mode.ctrl);
  settings.eQpTableMode = ConvertModuleToSoftQPTable(qps.mode.table);
  auto& rateControl = settings.tChParam[0].tRCParam;
  rateControl.iInitialQP = qps.initial;
  rateControl.uIPDelta = qps.deltaIP;
  rateControl.uPBDelta = qps.deltaPB;
  rateControl.iMinQP = qps.min;
  rateControl.iMaxQP = qps.max;

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

Format CreateFormat(AL_TEncSettings settings)
{
  Format format;
  auto channel = settings.tChParam[0];
  format.color = ConvertSoftToModuleColor(AL_GET_CHROMA_MODE(channel.ePicFormat));
  format.bitdepth = AL_GET_BITDEPTH(channel.ePicFormat);
  return format;
}

bool UpdateFormat(AL_TEncSettings& settings, Format format, vector<ColorType> colors, vector<int> bitdepths, Stride& stride, StrideAlignments strideAlignments)
{
  if(!CheckFormat(format, colors, bitdepths))
    return false;

  auto& channel = settings.tChParam[0];
  AL_SET_CHROMA_MODE(&channel.ePicFormat, ConvertModuleToSoftChroma(format.color));
  AL_SET_BITDEPTH(&channel.ePicFormat, format.bitdepth);
  channel.uSrcBitDepth = AL_GET_BITDEPTH(channel.ePicFormat);

  int minStride = static_cast<int>(RoundUp(AL_EncGetMinPitch(channel.uEncWidth, AL_GET_BITDEPTH(channel.ePicFormat), AL_FB_RASTER), strideAlignments.horizontal));
  stride.horizontal = max(minStride, stride.horizontal);

  return true;
}

Resolution CreateResolution(AL_TEncSettings settings, Stride stride)
{
  auto channel = settings.tChParam[0];
  Resolution resolution;
  resolution.dimension.horizontal = channel.uEncWidth;
  resolution.dimension.vertical = channel.uEncHeight;
  resolution.stride.horizontal = stride.horizontal;
  resolution.stride.vertical = stride.vertical;
  return resolution;
}

bool UpdateIsEnabledSubframe(AL_TEncSettings& settings, bool isSubframeEnabled)
{
  settings.tChParam[0].bSubframeLatency = isSubframeEnabled;
  return true;
}

bool UpdateResolution(AL_TEncSettings& settings, Stride& stride, StrideAlignments strideAlignments, Resolution resolution)
{
  auto& channel = settings.tChParam[0];
  channel.uEncWidth = resolution.dimension.horizontal;
  channel.uEncHeight = resolution.dimension.vertical;
  channel.uSrcWidth = channel.uEncWidth;
  channel.uSrcHeight = channel.uEncHeight;

  int minStride = RoundUp(AL_EncGetMinPitch(channel.uEncWidth, AL_GET_BITDEPTH(channel.ePicFormat), AL_FB_RASTER), strideAlignments.horizontal);
  stride.horizontal = max(minStride, static_cast<int>(RoundUp(resolution.stride.horizontal, strideAlignments.horizontal)));

  int minSliceHeight = RoundUp(static_cast<int>(channel.uEncHeight), strideAlignments.vertical);
  stride.vertical = max(minSliceHeight, static_cast<int>(RoundUp(resolution.stride.vertical, strideAlignments.vertical)));

  return true;
}

ColorPrimariesType CreateColorPrimaries(AL_TEncSettings settings)
{
  return ConvertSoftToModuleColorPrimaries(settings.eColourDescription);
}

bool UpdateColorPrimaries(AL_TEncSettings& settings, ColorPrimariesType colorPrimaries)
{
  if(!CheckColorPrimaries(colorPrimaries))
    return false;

  settings.eColourDescription = ConvertModuleToSoftColorPrimaries(colorPrimaries);
  return true;
}

TransferCharacteristicsType CreateTransferCharacteristics(AL_TEncSettings settings)
{
  return ConvertSoftToModuleTransferCharacteristics(settings.eTransferCharacteristics);
}

bool UpdateTransferCharacteristics(AL_TEncSettings& settings, TransferCharacteristicsType transferCharacteristics)
{
  if(!CheckTransferCharacteristics(transferCharacteristics))
    return false;

  settings.eTransferCharacteristics = ConvertModuleToSoftTransferCharacteristics(transferCharacteristics);
  return true;
}

ColourMatrixType CreateColourMatrix(AL_TEncSettings settings)
{
  return ConvertSoftToModuleColourMatrix(settings.eColourMatrixCoeffs);
}

bool UpdateColourMatrix(AL_TEncSettings& settings, ColourMatrixType colourMatrix)
{
  if(!CheckColourMatrix(colourMatrix))
    return false;

  settings.eColourMatrixCoeffs = ConvertModuleToSoftColourMatrix(colourMatrix);
  return true;
}

LookAhead CreateLookAhead(AL_TEncSettings settings)
{
  LookAhead la {};
  la.lookAhead = settings.LookAhead;
  la.isFirstPassSceneChangeDetectionEnabled = settings.bEnableFirstPassSceneChangeDetection;
  return la;
}

bool UpdateLookAhead(AL_TEncSettings& settings, LookAhead la)
{
  if(!CheckLookAhead(la))
    return false;

  settings.LookAhead = la.lookAhead;
  settings.bEnableFirstPassSceneChangeDetection = la.isFirstPassSceneChangeDetectionEnabled;

  return true;
}

TwoPass CreateTwoPass(AL_TEncSettings settings, string sTwoPassLogFile)
{
  TwoPass tp;
  tp.nPass = settings.TwoPass;
  tp.sLogFile = sTwoPassLogFile;
  return tp;
}

bool UpdateTwoPass(AL_TEncSettings& settings, string& sTwoPassLogFile, TwoPass tp)
{
  if(!CheckTwoPass(tp))
    return false;

  settings.TwoPass = tp.nPass;
  sTwoPassLogFile = tp.sLogFile;

  return true;
}

MaxPicturesSizes CreateMaxPictureSizes(AL_TEncSettings settings)
{
  auto rateControl = settings.tChParam[0].tRCParam;
  MaxPicturesSizes sizes;
  sizes.i = static_cast<int>(rateControl.pMaxPictureSize[AL_SLICE_I] / 1000);
  sizes.p = static_cast<int>(rateControl.pMaxPictureSize[AL_SLICE_P] / 1000);
  sizes.b = static_cast<int>(rateControl.pMaxPictureSize[AL_SLICE_B] / 1000);
  return sizes;
}

bool UpdateMaxPictureSizes(AL_TEncSettings& settings, MaxPicturesSizes sizes)
{
  if(!CheckMaxPictureSizes(sizes))
    return false;

  auto& rateControl = settings.tChParam[0].tRCParam;
  rateControl.pMaxPictureSize[AL_SLICE_I] = sizes.i * 1000;
  rateControl.pMaxPictureSize[AL_SLICE_P] = sizes.p * 1000;
  rateControl.pMaxPictureSize[AL_SLICE_B] = sizes.b * 1000;

  return true;
}

int CreateLoopFilterBeta(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return channel.iBetaOffset;
}

bool UpdateLoopFilterBeta(AL_TEncSettings& settings, int beta)
{
  if(!CheckLoopFilterBeta(beta))
    return false;

  auto& channel = settings.tChParam[0];
  channel.iBetaOffset = beta;

  return true;
}

int CreateLoopFilterTc(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  return channel.iTcOffset;
}

bool UpdateLoopFilterTc(AL_TEncSettings& settings, int tc)
{
  if(!CheckLoopFilterTc(tc))
    return false;

  auto& channel = settings.tChParam[0];
  channel.iTcOffset = tc;

  return true;
}

bool CreateAccessUnitDelimiter(AL_TEncSettings settings)
{
  return settings.bEnableAUD;
}

bool UpdateAccessUnitDelimiter(AL_TEncSettings& settings, bool isAUDEnabled)
{
  settings.bEnableAUD = isAUDEnabled;
  return true;
}

bool CreateBufferingPeriodSEI(AL_TEncSettings settings)
{
  return (settings.uEnableSEI & AL_SEI_BP) != 0;
}

bool UpdateBufferingPeriodSEI(AL_TEncSettings& settings, bool isBPEnabled)
{
  isBPEnabled ? settings.uEnableSEI |= AL_SEI_BP : settings.uEnableSEI &= ~AL_SEI_BP;
  return true;
}

bool CreatePictureTimingSEI(AL_TEncSettings settings)
{
  return (settings.uEnableSEI & AL_SEI_PT) != 0;
}

bool UpdatePictureTimingSEI(AL_TEncSettings& settings, bool isPTEnabled)
{
  isPTEnabled ? settings.uEnableSEI |= AL_SEI_PT : settings.uEnableSEI &= ~AL_SEI_PT;
  return true;
}

bool CreateRecoveryPointSEI(AL_TEncSettings settings)
{
  return (settings.uEnableSEI & AL_SEI_RP) != 0;
}

bool UpdateRecoveryPointSEI(AL_TEncSettings& settings, bool isRPEnabled)
{
  isRPEnabled ? settings.uEnableSEI |= AL_SEI_RP : settings.uEnableSEI &= ~AL_SEI_RP;
  return true;
}

bool CreateMasteringDisplayColourVolumeSEI(AL_TEncSettings settings)
{
  return (settings.uEnableSEI & AL_SEI_MDCV) != 0;
}

bool UpdateMasteringDisplayColourVolumeSEI(AL_TEncSettings& settings, bool isMDCVEnabled)
{
  isMDCVEnabled ? settings.uEnableSEI |= AL_SEI_MDCV : settings.uEnableSEI &= ~AL_SEI_MDCV;
  return true;
}

bool CreateContentLightLevelSEI(AL_TEncSettings settings)
{
  return (settings.uEnableSEI & AL_SEI_CLL) != 0;
}

bool UpdateContentLightLevelSEI(AL_TEncSettings& settings, bool isCLLEnabled)
{
  isCLLEnabled ? settings.uEnableSEI |= AL_SEI_CLL : settings.uEnableSEI &= ~AL_SEI_CLL;
  return true;
}

bool CreateST209410SEI(AL_TEncSettings settings)
{
  return (settings.uEnableSEI & AL_SEI_ST2094_10) != 0;
}

bool UpdateST209410SEI(AL_TEncSettings& settings, bool isST209410Enabled)
{
  isST209410Enabled ? settings.uEnableSEI |= AL_SEI_ST2094_10 : settings.uEnableSEI &= ~AL_SEI_ST2094_10;
  return true;
}

bool CreateST209440SEI(AL_TEncSettings settings)
{
  return (settings.uEnableSEI & AL_SEI_ST2094_40) != 0;
}

bool UpdateST209440SEI(AL_TEncSettings& settings, bool isST209440Enabled)
{
  isST209440Enabled ? settings.uEnableSEI |= AL_SEI_ST2094_40 : settings.uEnableSEI &= ~AL_SEI_ST2094_40;
  return true;
}

RateControlPlugin CreateRateControlPlugin(AL_TAllocator* allocator, AL_TEncSettings const& settings)
{
  RateControlPlugin rcp;
  AL_TLinuxDmaAllocator* linuxAllocator = (AL_TLinuxDmaAllocator*)allocator;
  rcp.dmaBuf = AL_LinuxDmaAllocator_GetFd(linuxAllocator, settings.hRcPluginDmaContext);
  rcp.dmaSize = settings.tChParam[0].zRcPluginDmaSize;
  return rcp;
}

bool SetRcPluginContext(AL_TAllocator* allocator, AL_TEncSettings* settings, RateControlPlugin const& rcp)
{
  AL_TLinuxDmaAllocator* linuxAllocator = (AL_TLinuxDmaAllocator*)allocator;
  AL_HANDLE hRcPluginDmaContext = NULL;

  if(rcp.dmaBuf != -1)
  {
    hRcPluginDmaContext = AL_LinuxDmaAllocator_ImportFromFd(linuxAllocator, rcp.dmaBuf);

    if(hRcPluginDmaContext == NULL)
      return false;
  }

  if(settings->hRcPluginDmaContext)
    AL_Allocator_Free(allocator, settings->hRcPluginDmaContext);

  settings->hRcPluginDmaContext = hRcPluginDmaContext;
  settings->tChParam[0].pRcPluginDmaContext = 0;
  settings->tChParam[0].zRcPluginDmaSize = rcp.dmaSize;

  return true;
}

void ResetRcPluginContext(AL_TAllocator* allocator, AL_TEncSettings* settings)
{
  RateControlPlugin rcp;
  rcp.dmaBuf = -1;
  rcp.dmaSize = 0;
  SetRcPluginContext(allocator, settings, rcp);
}

Region CreateCrop(AL_TEncSettings settings)
{
  auto channel = settings.tChParam[0];
  Region region;
  region.point.x = channel.uOutputCropPosX;
  region.point.y = channel.uOutputCropPosY;
  region.dimension.horizontal = channel.uOutputCropWidth;
  region.dimension.vertical = channel.uOutputCropHeight;
  return region;
}

bool UpdateCrop(AL_TEncSettings& settings, Region region)
{
  if(!CheckCrop(region))
    return false;

  auto& channel = settings.tChParam[0];
  channel.uOutputCropPosX = region.point.x;
  channel.uOutputCropPosY = region.point.y;
  channel.uOutputCropWidth = region.dimension.horizontal;
  channel.uOutputCropHeight = region.dimension.vertical;

  return true;
}
