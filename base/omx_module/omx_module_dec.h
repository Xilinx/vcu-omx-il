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

#pragma once
#include "omx_module_interface.h"
#include "omx_device_dec_interface.h"
#include "omx_module_enums.h"
#include "omx_module_codec_structs.h"

#include <vector>
#include <queue>

#include "base/omx_mediatype/omx_mediatype_dec_interface.h"
#include "base/omx_utils/threadsafe_map.h"

extern "C"
{
#include <lib_decode/lib_decode.h>
#include <lib_common/SliceConsts.h>
#include <lib_common/StreamBuffer.h>
}

class DecModule : public ModuleInterface
{
public:
  DecModule(std::unique_ptr<DecMediatypeInterface>&& media, std::unique_ptr<DecDevice>&& device, deleted_unique_ptr<AL_TAllocator>&& allocator);

  ~DecModule()
  {
  }

  void ResetRequirements();
  BufferRequirements GetBufferRequirements() const;

  Resolution GetResolution() const;
  Clock GetClock() const;
  Mimes GetMimes() const;
  Format GetFormat() const;
  std::vector<Format> GetFormatsSupported() const;
  ProfileLevelType GetProfileLevel() const;
  std::vector<ProfileLevelType> GetProfileLevelSupported() const;
  int GetLatency() const;
  Gop GetGop() const;
  FileDescriptors GetFileDescriptors() const;
  DecodedPictureBufferType GetDecodedPictureBuffer() const;
  int GetInternalEntropyBuffers() const;
  bool IsEnableSubframe() const;

  bool SetResolution(Resolution const& resolution);
  bool SetClock(Clock const& clock);
  bool SetFormat(Format const& format);
  bool SetProfileLevel(ProfileLevelType const& profileLevel);
  bool SetFileDescriptors(FileDescriptors const& fds);
  bool SetDecodedPictureBuffer(DecodedPictureBufferType const& decodedPictureBuffer);
  bool SetInternalEntropyBuffers(int const& num);
  bool SetEnableSubframe(bool const& enableSubframe);
  bool SetGop(Gop const& gop);

  bool CheckParam();
  bool Create();
  void Destroy();

  void Free(void* buffer);
  void* Allocate(size_t size);

  void FreeDMA(int fd);
  int AllocateDMA(int size);

  bool SetCallbacks(Callbacks callbacks);

  bool Empty(uint8_t* handle, int offset, int size);
  bool Fill(uint8_t* handle, int offset, int size);

  bool Run(bool shouldPrealloc);
  bool Pause();
  bool Flush();
  void Stop();

  ErrorType SetDynamic(DynamicIndexType index, void const* param);
  ErrorType GetDynamic(DynamicIndexType index, void* param);

private:
  std::unique_ptr<DecMediatypeInterface> media;
  std::unique_ptr<DecDevice> device;
  deleted_unique_ptr<AL_TAllocator> allocator;

  Callbacks callbacks;
  ThreadSafeMap<AL_TBuffer*, uint8_t*> translateIn;
  ThreadSafeMap<AL_TBuffer*, uint8_t*> translateOut;
  ThreadSafeMap<uint8_t*, AL_TBuffer*> dpb;
  ThreadSafeMap<AL_TBuffer*, uint8_t*> shouldBeCopied;

  ThreadSafeMap<void*, AL_HANDLE> allocated;
  ThreadSafeMap<int, AL_HANDLE> allocatedDMA;

  FileDescriptors fds;
  AL_TIDecChannel* channel;
  AL_HDecoder decoder;

  EOSHandles eosHandles;
  bool CreateDecoder(bool shouldPrealloc);
  bool DestroyDecoder();
  void ReleaseAllBuffers();
  void FlushEosHandles();
  bool isCreated;
  void CopyIfRequired(AL_TBuffer* frameToDisplay, int size);

  AL_TBuffer* CreateInputBuffer(uint8_t* buffer, int const& size);
  AL_TBuffer* CreateOutputBuffer(uint8_t* buffer, int const& size);

  static void RedirectionEndDecoding(AL_TBuffer* decodedFrame, void* userParam)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    pThis->EndDecoding(decodedFrame);
  };
  void EndDecoding(AL_TBuffer* decodedFrame);
  void ReleaseBufs(AL_TBuffer* frame);

  static void RedirectionDisplay(AL_TBuffer* frameToDisplay, AL_TInfoDecode* info, void* userParam)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    pThis->Display(frameToDisplay, info);
  };
  void Display(AL_TBuffer* frameToDisplay, AL_TInfoDecode* info);

  static void RedirectionResolutionFound(int buffersNumber, int bufferSize, AL_TStreamSettings const* settings, AL_TCropInfo const* crop, void* userParam)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    pThis->ResolutionFound(buffersNumber, bufferSize, *settings, *crop);
  };
  void ResolutionFound(int const& bufferNumber, int const& bufferSize, AL_TStreamSettings const& settings, AL_TCropInfo const& crop);

  static void RedirectionInputBufferDestroy(AL_TBuffer* input)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(input));
    pThis->InputBufferDestroy(input);
  };
  void InputBufferDestroy(AL_TBuffer* input);

  static void RedirectionInputDmaBufferDestroy(AL_TBuffer* input)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(input));
    pThis->InputDmaBufferDestroy(input);
  };
  void InputDmaBufferDestroy(AL_TBuffer* input);

  static void RedirectionOutputBufferDestroy(AL_TBuffer* output)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(output));
    pThis->OutputBufferDestroy(output);
  };
  void OutputBufferDestroy(AL_TBuffer* output);

  static void RedirectionOutputDmaBufferDestroy(AL_TBuffer* output)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(output));
    pThis->OutputDmaBufferDestroy(output);
  };
  void OutputDmaBufferDestroy(AL_TBuffer* output);

  static void RedirectionOutputBufferDestroyAndFree(AL_TBuffer* output)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(output));
    pThis->OutputBufferDestroyAndFree(output);
  };
  void OutputBufferDestroyAndFree(AL_TBuffer* output);
};

