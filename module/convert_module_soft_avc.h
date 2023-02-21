// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module_structs.h"

extern "C"
{
#include <lib_common/SliceConsts.h>
}

AVCProfileType ConvertSoftToModuleAVCProfile(AL_EProfile const& profile);
AL_EProfile ConvertModuleToSoftAVCProfile(AVCProfileType const& profile);
