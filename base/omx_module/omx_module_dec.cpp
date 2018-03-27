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

#include "omx_module_dec.h"
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <cmath>
#include <iostream>

extern "C"
{
#include <lib_common/BufferSrcMeta.h>
#include <lib_fpga/DmaAllocLinux.h>
#include <lib_common/FourCC.h>
}

#include "base/omx_settings/omx_convert_module_soft.h"
#include "base/omx_settings/omx_convert_module_soft_dec.h"
#include "base/omx_utils/roundup.h"

using namespace std;

DecModule::DecModule(unique_ptr<DecMediatypeInterface>&& media, unique_ptr<DecDevice>&& device, deleted_unique_ptr<AL_TAllocator>&& allocator) :
  media(move(media)),
  device(move(device)),
  allocator(move(allocator))
{
  assert(this->media);
  assert(this->device);
  assert(this->allocator);
  decoder = nullptr;
  isCreated = false;
  ResetRequirements();
}

void DecModule::ResetRequirements()
{
  media->Reset();
  fds.input = fds.output = false;
}

static int RawAllocationSize(int stride, int sliceHeight, AL_EChromaMode eChromaMode)
{
  auto const IP_WIDTH_ALIGNMENT = 64;
  auto const IP_HEIGHT_ALIGNMENT = 64;
  assert(stride % IP_WIDTH_ALIGNMENT == 0); // IP requirements
  assert(sliceHeight % IP_HEIGHT_ALIGNMENT == 0); // IP requirements

  auto size = stride * sliceHeight;

  switch(eChromaMode)
  {
    case CHROMA_MONO: break;
    case CHROMA_4_2_0:
    {
      size += size >> 1;
      break;
    }
    case CHROMA_4_2_2:
    {
      size += size;
      break;
    }
    case CHROMA_4_4_4:
    default:
    {
      assert(0);
      break;
    }
  }

  return size;
}

BufferRequirements DecModule::GetBufferRequirements() const
{
  auto const streamSettings = media->settings.tStream;
  BufferRequirements b;
  auto& input = b.input;
  input.min = 2; // We need 2 NALUs to detect the first one
  input.size = AL_GetMaxNalSize(streamSettings.tDim, streamSettings.eChroma, streamSettings.iBitDepth);
  input.bytesAlignment = device->GetAllocationRequirements().input.bytesAlignment;
  input.contiguous = device->GetAllocationRequirements().input.contiguous;

  auto& output = b.output;
  output.min = media->GetRequiredOutputBuffers() + 1; // 1 for eos
  output.size = RawAllocationSize(media->stride, media->sliceHeight, streamSettings.eChroma);
  output.bytesAlignment = device->GetAllocationRequirements().output.bytesAlignment;
  output.contiguous = device->GetAllocationRequirements().output.contiguous;

  return b;
}


Resolution DecModule::GetResolution() const
{
  auto const streamSettings = media->settings.tStream;
  Resolution resolution;
  resolution.width = streamSettings.tDim.iWidth;
  resolution.height = streamSettings.tDim.iHeight;
  resolution.stride = media->stride;
  resolution.sliceHeight = media->sliceHeight;

  return resolution;
}

Clock DecModule::GetClock() const
{
  Clock clock;
  clock.framerate = media->settings.uFrameRate / 1000;
  clock.clockratio = media->settings.uClkRatio;
  return clock;
}

Mimes DecModule::GetMimes() const
{
  Mimes mimes;
  auto& inMime = mimes.input;

  inMime.mime = media->Mime();
  inMime.compression = media->Compression();

  auto& outMime = mimes.output;

  outMime.mime = "video/x-raw"; // NV12
  outMime.compression = COMPRESSION_UNUSED;

  return mimes;
}

Format DecModule::GetFormat() const
{
  return media->GetFormat();
}

vector<Format> DecModule::GetFormatsSupported() const
{
  return media->FormatsSupported();
}

ProfileLevelType DecModule::GetProfileLevel() const
{
  return media->ProfileLevel();
}

vector<ProfileLevelType> DecModule::GetProfileLevelSupported() const
{
  return media->ProfileLevelSupported();
}

int DecModule::GetLatency() const
{
  auto const settings = media->settings;
  auto bufsCount = media->GetRequiredOutputBuffers();

  if(ConvertSoftToModuleDecodedPictureBuffer(settings.eDpbMode) == DECODED_PICTURE_BUFFER_LOW_REFERENCE)
    bufsCount -= settings.iStackSize;

  if(IsEnableSubframe())
    bufsCount = 1;

  auto const realFramerate = (static_cast<double>(settings.uFrameRate) / static_cast<double>(settings.uClkRatio));
  auto const timeInMilliseconds = (static_cast<double>(bufsCount * 1000.0) / realFramerate);

  return ceil(timeInMilliseconds);
}

FileDescriptors DecModule::GetFileDescriptors() const
{
  return fds;
}

DecodedPictureBufferType DecModule::GetDecodedPictureBuffer() const
{
  auto const settings = media->settings;
  return ConvertSoftToModuleDecodedPictureBuffer(settings.eDpbMode);
}

int DecModule::GetInternalEntropyBuffers() const
{
  auto const settings = media->settings;
  return settings.iStackSize;
}

bool DecModule::IsEnableSubframe() const
{
  auto const settings = media->settings;
  return DECODE_UNIT_SLICE == ConvertSoftToModuleDecodeUnit(settings.eDecUnit);
}

bool DecModule::SetResolution(Resolution const& resolution)
{
  if((resolution.width % 8) != 0)
    return false;

  if((resolution.height % 8) != 0)
    return false;

  auto& streamSettings = media->settings.tStream;
  streamSettings.tDim = { resolution.width, resolution.height };

  auto minStride = (int)RoundUp(AL_Decoder_RoundPitch(streamSettings.tDim.iWidth, streamSettings.iBitDepth, media->settings.eFBStorageMode), media->strideAlignment);
  media->stride = max(minStride, RoundUp(resolution.stride, media->strideAlignment));

  auto minSliceHeight = (int)RoundUp(AL_Decoder_RoundHeight(streamSettings.tDim.iHeight), media->sliceHeightAlignment);
  media->sliceHeight = max(minSliceHeight, RoundUp(resolution.sliceHeight, media->sliceHeightAlignment));

  return true;
}

bool DecModule::SetClock(Clock const& clock)
{
  auto& settings = media->settings;
  settings.uFrameRate = clock.framerate * 1000;
  settings.uClkRatio = clock.clockratio;
  return true;
}

bool DecModule::SetFormat(Format const& format)
{
  auto& streamSettings = media->settings.tStream;
  streamSettings.iBitDepth = format.bitdepth;
  streamSettings.eChroma = ConvertModuleToSoftChroma(format.color);

  auto minStride = (int)RoundUp(AL_Decoder_RoundPitch(streamSettings.tDim.iWidth, streamSettings.iBitDepth, media->settings.eFBStorageMode), media->strideAlignment);
  media->stride = max(minStride, media->stride);

  return true;
}

bool DecModule::SetProfileLevel(ProfileLevelType const& profileLevel)
{
  return media->SetProfileLevel(profileLevel);
}

bool DecModule::SetFileDescriptors(FileDescriptors const& fds)
{
  // TODO Check fds?
  this->fds = fds;
  return true;
}

bool DecModule::SetDecodedPictureBuffer(DecodedPictureBufferType const& decodedPictureBuffer)
{
  if(decodedPictureBuffer == DECODED_PICTURE_BUFFER_MAX)
    return false;

  auto& settings = media->settings;
  settings.bLowLat = decodedPictureBuffer == DECODED_PICTURE_BUFFER_LOW_REFERENCE;
  settings.eDpbMode = ConvertModuleToSoftDecodedPictureBuffer(decodedPictureBuffer);
  return true;
}

bool DecModule::SetInternalEntropyBuffers(int const& num)
{
  if(num < 1 && num > 16)
    return false;

  auto& settings = media->settings;
  settings.iStackSize = num;
  return true;
}

bool DecModule::SetEnableSubframe(bool const& enableSubframe)
{
  auto& settings = media->settings;
  settings.eDecUnit = enableSubframe ? ConvertModuleToSoftDecodeUnit(DECODE_UNIT_SLICE) : ConvertModuleToSoftDecodeUnit(DECODE_UNIT_FRAME);
  return true;
}

map<int, string> MapToStringDecodeError =
{
  { AL_ERR_INIT_FAILED, "decoder: initialization failure" },
  { AL_ERR_NO_FRAME_DECODED, "decoder: didn't decode any frames" },
  { AL_ERR_RESOLUTION_CHANGE, "decoder: doesn't support resolution change" },
  { AL_ERR_NO_MEMORY, "decoder: memory allocation failure (firmware or ctrlsw)" },
  { AL_ERR_CHAN_CREATION_NO_CHANNEL_AVAILABLE, "decoder: no channel available on the hardware" },
  { AL_ERR_CHAN_CREATION_RESOURCE_UNAVAILABLE, "decoder: hardware doesn't have enough resources" },
  { AL_ERR_CHAN_CREATION_NOT_ENOUGH_CORES, "decoder: hardware doesn't have enough resources (fragmentation)" },
  { AL_ERR_REQUEST_MALFORMED, "decoder: request to hardware was malformed" },
  { AL_WARN_CONCEAL_DETECT, "decoder, concealment" },
};

string ToStringDecodeError(int error)
{
  string str_error = "";
  try
  {
    str_error = MapToStringDecodeError.at(error);
  }
  catch(out_of_range& e)
  {
    str_error = "unknown error";
  }
  return str_error;
}

void DecModule::EndDecoding(AL_TBuffer* decodedFrame)
{
  if(!decodedFrame)
  {
    auto error = AL_Decoder_GetLastError(decoder);

    fprintf(stderr, "/!\\ %s (%d)\n", ToStringDecodeError(error).c_str(), error);

    if(error & AL_ERROR)
      callbacks.event(CALLBACK_EVENT_ERROR, nullptr);

    return;
  }

  auto const handleOut = translateOut.Get(decodedFrame);
  assert(handleOut);

  callbacks.associate(nullptr, handleOut);
}

void DecModule::ReleaseBufs(AL_TBuffer* frame)
{
  auto const handleOut = translateOut.Get(frame);
  assert(handleOut);
  dpb.Remove(handleOut);
  translateOut.Remove(frame);
  callbacks.release(false, handleOut);
  AL_Buffer_Unref(frame);
  return;
}

void DecModule::CopyIfRequired(AL_TBuffer* frameToDisplay, int size)
{
  if(shouldBeCopied.Exist(frameToDisplay))
  {
    auto buffer = shouldBeCopied.Get(frameToDisplay);
    memcpy(buffer, AL_Buffer_GetData(frameToDisplay), size);
  }
}

void DecModule::Display(AL_TBuffer* frameToDisplay, AL_TInfoDecode* info)
{
  auto const isRelease = (frameToDisplay && info == nullptr);

  if(isRelease)
    return ReleaseBufs(frameToDisplay);

  auto const isEOS = (frameToDisplay == nullptr && info == nullptr);

  if(isEOS)
  {
    auto const handleOut = translateOut.Pop(eosHandles.output);
    dpb.Remove(handleOut);

    auto const handleIn = translateIn.Get(eosHandles.input);

    auto const eosIsSent = handleIn != nullptr;

    if(eosIsSent)
    {
      callbacks.associate(handleIn, handleOut);
      AL_Buffer_Unref(eosHandles.input);
      eosHandles.input = nullptr;
    }

    callbacks.filled(handleOut, 0, 0);

    AL_Buffer_Unref(eosHandles.output);
    eosHandles.output = nullptr;

    return;
  }

  auto const size = GetBufferRequirements().output.size;
  CopyIfRequired(frameToDisplay, size);
  callbacks.filled(translateOut.Pop(frameToDisplay), 0, size);
}

void DecModule::ResolutionFound(int const& bufferNumber, int const& bufferSize, AL_TStreamSettings const& settings, AL_TCropInfo const& crop)
{
  (void)bufferNumber, (void)bufferSize, (void)crop;

  media->settings.tStream = settings;

  if(callbacks.event)
    callbacks.event(CALLBACK_EVENT_RESOLUTION_CHANGE, nullptr);
}

bool DecModule::CreateDecoder(bool shouldPrealloc)
{
  if(decoder)
  {
    fprintf(stderr, "Decoder is ALREADY created\n");
    assert(0);
  }

  channel = device->Init(media->settings.eDecUnit, *allocator.get());
  AL_TDecCallBacks decCallbacks {};
  decCallbacks.endDecodingCB = { RedirectionEndDecoding, this };
  decCallbacks.displayCB = { RedirectionDisplay, this };
  decCallbacks.resolutionFoundCB = { RedirectionResolutionFound, this };
  media->settings.bForceFrameRate = (media->settings.uFrameRate && media->settings.uClkRatio);
  auto errorCode = AL_Decoder_Create(&decoder, channel, allocator.get(), &media->settings, &decCallbacks);

  if(errorCode != AL_SUCCESS)
  {
    fprintf(stderr, "Failed to create Decoder: %d\n", errorCode);
    return false;
  }

  if(shouldPrealloc)
    AL_Decoder_PreallocateBuffers(decoder);

  return true;
}

bool DecModule::DestroyDecoder()
{
  if(!decoder)
  {
    fprintf(stderr, "Decoder isn't created\n");
    return false;
  }

  AL_Decoder_Destroy(decoder);
  device->Deinit();
  decoder = nullptr;
  channel = nullptr;

  return true;
}

bool DecModule::CheckParam()
{
  // TODO check some settings ?
  return true;
}

bool DecModule::Create()
{
  if(decoder)
  {
    fprintf(stderr, "Decoder should NOT be created\n");
    assert(0);
    return false;
  }
  isCreated = true;

  return true;
}

void DecModule::Destroy()
{
  if(decoder)
  {
    fprintf(stderr, "Decoder should ALREADY be destroyed\n");
    assert(0);
  }

  isCreated = false;
}

void DecModule::Free(void* buffer)
{
  if(!buffer)
    return;

  if(dpb.Exist((uint8_t*)buffer))
  {
    auto handle = dpb.Pop((uint8_t*)buffer);
    assert(!translateOut.Exist(handle));
    AL_Buffer_Unref(handle);
  }

  if(allocated.Exist(buffer))
  {
    auto handle = allocated.Pop(buffer);
    AL_Allocator_Free(allocator.get(), handle);
  }
}

void DecModule::FreeDMA(int fd)
{
  if(fd < 0)
    return;

  uint8_t* buffer = (uint8_t*)((intptr_t)fd);

  if(dpb.Exist(buffer))
  {
    auto handle = dpb.Pop(buffer);
    assert(!translateOut.Exist(handle));
    AL_Buffer_Unref(handle);
  }

  if(allocatedDMA.Exist(fd))
  {
    auto handle = allocatedDMA.Pop(fd);
    AL_Allocator_Free(allocator.get(), handle);
    close(fd);
  }
}

void* DecModule::Allocate(size_t size)
{
  auto handle = AL_Allocator_Alloc(allocator.get(), size);

  if(!handle)
  {
    fprintf(stderr, "No more memory\n");
    return nullptr;
  }

  auto addr = AL_Allocator_GetVirtualAddr(allocator.get(), handle);
  assert(addr);
  allocated.Add(addr, handle);
  return addr;
}

int DecModule::AllocateDMA(int size)
{
  auto handle = AL_Allocator_Alloc(allocator.get(), size);

  if(!handle)
  {
    fprintf(stderr, "No more memory\n");
    return -1;
  }

  auto fd = AL_LinuxDmaAllocator_ExportToFd((AL_TLinuxDmaAllocator*)allocator.get(), handle);
  allocatedDMA.Add(fd, handle);
  return fd;
}

bool DecModule::SetCallbacks(Callbacks callbacks)
{
  if(!callbacks.emptied || !callbacks.associate || !callbacks.filled || !callbacks.release)
    return false;

  this->callbacks = callbacks;
  return true;
}

void DecModule::InputBufferDestroy(AL_TBuffer* input)
{
  auto const handleIn = translateIn.Pop(input);
  auto const realHandle = handleMap.Pop(input);

  AL_Buffer_Destroy(input);

  callbacks.emptied(handleIn, 0, 0, realHandle);
}

void DecModule::InputDmaBufferDestroy(AL_TBuffer* input)
{
  auto const handleIn = translateIn.Pop(input);
  auto const realHandle = handleMap.Pop(input);

  AL_Allocator_Free(input->pAllocator, input->hBuf);
  AL_Buffer_Destroy(input);

  callbacks.emptied(handleIn, 0, 0, realHandle);
}

AL_TBuffer* DecModule::CreateInputBuffer(uint8_t* buffer, int const& size)
{
  AL_TBuffer* input = nullptr;

  if(fds.input)
  {
    auto const fd = static_cast<int>((intptr_t)buffer);

    if(fd < 0)
      throw invalid_argument("fd");

    auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator.get(), fd);

    if(!dmaHandle)
    {
      fprintf(stderr, "Failed to import fd : %i\n", fd);
      return nullptr;
    }

    input = AL_Buffer_Create(allocator.get(), dmaHandle, size, RedirectionInputDmaBufferDestroy);
  }
  else
  {
    if(allocated.Exist(buffer))
      input = AL_Buffer_Create(allocator.get(), allocated.Get(buffer), size, RedirectionInputBufferDestroy);
    else
      input = AL_Buffer_WrapData(buffer, size, RedirectionInputBufferDestroy);
  }

  if(input == nullptr)
    return nullptr;

  AL_Buffer_SetUserData(input, this);
  AL_Buffer_Ref(input);

  return input;
}

bool DecModule::Empty(uint8_t* buffer, int offset, int size, void* handle)
{
  (void)offset;
  if(!decoder)
    return false;

  AL_TBuffer* input = CreateInputBuffer(buffer, size);

  if(!input)
    return false;

  translateIn.Add(input, buffer);
  handleMap.Add(input, handle);

  auto const eos = (size == 0);

  if(eos)
  {
    eosHandles.input = input;
    AL_Decoder_Flush(decoder);
    return true;
  }

  auto const pushed = AL_Decoder_PushBuffer(decoder, input, size, AL_BUF_MODE_BLOCK);
  AL_Buffer_Unref(input);

  return pushed;
}

static AL_TMetaData* CreateSourceMeta(AL_TStreamSettings const& streamSettings, Resolution const& resolution)
{
  auto const fourCC = AL_GetSrcFourCC({ streamSettings.eChroma, static_cast<uint8_t>(streamSettings.iBitDepth), AL_FB_RASTER });
  AL_TPitches const pitches = { 0, 0 };
  AL_TOffsetYC const offsetYC = { 0, 0 };
  return (AL_TMetaData*)(AL_SrcMetaData_Create({ resolution.width, resolution.height }, pitches, offsetYC, fourCC));
}

void DecModule::OutputBufferDestroy(AL_TBuffer* output)
{
  AL_Buffer_Destroy(output);
}

void DecModule::OutputDmaBufferDestroy(AL_TBuffer* output)
{
  AL_Allocator_Free(output->pAllocator, output->hBuf);
  AL_Buffer_Destroy(output);
}

void DecModule::OutputBufferDestroyAndFree(AL_TBuffer* output)
{
  shouldBeCopied.Pop(output);

  AL_Allocator_Free(output->pAllocator, output->hBuf);
  AL_Buffer_Destroy(output);
}

AL_TBuffer* DecModule::CreateOutputBuffer(uint8_t* buffer, int const& size)
{
  auto const sourceMeta = CreateSourceMeta(media->settings.tStream, GetResolution());

  if(!sourceMeta)
    return nullptr;

  AL_TBuffer* output = nullptr;

  if(fds.output)
  {
    auto const fd = static_cast<int>((intptr_t)buffer);

    if(fd < 0)
      throw invalid_argument("fd");

    auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator.get(), fd);

    if(!dmaHandle)
    {
      fprintf(stderr, "Failed to import fd : %i\n", fd);
      ((AL_TMetaData*)sourceMeta)->MetaDestroy((AL_TMetaData*)sourceMeta);
      return nullptr;
    }

    output = AL_Buffer_Create(allocator.get(), dmaHandle, size, RedirectionOutputDmaBufferDestroy);
  }
  else
  {
    if(allocated.Exist(buffer))
      output = AL_Buffer_Create(allocator.get(), allocated.Get(buffer), size, RedirectionOutputBufferDestroy);
    else
    {
      output = AL_Buffer_Create_And_Allocate(allocator.get(), size, RedirectionOutputBufferDestroyAndFree);

      if(output)
        shouldBeCopied.Add(output, buffer);
    }
  }

  if(output == nullptr)
  {
    ((AL_TMetaData*)sourceMeta)->MetaDestroy((AL_TMetaData*)sourceMeta);
    return nullptr;
  }

  auto const attachedSourceMeta = AL_Buffer_AddMetaData(output, (AL_TMetaData*)sourceMeta);
  assert(attachedSourceMeta);

  AL_Buffer_SetUserData(output, this);
  dpb.Add(buffer, output);
  AL_Buffer_Ref(output);

  return output;
}

bool DecModule::Fill(uint8_t* buffer, int offset, int size)
{
  (void)offset;

  if(!decoder)
    return false;

  auto output = dpb.Exist(buffer) ? dpb.Get(buffer) : CreateOutputBuffer(buffer, size);

  if(!output)
    return false;

  translateOut.Add(output, buffer);

  if(!eosHandles.output)
  {
    eosHandles.output = output;
    return true;
  }

  AL_Decoder_PutDisplayPicture(decoder, output);
  return true;
}

bool DecModule::Run(bool shouldPrealloc)
{
  if(decoder)
  {
    fprintf(stderr, "You can't call Run twice\n");
    assert(0);
    return false;
  }

  if(!isCreated)
  {
    fprintf(stderr, "You should call Create before Run\n");
    return false;
  }

  eosHandles.input = nullptr;
  eosHandles.output = nullptr;

  return CreateDecoder(shouldPrealloc);
}

bool DecModule::Pause()
{
  if(!decoder)
    return false;

  DestroyDecoder();

  return false;
}

bool DecModule::Flush()
{
  if(!decoder)
  {
    FlushEosHandles();
    return false;
  }

  Stop();
  return Run(true);
}

void DecModule::FlushEosHandles()
{
  auto const handleOut = translateOut.Get(eosHandles.output);

  if(handleOut)
  {
    dpb.Remove(handleOut);
    translateOut.Remove(eosHandles.output);

    callbacks.release(false, handleOut);

    AL_Buffer_Unref(eosHandles.output);
    eosHandles.output = nullptr;
  }
}

void DecModule::ReleaseAllBuffers()
{
  DestroyDecoder();
  FlushEosHandles();
}

void DecModule::Stop()
{
  if(!decoder)
    return;

  ReleaseAllBuffers();
}

ErrorType DecModule::SetDynamic(std::string index, void const* param)
{
  (void)index, (void)param;
  return ERROR_NOT_IMPLEMENTED;
}

ErrorType DecModule::GetDynamic(std::string index, void* param)
{
  (void)index, (void)param;
  return ERROR_NOT_IMPLEMENTED;
}

Gop DecModule::GetGop() const
{
  // This will be replaced by omx_settings
  assert(0);
}

bool DecModule::SetGop(Gop const &)
{
  // This will be replaced by omx_settings
  assert(0);
}

