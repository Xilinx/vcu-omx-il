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
#include "omx_module_structs.h"

#include "omx_device_enc_interface.h"
#include "omx_module_codec_structs.h"

#include "ROIMngr.h"

#include <string.h>
#include <vector>
#include <list>

#include "base/omx_utils/threadsafe_map.h"
#include "base/omx_mediatype/omx_mediatype_enc_interface.h"

extern "C"
{
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

class EncModule : public ModuleInterface
{
public:
  EncModule(std::shared_ptr<EncMediatypeInterface> media, std::unique_ptr<EncDevice>&& device, std::shared_ptr<AL_TAllocator> allocator);
  ~EncModule();

  void ResetRequirements();
  BufferRequirements GetBufferRequirements() const;
  int GetLatency() const; // In milliseconds

  Resolution GetResolution() const;
  Clock GetClock() const;
  Mimes GetMimes() const;
  Format GetFormat() const;
  std::vector<Format> GetFormatsSupported() const;
  std::vector<VideoModeType> GetVideoModesSupported() const;
  Bitrate GetBitrate() const;
  Gop GetGop() const;
  QPs GetQPs() const;
  ProfileLevelType GetProfileLevel() const;
  std::vector<ProfileLevelType> GetProfileLevelSupported() const;
  EntropyCodingType GetEntropyCoding() const;
  VideoModeType GetVideoMode() const;

  bool IsConstrainedIntraPrediction() const;
  LoopFilterType GetLoopFilter() const;
  AspectRatioType GetAspectRatio() const;
  bool IsEnableLowBandwidth() const;
  bool IsEnablePrefetchBuffer() const;
  ScalingListType GetScalingList() const;
  bool IsEnableFillerData() const;
  Slices GetSlices() const;
  bool IsEnableSubframe() const;
  FileDescriptors GetFileDescriptors() const;

  bool SetBitrate(Bitrate const& bitrates);
  bool SetResolution(Resolution const& resolution);
  bool SetClock(Clock const& clock);
  bool SetFormat(Format const& format);
  bool SetGop(Gop const& gop);
  bool SetProfileLevel(ProfileLevelType const& profileLevel);
  bool SetEntropyCoding(EntropyCodingType const& entropyCoding);
  bool SetConstrainedIntraPrediction(bool constrainedIntraPrediction);
  bool SetLoopFilter(LoopFilterType const& loopFilter);
  bool SetQPs(QPs const& qps);
  bool SetAspectRatio(AspectRatioType const& aspectRatio);
  bool SetEnableLowBandwidth(bool enableLowBandwidth);
  bool SetEnablePrefetchBuffer(bool enablePrefetchBuffer);
  bool SetScalingList(ScalingListType const& scalingList);
  bool SetEnableFillerData(bool enableFillerData);
  bool SetSlices(Slices const& slices);
  bool SetEnableSubframe(bool enableSubframe);
  bool SetFileDescriptors(FileDescriptors const& fds);
  bool SetVideoMode(VideoModeType const& videoMode);

  bool SetCallbacks(Callbacks callbacks);

  bool CheckParam();
  bool Create();
  void Destroy();

  void Free(void* buffer);
  void* Allocate(size_t size);

  void FreeDMA(int fd);
  int AllocateDMA(int size);

  bool UseDMA(BufferHandleInterface* handle, int fd, int size);
  void UnuseDMA(BufferHandleInterface* handle);

  bool Empty(BufferHandleInterface* handle);
  bool Fill(BufferHandleInterface* handle);
  Flags GetFlags(BufferHandleInterface* handle);

  ErrorType Run(bool shouldPrealloc);
  bool Pause();
  bool Flush();
  void Stop();

  ErrorType SetDynamic(std::string index, void const* param);
  ErrorType GetDynamic(std::string index, void* param);

private:
  std::shared_ptr<EncMediatypeInterface> const media;
  std::unique_ptr<EncDevice> const device;
  std::shared_ptr<AL_TAllocator> const allocator;
  AL_HEncoder encoder;
  TScheduler* scheduler;
  Callbacks callbacks;
  FileDescriptors fds;

  AL_TRoiMngrCtx* roiCtx;
  std::list<AL_TBuffer*> roiBuffers;
  EOSHandles<BufferHandleInterface> eosHandles;

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
  void FlushEosHandles();

  ThreadSafeMap<AL_TBuffer const*, BufferHandleInterface*> handles;
  ThreadSafeMap<void*, AL_HANDLE> allocated;
  ThreadSafeMap<int, AL_HANDLE> allocatedDMA;
  ThreadSafeMap<AL_TBuffer*, AL_VADDR> shouldBeCopied;
  ThreadSafeMap<BufferHandleInterface*, AL_TBuffer*> pool;
};

