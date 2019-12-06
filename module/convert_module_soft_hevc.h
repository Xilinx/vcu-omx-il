#pragma once

#include "module_structs.h"

extern "C"
{
#include <lib_common/SliceConsts.h>
}

HEVCProfileType ConvertSoftToModuleHEVCMainTierProfile(AL_EProfile const& profile);
HEVCProfileType ConvertSoftToModuleHEVCHighTierProfile(AL_EProfile const& profile);
AL_EProfile ConvertModuleToSoftHEVCProfile(HEVCProfileType const& profile);
