// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include "module_enums.h"
#include "module_structs.h"

extern "C"
{
#include <lib_common/SliceConsts.h>
}

std::vector<ProfileLevel> CreateAVCProfileLevelSupported(std::vector<AVCProfileType> profiles, std::vector<int> levels);

ProfileLevel CreateAVCProfileLevel(AL_EProfile profile, int level);
