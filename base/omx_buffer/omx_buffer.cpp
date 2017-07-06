/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX OR ALLEGRO DVT2 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of  Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
* Except as contained in this notice, the name of Allegro DVT2 shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Allegro DVT2.
*
******************************************************************************/

#include "omx_buffer.h"

#include <assert.h>

extern "C"
{
#include "lib_fpga/DmaAllocLinux.h"
}

#include "base/omx_utils/omx_log.h"

BufferAllocator::BufferAllocator(AL_TAllocator* allocator, bool useFileDescriptor, PFN_RefCount_CallBack userCallback) : allocator(allocator), useFileDescriptor(useFileDescriptor), userCallback(userCallback)
{
  assert(allocator);
};

int BufferAllocator::GetBufferFileDescriptor(AL_TBuffer* pBuffer)
{
  if(!useFileDescriptor)
    return -1;

  if(!pBuffer)
    return -1;

  auto const handle = pBuffer->hBuf;

  if(!handle)
    return -1;

  return AL_LinuxDmaAllocator_ExportToFd((AL_TLinuxDmaAllocator*)allocator, handle);
};

AL_TBuffer* BufferAllocator::CreateBuffer(int size, void* userData)
{
  LOGV("_ %s _ : ", __func__);

  auto const frame = AL_Buffer_Create_And_Allocate(allocator, size, userCallback);

  if(frame)
    AL_Buffer_SetUserData(frame, userData);

  return frame;
};

void BufferAllocator::DestroyBuffer(AL_TBuffer* pBuffer)
{
  LOGV("_ %s _ : ", __func__);
  AL_Allocator_Free(allocator, pBuffer->hBuf);

  AL_Buffer_Destroy(pBuffer);
  pBuffer = NULL;
};

AL_HANDLE BufferAllocator::TryToGetBufferFromFd(unsigned char* data)
{
  AL_HANDLE handle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator, (int)(uintptr_t)data);

  if(!handle)
    LOGE("Failed to ImportFromFd %i", (int)(uintptr_t)data);
  return handle;
};

AL_TBuffer* BufferAllocator::UseBuffer(unsigned char* data, int size, void* userData)
{
  LOGV("_ %s _ : ", __func__);

  if(useFileDescriptor)
  {
    auto handle = TryToGetBufferFromFd(data);
    auto frame = AL_Buffer_Create(allocator, handle, size, userCallback);

    if(frame)
      AL_Buffer_SetUserData(frame, userData);

    return frame;
  }
  else
    return CreateBuffer(size, userData);
};

void BufferAllocator::UseFileDescriptor(bool useFileDescriptor)
{
  LOGV("_ %s _ : %s", __func__, useFileDescriptor ? "TRUE" : "FALSE");
  this->useFileDescriptor = useFileDescriptor;
};

bool BufferAllocator::IsFileDescriptor()
{
  return this->useFileDescriptor;
};

