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

#pragma once

extern "C"
{
#include "lib_common/IDriver.h"
#include "lib_common/BufferAPI.h"
}

#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <stdexcept>

static int constexpr MAX_FB_NUMBER = 3;
struct ChannelStatus
{
  bool fbAvail[MAX_FB_NUMBER];
  bool enable;
  bool syncError;
  bool watchdogError;
};

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

struct SyncIp
{
  SyncIp(AL_TDriver* driver, char const* device);
  ~SyncIp();
  int getFreeChannel();
  void enableChannel(int chanId);
  void disableChannel(int chanId);
  void addBuffer(struct xvsfsync_chan_config* fbConfig);

  template<typename T>
  void addListener(int chanId, T delegate);
  void removeListener(int chanId);
  ChannelStatus& getStatus(int chanId);
  int maxChannels;

private:
  void getLatestChanStatus();
  void parseChanStatus(uint32_t status);
  void resetStatus(int chanId);
  int fd = -1;
  bool quit = false;
  std::thread pollingThread;
  void pollingRoutine();
  void pollErrors(int timeout);

  AL_TDriver* driver;
  std::mutex mutex {};
  std::vector<std::function<void(ChannelStatus &)>> eventListeners {};
  std::vector<ChannelStatus> channelStatuses {};
};

struct SyncChannel
{
  SyncChannel(SyncIp* sync, int id);
  virtual ~SyncChannel();
  virtual void addBuffer(AL_TBuffer* buf) = 0;
  virtual void enable() = 0;
  void disable();

  int id;

protected:
  bool enabled = false;
  SyncIp* sync;
};

struct EncSyncChannel : SyncChannel
{
  EncSyncChannel(SyncIp* sync, int id);
  ~EncSyncChannel();
  void addBuffer(AL_TBuffer* buf) override;
  void enable() override;

private:
  std::queue<AL_TBuffer*> buffers {};
  std::mutex mutex {};
  bool isRunning = false;

  void addBuffer_(AL_TBuffer* buf, int numFbToEnable);
};

struct DecSyncChannel : SyncChannel
{
  DecSyncChannel(SyncIp* sync, int id);
  ~DecSyncChannel();
  void addBuffer(AL_TBuffer* buf) override;
  void enable() override;
};

