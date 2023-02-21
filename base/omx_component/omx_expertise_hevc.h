// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "omx_expertise_interface.h"

struct ExpertiseHEVC final : ExpertiseInterface
{
  ~ExpertiseHEVC() override;

  OMX_ERRORTYPE GetProfileLevelSupported(OMX_PTR param, std::shared_ptr<SettingsInterface> media) override;
  OMX_ERRORTYPE GetProfileLevel(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media) override;
  OMX_ERRORTYPE SetProfileLevel(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media) override;
  OMX_ERRORTYPE GetExpertise(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media) override;
  OMX_ERRORTYPE SetExpertise(OMX_PTR param, Port const& port, std::shared_ptr<SettingsInterface> media) override;
};

