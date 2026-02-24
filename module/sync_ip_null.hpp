// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once
#include "sync_ip_interface.hpp"

struct NullSyncIp final : SyncIpInterface
{
  ~NullSyncIp() override {};

  bool create() override { return true; };
  void destroy() override {};

  void addBuffer(BufferHandleInterface*) override {};
  void enable() override {};
};
