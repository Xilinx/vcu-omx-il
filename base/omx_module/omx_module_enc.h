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
#include "omx_module_structs_enc.h"
#include "omx_device_enc_interface.h"

#include <string.h>
#include <vector>
#include <condition_variable>
#include <mutex>

#include "base/omx_utils/threadsafe_map.h"
#include "base/omx_mediatype/omx_mediatype_enc_interface.h"

extern "C"
{
#include <lib_encode/lib_encoder.h>
}

struct Flags
{
  Flags() : isConfig(false), isSync(false), isEndOfSlice(false), isEndOfFrame(false)
  {
  };
  bool isConfig;
  bool isSync;
  bool isEndOfSlice;
  bool isEndOfFrame;
};

template<typename T>
using deleted_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

class EncModule : public ModuleInterface
{
public:
  EncModule(std::unique_ptr<EncMediatypeInterface>&& media, std::unique_ptr<EncDevice>&& device, deleted_unique_ptr<AL_TAllocator>&& allocator);
  ~EncModule()
  {
  }

  void ResetRequirements();
  BuffersRequirements GetBuffersRequirements() const;
  int GetLatency() const; // In milliseconds

  Resolutions GetResolutions() const;
  Clocks GetClocks() const;
  Formats GetFormats() const;
  Bitrates GetBitrates() const;
  Gop GetGop() const;
  QPs GetQPs() const;
  ProfileLevelType GetProfileLevel() const;
  std::vector<ProfileLevelType> GetProfileLevelSupported() const;
  EntropyCodingType GetEntropyCoding() const;
  bool IsConstrainedIntraPrediction() const;
  LoopFilterType GetLoopFilter() const;
  AspectRatioType GetAspectRatio() const;
  bool IsEnableLowBandwidth() const;
  int GetPrefetchBufferSize() const; // In units of 1024 bytes
  ScalingListType GetScalingList() const;
  bool IsEnableFillerData() const;
  Slices GetSlices() const;
  bool IsEnableSubframe() const;
  FileDescriptors GetFileDescriptors() const;

  bool SetBitrates(Bitrates const& bitrates);
  bool SetResolutions(Resolutions const& resolutions);
  bool SetClocks(Clocks const& clocks);
  bool SetFormats(Formats const& formats);
  bool SetGop(Gop const& gop);
  bool SetProfileLevel(ProfileLevelType const& profileLevel);
  bool SetEntropyCoding(EntropyCodingType const& entropyCoding);
  bool SetConstrainedIntraPrediction(bool const& constrainedIntraPrediction);
  bool SetLoopFilter(LoopFilterType const& loopFilter);
  bool SetQPs(QPs const& qps);
  bool SetAspectRatio(AspectRatioType const& aspectRatio);
  bool SetEnableLowBandwidth(bool const& enableLowBandwidth);
  bool SetPrefetchBufferSize(int const& size); // In units of 1024 bytes
  bool SetScalingList(ScalingListType const& scalingList);
  bool SetEnableFillerData(bool const& enableFillerData);
  bool SetSlices(Slices const& slices);
  bool SetEnableSubframe(bool const& enableSubframe);
  bool SetFileDescriptors(FileDescriptors const& fds);

  bool SetCallbacks(Callbacks callbacks);

  bool Create();
  void Destroy();

  void Free(void* buffer);
  void* Allocate(size_t size);

  void FreeDMA(int fd);
  int AllocateDMA(int size);

  bool UseDMA(void* handle, int fd, int size);
  void UnuseDMA(void* handle);

  bool Empty(uint8_t* handle, int offset, int size);
  bool Fill(uint8_t* handle, int offset, int size);
  Flags GetFlags(void* handle);

  bool Run();
  bool Pause();
  bool Flush();
  void Stop();

private:
  bool Use(void* handle, uint8_t* buffer, int size);
  void Unuse(void* handle);
  struct EOSHandles
  {
    EOSHandles()
    {
      input = nullptr;
      output = nullptr;
      eosSent = false;
      eosReiceive = false;
    }

    AL_TBuffer* input;
    AL_TBuffer* output;
    bool eosSent;
    bool eosReiceive;
    std::condition_variable cv;
    std::mutex mutex;
  };

  std::unique_ptr<EncMediatypeInterface> media;
  std::unique_ptr<EncDevice> device;
  deleted_unique_ptr<AL_TAllocator> allocator;
  AL_HEncoder encoder;
  TScheduler* scheduler;
  Callbacks callbacks;
  FileDescriptors fds;

  EOSHandles eosHandles;

  bool CreateEncoder();
  bool DestroyEncoder();
  bool isSettingsOk;

  static void RedirectionEndEncoding(void* userParam, AL_TBuffer* pStream, AL_TBuffer const* pSource)
  {
    auto pThis = static_cast<EncModule*>(userParam);
    pThis->EndEncoding(pStream, pSource);
  };
  void EndEncoding(AL_TBuffer* pStream, AL_TBuffer const* pSource);

  ThreadSafeMap<void*, AL_HANDLE> allocated;
  ThreadSafeMap<int, AL_HANDLE> allocatedDMA;
  ThreadSafeMap<AL_TBuffer*, AL_VADDR> shouldBeCopied;
  ThreadSafeMap<void*, AL_TBuffer*> pool;
  ThreadSafeMap<AL_TBuffer*, uint8_t*> translate;
};

