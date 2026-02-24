// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "sync_ip_interface.hpp"
#include "SyncIp.hpp"
#include "settings_interface.hpp"
#include <memory>

extern "C"
{
#include <lib_common/I_Communication.h>
#include <lib_common/Allocator.h>
}

struct DecSyncIp : SyncIpInterface
{
  DecSyncIp(std::shared_ptr<SettingsInterface> media, std::shared_ptr<AL_TAllocator> allocator);
  ~DecSyncIp();

  bool create() override;
  void destroy() override;

  void addBuffer(BufferHandleInterface*) override;
  void enable() override;

private:
  bool displayNotified;
  std::shared_ptr<SettingsInterface> media;
  std::shared_ptr<SyncIp> syncIp;
  std::shared_ptr<DecSyncChannel> syncChannel;
  std::shared_ptr<AL_TAllocator> allocator;
};
