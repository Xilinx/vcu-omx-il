/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
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

#include "omx_module_copy.h"
#include "base/omx_checker/omx_checker.h"
#include <assert.h>
#include <functional>

struct CopyTask
{
  uint8_t* input;
  uint8_t* output;
};

static void SetDefaultResolution(Resolution& r)
{
  r.width = 176;
  r.height = 144;
  r.stride = 192;
  r.sliceHeight = 144;
}

static void SetDefaultClock(Clock& c)
{
  c.framerate = 0;
  c.clockratio = 0;
}

static void SetDefaultFormat(Format& f)
{
  f.mime = "video/x-raw";
  f.compression = COMPRESSION_UNUSED;
  f.color = COLOR_420;
  f.bitdepth = 8;
}

static void reset(Resolutions& resolutions, Formats& formats, Clocks& clocks)
{
  SetDefaultResolution(resolutions.input);
  SetDefaultResolution(resolutions.output);
  SetDefaultClock(clocks.input);
  SetDefaultClock(clocks.output);
  SetDefaultFormat(formats.input);
  SetDefaultFormat(formats.output);
}

static void SetDefaultBufferDefinition(BufferDefinitions& definition)
{
  definition.min = 1;
  definition.size = 512;
  definition.bytesAlignment = 0;
  definition.contiguous = false;
}

CopyModule::CopyModule()
{
  SetDefaultBufferDefinition(reqBuf.input);
  SetDefaultBufferDefinition(reqBuf.output);
  reset(resolutions, formats, clocks);
  isCreated = false;
  callbacks =
  {
    0
  };
  eos = true;
}

void CopyModule::_Process(void* data)
{
  if(!data)
  {
    std::unique_lock<std::mutex> lck(mutex);
    eos = true;
    cv.notify_one();
    return;
  }

  auto task = static_cast<CopyTask*>(data);
  assert(task);

  auto handleIn = task->input;
  assert(handleIn);

  auto handleOut = task->output;
  assert(handleOut);

  if(callbacks.associate)
    callbacks.associate(handleIn, handleOut);

  auto input = buffers.Get(handleIn);
  assert(input);

  auto output = buffers.Get(handleOut);
  assert(output);

  assert(output->size >= input->copy);
  memcpy(output->buffer, input->buffer, input->copy);
  output->copy = input->copy;
  input->copy = 0;

  if(callbacks.emptied)
    callbacks.emptied(handleIn, 0, 0);

  RemoveBuffer(handleIn);

  if(callbacks.filled)
    callbacks.filled(handleOut, 0, output->copy);

  RemoveBuffer(handleOut);

  delete task;
}

void CopyModule::_Delete(void* data)
{
  (void)data;
}

bool CopyModule::Create()
{
  if(isCreated)
    return false;

  auto const p = std::bind(&CopyModule::_Process, this, std::placeholders::_1);
  auto const d = std::bind(&CopyModule::_Delete, this, std::placeholders::_1);
  eos = false;
  processor.reset(new ProcessorFifo(p, d));
  isCreated = true;
  return true;
}

void CopyModule::Destroy()
{
  if(!isCreated)
    return;

  processor.reset();
  isCreated = false;
}

bool CopyModule::Run()
{
  if(!isCreated)
    return false;

  return true;
}

bool CopyModule::Pause()
{
  return false;
}

void CopyModule::Stop()
{
  if(!isCreated)
    return;
}

void CopyModule::ResetRequirements()
{
  reset(resolutions, formats, clocks);
}

BuffersRequirements CopyModule::GetBuffersRequirements() const
{
  return reqBuf;
}

bool CopyModule::SetCallbacks(Callbacks callbacks)
{
  if(!callbacks.emptied || !callbacks.associate || !callbacks.filled)
    return false;

  this->callbacks.emptied = callbacks.emptied;
  this->callbacks.associate = callbacks.associate;
  this->callbacks.filled = callbacks.filled;
  return true;
}

bool CopyModule::Flush()
{
  std::unique_lock<std::mutex> lck(mutex);

  for(auto index = 0; index < 2; index++)
  {
    auto queue = (index == 0) ? input : output;

    while(queue.size() != 0)
    {
      auto handle = queue.front();

      if(index == 0)
        callbacks.emptied(handle, 0, 0);
      else
        callbacks.filled(handle, 0, 0);
      queue.pop();
    }
  }

  processor->queue(nullptr);
  cv.wait(lck, [&] { return eos;
          });
  return true;
}

void CopyModule::Free(void* buffer)
{
  if(!buffer)
    return;

  free(buffer);
}

void CopyModule::RemoveBuffer(void* handle)
{
  auto copy = buffers.Get(handle);

  if(!copy)
  {
    assert(0);
    return;
  }

  buffers.Remove(handle);
  delete copy;
}

void CopyModule::AddBuffer(void* handle, void* buffer, int size)
{
  if(buffers.Exist(handle))
    return;

  auto copy = new CopyBuffer {
    buffer, 0, size
  };
  buffers.Add(handle, copy);
}

void* CopyModule::Allocate(size_t size)
{
  return malloc(size);
}

bool CopyModule::Empty(uint8_t* buffer, int offset, int size)
{
  (void)offset;

  if(!buffer)
    return false;

  AddBuffer(buffer, buffer, size);

  auto input = buffers.Get(buffer);
  assert(input);
  input->copy = size;
  {
    std::lock_guard<std::mutex> lock(mutex);
    this->input.push(buffer);
    CreateOneCopyTask();
  }

  return true;
}

bool CopyModule::Fill(uint8_t* buffer, int offset, int size)
{
  (void)offset;
  (void)size;

  if(!buffer)
    return false;

  AddBuffer(buffer, buffer, size);

  auto output = buffers.Get(buffer);
  assert(output);
  output->copy = 0;
  {
    std::lock_guard<std::mutex> lock(mutex);
    this->output.push(buffer);
    CreateOneCopyTask();
  }

  return true;
}

void CopyModule::CreateOneCopyTask()
{
  if(input.empty() || output.empty())
    return;

  auto handleIn = this->input.front();
  assert(handleIn);
  this->input.pop();

  auto handleOut = this->output.front();
  assert(handleOut);
  this->output.pop();

  processor->queue(new CopyTask { handleIn, handleOut });
}

Resolutions CopyModule::GetResolutions() const
{
  return resolutions;
}

Clocks CopyModule::GetClocks() const
{
  return clocks;
}

Formats CopyModule::GetFormats() const
{
  return formats;
}

bool CopyModule::SetResolutions(Resolutions const& resolutions)
{
  this->resolutions = resolutions;
  return true;
}

bool CopyModule::SetClocks(Clocks const& clocks)
{
  this->clocks = clocks;
  return true;
}

bool CopyModule::SetFormats(Formats const& formats)
{
  this->formats = formats;
  return true;
}

