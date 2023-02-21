// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>

extern "C"
{
#include "lib_common/BufferAPI.h"
}

struct MemoryInterface
{
  virtual ~MemoryInterface() = 0;
  virtual void move(AL_TBuffer* destination, int destination_offset, AL_TBuffer const* source, int source_offset, size_t size) = 0;
  virtual void set(AL_TBuffer* destination, int destination_offset, int value, size_t size) = 0;
};

