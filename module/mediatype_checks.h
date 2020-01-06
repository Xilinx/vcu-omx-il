#pragma once

#include "module_structs.h"
#include <vector>
#include <algorithm>

template<class T>
bool IsSupported(T value, std::vector<T> supported)
{
  return std::any_of(supported.cbegin(), supported.cend(), [&](T const& each) { return each == value; });
}

bool CheckClock(Clock clock);
bool CheckGroupOfPictures(Gop gop);
bool CheckInternalEntropyBuffer(int internalEntropyBuffer);
bool CheckVideoMode(VideoModeType videoMode);
bool CheckSequenceMode(SequencePictureModeType sequenceMode, std::vector<SequencePictureModeType> sequenceModes);
bool CheckBitrate(Bitrate bitrate, Clock clock);
bool CheckAspectRatio(AspectRatioType aspectRatio);
bool CheckScalingList(ScalingListType scalingList);
bool CheckQuantizationParameter(QPs qps);
bool CheckSlicesParameter(Slices slices);
bool CheckFormat(Format format, std::vector<ColorType> colors, std::vector<int> bitdepths);
bool CheckBufferHandles(BufferHandles bufferHandles);
bool CheckColorPrimaries(ColorPrimariesType colorPrimaries);
bool CheckTransferCharacteristics(TransferCharacteristicsType transferCharacteristics);
bool CheckColourMatrix(ColourMatrixType colourMatrix);
bool CheckLookAhead(LookAhead la);
bool CheckTwoPass(TwoPass tp);
bool CheckMaxPictureSizes(MaxPicturesSizes sizes);
bool CheckLoopFilterBeta(int beta);
bool CheckLoopFilterTc(int tc);
