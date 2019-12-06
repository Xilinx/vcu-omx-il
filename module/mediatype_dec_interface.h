#pragma once

#include "mediatype_interface.h"
#include "module_structs.h"

extern "C"
{
#include <lib_decode/lib_decode.h>
}

struct DecMediatypeInterface : MediatypeInterface
{
  virtual ~DecMediatypeInterface() override = default;

  virtual void Reset() override = 0;
  virtual ErrorType Get(std::string index, void* settings) const override = 0;
  virtual ErrorType Set(std::string index, void const* settings) override = 0;

  AL_TDecSettings settings;
  Stride stride;
};
