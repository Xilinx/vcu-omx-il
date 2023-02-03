/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

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
