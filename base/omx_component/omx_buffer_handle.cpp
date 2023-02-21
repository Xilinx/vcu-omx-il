// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "omx_buffer_handle.h"

OMXBufferHandle::OMXBufferHandle(OMX_BUFFERHEADERTYPE* header) : BufferHandleInterface((char*)header->pBuffer, header->nAllocLen), header(header)
{
  offset = header->nOffset;
  payload = header->nFilledLen;
}

OMXBufferHandle::~OMXBufferHandle() = default;

