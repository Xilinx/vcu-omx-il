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

#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

inline static size_t AlignToPageSize(size_t zSize)
{
  unsigned long pagesize = sysconf(_SC_PAGESIZE);

  if((zSize % pagesize) == 0)
    return zSize;
  return zSize + pagesize - (zSize % pagesize);
}

void Buffer_FreeData(char* data, bool use_dmabuf)
{
  if(use_dmabuf)
    close((int)(uintptr_t)data);
  else
    free(data);
}

char* Buffer_MapData(char* data, size_t zSize, bool use_dmabuf)
{
  if(use_dmabuf)
  {
    int fd = (int)(uintptr_t)data;
    auto zMapSize = AlignToPageSize(zSize);

    data = (char*)mmap(0, zMapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(data == MAP_FAILED)
    {
      std::cerr << "MAP_FAILED!" << std::endl;
      return nullptr;
    }
  }
  return data;
}

void Buffer_UnmapData(char* data, size_t zSize, bool use_dmabuf)
{
  if(use_dmabuf)
    munmap(data, zSize);
}

