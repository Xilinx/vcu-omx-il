/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

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
