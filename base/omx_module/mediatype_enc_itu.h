/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

BufferSizes CreateBufferSizes(AL_TEncSettings settings, int stride, int sliceHeight);

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
bool UpdateFormat(AL_TEncSettings& settings, Format format, std::vector<ColorType> colors, std::vector<int> bitdepths, int& horizontalStride, StrideAlignments strideAlignments);

bool UpdateIsEnabledSubframe(AL_TEncSettings& settings, bool isSubframeEnabled);

Resolution CreateResolution(AL_TEncSettings settings, int horizontalStride, int verticalStride);
bool UpdateResolution(AL_TEncSettings& settings, int& horizontalStride, int& verticalStride, StrideAlignments strideAlignments, Resolution resolution);

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

