/******************************************************************************
*
* Copyright (C) 2018 Allegro DVT2.  All rights reserved.
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
#include "omx_module_interface.h"
#include "omx_module_structs.h"

#include "omx_device_enc_interface.h"
#include "omx_module_codec_structs.h"

#include "ROIMngr.h"

#include <cstring>
#include <vector>
#include <deque>
#include <list>
#include <future>
#include <memory>

#include "base/omx_utils/threadsafe_map.h"
#include "base/omx_mediatype/omx_mediatype_enc_interface.h"

#if AL_ENABLE_TWOPASS
#include "TwoPassMngr.h"
#endif

extern "C"
{
#include <lib_common/Allocator.h>
#include <lib_encode/lib_encoder.h>
}

struct Flags
{
  Flags() = default;
  ~Flags() = default;
  bool isConfig = false;
  bool isSync = false;
  bool isEndOfSlice = false;
  bool isEndOfFrame = false;
};

struct LookAheadCallBackParam
{
  void* module;
  int index;
};

struct LookAheadParams
{
  LookAheadCallBackParam callbackParam;
  int fifoSize;
  int complexityCount;
  int complexity;
  int complexityDiff;
};

struct PassEncoder
{
  AL_HEncoder enc;
  int index;
  std::list<AL_TBuffer*> roiBuffers;
  std::vector<AL_TBuffer*> streamBuffers;
  std::deque<AL_TBuffer*> fifo;
  std::promise<int>* EOSFinished;
  LookAheadParams lookAheadParams;

#if AL_ENABLE_TWOPASS
  void ProcessLookAheadParams(AL_TBuffer* src);
  void ComputeComplexity(bool isEOS);
#endif
};

struct EncModule : public ModuleInterface
{
  EncModule(std::shared_ptr<EncMediatypeInterface> media, std::shared_ptr<EncDevice> device, std::shared_ptr<AL_TAllocator> allocator);
  ~EncModule() override;

  void ResetRequirements() override;
  BufferRequirements GetBufferRequirements() const;

  BufferHandles GetBufferHandles() const;

  Resolution GetResolution() const;

  bool SetCallbacks(Callbacks callbacks) override;

  bool CreateAndAttachStreamMeta(AL_TBuffer& buf);
  void AddFifo(PassEncoder& encoder, AL_TBuffer* src);
  void EmptyFifo(PassEncoder& encoder, bool isEOS);

  bool CheckParam() override;
  bool Create() override;
  void Destroy() override;

  void Free(void* buffer) override;
  void* Allocate(size_t size) override;

  void FreeDMA(int fd);
  int AllocateDMA(int size);

  bool UseDMA(BufferHandleInterface* handle, int fd, int size);
  void UnuseDMA(BufferHandleInterface* handle);

  bool Empty(BufferHandleInterface* handle) override;
  bool Fill(BufferHandleInterface* handle) override;
  Flags GetFlags(BufferHandleInterface* handle);

  ErrorType Run(bool shouldPrealloc) override;
  bool Flush() override;
  void Stop() override;

  ErrorType SetDynamic(std::string index, void const* param) override;
  ErrorType GetDynamic(std::string index, void* param) override;

private:
  std::shared_ptr<EncMediatypeInterface> const media;
  std::shared_ptr<EncDevice> const device;
  std::shared_ptr<AL_TAllocator> const allocator;
  std::vector<PassEncoder> encoders;
  TScheduler* scheduler;
  Callbacks callbacks;

  AL_TRoiMngrCtx* roiCtx;
  EOSHandles<BufferHandleInterface*> eosHandles;

  void InitEncoders(int numPass);
  bool Use(BufferHandleInterface* handle, uint8_t* buffer, int size);
  void Unuse(BufferHandleInterface* handle);
  ErrorType CreateEncoder();
  bool DestroyEncoder();
  bool isCreated;
  void ReleaseBuf(AL_TBuffer const* buf, bool isDma, bool isSrc);
  bool isEndOfFrame(AL_TBuffer* stream);
  Flags GetFlags(AL_TBuffer* handle);

  static void RedirectionEndEncoding(void* userParam, AL_TBuffer* pStream, AL_TBuffer const* pSource, int)
  {
    auto pThis = static_cast<EncModule*>(userParam);
    pThis->EndEncoding(pStream, pSource);
  };
  void EndEncoding(AL_TBuffer* pStream, AL_TBuffer const* pSource);
  static void RedirectionEndEncodingLookAhead(void* userParam, AL_TBuffer* pStream, AL_TBuffer const* pSource, int)
  {
    auto params = static_cast<LookAheadCallBackParam*>(userParam);
    auto pThis = static_cast<EncModule*>(params->module);
    pThis->EndEncodingLookAhead(pStream, pSource, params->index);
  };
  void EndEncodingLookAhead(AL_TBuffer* pStream, AL_TBuffer const* pSource, int index);
  void FlushEosHandles();

  ThreadSafeMap<AL_TBuffer const*, BufferHandleInterface*> handles;
  ThreadSafeMap<void*, AL_HANDLE> allocated;
  ThreadSafeMap<int, AL_HANDLE> allocatedDMA;
  ThreadSafeMap<AL_TBuffer*, AL_VADDR> shouldBeCopied;
  ThreadSafeMap<BufferHandleInterface*, AL_TBuffer*> pool;
};

