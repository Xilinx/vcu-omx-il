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

#include "omx_convert_module_soft.h"
#include "omx_convert_module_soft_dec.h"

DecModule::DecModule(std::unique_ptr<DecMediatypeInterface>&& media, std::unique_ptr<DecDevice>&& device, deleted_unique_ptr<AL_TAllocator>&& allocator) :
  media(std::move(media)),
  device(std::move(device)),
  allocator(std::move(allocator))
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

BuffersRequirements DecModule::GetBuffersRequirements() const
{
  auto const streamSettings = media->settings.tStream;
  BuffersRequirements b;
  auto& input = b.input;
  input.min = 2; // We need 2 NALUs to detect the first one
  input.size = AL_GetMaxNalSize(streamSettings.tDim, streamSettings.eChroma);
  input.bytesAlignment = device->GetAllocationRequirements().input.bytesAlignment;
  input.contiguous = device->GetAllocationRequirements().input.contiguous;

  auto& output = b.output;
  output.min = media->GetRequiredOutputBuffers() + 1; // 1 for eos
  output.size = AL_GetAllocSize_Frame(streamSettings.tDim, streamSettings.eChroma, streamSettings.iBitDepth, false, media->settings.eFBStorageMode);
  output.bytesAlignment = device->GetAllocationRequirements().output.bytesAlignment;
  output.contiguous = device->GetAllocationRequirements().output.contiguous;

  return b;
}

Resolutions DecModule::GetResolutions() const
{
  auto const streamSettings = media->settings.tStream;
  Resolutions resolutions;
  auto& inRes = resolutions.input;
  auto& outRes = resolutions.output;

  inRes.width = outRes.width = streamSettings.tDim.iWidth;
  inRes.height = outRes.height = streamSettings.tDim.iHeight;

  inRes.stride = outRes.stride = AL_Decoder_RoundPitch(streamSettings.tDim.iWidth, streamSettings.iBitDepth, media->settings.eFBStorageMode);
  inRes.sliceHeight = outRes.sliceHeight = AL_Decoder_RoundHeight(streamSettings.tDim.iHeight);

  return resolutions;
}

Clocks DecModule::GetClocks() const
{
  Clocks clocks;

  auto& inClock = clocks.input;
  auto& outClock = clocks.output;

  inClock.framerate = outClock.framerate = media->settings.uFrameRate / 1000;
  inClock.clockratio = outClock.clockratio = media->settings.uClkRatio;
  return clocks;
}

Formats DecModule::GetFormats() const
{
  Formats formats;

  auto& inFormat = formats.input;
  inFormat.mime = media->Mime();
  inFormat.compression = media->Compression();

  auto& outFormat = formats.output;
  outFormat.mime = "video/x-raw"; // NV12
  outFormat.compression = COMPRESSION_UNUSED;

  auto const streamSettings = media->settings.tStream;
  inFormat.color = outFormat.color = ConvertToModuleColor(streamSettings.eChroma);
  inFormat.bitdepth = outFormat.bitdepth = streamSettings.iBitDepth;

  return formats;
}

ProfileLevelType DecModule::GetProfileLevel() const
{
  return media->ProfileLevel();
}

std::vector<ProfileLevelType> DecModule::GetProfileLevelSupported() const
{
  return media->ProfileLevelSupported();
}

int DecModule::GetLatency() const
{
  auto const settings = media->settings;
  auto const bufsCount = media->GetRequiredOutputBuffers() - settings.iStackSize;

  auto const realFramerate = (static_cast<double>(settings.uFrameRate) / static_cast<double>(settings.uClkRatio));

  auto const timeInMilliseconds = (static_cast<double>(bufsCount * 1000.0) / realFramerate);

  return std::ceil(timeInMilliseconds);
}

FileDescriptors DecModule::GetFileDescriptors() const
{
  return fds;
}

DecodedPictureBufferType DecModule::GetDecodedPictureBuffer() const
{
  auto const settings = media->settings;
  return ConvertToModuleDecodedPictureBuffer(settings.eDpbMode);
}

int DecModule::GetInternalEntropyBuffers() const
{
  auto const settings = media->settings;
  return settings.iStackSize;
}

bool DecModule::IsEnableSubframe() const
{
  auto const settings = media->settings;
  return DECODE_UNIT_SLICE == ConvertToModuleDecodeUnit(settings.eDecUnit);
}

static int GetPow2MaxAlignment(int const& pow2startAlignment, int const& value)
{
  if((pow2startAlignment % 2) != 0)
    return 0;
  if((value % pow2startAlignment) != 0)
    return 0;

  int n = 0;
  while((1 << n) != pow2startAlignment)
    n++;
  while((value % (1 << n)) == 0)
    n++;

  return 1 << (n - 1);
}

bool DecModule::SetResolutions(Resolutions const& resolutions)
{
  if(resolutions.input != resolutions.output)
    return false;

  if((GetPow2MaxAlignment(8, resolutions.input.stride) == 0)||(GetPow2MaxAlignment(8, resolutions.input.sliceHeight) == 0))
    return false;

  media->strideAlignment = GetPow2MaxAlignment(8, resolutions.input.stride);
  media->sliceHeightAlignment = GetPow2MaxAlignment(8, resolutions.input.sliceHeight);
  auto& streamSettings = media->settings.tStream;
  streamSettings.tDim = { resolutions.input.width, resolutions.input.height };
  return true;
}

bool DecModule::SetClocks(Clocks const& clocks)
{
  if(clocks.input != clocks.output)
    return false;

  auto& settings = media->settings;
  settings.uFrameRate = clocks.input.framerate * 1000;
  settings.uClkRatio = clocks.input.clockratio;
  return true;
}

bool DecModule::SetFormats(Formats const& formats)
{
  if(formats.input.bitdepth != formats.output.bitdepth)
    return false;

  if(formats.input.color != formats.output.color)
    return false;

  auto& streamSettings = media->settings.tStream;
  streamSettings.iBitDepth = formats.input.bitdepth;
  streamSettings.eChroma = ConvertToSoftChroma(formats.input.color);
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
  settings.eDpbMode = ConvertToSoftDecodedPictureBuffer(decodedPictureBuffer);
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
  settings.eDecUnit = enableSubframe ? ConvertToSoftDecodeUnit(DECODE_UNIT_SLICE) : ConvertToSoftDecodeUnit(DECODE_UNIT_FRAME);
  return true;
}

void DecModule::EndDecoding(AL_TBuffer* decodedFrame)
{
  if(!decodedFrame)
  {
    callbacks.event(CALLBACK_EVENT_ERROR, nullptr);
    return;
  }

  auto const handleOut = translateOut.Get(decodedFrame);
  assert(handleOut);

  callbacks.associate(nullptr, handleOut);
}

void DecModule::Display(AL_TBuffer* frameToDisplay, AL_TInfoDecode* info)
{
  auto const isRelease = (frameToDisplay && info == nullptr);

  if(isRelease)
  {
    auto const handleOut = translateOut.Get(frameToDisplay);
    assert(handleOut);
    dpb.Remove(handleOut);
    translateOut.Remove(frameToDisplay);

    callbacks.release(false, handleOut);

    AL_Buffer_Unref(frameToDisplay);
    return;
  }

  auto const isEOS = (frameToDisplay == nullptr && info == nullptr);

  if(isEOS)
  {
    auto const handleOut = translateOut.Get(eosHandles.output);
    assert(handleOut);

    dpb.Remove(handleOut);
    translateOut.Remove(eosHandles.output);

    auto const handleIn = translateIn.Get(eosHandles.input);
    auto const eosIsSent = handleIn != nullptr;

    if(eosIsSent)
    {
      callbacks.associate(handleIn, handleOut);
      AL_Buffer_Unref(eosHandles.input);
    }

    callbacks.filled(handleOut, 0, 0);

    std::lock_guard<std::mutex> lock(eosHandles.mutex);
    eosHandles.eosReceive = true;
    eosHandles.cv.notify_one();

    AL_Buffer_Unref(eosHandles.output);
    eosHandles.output = nullptr;

    return;
  }

  auto const size = GetBuffersRequirements().output.size;

  if(shouldBeCopied.Exist(frameToDisplay))
  {
    auto buffer = shouldBeCopied.Get(frameToDisplay);
    memcpy(buffer, AL_Buffer_GetData(frameToDisplay), size);
  }

  auto const handleOut = translateOut.Get(frameToDisplay);
  assert(handleOut);
  translateOut.Remove(frameToDisplay);

  callbacks.filled(handleOut, 0, size);
}

void DecModule::ResolutionFound(int const& bufferNumber, int const& bufferSize, AL_TStreamSettings const& settings, AL_TCropInfo const& crop)
{
  (void)bufferNumber;
  (void)bufferSize;
  (void)crop;

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

  auto handle = allocated.Get(buffer);
  assert(handle);

  AL_Allocator_Free(allocator.get(), handle);

  allocated.Remove(buffer);
}

void DecModule::FreeDMA(int fd)
{
  if(fd < 0)
    return;

  auto handle = allocatedDMA.Get(fd);
  assert(handle);

  AL_Allocator_Free(allocator.get(), handle);
  close(fd);

  allocatedDMA.Remove(fd);
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
  auto handleIn = translateIn.Get(input);
  assert(handleIn);
  translateIn.Remove(input);

  AL_Buffer_Destroy(input);

  callbacks.emptied(handleIn, 0, 0);
}

AL_TBuffer* DecModule::CreateInputBuffer(uint8_t* buffer, int const& size)
{
  AL_TBuffer* input = nullptr;

  if(fds.input)
  {
    auto const fd = static_cast<int>((intptr_t)buffer);

    if(fd < 0)
      throw std::invalid_argument("fd");

    auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator.get(), fd);

    if(!dmaHandle)
    {
      fprintf(stderr, "Failed to import fd : %i\n", fd);
      return nullptr;
    }

    input = AL_Buffer_Create(allocator.get(), dmaHandle, size, RedirectionInputBufferDestroy);
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

bool DecModule::Empty(uint8_t* buffer, int offset, int size)
{
  (void)offset;

  if(!decoder)
    return false;

  if(!buffer)
    return false;

  AL_TBuffer* input = CreateInputBuffer(buffer, size);

  if(!input)
    return false;

  translateIn.Add(input, buffer);

  auto const eos = (size == 0);

  if(eos)
  {
    eosHandles.input = input;
    AL_Decoder_Flush(decoder);
    eosHandles.eosSent = true;
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

void DecModule::OutputBufferDestroyAndFree(AL_TBuffer* output)
{
  assert(shouldBeCopied.Get(output));
  shouldBeCopied.Remove(output);

  AL_Allocator_Free(output->pAllocator, output->hBuf);
  AL_Buffer_Destroy(output);
}

AL_TBuffer* DecModule::CreateOutputBuffer(uint8_t* buffer, int const& size)
{
  auto const sourceMeta = CreateSourceMeta(media->settings.tStream, GetResolutions().output);

  if(!sourceMeta)
    return nullptr;

  AL_TBuffer* output = nullptr;

  if(fds.output)
  {
    auto const fd = static_cast<int>((intptr_t)buffer);

    if(fd < 0)
      throw std::invalid_argument("fd");

    auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator.get(), fd);

    if(!dmaHandle)
    {
      fprintf(stderr, "Failed to import fd : %i\n", fd);
      ((AL_TMetaData*)sourceMeta)->MetaDestroy((AL_TMetaData*)sourceMeta);
      return nullptr;
    }

    output = AL_Buffer_Create(allocator.get(), dmaHandle, size, RedirectionOutputBufferDestroy);
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

  if(!buffer)
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
  eosHandles.eosSent = false;
  eosHandles.eosReceive = false;

  return CreateDecoder(shouldPrealloc);
}

bool DecModule::Pause()
{
  if(!decoder)
    return false;

  DestroyDecoder();

  return false;

  // This is not fully supported yet

  if(!eosHandles.eosSent)
  {
    fprintf(stderr, "Passing in Pause without sending eos\n");
    assert(0);
    return false;
  }

  std::unique_lock<std::mutex> lock(eosHandles.mutex);
  eosHandles.cv.wait(lock, [&] { return eosHandles.eosReceive == true;
                     });
  return DestroyDecoder();
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

void DecModule::Stop()
{
  if(!decoder)
    return;

  DestroyDecoder();

  FlushEosHandles();
}

void DecModule::FlushEosHandles()
{
  auto const handleOut = translateOut.Get(eosHandles.output);

  if(handleOut)
  {
    dpb.Remove(handleOut);
    translateOut.Remove(eosHandles.output);

    callbacks.filled(handleOut, 0, 0);

    AL_Buffer_Unref(eosHandles.output);
    eosHandles.output = nullptr;
  }  
}