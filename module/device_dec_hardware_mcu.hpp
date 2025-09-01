// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "device_dec_interface.hpp"
#include <string>

struct DecDeviceHardwareMcu final : DecDeviceInterface
{
  DecDeviceHardwareMcu(std::string device);
  ~DecDeviceHardwareMcu() override;

  AL_IDecScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;
  void* GetDeviceContext() override;

private:
  std::string const device;
  AL_IDecScheduler* scheduler {};
};
