/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX OR ALLEGRO DVT2 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of  Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
* Except as contained in this notice, the name of Allegro DVT2 shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Allegro DVT2.
*
******************************************************************************/

#include "omx_module_dummy.h"

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

ModuleInterface::ErrorType DummyModule::Run(bool)
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

