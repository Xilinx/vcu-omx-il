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

#include "DummySyncDriver.h"
#include "SyncLog.h"
#include <cassert>

extern "C"
{
/* needed definition for xvsfsync.h */
#include <sys/ioctl.h>
#define BIT(x) (1 << (x))
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
#include "xvsfsync.h"
}

using namespace std;

bool isSane(struct xvsfsync_chan_config const& config, int maxChan)
{
  if(config.channel_id > maxChan)
    return false;

  if(config.fb_id != XVSFSYNC_AUTO_SEARCH && config.fb_id >= MAX_FB_NUMBER)
    return false;

  return true;
}

int findAvailableFrameBuffer(ChannelStatus const& channelStatus)
{
  for(int fbNum = 0; fbNum < MAX_FB_NUMBER; ++fbNum)
  {
    if(channelStatus.fbAvail[fbNum])
      return fbNum;
  }

  return -1;
}

int findFirstBusyFrameBuffer(ChannelStatus const& channelStatus)
{
  for(int fbNum = 0; fbNum < MAX_FB_NUMBER; ++fbNum)
  {
    if(!channelStatus.fbAvail[fbNum])
      return fbNum;
  }

  return -1;
}

static u32 encodeChannelStatus(vector<ChannelStatus>& channelStatuses)
{
  u32 status = 0;

  for(size_t i = 0; i < channelStatuses.size(); ++i)
  {
    auto& chan = channelStatuses[i];

    if(chan.fbAvail[0])
      status |= XVSFSYNC_CHX_FB0_MASK(i);

    if(chan.fbAvail[1])
      status |= XVSFSYNC_CHX_FB1_MASK(i);

    if(chan.fbAvail[2])
      status |= XVSFSYNC_CHX_FB2_MASK(i);

    if(chan.enable)
      status |= XVSFSYNC_CHX_ENB_MASK(i);

    if(chan.syncError)
      status |= XVSFSYNC_CHX_SYNC_ERR_MASK(i);

    if(chan.watchdogError)
      status |= XVSFSYNC_CHX_WDG_ERR_MASK(i);
  }

  return status;
}

static int sOpen(AL_TDriver* driver, const char* device)
{
  auto pThis = static_cast<DummyDriver*>(driver);
  return pThis->Open(device);
}

static void sClose(AL_TDriver* driver, int fd)
{
  auto pThis = static_cast<DummyDriver*>(driver);
  pThis->Close(fd);
}

static AL_EDriverError sPostMessage(AL_TDriver* driver, int fd, long unsigned int messageId, void* data)
{
  auto pThis = static_cast<DummyDriver*>(driver);
  return pThis->PostMessage(fd, messageId, data);
}

DummyDriver::DummyDriver()
{
  static const AL_DriverVtable myVtable =
  {
    &sOpen,
    &sClose,
    &sPostMessage,
  };

  AL_TDriver::vtable = &myVtable;
}

int DummyDriver::Open(const char* device)
{
  (void)device;
  return 1;
}

void DummyDriver::Close(int fd)
{
  (void)fd;
}

static AL_EDriverError xvsfsync_get_cfg(DummyDriver* pThis, struct xvsfsync_config* config)
{
  config->encode = pThis->encode;
  config->max_channels = pThis->numChan;
  return DRIVER_SUCCESS;
}

static AL_EDriverError xvsfsync_get_chan_status(DummyDriver* pThis, u32* channelStatus)
{
  *channelStatus = encodeChannelStatus(pThis->channelStatuses);
  return DRIVER_SUCCESS;
}

static AL_EDriverError xvsfsync_set_chan_config(DummyDriver* pThis, struct xvsfsync_chan_config* config)
{
  assert(isSane(*config, pThis->numChan));
  auto& channelStatus = pThis->channelStatuses[config->channel_id];
  int fbNum = config->fb_id;

  if(fbNum == XVSFSYNC_AUTO_SEARCH)
    fbNum = findAvailableFrameBuffer(channelStatus);

  if(fbNum == -1 || !channelStatus.fbAvail[fbNum])
    return DRIVER_ERROR_UNKNOWN;
  channelStatus.fbAvail[fbNum] = false;
  return DRIVER_SUCCESS;
}

static AL_EDriverError xvsfsync_chan_enable(DummyDriver* pThis, u8 chanId)
{
  assert(chanId <= pThis->numChan);
  pThis->channelStatuses[chanId].enable = true;
  return DRIVER_SUCCESS;
}

static AL_EDriverError xvsfsync_chan_disable(DummyDriver* pThis, u8 chanId)
{
  assert(chanId <= pThis->numChan);
  pThis->channelStatuses[chanId].enable = false;
  return DRIVER_SUCCESS;
}

static AL_EDriverError xvsfsync_clear_chan_errors(DummyDriver* pThis, struct xvsfsync_clr_err* clr)
{
  assert(clr->channel_id <= pThis->numChan);

  if(clr->sync_err)
    pThis->channelStatuses[clr->channel_id].syncError = false;

  if(clr->wdg_err)
    pThis->channelStatuses[clr->channel_id].watchdogError = false;
  return DRIVER_SUCCESS;
}

static AL_EDriverError xvsfsync_poll(DummyDriver* pThis)
{
  for(int i = 0; i < pThis->numChan; ++i)
  {
    auto& channelStatus = pThis->channelStatuses[i];

    if(channelStatus.watchdogError || channelStatus.syncError)
      return DRIVER_SUCCESS;
  }

  /* no errors here, we just didn't find anything */
  return DRIVER_TIMEOUT;
}

AL_EDriverError DummyDriver::PostMessage(int fd, long unsigned int messageId, void* data)
{
  (void)fd;
  switch(messageId)
  {
  case XVSFSYNC_GET_CFG:
    return xvsfsync_get_cfg(this, (struct xvsfsync_config*)data);

  case XVSFSYNC_GET_CHAN_STATUS:
    return xvsfsync_get_chan_status(this, (u32*)data);

  case XVSFSYNC_SET_CHAN_CONFIG:
    return xvsfsync_set_chan_config(this, (struct xvsfsync_chan_config*)data);

  case XVSFSYNC_CHAN_ENABLE:
    return xvsfsync_chan_enable(this, (u8)(uintptr_t)data);

  case XVSFSYNC_CHAN_DISABLE:
    return xvsfsync_chan_disable(this, (u8)(uintptr_t)data);

  case XVSFSYNC_CLR_CHAN_ERR:
    return xvsfsync_clear_chan_errors(this, (struct xvsfsync_clr_err*)data);

  case AL_POLL_MSG:
    return xvsfsync_poll(this);

  default:
    return DRIVER_ERROR_UNKNOWN;
  }
}

void DummyDriver::FinalizeBuffer(int chanId, int fb_id)
{
  auto& channelStatus = channelStatuses[chanId];

  if(!channelStatus.enable)
    throw runtime_error("Can't finalize a buffer if the channel isn't enabled");

  if(fb_id == -1)
    fb_id = findFirstBusyFrameBuffer(channelStatus);

  auto& fbAvail = channelStatus.fbAvail[fb_id];

  if(fbAvail)
    throw runtime_error("Frame buffer isn't busy");
  fbAvail = true;

  Log("framebuffer", "Finalize framebuffer id %d for channel %d\n", fb_id, chanId);
}

void DummyDriver::SignalSyncError(int chanId)
{
  channelStatuses[chanId].syncError = true;
}

void DummyDriver::SignalWatchdogError(int chanId)
{
  channelStatuses[chanId].watchdogError = true;
}

static DummyDriver dummyDriver;

DummyDriver* AL_InitDummyDriver(bool encode, int numChan)
{
  dummyDriver.encode = encode;
  dummyDriver.numChan = numChan;
  dummyDriver.channelStatuses.resize(numChan);

  for(auto& channelStatus : dummyDriver.channelStatuses)
  {
    for(auto& fbAvail : channelStatus.fbAvail)
      fbAvail = true;
  }

  return &dummyDriver;
}

DummyDriver* AL_GetDummyDriver()
{
  return &dummyDriver;
}

