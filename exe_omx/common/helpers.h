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

#pragma once

#include <string.h>
#include <OMX_IVCommon.h>
#include <OMX_VideoExt.h>

#define OMX_CALL(a) \
  do { \
    auto const r = a; \
    if(r != OMX_ErrorNone) \
    { \
      stringstream ss; \
      ss << "OMX error 0x" << std::hex << (int)r << " while executing " # a << "(FILE " << __FILE__ << ":" << std::dec << __LINE__ << ")"; \
      return r; \
    } \
  } while(false)

inline bool is422(OMX_COLOR_FORMATTYPE format)
{
  OMX_U32 extendedFormat = format;
  return extendedFormat == OMX_COLOR_FormatYUV422SemiPlanar || extendedFormat == OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked;
};

inline bool is10bits(OMX_COLOR_FORMATTYPE format)
{
  OMX_U32 extendedFormat = format;
  return extendedFormat == OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked || extendedFormat == OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked;
};

template<typename T>
inline
void initHeader(T& header)
{
  memset(&header, 0x0, sizeof(T));
  header.nSize = sizeof(header);

  header.nVersion.s.nVersionMajor = OMX_VERSION_MAJOR;
  header.nVersion.s.nVersionMinor = OMX_VERSION_MINOR;
  header.nVersion.s.nRevision = OMX_VERSION_REVISION;
  header.nVersion.s.nStep = OMX_VERSION_STEP;
};

inline char const* toStringCompState(OMX_STATETYPE state)
{
  switch(state)
  {
  case OMX_StateInvalid: return "OMX_StateInvalid";
  case OMX_StateLoaded: return "OMX_StateLoaded";
  case OMX_StateIdle: return "OMX_StateIdle";
  case OMX_StateExecuting: return "OMX_StateExecuting";
  case OMX_StatePause: return "OMX_StatePause";
  case OMX_StateWaitForResources: return "OMX_StateWaitForResources";
  default: return "?";
  }
}

void Buffer_FreeData(char* data, bool use_dmabuf);
char* Buffer_MapData(char* data, size_t zSize, bool use_dmabuf);
void Buffer_UnmapData(char* data, size_t zSize, bool use_dmabuf);

