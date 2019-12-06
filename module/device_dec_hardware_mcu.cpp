#include "device_dec_hardware_mcu.h"

extern "C"
{
#include "lib_common/HardwareDriver.h"
#include "lib_decode/DecSchedulerMcu.h"
}

DecDeviceHardwareMcu::~DecDeviceHardwareMcu() = default;

AL_IDecScheduler* DecDeviceHardwareMcu::Init()
{
  auto constexpr deviceFile = "/dev/allegroDecodeIP";
  scheduler = AL_DecSchedulerMcu_Create(AL_GetHardwareDriver(), deviceFile);
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
