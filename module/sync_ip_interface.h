#pragma once

#include "buffer_handle_interface.h"

struct SyncIpInterface
{
  virtual ~SyncIpInterface() = 0;

  virtual bool create() = 0;
  virtual void destroy() = 0;

  virtual void addBuffer(BufferHandleInterface* buffer) = 0;
  virtual void enable() = 0;
};
