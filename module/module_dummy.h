// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include "module_interface.h"

struct DummyModule final : ModuleInterface
{
  DummyModule();
  ~DummyModule() override;

  void Free(void* buffer) override;
  void* Allocate(size_t size) override;

  bool SetCallbacks(Callbacks const callbacks) override;

  bool Empty(BufferHandleInterface* handle) override;
  bool Fill(BufferHandleInterface* handle) override;

  ErrorType Start(bool) override;
  bool Stop() override;
  ErrorType Restart() override;

  ErrorType SetDynamic(std::string index, void const* param) override;
  ErrorType GetDynamic(std::string index, void* param) override;

private:
  Callbacks c;
};
