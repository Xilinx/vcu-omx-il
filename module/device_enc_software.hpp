// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "device_enc_interface.hpp"

#include <memory>

extern "C"
{
#include <lib_common_enc/Settings.h>
#include <lib_fpga/DmaAlloc.h>
#include <lib_ip_ctrl/IpCtrl.h>
}

struct EncDeviceSoftware final : EncDeviceInterface
{
  EncDeviceSoftware(std::shared_ptr<AL_TAllocator> allocator);
  ~EncDeviceSoftware() override;

  void SoftwarePreInit(AL_TEncSettings const* settings);
  AL_IEncScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;
  void* GetDeviceContext() override;

private:
  std::shared_ptr<AL_TAllocator> allocator;
  AL_TEncSettings const* settings;
  AL_TIpCtrl* ipCtrl {};
  AL_IEncScheduler* scheduler {};
};
