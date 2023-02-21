// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "device_enc_interface.h"

#include <memory>
#include <string>

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
}

struct EncDeviceHardwareMcu final : EncDeviceInterface
{
  explicit EncDeviceHardwareMcu(std::string device, std::shared_ptr<AL_TAllocator> allocator);
  ~EncDeviceHardwareMcu() override;

  AL_IEncScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;

private:
  std::string const device;
  std::shared_ptr<AL_TAllocator> allocator;
  AL_IEncScheduler* scheduler {};
};
