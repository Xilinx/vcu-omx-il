#pragma once

#include "mediatype_interface.h"

struct DummyMediatype final : MediatypeInterface
{
  DummyMediatype();
  ~DummyMediatype() override;

  void Reset() override;
  ErrorType Get(std::string, void*) const override;
  ErrorType Set(std::string, void const*) override;
  bool Check() override;
};
