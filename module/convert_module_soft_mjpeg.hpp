// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module_structs.hpp"

extern "C"
{
#include <lib_common/SliceConsts.h>
}

JPEGProfileType ConvertSoftToModuleJPEGProfile(AL_EProfile const& profile);
AL_EProfile ConvertModuleToSoftJPEGProfile(AVCProfileType const& profile);
