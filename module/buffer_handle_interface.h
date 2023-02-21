// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

struct BufferHandleInterface
{
  virtual ~BufferHandleInterface() = 0;

  char* const data {};
  int const size {};

  int offset {};
  int payload {};

protected:
  BufferHandleInterface(char* data, int size);
  BufferHandleInterface();
};
