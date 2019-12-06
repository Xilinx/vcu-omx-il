#pragma once

#include "device_enc_interface.h"

#include <memory>

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
}

struct EncDeviceHardwareMcu final : EncDeviceInterface
{
  EncDeviceHardwareMcu(std::shared_ptr<AL_TAllocator> allocator);
  ~EncDeviceHardwareMcu() override;

  AL_IEncScheduler* Init() override;
  void Deinit() override;
  BufferContiguities GetBufferContiguities() const override;
  BufferBytesAlignments GetBufferBytesAlignments() const override;

private:
  std::shared_ptr<AL_TAllocator> allocator;
  AL_IEncScheduler* scheduler {};
};
