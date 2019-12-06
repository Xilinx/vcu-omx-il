#pragma once

#include "module_structs.h"

extern "C"
{
#include <lib_common/SliceConsts.h>
}

AVCProfileType ConvertSoftToModuleAVCProfile(AL_EProfile const& profile);
AL_EProfile ConvertModuleToSoftAVCProfile(AVCProfileType const& profile);
