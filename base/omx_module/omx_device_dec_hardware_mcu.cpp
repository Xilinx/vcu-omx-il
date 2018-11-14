/******************************************************************************
*
* Copyright (C) 2018 Allegro DVT2.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX OR ALLEGRO DVT2 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of  Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
* Except as contained in this notice, the name of Allegro DVT2 shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Allegro DVT2.
*
******************************************************************************/

#include "omx_device_dec_hardware_mcu.h"

extern "C"
{
#include "lib_common/HardwareDriver.h"
AL_TIDecChannel* AL_DecChannelMcu_Create(AL_TDriver* driver);
}

DecDeviceHardwareMcu::~DecDeviceHardwareMcu() = default;

AL_TIDecChannel* DecDeviceHardwareMcu::Init()
{
  return AL_DecChannelMcu_Create(AL_GetHardwareDriver());
}

void DecDeviceHardwareMcu::Deinit()
{
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

