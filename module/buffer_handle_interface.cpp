// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "buffer_handle_interface.hpp"

BufferHandleInterface::~BufferHandleInterface() = default;
BufferHandleInterface::BufferHandleInterface() = default;
BufferHandleInterface::BufferHandleInterface(char* data, int size) : data{data}, size{size}
{
}
