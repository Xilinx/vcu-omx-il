#pragma once

#include "module_structs.h"

extern "C"
{
#include <lib_encode/lib_encoder.h>
}

struct EncDeviceInterface
{
  virtual ~EncDeviceInterface() = 0;

  virtual AL_IEncScheduler* Init() = 0;
  virtual void Deinit() = 0;
  virtual BufferContiguities GetBufferContiguities() const = 0;
  virtual BufferBytesAlignments GetBufferBytesAlignments() const = 0;
};
