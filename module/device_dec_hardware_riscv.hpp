// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "device_dec_interface.hpp"

#include <memory>
#include <string>

extern "C"
{
#include <lib_common/Context.h>
#include "lib_decode/LibDecoderRiscv.h"
}

struct DecDeviceHardwareRiscV final : DecDeviceInterface
{
  DecDeviceHardwareRiscV(std::string device);
  ~DecDeviceHardwareRiscV() override;

  AL_IDecScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;
  void* GetDeviceContext() override;

private:
  std::string const device;
  AL_RiscV_Ctx riscv_ctx;
};
