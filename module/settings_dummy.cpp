// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "settings_dummy.h"

using namespace std;

DummySettings::DummySettings() = default;
DummySettings::~DummySettings() = default;

void DummySettings::Reset()
{
}

SettingsInterface::ErrorType DummySettings::Get(string, void*) const
{
  return ErrorType::SUCCESS;
}

SettingsInterface::ErrorType DummySettings::Set(string, void const*)
{
  return ErrorType::SUCCESS;
}

bool DummySettings::Check()
{
  return true;
}
