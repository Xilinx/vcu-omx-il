// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once
#include <map>
#include "SyncIp.hpp"

struct LogCategories
{
  static LogCategories& getInstance(void)
  {
    static LogCategories instance;
    return instance;
  }

  std::map<char const*, bool> categories;

  LogCategories(const LogCategories &) = delete;
  LogCategories & operator = (const LogCategories &) = delete;
  LogCategories(const LogCategories &&) = delete;
  LogCategories & operator = (const LogCategories &&) = delete;

private:
  LogCategories();
  ~LogCategories();
};

#define Log(category, ...) \
  do \
  { \
    if(LogCategories::getInstance().categories[category]) \
      printf(__VA_ARGS__); \
  } \
  while(0)

void printChannelStatus(ChannelStatus const& status);
void printChannelStatus(int32_t channelId, ChannelStatus const& status);
void printAllChannelStatuses(SyncIp* syncIp);
