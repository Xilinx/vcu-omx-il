/******************************************************************************
*
* Copyright (C) 2015-2022 Allegro DVT2
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
#include <lib_common_enc/EncChanParam.h>
#include <lib_common_enc/Settings.h>
}

#include "module_enums.h"
#include "module_structs.h"

RateControlType ConvertSoftToModuleRateControl(AL_ERateCtrlMode mode);
AspectRatioType ConvertSoftToModuleAspectRatio(AL_EAspectRatio aspectRatio);
GopControlType ConvertSoftToModuleGopControl(AL_EGopCtrlMode mode);
GdrType ConvertSoftToModuleGdr(AL_EGdrMode gdr);
RateControlOptions ConvertSoftToModuleRateControlOption(AL_ERateCtrlOption options);
QPControlType ConvertSoftToModuleQPControl(AL_EQpCtrlMode mode);
QPTableType ConvertSoftToModuleQPTable(AL_EQpTableMode mode);
ScalingListType ConvertSoftToModuleScalingList(AL_EScalingList scalingList);
LoopFilterType ConvertSoftToModuleLoopFilter(AL_EChEncTool tools);

AL_ERateCtrlMode ConvertModuleToSoftRateControl(RateControlType mode);
AL_EAspectRatio ConvertModuleToSoftAspectRatio(AspectRatioType aspectRatio);
AL_EGopCtrlMode ConvertModuleToSoftGopControl(GopControlType mode);
AL_EScalingList ConvertModuleToSoftScalingList(ScalingListType scalingList);
AL_EGdrMode ConvertModuleToSoftGdr(GdrType gdr);
AL_ERateCtrlOption ConvertModuleToSoftRateControlOption(RateControlOptions options);
AL_EQpCtrlMode ConvertModuleToSoftQPControl(QPControlType mode);
AL_EQpTableMode ConvertModuleToSoftQPTable(QPTableType mode);
AL_EChEncTool ConvertModuleToSoftLoopFilter(LoopFilterType loopFilter);
