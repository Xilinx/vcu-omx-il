// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "device_dec_interface.hpp"
#include <memory>

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
#include <lib_ip_ctrl/IpCtrl.h>
}

struct DecDeviceSoftware final : DecDeviceInterface
{
  explicit DecDeviceSoftware(std::shared_ptr<AL_TAllocator> allocator);
  ~DecDeviceSoftware() override;

  AL_IDecScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;
  void* GetDeviceContext() override;

private:
  std::shared_ptr<AL_TAllocator> allocator;
  AL_TIpCtrl* ipCtrl {};
  AL_IDecScheduler* scheduler {};
};
