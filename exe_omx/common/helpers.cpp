/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
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

#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

#include "helpers.h"

static inline size_t AlignToPageSize(size_t size)
{
  unsigned long pagesize = sysconf(_SC_PAGESIZE);

  if((size % pagesize) == 0)
    return size;
  return size + pagesize - (size % pagesize);
}

void Buffer_FreeData(char* data, bool use_dmabuf)
{
  if(use_dmabuf)
    close((int)(uintptr_t)data);
  else
    free(data);
}

char* Buffer_MapData(char* data, size_t offset, size_t size, bool use_dmabuf)
{
  if(!use_dmabuf)
    return data + offset;

  int fd = (int)(intptr_t)data;
  auto mapSize = AlignToPageSize(size);

  data = (char*)mmap(0, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  if(data == MAP_FAILED)
  {
    std::cerr << "MAP_FAILED!" << std::endl;
    return nullptr;
  }
  return data + offset;
}

void Buffer_UnmapData(char* data, size_t zSize, bool use_dmabuf)
{
  if(!use_dmabuf)
    return;
  munmap(data, zSize);
}

bool setChroma(std::string user_chroma, OMX_COLOR_FORMATTYPE* chroma)
{
  if(user_chroma == "y800")
    *chroma = OMX_COLOR_FormatL8;
  else if(user_chroma == "nv12")
    *chroma = OMX_COLOR_FormatYUV420SemiPlanar;
  else if(user_chroma == "nv16")
    *chroma = OMX_COLOR_FormatYUV422SemiPlanar;
  else if(user_chroma == "i444")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV444Planar8bit);

  else if(user_chroma == "xv10")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL10bitPacked);
  else if(user_chroma == "xv15")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked);
  else if(user_chroma == "xv20")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked);

  else if(user_chroma == "y010")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL10bit);
  else if(user_chroma == "p010")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bit);
  else if(user_chroma == "p210")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bit);
  else if(user_chroma == "i4al")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV444Planar10bit);

  else if(user_chroma == "y012")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL12bit);
  else if(user_chroma == "p012")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar12bit);
  else if(user_chroma == "p212")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar12bit);
  else if(user_chroma == "i4cl")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV444Planar12bit);
  else
    return false;
  return true;
}

extern "C" bool setChromaWrapper(char* user_chroma, OMX_COLOR_FORMATTYPE* chroma)
{
  std::string user_chroma_string {
    user_chroma
  };
  return setChroma(user_chroma_string, chroma);
}
