// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "module_dummy.h"

DummyModule::DummyModule() = default;
DummyModule::~DummyModule() = default;

bool DummyModule::SetCallbacks(Callbacks const callbacks)
{
  c.emptied = callbacks.emptied;
  c.associate = callbacks.associate;
  c.filled = callbacks.filled;
  c.event = callbacks.event;
  return true;
}

void DummyModule::Free(void* buffer)
{
  if(!buffer)
    return;

  free(buffer);
}

void* DummyModule::Allocate(size_t size)
{
  return malloc(size);
}

bool DummyModule::Empty(BufferHandleInterface* handle)
{
  if(!handle)
    return false;

  auto buffer = handle->data;

  if(!buffer)
    return false;

  handle->offset = 0;
  handle->payload = 0;
  c.emptied(handle);
  return true;
}

bool DummyModule::Fill(BufferHandleInterface* handle)
{
  if(!handle)
    return false;

  auto buffer = handle->data;
  handle->offset = 0;
  handle->payload = 1;

  if(!buffer)
    return false;

  c.filled(handle);
  return true;
}

bool DummyModule::Stop()
{
  return true;
}

ModuleInterface::ErrorType DummyModule::Start(bool)
{
  return SUCCESS;
}

ModuleInterface::ErrorType DummyModule::Restart()
{
  return SUCCESS;
}

ModuleInterface::ErrorType DummyModule::SetDynamic(std::string index, void const* param)
{
  (void)index;
  (void)param;
  return NOT_IMPLEMENTED;
}

ModuleInterface::ErrorType DummyModule::GetDynamic(std::string index, void* param)
{
  (void)index;
  (void)param;
  return NOT_IMPLEMENTED;
}
