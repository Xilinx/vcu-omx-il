/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

#include "omx_sync_ip_dec.h"
#include <cassert>
#include "DummySyncDriver.h"

extern "C"
{
#include <lib_common/BufferSrcMeta.h>
#include <lib_common/BufferStreamMeta.h>
#include <lib_common/StreamBuffer.h>

#include <lib_common_dec/IpDecFourCC.h>
#include <lib_common_dec/DecInfo.h>

#include <lib_fpga/DmaAllocLinux.h>
#include <lib_common/HardwareDriver.h>
}

#include "base/omx_checker/omx_checker.h"
#include "base/omx_mediatype/omx_convert_module_soft.h"
#include "base/omx_utils/round.h"
#include "base/omx_utils/logger.h"
#include "base/omx_module/omx_module_structs.h"

#include "base/omx_mediatype/omx_mediatype_dec_interface.h"

#include <chrono>
#include <thread>

using namespace std;

static char const* syncDeviceNameDec = "/dev/xvsfsync1";
static constexpr bool usingDummy = false;

static AL_TDriver* getDriver()
{
  if(usingDummy)
    return AL_InitDummyDriver(true, 4);

  return AL_GetHardwareDriver();
}

template<typename Func>
static bool CreateAndAttachSourceMeta(AL_TBuffer* buf, shared_ptr<MediatypeInterface> media, Func CreateSourceMeta)
{
  auto meta = CreateSourceMeta(media.get());

  if(!meta)
    return false;

  if(!AL_Buffer_AddMetaData(buf, meta))
  {
    AL_MetaData_Destroy(meta);
    return false;
  }
  return true;
}

static AL_TBuffer* CreateBuffer(AL_TLinuxDmaAllocator* allocator, int fd, int size)
{
  if(fd < 0)
    throw invalid_argument("fd");

  auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd(allocator, fd);

  if(!dmaHandle)
  {
    fprintf(stderr, "SyncIp: Failed to import fd : %i\n", fd);
    return nullptr;
  }

  return AL_Buffer_Create((AL_TAllocator*)allocator, dmaHandle, size, AL_Buffer_Destroy);
}

OMXDecSyncIp::OMXDecSyncIp(shared_ptr<MediatypeInterface> media, shared_ptr<AL_TAllocator> allocator) : media(media), allocator(allocator), syncIp(getDriver(), syncDeviceNameDec), sync{&syncIp, syncIp.getFreeChannel()}
{
  assert(media);
  assert(allocator);
  bool enableEarlyCB = true;
  media->Set(SETTINGS_INDEX_LLP2_EARLY_CB, &enableEarlyCB);
}

#if AL_ENABLE_SYNCIP_DEC

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

static void TxStreamUp()
{
  int fd = open("/sys/kernel/debug/xlnx-hdmi/hdmitx_57", O_RDWR);

  if(fd < 0)
  {
    perror(__func__);
    return;
  }

  if(write(fd, "enable", 6) != 6)
    printf("Failed to call TxStreamUp");
}

static void wTxStreamUp(union sigval)
{
  LOG("End timer");
  TxStreamUp();
}

template<typename Func>
static void call_after(Func callback, void* user_param, int64_t sec, int64_t nsec)
{
  struct sigevent sev {};
  timer_t timer;

  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_notify_function = callback;
  sev.sigev_value.sival_ptr = user_param;
  timer_create(CLOCK_MONOTONIC, &sev, &timer);

  /* Start the timer */

  struct itimerspec its;

  its.it_value.tv_sec = sec;
  its.it_value.tv_nsec = nsec;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;

  timer_settime(timer, 0, &its, NULL);
}

#else

static void wTxStreamUp(void*)
{
  throw runtime_error("TxStreamUp isn't implemented (needs debug-fs file support)");
}

template<typename Func>
static void call_after(Func, void*, int64_t, int64_t)
{
  throw runtime_error("call_after isn't implemented (needs -lrt)");
}

#endif

static int divideRoundUp(uint64_t dividende, uint64_t divisor)
{
  return (dividende + divisor - 1) / divisor;
}

static void notifyDisplay(Clock const& clock)
{
  /*
   * VSYNC interface to program the display
   * On first add buffer:
   * From LLP2 doc: we set the Triple Timer Counter (TTC0)
   * Here we use the linux timer
   * timeout value will be frame_period / 2 - DRM latency init (~3ms)
   */

  int drmLatency = 3000000;
  int frameRate = divideRoundUp(clock.framerate * 1000, clock.clockratio);
  int64_t framePeriod = 1000000000 / frameRate;
  /* nano seconds */
  int64_t timeout = framePeriod / 2 - drmLatency;

  fprintf(stderr, "will ask for VSYNC in %ld ns\n", timeout);
  LOG("start timer");
  call_after(wTxStreamUp, NULL, 0, timeout);
}

static AL_TMetaData* CreateDecSourceMeta(MediatypeInterface* media_)
{
  Resolution resolution {};
  auto media = static_cast<DecMediatypeInterface*>(media_);
  AL_TStreamSettings const& streamSettings = media->settings.tStream;
  media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
  auto picFormat = AL_GetDecPicFormat(streamSettings.eChroma, static_cast<uint8_t>(streamSettings.iBitDepth), AL_FB_RASTER, false);
  auto fourCC = AL_GetDecFourCC(picFormat);
  auto stride = resolution.stride.widthStride;
  auto sliceHeight = resolution.stride.heightStride;
  AL_TPlane planeY = { 0, stride };
  AL_TPlane planeUV = { stride* sliceHeight, stride };
  return (AL_TMetaData*)(AL_SrcMetaData_Create({ resolution.width, resolution.height }, planeY, planeUV, fourCC));
}

void OMXDecSyncIp::addBuffer(BufferHandleInterface* handle)
{
  if(!displayNotified)
  {
    Clock clock;
    media->Get(SETTINGS_INDEX_CLOCK, &clock);
    notifyDisplay(clock);
    displayNotified = true;
  }

  BufferHandles bufferHandles {};
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &bufferHandles);

  if(bufferHandles.output != BufferHandleType::BUFFER_HANDLE_FD)
    throw runtime_error("We only support dmabuf when the sync ip is activated");

  int fd = static_cast<int>((intptr_t)handle->data);
  AL_TBuffer* buf = CreateBuffer((AL_TLinuxDmaAllocator*)allocator.get(), fd, handle->payload);
  AL_Buffer_Ref(buf);
  bool attached = CreateAndAttachSourceMeta(buf, media, CreateDecSourceMeta);
  assert(attached);

  try
  {
    sync.addBuffer(buf);
  }
  catch(sync_no_buf_slot_available& e)
  {
    (void)e;
    fprintf(stderr, "Error while using the sync ip (Continuing): %s\n", e.what());
  }

  AL_Buffer_Unref(buf);
}

void OMXDecSyncIp::enable()
{
  sync.enable();
}

