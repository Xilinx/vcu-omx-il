// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "device_dec_hardware_mcu.h"

extern "C"
{
#include "lib_common/HardwareDriver.h"
#include "lib_decode/DecSchedulerMcu.h"
}

using namespace std;

DecDeviceHardwareMcu::DecDeviceHardwareMcu(string device) :
  device(device)
{
}

DecDeviceHardwareMcu::~DecDeviceHardwareMcu() = default;

AL_IDecScheduler* DecDeviceHardwareMcu::Init()
{
  scheduler = AL_DecSchedulerMcu_Create(AL_GetHardwareDriver(), device.c_str());
  return scheduler;
}

void DecDeviceHardwareMcu::Deinit()
{
  if(scheduler)
    AL_IDecScheduler_Destroy(scheduler);
}

BufferContiguities DecDeviceHardwareMcu::GetBufferContiguities() const
{
  BufferContiguities bufferContiguities;
  bufferContiguities.input = false;
  bufferContiguities.output = true;
  return bufferContiguities;
}

BufferBytesAlignments DecDeviceHardwareMcu::GetBufferBytesAlignments() const
{
  BufferBytesAlignments bufferBytesAlignments;
  bufferBytesAlignments.input = 0;
  bufferBytesAlignments.output = 32;
  return bufferBytesAlignments;
}
