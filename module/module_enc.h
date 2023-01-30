/******************************************************************************
*
* Copyright (C) 2015-2022 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#pragma once
#include "module_interface.h"
#include "module_structs.h"
#include "device_enc_interface.h"
#include "memory_interface.h"
#include "settings_enc_interface.h"

#include "ROIMngr.h"

#include <cstring>
#include <vector>
#include <list>
#include <future>
#include <memory>
#include <mutex>

#include <utility/threadsafe_map.h>
#include <utility/semaphore.h>
#include <utility/processor_fifo.h>

#include "TwoPassMngr.h"

extern "C"
{
#include <lib_common/Allocator.h>
#include <lib_encode/lib_encoder.h>
}

struct LookAheadCallBackParam
{
  void* module;
  int index;
};

struct GenericEncoder;

struct EmptyFifoParam
{
  GenericEncoder* encoder;
  bool isEOS;
};

struct GenericEncoder
{
  GenericEncoder(int pass) : index{pass} {}

  ~GenericEncoder() = default;
  AL_HEncoder enc {};
  int index {};
  AL_TBuffer* nextQPBuffer {};
  std::vector<AL_TBuffer*> streamBuffers {};
  std::shared_ptr<ProcessorFifo<EmptyFifoParam>> threadFifo {};
  std::shared_ptr<LookAheadMngr> lookAheadMngr {};
  LookAheadCallBackParam callbackParam {};
};

struct EncModule final : ModuleInterface
{
  EncModule(std::shared_ptr<EncSettingsInterface> media, std::shared_ptr<EncDeviceInterface> device, std::shared_ptr<AL_TAllocator> allocator, std::shared_ptr<MemoryInterface> memory);
  ~EncModule() override;

  bool SetCallbacks(Callbacks callbacks) override;

  void Free(void* buffer) override;
  void* Allocate(size_t size) override;

  void FreeDMA(int fd);
  int AllocateDMA(int size);

  bool Empty(BufferHandleInterface* handle) override;
  bool Fill(BufferHandleInterface* handle) override;

  ErrorType Start(bool shouldPrealloc) override;
  bool Stop() override;
  ErrorType Restart() override;

  ErrorType SetDynamic(std::string index, void const* param) override;
  ErrorType GetDynamic(std::string index, void* param) override;

private:
  std::shared_ptr<EncSettingsInterface> const media;
  std::shared_ptr<EncDeviceInterface> const device;
  std::shared_ptr<AL_TAllocator> const allocator;
  std::shared_ptr<MemoryInterface> const memory;
  std::vector<GenericEncoder> encoders;
  std::mutex mutex;
  semaphore sem;
  AL_TBuffer* configHandle;
  Callbacks callbacks;
  AL_TBuffer* nextQPBuffer;
  Dimension<int> initialDimension;
  Dimension<int> currentDimension;
  AL_ESliceType currentPictureType;
  bool currentPictureIsSkipped;

  AL_TRoiMngrCtx* roiCtx;
  std::shared_ptr<TwoPassMngr> twoPassMngr;
  AL_TBuffer* currentOutputedStreamForSei;
  int currentTemporalId;
  Flags currentFlags;

  void InitEncoders(int numPass);
  bool Use(BufferHandleInterface* handle, uint8_t* buffer, int size);
  void Unuse(BufferHandleInterface* handle);
  bool UseDMA(BufferHandleInterface* handle, int fd, int size);
  void UnuseDMA(BufferHandleInterface* handle);
  ErrorType CreateEncoder();
  bool DestroyEncoder();
  void ReleaseBuf(AL_TBuffer const* buf, bool isDma, bool isSrc);
  bool isEndOfFrame(AL_TBuffer* stream);

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
  void _ProcessEmptyFifo(EmptyFifoParam param);
  void AddFifo(GenericEncoder& encoder, AL_TBuffer* src);
  void EmptyFifo(GenericEncoder& encoder, bool isEOS);

  ThreadSafeMap<AL_TBuffer const*, BufferHandleInterface*> handles;
  ThreadSafeMap<void*, AL_HANDLE> allocated;
  ThreadSafeMap<int, AL_HANDLE> allocatedDMA;
  ThreadSafeMap<AL_TBuffer*, AL_VADDR> shouldBeCopied;
  ThreadSafeMap<BufferHandleInterface*, AL_TBuffer*> pool;
};
