#pragma once

#include "module_structs.h"

#include <vector>

extern "C"
{
#include <lib_decode/lib_decode.h>
}

Clock CreateClock(AL_TDecSettings settings);
bool UpdateClock(AL_TDecSettings& settings, Clock clock);

int CreateInternalEntropyBuffer(AL_TDecSettings settings);
bool UpdateInternalEntropyBuffer(AL_TDecSettings& settings, int internalEntropyBuffer);

SequencePictureModeType CreateSequenceMode(AL_TDecSettings settings);
bool UpdateSequenceMode(AL_TDecSettings& settings, SequencePictureModeType sequenceMode, std::vector<SequencePictureModeType> sequenceModes);

Format CreateFormat(AL_TDecSettings settings);
bool UpdateFormat(AL_TDecSettings& settings, Format format, std::vector<ColorType> colors, std::vector<int> bitdepths, Stride& stride, StrideAlignments strideAlignments);

Resolution CreateResolution(AL_TDecSettings settings, Stride stride);
bool UpdateResolution(AL_TDecSettings& settings, Stride& stride, StrideAlignments strideAlignments, Resolution resolution);

BufferSizes CreateBufferSizes(AL_TDecSettings settings, Stride stride);

DecodedPictureBufferType CreateDecodedPictureBuffer(AL_TDecSettings settings);

bool UpdateIsEnabledSubframe(AL_TDecSettings& settings, bool isSubframeEnabled);

bool UpdateDecodedPictureBuffer(AL_TDecSettings& settings, DecodedPictureBufferType decodedPictureBuffer);
