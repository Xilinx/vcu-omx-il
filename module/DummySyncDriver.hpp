// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

extern "C"
{
#include "lib_common/I_Communication.h"
}

#include <vector>

#include "SyncIp.hpp"

struct DummyDriver : public AL_ICommunication
{
  DummyDriver();
  virtual ~DummyDriver() {};

  int32_t Open(const char* device);
  void Close(int32_t fd);
  AL_ECommunicationError PostMessage(int32_t fd, uint32_t messageId, void* data, bool isBlocking);

  /* mock-up interface */

  void FinalizeBuffer(int32_t chanId, int32_t fb_id = -1);
  void SignalSyncError(int32_t chanId);
  void SignalWatchdogError(int32_t chanId);
  void SignalLumaDiffError(int32_t chanId);
  void SignalChromaDiffError(int32_t chanId);
  bool encode = true;
  int32_t numChan = 4;
  std::vector<ChannelStatus> channelStatuses {};
};

DummyDriver* AL_InitDummyDriver(bool encode, int32_t numChan);
DummyDriver* AL_GetDummyDriver();
