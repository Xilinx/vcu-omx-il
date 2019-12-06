#pragma once

#include "device_dec_interface.h"

struct DecDeviceHardwareMcu final : DecDeviceInterface
{
  ~DecDeviceHardwareMcu() override;

  AL_IDecScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;

private:
  AL_IDecScheduler* scheduler {};
};
