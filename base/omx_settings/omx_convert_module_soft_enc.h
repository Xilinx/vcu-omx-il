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

extern "C"
{
#include <lib_common_enc/EncChanParam.h>
#include <lib_common_enc/Settings.h>
}

#include "base/omx_module/omx_module_enums.h"

RateControlType ConvertSoftToModuleRateControl(AL_ERateCtrlMode const& mode);
AspectRatioType ConvertSoftToModuleAspectRatio(AL_EAspectRatio const& aspectRatio);
GopControlType ConvertSoftToModuleGopControl(AL_EGopCtrlMode const& mode);
GdrType ConvertSoftToModuleGdr(AL_EGdrMode const& gdr);
RateControlOptionType ConvertSoftToModuleRateControlOption(AL_ERateCtrlOption const& option);
QPControlType ConvertSoftToModuleQPControl(AL_EQpCtrlMode const& mode);
ScalingListType ConvertSoftToModuleScalingList(AL_EScalingList const& scalingList);
VideoModeType ConvertSoftToModuleVideoMode(AL_EVideoMode const& videoMode);
LoopFilterType ConvertSoftToModuleLoopFilter(AL_EChEncOption const& option);

AL_ERateCtrlMode ConvertModuleToSoftRateControl(RateControlType const& mode);
AL_EAspectRatio ConvertModuleToSoftAspectRatio(AspectRatioType const& aspectRatio);
AL_EGopCtrlMode ConvertModuleToSoftGopControl(GopControlType const& mode);
AL_EScalingList ConvertModuleToSoftScalingList(ScalingListType const& scalingList);
AL_EGdrMode ConvertModuleToSoftGdr(GdrType const& gdr);
AL_ERateCtrlOption ConvertModuleToSoftRateControlOption(RateControlOptionType const& option);
AL_EQpCtrlMode ConvertModuleToSoftQPControl(QPControlType const& mode);
AL_EVideoMode ConvertModuleToSoftVideoMode(VideoModeType const& videoMode);
AL_EChEncOption ConvertModuleToSoftLoopFilter(LoopFilterType const& loopFilter);

