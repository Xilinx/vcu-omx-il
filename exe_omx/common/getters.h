// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <OMX_Core.h>

struct Getters
{
  Getters(OMX_HANDLETYPE* component);
  int GetLatency();
  int GetBuffersSize(int index);
  int GetBuffersCount(int index);
  bool IsComponentSupplier(int index);

private:
  OMX_HANDLETYPE* component;
};

