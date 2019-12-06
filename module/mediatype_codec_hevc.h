#pragma once

#include <vector>
#include "module_enums.h"
#include "module_structs.h"

extern "C"
{
#include <lib_common/SliceConsts.h>
}

std::vector<ProfileLevel> CreateHEVCProfileLevelSupported(std::vector<HEVCProfileType> profiles, std::vector<int> levels);

bool IsHighTierProfile(HEVCProfileType profile);
ProfileLevel CreateHEVCMainTierProfileLevel(AL_EProfile profile, int level);
ProfileLevel CreateHEVCHighTierProfileLevel(AL_EProfile profile, int level);
