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
