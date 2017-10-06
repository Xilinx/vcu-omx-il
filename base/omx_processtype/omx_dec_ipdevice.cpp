/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
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

#include <stdexcept>
#include <memory>
#include <fstream>

extern "C"
{
#include "lib_fpga/DmaAlloc.h"
#include "lib_decode/I_DecChannel.h"
}

#include "omx_dec_ipdevice.h"
#include "omx_schedulertype.h"

using namespace std;


extern "C"
{
AL_TIDecChannel* AL_DecChannelMcu_Create();
}
struct CMcuIpDevice : public CIpDevice
{
  CMcuIpDevice()
  {
    m_pAllocator = DmaAlloc_Create("/dev/allegroDecodeIP");

    if(!m_pAllocator)
      throw runtime_error("Can't open DMA allocator");

    m_pScheduler = AL_DecChannelMcu_Create();

    if(!m_pScheduler)
    {
      AL_Allocator_Destroy(m_pAllocator);
      throw runtime_error("Failed to create MCU scheduler");
    }
  }

  ~CMcuIpDevice()
  {
    AL_Allocator_Destroy(m_pAllocator);
  }
};

unique_ptr<CIpDevice> CreateIpDevice(bool bUseVCU, int iSchedulerType, AL_EDecUnit eDecUnit)
{
  unique_ptr<CIpDevice> pIpDevice;
  (void)bUseVCU;
  (void)eDecUnit;

  if(iSchedulerType == SCHEDULER_CPU)
  {
    throw runtime_error("No support for on-CPU scheduling");
  }
  else if(iSchedulerType == SCHEDULER_MCU)
    pIpDevice.reset(new CMcuIpDevice());

  if(!pIpDevice)
    throw runtime_error("No device found");

  return pIpDevice;
}

