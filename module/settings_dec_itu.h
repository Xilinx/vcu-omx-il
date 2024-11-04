// SPDX-FileCopyrightText: © 2024 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module/module_enums.h"
#include "module_structs.h"

#include <vector>

extern "C"
{
#include <lib_decode/lib_decode.h>
#include <lib_common_dec/IpDecFourCC.h>
}

Clock CreateClock(AL_TDecSettings const& settings);
bool UpdateClock(AL_TDecSettings& settings, Clock clock);

int CreateInternalEntropyBuffer(AL_TDecSettings const& settings);
bool UpdateInternalEntropyBuffer(AL_TDecSettings& settings, int internalEntropyBuffer);

SequencePictureModeType CreateSequenceMode(AL_TDecSettings const& settings);
bool UpdateSequenceMode(AL_TDecSettings& settings, SequencePictureModeType sequenceMode, std::vector<SequencePictureModeType> sequenceModes);

Format CreateFormat(AL_TDecSettings const& settings);
bool UpdateFormat(AL_TDecSettings& settings, Format format, std::vector<ColorType> colors, std::vector<int> bitdepths, std::vector<StorageType> storage, Stride& stride, StrideAlignments strideAlignments);

Resolution CreateResolution(AL_TDecSettings const& settings, Stride stride);
bool UpdateResolution(AL_TDecSettings& settings, Stride& stride, StrideAlignments strideAlignments, Resolution resolution);

BufferSizes CreateBufferSizes(AL_TDecSettings const& settings, Stride stride);

DecodedPictureBufferType CreateDecodedPictureBuffer(AL_TDecSettings const& settings);
bool UpdateDecodedPictureBuffer(AL_TDecSettings& settings, DecodedPictureBufferType decodedPictureBuffer);

bool UpdateIsEnabledSubframe(AL_TDecSettings& settings, bool isSubframeEnabled);

bool CreateRealtime(AL_TDecSettings const& settings);
bool UpdateRealtime(AL_TDecSettings& settings, bool isSubframeDisabled);

Point<int> CreateOutputPosition(AL_TDecSettings const& settings);
bool UpdateOutputPosition(AL_TDecSettings& settings, Point<int> position);
