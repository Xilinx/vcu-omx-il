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

/* for u64 print format */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "SyncIp.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <map>
#include <algorithm>

extern "C"
{
/* needed definition for xvsfsync.h */
#include <sys/ioctl.h>
#define BIT(x) (1 << (x))
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
#include "xvsfsync.h"

#include "lib_common/BufferSrcMeta.h"
#include "lib_common/FourCC.h"
}

#include "SyncLog.h"

using namespace std;

struct sync_error : public std::runtime_error
{
  explicit sync_error(const char* msg) : std::runtime_error(msg)
  {
  }
};

struct sync_no_buf_slot_available : public sync_error
{
  explicit sync_no_buf_slot_available() : sync_error("Couldn't add buffer to the sync ip channel")
  {
  }
};

template<typename L>
std::unique_lock<L> Lock(L& lockMe)
{
  return std::unique_lock<L>(lockMe);
}

SyncIp::SyncIp(AL_TDriver* driver, char const* device) : driver{driver}
{
  fd = AL_Driver_Open(driver, device);

  if(fd == -1)
    throw runtime_error("Couldn't open the sync ip (trying to use " + string(device) + ")");

  struct xvsfsync_config config {};

  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_GET_CFG, &config) != DRIVER_SUCCESS)
    throw runtime_error("Couldn't get sync ip configuration");

  Log("driver", "[fd: %d] mode: %s, channel number: %d\n", fd, config.encode ? "encode" : "decode", config.max_channels);
  maxChannels = config.max_channels;
  channelStatuses.resize(config.max_channels);
  eventListeners.resize(config.max_channels);

  pollingThread = std::thread(&SyncIp::pollingRoutine, this);
}

SyncIp::~SyncIp()
{
  quit = true;
  pollingThread.join();
  AL_Driver_Close(driver, fd);
}

void SyncIp::getLatestChanStatus()
{
  u32 chan_status;

  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_GET_CHAN_STATUS, &chan_status) != DRIVER_SUCCESS)
    throw runtime_error("Couldn't get sync ip channel status");
  parseChanStatus(chan_status);
}

void SyncIp::resetStatus(int chanId)
{
  struct xvsfsync_clr_err clr;
  clr.channel_id = chanId;
  clr.sync_err = 1;
  clr.wdg_err = 1;

  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_CLR_CHAN_ERR, &clr) != DRIVER_SUCCESS)
    throw runtime_error("Couldnt reset status of channel " + to_string(chanId));
}

int SyncIp::getFreeChannel()
{
  auto lock = Lock(mutex);
  getLatestChanStatus();

  /* TODO(driver lowlat2 xilinx) give a non racy way to choose a free channel
   * For now we look if all the framebuffer of a channel are available to
   * decide if a channel is free or not
   */
  for(int i = 0; i < maxChannels; i++)
  {
    bool isAvailable = true;

    for(int j = 0; j < MAX_FB_NUMBER; ++j)
      isAvailable = isAvailable && channelStatuses[i].fbAvail[j];

    if(isAvailable)
      return i;
  }

  throw runtime_error("No channel available");
}

void SyncIp::enableChannel(int chanId)
{
  u8 chan = chanId;

  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_CHAN_ENABLE, (void*)(uintptr_t)chan) != DRIVER_SUCCESS)
    throw runtime_error("Couldn't enable channel");
}

void SyncIp::disableChannel(int chanId)
{
  u8 chan = chanId;

  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_CHAN_DISABLE, (void*)(uintptr_t)chan) != DRIVER_SUCCESS)
    throw runtime_error("Couldn't disable channel");
}

void SyncIp::addBuffer(struct xvsfsync_chan_config* fbConfig)
{
  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_SET_CHAN_CONFIG, fbConfig) != DRIVER_SUCCESS)
    throw sync_no_buf_slot_available();
}

void SyncIp::pollErrors(int timeout)
{
  AL_EDriverError retCode = AL_Driver_PostMessage(driver, fd, AL_POLL_MSG, &timeout);

  if(retCode == DRIVER_TIMEOUT)
    return;

  if(retCode != DRIVER_SUCCESS)
    Log("driver", "Error while polling the errors. (driver error: %d)\n", retCode);

  auto lock = Lock(mutex);
  getLatestChanStatus();

  for(size_t i = 0; i < channelStatuses.size(); ++i)
  {
    ChannelStatus& status = channelStatuses[i];

    if(eventListeners[i] && (status.syncError || status.watchdogError))
    {
      eventListeners[i] (status);
      resetStatus(i);
    }
  }
}

void SyncIp::pollingRoutine()
{
  while(true)
  {
    {
      Lock(mutex);

      if(quit)
        break;
    }
    pollErrors(5000);
  }

  pollErrors(0);
}

template<typename T>
void SyncIp::addListener(int chanId, T delegate)
{
  auto lock = Lock(mutex);
  eventListeners[chanId] = delegate;
}

void SyncIp::removeListener(int chanId)
{
  auto lock = Lock(mutex);
  eventListeners[chanId] = nullptr;
}

ChannelStatus & SyncIp::getStatus(int chanId)
{
  auto lock = Lock(mutex);
  getLatestChanStatus();
  return channelStatuses[chanId];
}

void SyncIp::parseChanStatus(u32 status)
{
  for(int i = 0; i < maxChannels; ++i)
  {
    auto& chan = channelStatuses[i];
    chan.fbAvail[0] = status & XVSFSYNC_CHX_FB0_MASK(i);
    chan.fbAvail[1] = status & XVSFSYNC_CHX_FB1_MASK(i);
    chan.fbAvail[2] = status & XVSFSYNC_CHX_FB2_MASK(i);
    chan.enable = status & XVSFSYNC_CHX_ENB_MASK(i);
    chan.syncError = status & XVSFSYNC_CHX_SYNC_ERR_MASK(i);
    chan.watchdogError = status & XVSFSYNC_CHX_WDG_ERR_MASK(i);
  }
}

void printFrameBufferConfig(struct xvsfsync_chan_config const& config)
{
  Log("framebuffer", "********************************\n");
  Log("framebuffer", "fb_id: %d, channel_id: %d\n", config.fb_id, config.channel_id);
  Log("framebuffer", "luma_start_address: %" PRIx64 "\n", config.luma_start_address);
  Log("framebuffer", "luma_end_address: %" PRIx64 "\n", config.luma_end_address);
  Log("framebuffer", "luma_margin %d\n", config.luma_margin);
  Log("framebuffer", "chroma_start_address: %" PRIx64 "\n", config.chroma_start_address);
  Log("framebuffer", "chroma_end_address: %" PRIx64 "\n", config.chroma_end_address);
  Log("framebuffer", "chroma_margin %d\n", config.chroma_margin);
  Log("framebuffer", "********************************\n");
}

static struct xvsfsync_chan_config setFrameBufferConfig(int channelId, AL_TBuffer* buf)
{
  AL_TSrcMetaData* srcMeta = (AL_TSrcMetaData*)AL_Buffer_GetMetaData(buf, AL_META_TYPE_SOURCE);
  AL_PADDR physical = AL_Allocator_GetPhysicalAddr(buf->pAllocator, buf->hBuf);

  if(!srcMeta)
    throw runtime_error("source buffer requires an AL_META_TYPE_SOURCE metadata");

  struct xvsfsync_chan_config config {};

  config.luma_start_address = physical + srcMeta->tOffsetYC.iLuma;

  /*           <------------> stride
   *           <--------> width
   * height   ^
   *          |
   *          |
   *          v         x last pixel of the image
   * end = (height - 1) * stride + width - 1 (to get the last pixel of the image)
   * total_size = height * stride
   * end = total_size - stride + width - 1
   */
  config.luma_end_address = config.luma_start_address + AL_SrcMetaData_GetLumaSize(srcMeta) - srcMeta->tPitches.iLuma + srcMeta->tDim.iWidth - 1;

  /* chroma is the same, but the width depends on the format of the yuv
   * here we make the assumption that the fourcc is semi planar */
  if(!AL_IsMonochrome(srcMeta->tFourCC))
  {
    assert(AL_IsSemiPlanar(srcMeta->tFourCC));
    config.chroma_start_address = physical + AL_SrcMetaData_GetOffsetC(srcMeta);
    config.chroma_end_address = config.chroma_start_address + AL_SrcMetaData_GetChromaSize(srcMeta) - srcMeta->tPitches.iChroma + srcMeta->tDim.iWidth - 1;
  }
  else
  {
    config.chroma_start_address = 0;
    config.chroma_end_address = 0;
    config.ismono = 1;
  }

  /* no margin for now (only needed for the decoder) */
  config.luma_margin = 0;
  config.chroma_margin = 0;

  config.fb_id = XVSFSYNC_AUTO_SEARCH;
  config.channel_id = channelId;

  return config;
}

SyncChannel::SyncChannel(SyncIp* sync, int id) :  id{id}, sync{sync}
{
  sync->addListener(id, [&](ChannelStatus& status)
  {
    Log("channel", "watchdog: %d, sync: %d\n", status.watchdogError, status.syncError);
  });
}

SyncChannel::~SyncChannel()
{
  if(enabled)
    disable();

  while(!buffers.empty())
  {
    auto& buf = buffers.front();
    buffers.pop();
    AL_Buffer_Unref(buf);
  }

  sync->removeListener(id);
}

void SyncChannel::addBuffer_(AL_TBuffer* buf, int numFbToEnable)
{
  if(buf)
  {
    /* we do not support adding buffer when the pipeline is running */
    assert(!isRunning);
    AL_Buffer_Ref(buf);
    buffers.push(buf);
  }

  /* If we don't want to start the ip yet, we do not program
   * the ip registers, we just keep the buffer in our queue
   *
   * When we start the channel, we try to add as much buffer
   * as we can inside the hw ip.
   *
   * Once we are running, we keep the same set of buffer and when
   * one of the buffer is finished we replace it with a new one from the queue
   * in a round robin fashion */

  while(isRunning && numFbToEnable > 0 && !buffers.empty())
  {
    buf = buffers.front();

    auto config = setFrameBufferConfig(id, buf);
    printFrameBufferConfig(config);

    try
    {
      sync->addBuffer(&config);
      Log("framebuffer", "Pushed buffer in sync ip\n");
      printChannelStatus(sync->getStatus(id));
      buffers.pop();
    }
    catch(sync_no_buf_slot_available const& error)
    {
      /* Will try again later */
      break;
    }

    buffers.push(buf);
    --numFbToEnable;
  }
}

void SyncChannel::addBuffer(AL_TBuffer* buf)
{
  auto lock = Lock(mutex);
  addBuffer_(buf, 1);
}

void SyncChannel::enable()
{
  auto lock = Lock(mutex);
  isRunning = true;
  auto numFbToEnable = std::min((int)buffers.size(), MAX_FB_NUMBER);
  addBuffer_(nullptr, numFbToEnable);
  sync->enableChannel(id);
  enabled = true;
  Log("channel", "Enable channel %d\n", id);
}

void SyncChannel::disable()
{
  if(!enabled)
    assert(0 == "Tried to disable a channel twice");

  sync->disableChannel(id);
  enabled = false;
  Log("channel", "Disable channel %d\n", id);
}

