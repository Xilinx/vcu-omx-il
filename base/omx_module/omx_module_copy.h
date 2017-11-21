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

#include "omx_module_interface.h"
#include "base/omx_utils/threadsafe_map.h"
#include "base/omx_utils/processor_fifo.h"

#include <queue>
#include <mutex>
#include <condition_variable>

struct CopyBuffer
{
  void* buffer;
  int copy;
  int size;
};

class CopyModule : public ModuleInterface
{
public:
  CopyModule();
  ~CopyModule()
  {
  }

  void ResetRequirements();
  BuffersRequirements GetBuffersRequirements() const;

  Resolutions GetResolutions() const;
  Clocks GetClocks() const;
  Formats GetFormats() const;

  bool SetResolutions(Resolutions const& resolutions);
  bool SetClocks(Clocks const& clocks);
  bool SetFormats(Formats const& formats);

  bool SetCallbacks(Callbacks callbacks);

  bool Create();
  void Destroy();

  void Free(void* buffer);
  void* Allocate(size_t size);

  bool Empty(uint8_t* handle, int offset, int size);
  bool Fill(uint8_t* handle, int offset, int size);

  bool Run();
  bool Pause();
  bool Flush();
  void Stop();

private:
  std::queue<uint8_t*> input;
  std::queue<uint8_t*> output;
  std::mutex mutex;
  bool eos;
  std::condition_variable cv;

  BuffersRequirements reqBuf;
  Resolutions resolutions;
  Formats formats;
  Clocks clocks;
  Callbacks callbacks;
  ThreadSafeMap<void*, CopyBuffer*> buffers;
  bool isCreated;

  void CreateOneCopyTask();
  void AddBuffer(void* handle, void* buffer, int size);
  void RemoveBuffer(void* buffer);

  std::unique_ptr<ProcessorFifo> processor;
  void _Process(void* data);
  void _Delete(void* data);
};

