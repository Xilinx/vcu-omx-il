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
#include <list>
#include <future>
#include <memory>

#include "base/omx_utils/threadsafe_map.h"
#include "base/omx_utils/processor_fifo.h"
#include "base/omx_mediatype/omx_mediatype_enc_interface.h"

#include "TwoPassMngr.h"

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

struct GenericEncoder
{
  GenericEncoder(int pass) : index{pass} {}

  ~GenericEncoder() = default;
  AL_HEncoder enc {};
  int index {};
  std::list<AL_TBuffer*> roiBuffers {};
  std::vector<AL_TBuffer*> streamBuffers {};
  std::shared_ptr<ProcessorFifo> threadFifo {};
  std::shared_ptr<LookAheadMngr> lookAheadMngr {};
  LookAheadCallBackParam callbackParam;
};

struct EncModule final : public ModuleInterface
{
  EncModule(std::shared_ptr<EncMediatypeInterface> media, std::shared_ptr<EncDevice> device, std::shared_ptr<AL_TAllocator> allocator);
  ~EncModule() override;

  bool SetCallbacks(Callbacks callbacks) override;

  void Free(void* buffer) override;
  void* Allocate(size_t size) override;

  void FreeDMA(int fd);
  int AllocateDMA(int size);

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
  std::vector<GenericEncoder> encoders;
  Callbacks callbacks;

  AL_TRoiMngrCtx* roiCtx;
  std::shared_ptr<TwoPassMngr> twoPassMngr;

  bool CreateAndAttachStreamMeta(AL_TBuffer& buf);
  void InitEncoders(int numPass);
  bool Use(BufferHandleInterface* handle, uint8_t* buffer, int size);
  void Unuse(BufferHandleInterface* handle);
  bool UseDMA(BufferHandleInterface* handle, int fd, int size);
  void UnuseDMA(BufferHandleInterface* handle);
  ErrorType CreateEncoder();
  bool DestroyEncoder();
  void ReleaseBuf(AL_TBuffer const* buf, bool isDma, bool isSrc);
  bool isEndOfFrame(AL_TBuffer* stream);
  Flags GetFlags(AL_TBuffer* handle);

  void AddFifo(GenericEncoder& encoder, AL_TBuffer* src);
  void EmptyFifo(GenericEncoder& encoder, bool isEOS);

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
  void _ProcessEmptyFifo(void* data);
  void _DeleteEmptyFifo(void* data);

  ThreadSafeMap<AL_TBuffer const*, BufferHandleInterface*> handles;
  ThreadSafeMap<void*, AL_HANDLE> allocated;
  ThreadSafeMap<int, AL_HANDLE> allocatedDMA;
  ThreadSafeMap<AL_TBuffer*, AL_VADDR> shouldBeCopied;
  ThreadSafeMap<BufferHandleInterface*, AL_TBuffer*> pool;
};

struct EmptyFifoParam
{
  EmptyFifoParam(GenericEncoder* encoder, bool isEOS) :
    encoder{encoder}, isEOS{isEOS} {};

  GenericEncoder* encoder;
  bool isEOS;
};

