// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module/module_enums.h"
#include "module_structs.h"
#include <vector>

extern "C"
{
#include <lib_common_enc/Settings.h>
#include <lib_common/Allocator.h>
}

Clock CreateClock(AL_TEncSettings const& settings);
bool UpdateClock(AL_TEncSettings& settings, Clock clock);

Gop CreateGroupOfPictures(AL_TEncSettings const& settings);
bool UpdateGroupOfPictures(AL_TEncSettings& settings, Gop gop);

bool CreateConstrainedIntraPrediction(AL_TEncSettings const& settings);
bool UpdateConstrainedIntraPrediction(AL_TEncSettings& settings, bool isConstrainedIntraPredictionEnabled);

VideoModeType CreateVideoMode(AL_TEncSettings const& settings);
bool UpdateVideoMode(AL_TEncSettings& settings, VideoModeType videoMode);

Bitrate CreateBitrate(AL_TEncSettings const& settings);
bool UpdateBitrate(AL_TEncSettings& settings, Bitrate bitrate);

bool CreateCacheLevel2(AL_TEncSettings const& settings);
bool UpdateCacheLevel2(AL_TEncSettings& settings, bool isCacheLevel2Enabled);

bool CreateCacheLevel2ReducedRange(AL_TEncSettings const& settings);
bool UpdateCacheLevel2ReducedRange(AL_TEncSettings& settings, bool isCacheLevel2ReduceRangeEnabled);

BufferSizes CreateBufferSizes(AL_TEncSettings const& settings, Stride stride);

bool CreateFillerData(AL_TEncSettings const& settings);
bool UpdateFillerData(AL_TEncSettings& settings, bool isFillerDataEnabled);

AspectRatioType CreateAspectRatio(AL_TEncSettings const& settings);
bool UpdateAspectRatio(AL_TEncSettings& settings, AspectRatioType aspectRatio);

ScalingListType CreateScalingList(AL_TEncSettings const& settings);
bool UpdateScalingList(AL_TEncSettings& settings, ScalingListType scalingList);

QPs CreateQuantizationParameter(AL_TEncSettings const& settings);
bool UpdateQuantizationParameter(AL_TEncSettings& settings, QPs qps);

Slices CreateSlicesParameter(AL_TEncSettings const& settings);
bool UpdateSlicesParameter(AL_TEncSettings& settings, Slices slices);

Format CreateFormat(AL_TEncSettings const& settings);
bool UpdateFormat(AL_TEncSettings& settings, Format format, std::vector<ColorType> colors, std::vector<int> bitdepths, std::vector<StorageType> storages, Stride& stride, StrideAlignments strideAlignments);

bool UpdateIsEnabledSubframe(AL_TEncSettings& settings, bool isSubframeEnabled);

Resolution CreateResolution(AL_TEncSettings const& settings, Stride stride);
bool UpdateResolution(AL_TEncSettings& settings, Stride& stride, StrideAlignments strideAlignments, Resolution resolution);

ColorPrimariesType CreateColorPrimaries(AL_TEncSettings const& settings);
bool UpdateColorPrimaries(AL_TEncSettings& settings, ColorPrimariesType colorPrimaries);

LookAhead CreateLookAhead(AL_TEncSettings const& settings);
bool UpdateLookAhead(AL_TEncSettings& settings, LookAhead la);

TwoPass CreateTwoPass(AL_TEncSettings const& settings, std::string sTwoPassLogFile);
bool UpdateTwoPass(AL_TEncSettings& settings, std::string& sTwoPassLogFile, TwoPass tp);

ColorPrimariesType CreateColorPrimaries(AL_TEncSettings const& settings);
bool UpdateColorPrimaries(AL_TEncSettings& settings, ColorPrimariesType colorPrimaries);

TransferCharacteristicsType CreateTransferCharacteristics(AL_TEncSettings const& settings);
bool UpdateTransferCharacteristics(AL_TEncSettings& settings, TransferCharacteristicsType transferCharacteristics);

ColourMatrixType CreateColourMatrix(AL_TEncSettings const& settings);
bool UpdateColourMatrix(AL_TEncSettings& settings, ColourMatrixType colourMatrix);

MaxPicturesSizes CreateMaxPictureSizes(AL_TEncSettings const& settings);
bool UpdateMaxPictureSizes(AL_TEncSettings& settings, MaxPicturesSizes sizes);

int CreateLoopFilterBeta(AL_TEncSettings const& settings);
bool UpdateLoopFilterBeta(AL_TEncSettings& settings, int beta);

int CreateLoopFilterTc(AL_TEncSettings const& settings);
bool UpdateLoopFilterTc(AL_TEncSettings& settings, int tc);

bool CreateAccessUnitDelimiter(AL_TEncSettings const& settings);
bool UpdateAccessUnitDelimiter(AL_TEncSettings& settings, bool isAUDEnabled);

bool CreateBufferingPeriodSEI(AL_TEncSettings const& settings);
bool UpdateBufferingPeriodSEI(AL_TEncSettings& settings, bool isBPEnabled);

bool CreatePictureTimingSEI(AL_TEncSettings const& settings);
bool UpdatePictureTimingSEI(AL_TEncSettings& settings, bool isPTEnabled);

bool CreateRecoveryPointSEI(AL_TEncSettings const& settings);
bool UpdateRecoveryPointSEI(AL_TEncSettings& settings, bool isRPEnabled);

bool CreateMasteringDisplayColourVolumeSEI(AL_TEncSettings const& settings);
bool UpdateMasteringDisplayColourVolumeSEI(AL_TEncSettings& settings, bool isMDCVEnabled);

bool CreateContentLightLevelSEI(AL_TEncSettings const& settings);
bool UpdateContentLightLevelSEI(AL_TEncSettings& settings, bool isCLLEnabled);

bool CreateAlternativeTransferCharacteristicsSEI(AL_TEncSettings const& settings);
bool UpdateAlternativeTransferCharacteristicsSEI(AL_TEncSettings& settings, bool isATCEnabled);

bool CreateST209410SEI(AL_TEncSettings const& settings);
bool UpdateST209410SEI(AL_TEncSettings& settings, bool isST209410Enabled);

bool CreateST209440SEI(AL_TEncSettings const& settings);
bool UpdateST209440SEI(AL_TEncSettings& settings, bool isST209440Enabled);

bool CreateVideoFullRange(AL_TEncSettings const& settings);
bool UpdateVideoFullRange(AL_TEncSettings& settings, bool isVideoFullRangeEnabled);

bool CreateRealtime(AL_TEncSettings const& settings);
bool UpdateRealtime(AL_TEncSettings& settings, bool isRealtimeDisabled);

RateControlPlugin CreateRateControlPlugin(AL_TAllocator* allocator, AL_TEncSettings const& settings);
bool SetRcPluginContext(AL_TAllocator* allocator, AL_TEncSettings* settings, RateControlPlugin const& rcp);
void ResetRcPluginContext(AL_TAllocator* allocator, AL_TEncSettings* settings);

Region CreateOutputCrop(AL_TEncSettings const& settings);
bool UpdateOutputCrop(AL_TEncSettings& settings, Region region);

Region CreateInputCrop(AL_TEncSettings const& settings);
bool UpdateInputCrop(AL_TEncSettings& settings, Region region);

MaxPicturesSizes CreateMaxPictureSizesInBits(AL_TEncSettings const& settings);
bool UpdateMaxPictureSizesInBits(AL_TEncSettings& settings, MaxPicturesSizes sizes);

bool CreateUniformSliceType(AL_TEncSettings const& settings);
bool UpdateUniformeSliceType(AL_TEncSettings& settings, bool isUniformSliceTypeEnable);

MinMax<int> CreateLog2CodingUnit(AL_TEncSettings const& settings);
bool UpdateLog2CodingUnit(AL_TEncSettings& settings, MinMax<int> log2CodingUnit);

StartCodeBytesAlignmentType CreateStartCodeBytesAlignment(AL_TEncSettings const& settings);
bool UpdateStartCodeBytesAlignment(AL_TEncSettings& settings, StartCodeBytesAlignmentType startCodeBytesAlignment);
