// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

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

static int32_t constexpr MAX_FB_NUMBER = 3;
static int32_t constexpr MAX_USER = 2; /* consumer and producter */

struct ChannelStatus
{
  bool fbAvail[MAX_FB_NUMBER][MAX_USER];
  bool enable;
  bool syncError;
  bool watchdogError;
  bool lumaDiffError;
  bool chromaDiffError;
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
  int32_t getFreeChannel();
  void enableChannel(int32_t chanId);
  void disableChannel(int32_t chanId);
  void addBuffer(struct xvsfsync_chan_config* fbConfig);

  template<typename T>
  void addListener(int32_t chanId, T delegate);
  void removeListener(int32_t chanId);
  ChannelStatus& getStatus(int32_t chanId);
  int32_t maxChannels;
  int32_t maxUsers;
  int32_t maxBuffers;
  int32_t maxCores;

private:
  void getLatestChanStatus();
  void resetStatus(int32_t chanId);
  int32_t fd = -1;
  bool quit = false;
  std::thread pollingThread;
  void pollingRoutine();
  void pollErrors(int32_t timeout);

  AL_TDriver* driver;
  std::mutex mutex {};
  std::vector<std::function<void(ChannelStatus &)>> eventListeners {};
  std::vector<ChannelStatus> channelStatuses {};
};

struct SyncChannel
{
  SyncChannel(SyncIp* sync, int32_t id);
  virtual ~SyncChannel();
  virtual void addBuffer(AL_TBuffer* buf) = 0;
  virtual void enable() = 0;
  void disable();

  int32_t id;

protected:
  bool enabled = false;
  SyncIp* sync;
};

struct EncSyncChannel : SyncChannel
{
  EncSyncChannel(SyncIp* sync, int32_t id, int32_t hardwareHorizontalStrideAlignment, int32_t hardwareVerticalStrideAlignment);
  ~EncSyncChannel();
  void addBuffer(AL_TBuffer* buf) override;
  void enable() override;

private:
  std::queue<AL_TBuffer*> buffers {};
  std::mutex mutex {};
  bool isRunning = false;
  int32_t const hardwareHorizontalStrideAlignment;
  int32_t const hardwareVerticalStrideAlignment;

  void addBuffer_(AL_TBuffer* buf, int32_t numFbToEnable);
};

struct DecSyncChannel : SyncChannel
{
  DecSyncChannel(SyncIp* sync, int32_t id);
  ~DecSyncChannel();
  void addBuffer(AL_TBuffer* buf) override;
  void enable() override;
};
