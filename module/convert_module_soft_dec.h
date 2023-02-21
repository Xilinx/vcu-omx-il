// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

extern "C"
{
#include <lib_common_dec/DecDpbMode.h>
#include <lib_common_dec/DecSynchro.h>
}

#include "module_enums.h"

AL_EDpbMode ConvertModuleToSoftDecodedPictureBuffer(DecodedPictureBufferType mode);
AL_EDecUnit ConvertModuleToSoftDecodeUnit(DecodeUnitType unit);

DecodedPictureBufferType ConvertSoftToModuleDecodedPictureBuffer(AL_EDpbMode mode);
DecodeUnitType ConvertSoftToModuleDecodeUnit(AL_EDecUnit unit);
