#pragma once

#include "module_structs.h"
#include <vector>

extern "C"
{
#include <lib_common_enc/Settings.h>
}

Clock CreateClock(AL_TEncSettings settings);
bool UpdateClock(AL_TEncSettings& settings, Clock clock);

Gop CreateGroupOfPictures(AL_TEncSettings settings);
bool UpdateGroupOfPictures(AL_TEncSettings& settings, Gop gop);

bool CreateConstrainedIntraPrediction(AL_TEncSettings settings);
bool UpdateConstrainedIntraPrediction(AL_TEncSettings& settings, bool isConstrainedIntraPredictionEnabled);

VideoModeType CreateVideoMode(AL_TEncSettings settings);
bool UpdateVideoMode(AL_TEncSettings& settings, VideoModeType videoMode);

Bitrate CreateBitrate(AL_TEncSettings settings);
bool UpdateBitrate(AL_TEncSettings& settings, Bitrate bitrate);

bool CreateCacheLevel2(AL_TEncSettings settings);
bool UpdateCacheLevel2(AL_TEncSettings& settings, bool isCacheLevel2Enabled);

BufferSizes CreateBufferSizes(AL_TEncSettings settings, Stride stride);

bool CreateFillerData(AL_TEncSettings settings);
bool UpdateFillerData(AL_TEncSettings& settings, bool isFillerDataEnabled);

AspectRatioType CreateAspectRatio(AL_TEncSettings settings);
bool UpdateAspectRatio(AL_TEncSettings& settings, AspectRatioType aspectRatio);

ScalingListType CreateScalingList(AL_TEncSettings settings);
bool UpdateScalingList(AL_TEncSettings& settings, ScalingListType scalingList);

QPs CreateQuantizationParameter(AL_TEncSettings settings);
bool UpdateQuantizationParameter(AL_TEncSettings& settings, QPs qps);

Slices CreateSlicesParameter(AL_TEncSettings settings);
bool UpdateSlicesParameter(AL_TEncSettings& settings, Slices slices);

Format CreateFormat(AL_TEncSettings settings);
bool UpdateFormat(AL_TEncSettings& settings, Format format, std::vector<ColorType> colors, std::vector<int> bitdepths, Stride& stride, StrideAlignments strideAlignments);

bool UpdateIsEnabledSubframe(AL_TEncSettings& settings, bool isSubframeEnabled);

Resolution CreateResolution(AL_TEncSettings settings, Stride stride);
bool UpdateResolution(AL_TEncSettings& settings, Stride& stride, StrideAlignments strideAlignments, Resolution resolution);

ColorPrimariesType CreateColorPrimaries(AL_TEncSettings settings);
bool UpdateColorPrimaries(AL_TEncSettings& settings, ColorPrimariesType colorPrimaries);

LookAhead CreateLookAhead(AL_TEncSettings settings);
bool UpdateLookAhead(AL_TEncSettings& settings, LookAhead la);

TwoPass CreateTwoPass(AL_TEncSettings settings, std::string sTwoPassLogFile);
bool UpdateTwoPass(AL_TEncSettings& settings, std::string& sTwoPassLogFile, TwoPass tp);

ColorPrimariesType CreateColorPrimaries(AL_TEncSettings settings);
bool UpdateColorPrimaries(AL_TEncSettings& settings, ColorPrimariesType colorPrimaries);

TransferCharacteristicsType CreateTransferCharacteristics(AL_TEncSettings settings);
bool UpdateTransferCharacteristics(AL_TEncSettings& settings, TransferCharacteristicsType transferCharacteristics);

ColourMatrixType CreateColourMatrix(AL_TEncSettings settings);
bool UpdateColourMatrix(AL_TEncSettings& settings, ColourMatrixType colourMatrix);

MaxPicturesSizes CreateMaxPictureSizes(AL_TEncSettings settings);
bool UpdateMaxPictureSizes(AL_TEncSettings& settings, MaxPicturesSizes sizes);

int CreateLoopFilterBeta(AL_TEncSettings settings);
bool UpdateLoopFilterBeta(AL_TEncSettings& settings, int beta);

int CreateLoopFilterTc(AL_TEncSettings settings);
bool UpdateLoopFilterTc(AL_TEncSettings& settings, int tc);

bool CreateAccessUnitDelimiter(AL_TEncSettings settings);
bool UpdateAccessUnitDelimiter(AL_TEncSettings& settings, bool isAUDEnabled);

bool CreateBufferingPeriodSEI(AL_TEncSettings settings);
bool UpdateBufferingPeriodSEI(AL_TEncSettings& settings, bool isBPEnabled);

bool CreatePictureTimingSEI(AL_TEncSettings settings);
bool UpdatePictureTimingSEI(AL_TEncSettings& settings, bool isPTEnabled);

bool CreateRecoveryPointSEI(AL_TEncSettings settings);
bool UpdateRecoveryPointSEI(AL_TEncSettings& settings, bool isRPEnabled);

bool CreateMasteringDisplayColourVolumeSEI(AL_TEncSettings settings);
bool UpdateMasteringDisplayColourVolumeSEI(AL_TEncSettings& settings, bool isMDCVEnabled);

bool CreateContentLightLevelSEI(AL_TEncSettings settings);
bool UpdateContentLightLevelSEI(AL_TEncSettings& settings, bool isCLLEnabled);
