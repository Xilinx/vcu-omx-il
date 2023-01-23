/******************************************************************************
*
* Copyright (C) 2015-2022 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#include "dma_memory.h"
#include "dmaproxy.h"

#include <algorithm> // move
#include <cstring> // memset
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <utility/logger.h>

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
#include <lib_fpga/DmaAllocLinux.h>
}

DMAMemory::DMAMemory(char const* device)
{
  fd = ::open(device, O_RDWR);

  if(fd < 0)
  {
    LOG_ERROR(::strerror(errno) + std::string { ": '" } +std::string { device } +std::string { "'. DMA channel is not available, CPU move will be performed" });
  }
}

DMAMemory::~DMAMemory()
{
  if(fd >= 0)
    ::close(fd);
}

void DMAMemory::move(AL_TBuffer* destination, int destination_offset, AL_TBuffer const* source, int source_offset, size_t size)
{
  if(fd < 0)
  {
    std::move(AL_Buffer_GetData(source) + source_offset, AL_Buffer_GetData(source) + source_offset + size, AL_Buffer_GetData(destination) + destination_offset);
    return;
  }

  auto src_allocator = source->pAllocator;
  int src_fd = AL_LinuxDmaAllocator_GetFd((AL_TLinuxDmaAllocator*)src_allocator, source->hBufs[0]);

  auto dst_allocator = destination->pAllocator;
  int dst_fd = AL_LinuxDmaAllocator_GetFd((AL_TLinuxDmaAllocator*)dst_allocator, destination->hBufs[0]);

  dmaproxy_arg_t dmaproxy {};
  dmaproxy.size = size;
  dmaproxy.dst_offset = destination_offset;
  dmaproxy.src_offset = source_offset;
  dmaproxy.src_fd = src_fd;
  dmaproxy.dst_fd = dst_fd;

  if(::ioctl(fd, DMAPROXY_COPY, &dmaproxy) < 0)
  {
    LOG_WARNING(::strerror(errno) + std::string { ": DMA channel is not available, CPU move will be performed" });
    std::move(AL_Buffer_GetData(source) + source_offset, AL_Buffer_GetData(source) + source_offset + size, AL_Buffer_GetData(destination) + destination_offset);
  }
}

void DMAMemory::set(AL_TBuffer* destination, int destination_offset, int value, size_t size)
{
  // To be optimized later if such DMA capability exists
  std::memset(AL_Buffer_GetData(destination) + destination_offset, value, size);
}

