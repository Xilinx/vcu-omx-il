// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "settings_interface.h"

struct DummySettings final : SettingsInterface
{
  DummySettings();
  ~DummySettings() override;

  void Reset() override;
  ErrorType Get(std::string, void*) const override;
  ErrorType Set(std::string, void const*) override;
  bool Check() override;
};
