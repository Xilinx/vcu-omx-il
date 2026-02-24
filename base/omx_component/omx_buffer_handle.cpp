// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "omx_buffer_handle.hpp"

OMXBufferHandle::OMXBufferHandle(OMX_BUFFERHEADERTYPE* header) : BufferHandleInterface((char*)header->pBuffer, header->nAllocLen), header(header)
{
  offset = header->nOffset;
  payload = header->nFilledLen;
}

OMXBufferHandle::~OMXBufferHandle() = default;
