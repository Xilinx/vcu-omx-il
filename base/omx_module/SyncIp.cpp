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

/* for u64 print format */
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "SyncIp.h"
#include <cassert>
#include <iostream>
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
#include "lib_rtos/lib_rtos.h"
}

#include "SyncLog.h"

static int RoundUp(int iVal, int iRnd)
{
  assert((iRnd % 2) == 0);
  return (iVal + iRnd - 1) / iRnd * iRnd;
}

using namespace std;

static void parseChanStatus(struct xvsfsync_stat const& status, vector<ChannelStatus>& channelStatuses, int maxChannels, int maxUsers, int maxBuffers)
{
  for(int channel = 0; channel < maxChannels; ++channel)
  {
    auto& channelStatus = channelStatuses[channel];

    for(int buffer = 0; buffer < maxBuffers; ++buffer)
    {
      for(int user = 0; user < maxUsers; ++user)
        channelStatus.fbAvail[buffer][user] = status.fbdone[channel][buffer][user];
    }

    channelStatus.enable = status.enable[channel];
    channelStatus.syncError = status.sync_err[channel];
    channelStatus.watchdogError = status.wdg_err[channel];
    channelStatus.lumaDiffError = status.ldiff_err[channel];
    channelStatus.chromaDiffError = status.cdiff_err[channel];
  }
}

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
  maxUsers = XVSFSYNC_IO;
  maxBuffers = XVSFSYNC_BUF_PER_CHANNEL;
  maxCores = XVSFSYNC_MAX_CORES;
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
  struct xvsfsync_stat chan_status;

  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_GET_CHAN_STATUS, &chan_status) != DRIVER_SUCCESS)
    throw runtime_error("Couldn't get sync ip channel status");
  parseChanStatus(chan_status, channelStatuses, maxChannels, maxUsers, maxBuffers);
}

void SyncIp::resetStatus(int chanId)
{
  struct xvsfsync_clr_err clr;
  clr.channel_id = chanId;
  clr.sync_err = 1;
  clr.wdg_err = 1;
  clr.ldiff_err = 1;
  clr.cdiff_err = 1;

  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_CLR_CHAN_ERR, &clr) != DRIVER_SUCCESS)
    throw runtime_error("Couldnt reset status of channel " + to_string(chanId));
}

int SyncIp::getFreeChannel()
{
  auto lock = Lock(mutex);
  u8 chanId;
  getLatestChanStatus();


  if(AL_Driver_PostMessage(driver, fd, XVSFSYNC_RESERVE_GET_CHAN_ID, &chanId) != DRIVER_SUCCESS)
    throw runtime_error("Couldn't get sync ip channel ID");

    return chanId;
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

    if(eventListeners[i] && (status.syncError || status.watchdogError || status.lumaDiffError || status.chromaDiffError))
    {
      eventListeners[i] (status);
      resetStatus(i);
    }
  }
}

void SyncIp::pollingRoutine()
{
  Rtos_SetCurrentThreadName("SyncIP - Poll");

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

static char const* ToStringUser(int user)
{
  if(user == XVSFSYNC_PROD)
    return "prod";

  if(user == XVSFSYNC_CONS)
    return "cons";
  return "unknown";
}

static void printFrameBufferConfig(struct xvsfsync_chan_config const& config, int maxUsers, int maxCores)
{
  Log("framebuffer", "********************************\n");
  Log("framebuffer", "channel_id:%d\n", config.channel_id);
  Log("framebuffer", "luma_margin:%d\n", config.luma_margin);
  Log("framebuffer", "chroma_margin:%d\n", config.chroma_margin);

  for(int user = 0; user < maxUsers; ++user)
  {
    Log("framebuffer", "%s[%d]:\n", ToStringUser(user), user);
    Log("framebuffer", "\t-fb_id:%d %s\n", config.fb_id[user], config.fb_id[user] == XVSFSYNC_AUTO_SEARCH ? "(auto_search)" : "");
    Log("framebuffer", "\t-ismono:%s\n", (config.ismono[user] == 0) ? "false" : "true");
    Log("framebuffer", "\t-luma_start_address:%" PRIx64 "\n", config.luma_start_address[user]);
    Log("framebuffer", "\t-luma_end_address:%" PRIx64 "\n", config.luma_end_address[user]);
    Log("framebuffer", "\t-chroma_start_address:%" PRIx64 "\n", config.chroma_start_address[user]);
    Log("framebuffer", "\t-chroma_end_address:%" PRIx64 "\n", config.chroma_end_address[user]);
  }

  for(int core = 0; core < maxCores; ++core)
  {
    Log("framebuffer", "core[%i]:\n", core);
    Log("framebuffer", "\t-luma_core_offset:%d\n", config.luma_core_offset[core]);
    Log("framebuffer", "\t-chroma_core_offset:%d\n", config.chroma_core_offset[core]);
  }

  Log("framebuffer", "********************************\n");
}

static int widthToRow(int width, TFourCC fourCC)
{
  return AL_Is10bitPacked(fourCC) ? ((width + 2) / 3 * 4) : width* AL_GetPixelSize(fourCC);
}

static struct xvsfsync_chan_config setEncFrameBufferConfig(int channelId, AL_TBuffer* buf, int hardwareHorizontalStrideAlignment, int hardwareVerticalStrideAlignment)
{
  AL_TSrcMetaData* srcMeta = (AL_TSrcMetaData*)AL_Buffer_GetMetaData(buf, AL_META_TYPE_SOURCE);
  AL_PADDR physical = AL_Allocator_GetPhysicalAddr(buf->pAllocator, buf->hBuf);

  if(!srcMeta)
    throw runtime_error("source buffer requires an AL_META_TYPE_SOURCE metadata");

  struct xvsfsync_chan_config config {};

  config.luma_start_address[XVSFSYNC_PROD] = physical + srcMeta->tPlanes[AL_PLANE_Y].iOffset;
  int srcRow = widthToRow(srcMeta->tDim.iWidth, srcMeta->tFourCC);
  config.luma_end_address[XVSFSYNC_PROD] = config.luma_start_address[XVSFSYNC_PROD] + AL_SrcMetaData_GetLumaSize(srcMeta) - srcMeta->tPlanes[AL_PLANE_Y].iPitch + srcRow - 1;

  config.luma_start_address[XVSFSYNC_CONS] = physical + srcMeta->tPlanes[AL_PLANE_Y].iOffset;
  /*           <------------> stride
   *           <--------> row
   * height   ^
   *          |
   *          |
   *          v         x last pixel of the image
   * end = (height - 1) * stride + row - 1 (to get the last pixel of the image)
   * total_size = height * stride
   * end = total_size - stride + row - 1
   */
  int iHardwarePitch = RoundUp(srcMeta->tPlanes[AL_PLANE_Y].iPitch, hardwareHorizontalStrideAlignment);
  int iHardwareLumaVerticalPitch = RoundUp(srcMeta->tDim.iHeight, hardwareVerticalStrideAlignment);
  config.luma_end_address[XVSFSYNC_CONS] = config.luma_start_address[XVSFSYNC_CONS] + (iHardwarePitch * (iHardwareLumaVerticalPitch - 1)) + RoundUp(srcRow, hardwareHorizontalStrideAlignment) - 1;

  /* chroma is the same, but the row depends on the format of the yuv
   * here we make the assumption that the fourcc is semi planar */
  if(!AL_IsMonochrome(srcMeta->tFourCC))
  {
    assert(AL_IsSemiPlanar(srcMeta->tFourCC));
    config.chroma_start_address[XVSFSYNC_PROD] = physical + AL_SrcMetaData_GetOffsetUV(srcMeta);
    config.chroma_end_address[XVSFSYNC_PROD] = config.chroma_start_address[XVSFSYNC_PROD] + AL_SrcMetaData_GetChromaSize(srcMeta) - srcMeta->tPlanes[AL_PLANE_UV].iPitch + srcRow - 1;
    config.chroma_start_address[XVSFSYNC_CONS] = physical + AL_SrcMetaData_GetOffsetUV(srcMeta);
    int iVerticalFactor = (AL_GetChromaMode(srcMeta->tFourCC) == AL_CHROMA_4_2_0) ? 2 : 1;
    int iHardwareChromaVerticalPitch = RoundUp((srcMeta->tDim.iHeight / iVerticalFactor), (hardwareVerticalStrideAlignment / iVerticalFactor));
    config.chroma_end_address[XVSFSYNC_CONS] = config.chroma_start_address[XVSFSYNC_CONS] + (iHardwarePitch * (iHardwareChromaVerticalPitch - 1)) + RoundUp(srcRow, hardwareHorizontalStrideAlignment) - 1;
  }
  else
  {
    for(int user = 0; user < XVSFSYNC_IO; user++)
    {
      config.chroma_start_address[user] = 0;
      config.chroma_end_address[user] = 0;
      config.ismono[user] = 1;
    }
  }

  for(int core = 0; core < XVSFSYNC_MAX_CORES; core++)
  {
    config.luma_core_offset[core] = 0;
    config.chroma_core_offset[core] = 0;
  }

  /* no margin for now (only needed for the decoder) */
  config.luma_margin = 0;
  config.chroma_margin = 0;

  config.fb_id[XVSFSYNC_PROD] = XVSFSYNC_AUTO_SEARCH;
  config.fb_id[XVSFSYNC_CONS] = XVSFSYNC_AUTO_SEARCH;
  config.channel_id = channelId;

  return config;
}

static struct xvsfsync_chan_config setDecFrameBufferConfig(int channelId, AL_TBuffer* buf)
{
  AL_TSrcMetaData* srcMeta = (AL_TSrcMetaData*)AL_Buffer_GetMetaData(buf, AL_META_TYPE_SOURCE);
  AL_PADDR physical = AL_Allocator_GetPhysicalAddr(buf->pAllocator, buf->hBuf);

  if(!srcMeta)
    throw runtime_error("source buffer requires an AL_META_TYPE_SOURCE metadata");

  struct xvsfsync_chan_config config {};

  config.luma_start_address[XVSFSYNC_PROD] = physical + srcMeta->tPlanes[AL_PLANE_Y].iOffset;

  /*           <------------> stride
   *           <--------> row
   * height   ^
   *          |
   *          |
   *          v         x last pixel of the image
   * end = (height - 1) * stride + row - 1 (to get the last pixel of the image)
   * total_size = height * stride
   * end = total_size - stride + row - 1
   */
  // TODO : This should be LCU and 64 aligned
  int srcRow = widthToRow(srcMeta->tDim.iWidth, srcMeta->tFourCC);
  config.luma_end_address[XVSFSYNC_PROD] = config.luma_start_address[XVSFSYNC_PROD] + AL_SrcMetaData_GetLumaSize(srcMeta) - srcMeta->tPlanes[AL_PLANE_Y].iPitch + srcRow - 1;
  config.luma_start_address[XVSFSYNC_CONS] = physical + srcMeta->tPlanes[AL_PLANE_Y].iOffset;
  config.luma_end_address[XVSFSYNC_CONS] = config.luma_start_address[XVSFSYNC_CONS] + AL_SrcMetaData_GetLumaSize(srcMeta) - srcMeta->tPlanes[AL_PLANE_Y].iPitch + srcRow - 1;

  /* chroma is the same, but the row depends on the format of the yuv
   * here we make the assumption that the fourcc is semi planar */
  if(!AL_IsMonochrome(srcMeta->tFourCC))
  {
    assert(AL_IsSemiPlanar(srcMeta->tFourCC));
    config.chroma_start_address[XVSFSYNC_PROD] = physical + AL_SrcMetaData_GetOffsetUV(srcMeta);
    // TODO : This should be LCU and 64 aligned
    config.chroma_end_address[XVSFSYNC_PROD] = config.chroma_start_address[XVSFSYNC_PROD] + AL_SrcMetaData_GetChromaSize(srcMeta) - srcMeta->tPlanes[AL_PLANE_UV].iPitch + srcRow - 1;
    config.chroma_start_address[XVSFSYNC_CONS] = physical + AL_SrcMetaData_GetOffsetUV(srcMeta);
    config.chroma_end_address[XVSFSYNC_CONS] = config.chroma_start_address[XVSFSYNC_CONS] + AL_SrcMetaData_GetChromaSize(srcMeta) - srcMeta->tPlanes[AL_PLANE_UV].iPitch + srcRow - 1;
  }
  else
  {
    for(int user = 0; user < XVSFSYNC_IO; user++)
    {
      config.chroma_start_address[user] = 0;
      config.chroma_end_address[user] = 0;
      config.ismono[user] = 1;
    }
  }

  for(int core = 0; core < XVSFSYNC_MAX_CORES; core++)
  {
    config.luma_core_offset[core] = 0;
    config.chroma_core_offset[core] = 0;
  }

  /* no margin for now (only needed for the decoder) */
  config.luma_margin = 0;
  config.chroma_margin = 0;

  config.fb_id[XVSFSYNC_PROD] = XVSFSYNC_AUTO_SEARCH;
  config.fb_id[XVSFSYNC_CONS] = XVSFSYNC_AUTO_SEARCH;
  config.channel_id = channelId;

  return config;
}

SyncChannel::SyncChannel(SyncIp* sync, int id) :  id{id}, sync{sync}
{
  sync->addListener(id, [&](ChannelStatus& status)
  {
    Log("channel", "watchdog: %d, sync: %d, ldiff: %d, cdiff: %d\n", status.watchdogError, status.syncError, status.lumaDiffError, status.chromaDiffError);
  });
}

SyncChannel::~SyncChannel()
{
  if(enabled)
    disable();

  sync->removeListener(id);
}

EncSyncChannel::EncSyncChannel(SyncIp* sync, int id, int hardwareHorizontalStrideAlignment, int hardwareVerticalStrideAlignment) :  SyncChannel{sync, id}, hardwareHorizontalStrideAlignment{hardwareHorizontalStrideAlignment}, hardwareVerticalStrideAlignment{hardwareVerticalStrideAlignment}
{
}

EncSyncChannel::~EncSyncChannel()
{
  if(enabled)
    disable();

  enabled = false;

  while(!buffers.empty())
  {
    auto& buf = buffers.front();
    buffers.pop();
    AL_Buffer_Unref(buf);
  }
}

void SyncChannel::disable()
{
  if(!enabled)
    assert(0 == "Tried to disable a channel twice");

  sync->disableChannel(id);
  enabled = false;
  Log("channel", "Disable channel %d\n", id);
}

void EncSyncChannel::addBuffer_(AL_TBuffer* buf, int numFbToEnable)
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

    auto config = setEncFrameBufferConfig(id, buf, hardwareHorizontalStrideAlignment, hardwareVerticalStrideAlignment);
    printFrameBufferConfig(config, sync->maxUsers, sync->maxCores);

    sync->addBuffer(&config);
    Log("framebuffer", "Pushed buffer in sync ip\n");
    printChannelStatus(sync->getStatus(id));
    buffers.pop();

    buffers.push(buf);
    --numFbToEnable;
  }
}

void EncSyncChannel::addBuffer(AL_TBuffer* buf)
{
  auto lock = Lock(mutex);
  addBuffer_(buf, 1);
}

void EncSyncChannel::enable()
{
  auto lock = Lock(mutex);
  isRunning = true;
  auto numFbToEnable = std::min((int)buffers.size(), sync->maxBuffers);
  addBuffer_(nullptr, numFbToEnable);
  sync->enableChannel(id);
  enabled = true;
  Log("channel", "Enable channel %d\n", id);
}

void DecSyncChannel::addBuffer(AL_TBuffer* buf)
{
  auto config = setDecFrameBufferConfig(id, buf);
  printFrameBufferConfig(config, sync->maxUsers, sync->maxCores);

  sync->addBuffer(&config);
  Log("framebuffer", "Pushed buffer in sync ip\n");
  printChannelStatus(sync->getStatus(id));
}

void DecSyncChannel::enable()
{
  sync->enableChannel(id);
  enabled = true;
}

DecSyncChannel::DecSyncChannel(SyncIp* sync, int id) :  SyncChannel{sync, id}
{
}

DecSyncChannel::~DecSyncChannel()
{
}

