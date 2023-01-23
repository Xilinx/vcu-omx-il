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

