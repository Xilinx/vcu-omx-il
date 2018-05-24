/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
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

#include "base/omx_module/omx_module_structs.h"

extern "C"
{
#include <lib_common_enc/Settings.h>
}

Clock CreateClock(AL_TEncSettings const& settings);
bool UpdateClock(AL_TEncSettings& settings, Clock const& clock);

Gop CreateGroupOfPictures(AL_TEncSettings const& settings);
bool UpdateGroupOfPictures(AL_TEncSettings& settings, Gop const& gop);

bool CreateConstrainedIntraPrediction(AL_TEncSettings const& settings);
bool UpdateConstrainedIntraPrediction(AL_TEncSettings& settings, bool const& isConstrainedIntraPredictionEnabled);

VideoModeType CreateVideoMode(AL_TEncSettings const& settings);
bool UpdateVideoMode(AL_TEncSettings& settings, VideoModeType const& videoMode);

Bitrate CreateBitrate(AL_TEncSettings const& settings);
bool UpdateBitrate(AL_TEncSettings& settings, Bitrate const& bitrate);

bool CreateCacheLevel2(AL_TEncSettings const& settings);
bool UpdateCacheLevel2(AL_TEncSettings& settings, bool const& isCacheLevel2Enabled);

bool CreateFillerData(AL_TEncSettings const& settings);
bool UpdateFillerData(AL_TEncSettings& settings, bool const& isFillerDataEnabled);

AspectRatioType CreateAspectRatio(AL_TEncSettings const& settings);
bool UpdateAspectRatio(AL_TEncSettings& settings, AspectRatioType const& aspectRatio);

ScalingListType CreateScalingList(AL_TEncSettings const& settings);
bool UpdateScalingList(AL_TEncSettings& settings, ScalingListType const& scalingList);

QPs CreateQuantizationParameter(AL_TEncSettings const& settings);
bool UpdateQuantizationParameter(AL_TEncSettings& settings, QPs const& qps);

Slices CreateSlicesParameter(AL_TEncSettings const& settings);
bool UpdateSlicesParameter(AL_TEncSettings& settings, Slices const& slices);

