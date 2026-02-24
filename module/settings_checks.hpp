// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module/module_enums.hpp"
#include "module_structs.hpp"
#include <vector>
#include <algorithm>

template<class T>
bool IsSupported(T const& value, std::vector<T> const& supported)
{
  return std::any_of(supported.cbegin(), supported.cend(), [&](T const& each) { return each == value; });
}

bool CheckClock(Clock const& clock);
bool CheckGroupOfPictures(Gop const& gop);
bool CheckInternalEntropyBuffer(int const& internalEntropyBuffer);
bool CheckVideoMode(VideoModeType const& videoMode);
bool CheckSequenceMode(SequencePictureModeType const& sequenceMode, std::vector<SequencePictureModeType> const& sequenceModes);
bool CheckBitrate(Bitrate const& bitrate, Clock const& clock);
bool CheckAspectRatio(AspectRatioType const& aspectRatio);
bool CheckScalingList(ScalingListType const& scalingList);
bool CheckQuantizationParameter(QPs const& qps);
bool CheckSlicesParameter(Slices const& slices);
bool CheckFormat(Format const& format, std::vector<ColorType> const& colors, std::vector<int> const& bitdepths, std::vector<StorageType> const& storages);
bool CheckBufferHandles(BufferHandles const& bufferHandles);
bool CheckColorPrimaries(ColorPrimariesType const& colorPrimaries);
bool CheckTransferCharacteristics(TransferCharacteristicsType const& transferCharacteristics);
bool CheckColourMatrix(ColourMatrixType const& colourMatrix);
bool CheckLookAhead(LookAhead const& la);
bool CheckTwoPass(TwoPass const& tp);
bool CheckMaxPictureSizes(MaxPicturesSizes const& sizes);
bool CheckLoopFilterBeta(int const& beta);
bool CheckLoopFilterTc(int const& tc);
bool CheckCrop(Region const& region);
bool CheckLog2CodingUnit(MinMax<int> const& log2CodingUnit);
bool CheckStartCodeBytesAlignment(StartCodeBytesAlignmentType const& startCodeBytesAlignment);
