// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "settings_interface.h"
#include "module_structs.h"

extern "C"
{
#include <lib_common/SliceConsts.h>
#include <lib_common_enc/Settings.h>
}

struct EncSettingsInterface : SettingsInterface
{
  virtual ~EncSettingsInterface() override = default;

  virtual void Reset() override = 0;
  virtual ErrorType Get(std::string index, void* settings) const override = 0;
  virtual ErrorType Set(std::string index, void const* settings) override = 0;

  AL_TEncSettings settings;
  Stride stride;
};