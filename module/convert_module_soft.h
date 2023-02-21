// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

extern "C"
{
#include <lib_common/SliceConsts.h>
#include <lib_common/PicFormat.h>
#include <lib_common/VideoMode.h>
#include <lib_common/HDR.h>
}
#include "module_enums.h"
#include "module_structs.h"

AL_EChromaMode ConvertModuleToSoftChroma(ColorType color);
ColorType ConvertSoftToModuleColor(AL_EChromaMode chroma);

AL_EEntropyMode ConvertModuleToSoftEntropyCoding(EntropyCodingType entropy);
EntropyCodingType ConvertSoftToModuleEntropyCoding(AL_EEntropyMode entropy);

VideoModeType ConvertSoftToModuleVideoMode(AL_EVideoMode videoMode);
AL_EVideoMode ConvertModuleToSoftVideoMode(VideoModeType videoMode);

SequencePictureModeType ConvertSoftToModuleSequenceMode(AL_ESequenceMode sequenceMode);
AL_ESequenceMode ConvertModuleToSoftSequenceMode(SequencePictureModeType sequenceMode);

TransferCharacteristicsType ConvertSoftToModuleTransferCharacteristics(AL_ETransferCharacteristics transferCharac);
AL_ETransferCharacteristics ConvertModuleToSoftTransferCharacteristics(TransferCharacteristicsType transferCharac);

ColourMatrixType ConvertSoftToModuleColourMatrix(AL_EColourMatrixCoefficients colourMatrix);
AL_EColourMatrixCoefficients ConvertModuleToSoftColourMatrix(ColourMatrixType colourMatrix);

ColorPrimariesType ConvertSoftToModuleColorPrimaries(AL_EColourDescription colourDescription);
AL_EColourDescription ConvertModuleToSoftColorPrimaries(ColorPrimariesType colorPrimaries);

HighDynamicRangeSeis ConvertSoftToModuleHDRSEIs(AL_THDRSEIs const& hdrSEIs);
AL_THDRSEIs ConvertModuleToSoftHDRSEIs(HighDynamicRangeSeis const& hdrSEIs);

StartCodeBytesAlignmentType ConvertSoftToModuleStartCodeBytesAlignment(AL_EStartCodeBytesAlignedMode startCodeBytesAlignmentMode);
AL_EStartCodeBytesAlignedMode ConvertModuleToSoftStartCodeBytesAlignment(StartCodeBytesAlignmentType startCodeBytesAlignmentMode);
