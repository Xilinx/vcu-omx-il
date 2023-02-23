// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "memory_interface.h"

struct CPPMemory final : MemoryInterface
{
  CPPMemory();
  ~CPPMemory() override;
  void move(AL_TBuffer* destination, int destination_offset, AL_TBuffer const* source, int source_offset, size_t size) override;
  void set(AL_TBuffer* destination, int destination_offset, int value, size_t size) override;
};

