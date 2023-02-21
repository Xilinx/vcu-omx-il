// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "cpp_memory.h"

#include <algorithm> // move
#include <cstring> // memset

CPPMemory::CPPMemory() = default;
CPPMemory::~CPPMemory() = default;

void CPPMemory::move(AL_TBuffer* destination, int destination_offset, AL_TBuffer const* source, int source_offset, size_t size)
{
  std::move(AL_Buffer_GetData(source) + source_offset, AL_Buffer_GetData(source) + source_offset + size, AL_Buffer_GetData(destination) + destination_offset);
}

void CPPMemory::set(AL_TBuffer* destination, int destination_offset, int value, size_t size)
{
  std::memset(AL_Buffer_GetData(destination) + destination_offset, value, size);
}

