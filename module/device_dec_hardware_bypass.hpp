// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "device_dec_interface.hpp"

#include <memory>
#include <string>

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
#include <lib_ip_ctrl/IpCtrl.h>
}

struct DecDeviceHardwareBypass final : DecDeviceInterface
{
  explicit DecDeviceHardwareBypass(std::string device, std::shared_ptr<AL_TAllocator> allocator);
  ~DecDeviceHardwareBypass() override;

  AL_IDecScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;
  void* GetDeviceContext() override;

private:
  std::string const device;
  std::shared_ptr<AL_TAllocator> allocator;
  AL_TIpCtrl* ipCtrl {};
  AL_IDecScheduler* scheduler {};
};
