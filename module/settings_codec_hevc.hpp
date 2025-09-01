// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include "module_enums.hpp"
#include "module_structs.hpp"

extern "C"
{
#include <lib_common/SliceConsts.h>
}

std::vector<ProfileLevel> CreateHEVCProfileLevelSupported(std::vector<HEVCProfileType> profiles, std::vector<int> levels);

bool IsHighTierProfile(HEVCProfileType profile);
ProfileLevel CreateHEVCMainTierProfileLevel(AL_EProfile profile, int level);
ProfileLevel CreateHEVCHighTierProfileLevel(AL_EProfile profile, int level);
