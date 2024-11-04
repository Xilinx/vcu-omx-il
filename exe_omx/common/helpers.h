// SPDX-FileCopyrightText: © 2024 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <cstring>
#include <string>
#include <OMX_IVCommonAlg.h>
#include <OMX_VideoAlg.h>
#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <sstream>
#include <iostream>
#include <utility/logger.h>
#include <utility/scope_exit.h>

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

bool is8bits(OMX_COLOR_FORMATTYPE format);
bool isFormatSupported(OMX_COLOR_FORMATTYPE format);

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
void InitHeader(T& header)
{
  memset(&header, 0, sizeof(T));
  header.nSize = sizeof(header);

  header.nVersion.s.nVersionMajor = OMX_VERSION_MAJOR;
  header.nVersion.s.nVersionMinor = OMX_VERSION_MINOR;
  header.nVersion.s.nRevision = OMX_VERSION_REVISION;
  header.nVersion.s.nStep = OMX_VERSION_STEP;
}

void Buffer_FreeData(char* data, bool use_dmabuf);
char* Buffer_MapData(char* data, size_t offset, size_t size, bool use_dmabuf);
void Buffer_UnmapData(char* data, size_t size, bool use_dmabuf);

bool setChroma(std::string user_chroma, OMX_COLOR_FORMATTYPE* chroma);
extern "C" bool setChromaWrapper(char* user_chroma, OMX_COLOR_FORMATTYPE* chroma);

template<typename T>
static OMX_ERRORTYPE PortSetup(OMX_HANDLETYPE handle, OMX_INDEXTYPE index, std::function<void(T &)> modification, int port)
{
  T setup;
  InitHeader(setup);
  setup.nPortIndex = port;

  auto error = OMX_GetParameter(handle, index, &setup);

  if(error != OMX_ErrorNone)
    return error;

  T rollback = setup;

  modification(setup);
  error = OMX_SetParameter(handle, index, &setup);

  if(error != OMX_ErrorNone)
    OMX_SetParameter(handle, index, &rollback);
  return error;
}

template<typename T>
static OMX_ERRORTYPE Setup(OMX_HANDLETYPE handle, OMX_INDEXTYPE index, std::function<void(T &)> modification)
{
  T setup;
  InitHeader(setup);

  auto error = OMX_GetParameter(handle, index, &setup);

  if(error != OMX_ErrorNone)
    return error;

  T rollback = setup;

  modification(setup);
  error = OMX_SetParameter(handle, index, &setup);

  if(error != OMX_ErrorNone)
    OMX_SetParameter(handle, index, &rollback);
  return error;
}

void appendSupportedFourccString(std::string& str);
