// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <OMX_Types.h>
#include <cassert>

#include "module/settings_interface.h"
#include "omx_component_structs.h"

#define OMX_CHECK_MEDIA_GET(ret) \
  if(ret == SettingsInterface::BAD_INDEX) \
    throw OMX_ErrorUnsupportedIndex; \
  assert(ret == SettingsInterface::SUCCESS);

#define OMX_CHECK_MEDIA_SET(ret) \
  if(ret == SettingsInterface::BAD_INDEX) \
    return OMX_ErrorUnsupportedIndex; \
  if(ret == SettingsInterface::BAD_PARAMETER) \
    return OMX_ErrorBadParameter; \
  assert(ret == SettingsInterface::SUCCESS);

struct ExpertiseInterface
{
  virtual ~ExpertiseInterface() = 0;

  virtual OMX_ERRORTYPE GetProfileLevelSupported(OMX_PTR param, std::shared_ptr<SettingsInterface> media) = 0;
  virtual OMX_ERRORTYPE GetProfileLevel(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media) = 0;
  virtual OMX_ERRORTYPE SetProfileLevel(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media) = 0;
  virtual OMX_ERRORTYPE GetExpertise(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media) = 0;
  virtual OMX_ERRORTYPE SetExpertise(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media) = 0;
};

