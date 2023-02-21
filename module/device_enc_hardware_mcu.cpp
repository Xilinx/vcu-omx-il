// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "device_enc_hardware_mcu.h"

extern "C"
{
#include <lib_encode/EncSchedulerMcu.h>
#include <lib_common/HardwareDriver.h>
}

using namespace std;

EncDeviceHardwareMcu::EncDeviceHardwareMcu(string device, shared_ptr<AL_TAllocator> allocator) : device{device}, allocator{allocator}
{
}

EncDeviceHardwareMcu::~EncDeviceHardwareMcu() = default;

AL_IEncScheduler* EncDeviceHardwareMcu::Init()
{
  scheduler = AL_SchedulerMcu_Create(AL_GetHardwareDriver(), (AL_TLinuxDmaAllocator*)allocator.get(), device.c_str());
  return scheduler;
}

void EncDeviceHardwareMcu::Deinit()
{
  if(scheduler)
    AL_IEncScheduler_Destroy(scheduler);
}

BufferContiguities EncDeviceHardwareMcu::GetBufferContiguities() const
{
  BufferContiguities bufferContiguities;
  bufferContiguities.input = true;
  bufferContiguities.output = true;
  return bufferContiguities;
}

BufferBytesAlignments EncDeviceHardwareMcu::GetBufferBytesAlignments() const
{
  BufferBytesAlignments bufferBytesAlignments;
  bufferBytesAlignments.input = 32;
  bufferBytesAlignments.output = 32;
  return bufferBytesAlignments;
}
