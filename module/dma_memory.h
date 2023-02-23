// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "memory_interface.h"

struct DMAMemory final : MemoryInterface
{
  explicit DMAMemory(char const* device);
  ~DMAMemory() override;
  void move(AL_TBuffer* destination, int destination_offset, AL_TBuffer const* source, int source_offset, size_t size) override;
  void set(AL_TBuffer* destination, int destination_offset, int value, size_t size) override;

private:
  int fd;
};
