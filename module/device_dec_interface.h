#pragma once

#include "module_structs.h"

extern "C"
{
#include <lib_decode/lib_decode.h>
}

struct DecDeviceInterface
{
  virtual ~DecDeviceInterface() = 0;

  virtual AL_IDecScheduler* Init() = 0;
  virtual void Deinit() = 0;
  virtual BufferContiguities GetBufferContiguities() const = 0;
  virtual BufferBytesAlignments GetBufferBytesAlignments() const = 0;
};
