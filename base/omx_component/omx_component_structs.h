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

#pragma once

#include "omx_buffer_handle.h"

#include <utility/processor_fifo.h>
#include <algorithm>
#include <mutex>
#include <memory>

enum class Command
{
  SetState,
  Flush,
  DisablePort,
  EnablePort,
  MarkBuffer,
  EmptyBuffer,
  FillBuffer,
  SetDynamic,
  SharedFence,
  Signal,
  Max,
};

enum class TransientState
{
  Invalid,
  LoadedToIdle,
  IdleToPause,
  IdleToLoaded,
  IdleToExecuting,
  PauseToExecuting,
  PauseToIdle,
  ExecutingToIdle,
  ExecutingToPause,
  Max,
};

struct Task
{
  Task() :
    cmd{Command::Max}, data{nullptr}, opt{nullptr}
  {
  }

  Command cmd;
  void* data;
  std::shared_ptr<void> opt;
};

struct Port
{
  Port(int index, int expected) :
    index{index}
  {
    setExpected(expected);
  };

  ~Port() = default;

  int const index;
  bool enable = true;
  bool playable = false;
  bool error = false;
  bool isTransientToEnable = false;
  bool isTransientToDisable = false;

  void ResetError()
  {
    std::lock_guard<std::mutex> lock(mutex);
    error = false;
  }

  void ErrorOccured()
  {
    std::lock_guard<std::mutex> lock(mutex);
    error = true;
    cv_full.notify_one();
    cv_empty.notify_one();
  }

  int getExpected()
  {
    return expected;
  }

  void setExpected(int iExpected)
  {
    expected = iExpected;
    playable = ((int)buffers.size() >= expected);
  }

  void Add(OMX_BUFFERHEADERTYPE* header)
  {
    std::lock_guard<std::mutex> lock(mutex);
    buffers.push_back(header);

    if((int)buffers.size() < expected)
      return;

    playable = true;
    cv_full.notify_one();
  }

  void Remove(OMX_BUFFERHEADERTYPE* header)
  {
    std::lock_guard<std::mutex> lock(mutex);
    buffers.erase(std::remove(buffers.begin(), buffers.end(), header), buffers.end());

    if((buffers.size() > 0 || expected == 0))
      return;

    playable = false;
    cv_empty.notify_one();
  }

  void WaitEmpty()
  {
    std::unique_lock<std::mutex> lck(mutex);
    cv_empty.wait(lck, [&] {
      return !playable || expected == 0 || error;
    });
  }

  void WaitFull()
  {
    std::unique_lock<std::mutex> lck(mutex);
    cv_full.wait(lck, [&] {
      return playable || error;
    });
  }

private:
  int expected;

  std::mutex mutex;
  std::vector<OMX_BUFFERHEADERTYPE*> buffers;
  std::condition_variable cv_full;
  std::condition_variable cv_empty;
};

