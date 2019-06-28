/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

#include <cstring>
#include <string>
#include <OMX_IVCommon.h>
#include <OMX_VideoExt.h>
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <sstream>
#include <iostream>
#include <utility/logger.h>

#define OMX_CALL(a) \
  do { \
    auto const r = a; \
    if(r != OMX_ErrorNone) \
    { \
      std::stringstream ss; \
      ss << "OMX error 0x" << std::hex << (int)r << " while executing " # a << "(FILE " << __FILE__ << ":" << std::dec << __LINE__ << ")"; \
      LOG_ERROR(ss.str()); \
      return r; \
    } \
  } while(false)

static inline bool is400(OMX_COLOR_FORMATTYPE format)
{
  OMX_U32 extendedFormat = format;
  return extendedFormat == OMX_COLOR_FormatL8 || extendedFormat == OMX_ALG_COLOR_FormatL10bitPacked;
}

static inline bool is420(OMX_COLOR_FORMATTYPE format)
{
  OMX_U32 extendedFormat = format;
  return extendedFormat == OMX_COLOR_FormatYUV420SemiPlanar || extendedFormat == OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked;
}

static inline bool is422(OMX_COLOR_FORMATTYPE format)
{
  OMX_U32 extendedFormat = format;
  return extendedFormat == OMX_COLOR_FormatYUV422SemiPlanar || extendedFormat == OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked;
}

static inline bool is8bits(OMX_COLOR_FORMATTYPE format)
{
  return format == OMX_COLOR_FormatL8 || format == OMX_COLOR_FormatYUV420SemiPlanar || format == OMX_COLOR_FormatYUV422SemiPlanar;
}

static inline bool is10bits(OMX_COLOR_FORMATTYPE format)
{
  OMX_U32 extendedFormat = format;
  return extendedFormat == OMX_ALG_COLOR_FormatL10bitPacked || extendedFormat == OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked || extendedFormat == OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked;
}

static inline OMX_ERRORTYPE showComponentVersion(OMX_HANDLETYPE* handle)
{
  char name[OMX_MAX_STRINGNAME_SIZE];
  OMX_VERSIONTYPE compType;
  OMX_VERSIONTYPE ilType;

  OMX_CALL(OMX_GetComponentVersion(*handle, (OMX_STRING)name, &compType, &ilType, nullptr));

  LOG_IMPORTANT(std::string { "Component: " } +std::string { name } +std::string { "(v." } +std::to_string(compType.nVersion) + std::string { ") made for OMX_IL client: " } +std::to_string(ilType.s.nVersionMajor) + std::string { "." } +std::to_string(ilType.s.nVersionMinor) + std::string { "." } +std::to_string(ilType.s.nRevision));
  return OMX_ErrorNone;
}

template<typename T>
static
inline
void initHeader(T& header)
{
  memset(&header, 0, sizeof(T));
  header.nSize = sizeof(header);

  header.nVersion.s.nVersionMajor = OMX_VERSION_MAJOR;
  header.nVersion.s.nVersionMinor = OMX_VERSION_MINOR;
  header.nVersion.s.nRevision = OMX_VERSION_REVISION;
  header.nVersion.s.nStep = OMX_VERSION_STEP;
}

void Buffer_FreeData(char* data, bool use_dmabuf);
char* Buffer_MapData(char* data, size_t zSize, bool use_dmabuf);
void Buffer_UnmapData(char* data, size_t zSize, bool use_dmabuf);

template<typename Lambda>
struct ScopeExitClass
{
  ScopeExitClass(Lambda fn) :
    m_fn(fn)
  {
  }

  ~ScopeExitClass()
  {
    m_fn();
  }

private:
  Lambda m_fn;
};

template<typename Lambda>
ScopeExitClass<Lambda> scopeExit(Lambda fn)
{
  return ScopeExitClass<Lambda>(fn);
}

