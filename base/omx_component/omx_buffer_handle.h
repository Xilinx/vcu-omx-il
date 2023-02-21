// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module/buffer_handle_interface.h"

#include <OMX_Core.h>

struct OMXBufferHandle : BufferHandleInterface
{
  explicit OMXBufferHandle(OMX_BUFFERHEADERTYPE* header);
  ~OMXBufferHandle() override;

  OMX_BUFFERHEADERTYPE* const header;
};

