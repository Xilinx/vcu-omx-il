// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "device_enc_interface.hpp"

#include <memory>
#include <string>

extern "C" {
#include <lib_ip_ctrl/IpCtrl.h>
#include <lib_fpga/DmaAlloc.h>
}

struct EncDeviceHardwareBypass final : EncDeviceInterface
{
  explicit EncDeviceHardwareBypass(std::string device, std::shared_ptr<AL_TAllocator> allocator);
  ~EncDeviceHardwareBypass() override;

  AL_IEncScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;
  void* GetDeviceContext() override;

private:
  std::string const device;
  std::shared_ptr<AL_TAllocator> allocator;
  AL_TIpCtrl* ipCtrl {};
  AL_IEncScheduler* scheduler {};
};
