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

#include "module_dec.h"
#include <cmath>
#include <cassert>
#include <unistd.h> // close fd
#include <algorithm>
#include <utility/round.h>
#include <utility/logger.h>

extern "C"
{
#include <lib_common/BufferSrcMeta.h>
#include <lib_common/BufferStreamMeta.h>
#include <lib_common/BufferBufHandleMeta.h>
#include <lib_fpga/DmaAllocLinux.h>
#include <lib_common_dec/IpDecFourCC.h>
}

using namespace std;

DecModule::DecModule(shared_ptr<DecMediatypeInterface> media, shared_ptr<DecDeviceInterface> device, shared_ptr<AL_TAllocator> allocator) :
  media{media},
  device{device},
  allocator{allocator},
  decoder{nullptr},
  resolutionFoundAsBeenCalled{false}
{
  assert(this->media);
  assert(this->device);
  assert(this->allocator);
  currentDisplayPictureInfo.type = -1;
  currentDisplayPictureInfo.concealed = false;
  media->Reset();
}

DecModule::~DecModule() = default;

static map<int, string> MapToStringDecodeError =
{
  { AL_ERR_NO_MEMORY, "decoder: memory allocation failure (firmware or ctrlsw)" },
  { AL_ERR_CHAN_CREATION_NO_CHANNEL_AVAILABLE, "decoder: no channel available on the hardware" },
  { AL_ERR_CHAN_CREATION_RESOURCE_UNAVAILABLE, "decoder: hardware doesn't have enough resources" },
  { AL_ERR_CHAN_CREATION_NOT_ENOUGH_CORES, "decoder: hardware doesn't have enough resources (fragmentation)" },
  { AL_ERR_REQUEST_MALFORMED, "decoder: request to hardware was malformed" },
  { AL_WARN_CONCEAL_DETECT, "decoder: concealment" },
  { AL_WARN_SPS_NOT_COMPATIBLE_WITH_CHANNEL_SETTINGS, "decoder: some SPS not compatible with the channel settings has been disacarded" },
  { AL_WARN_SEI_OVERFLOW, "decoder: some SEI metadata buffer was too small" },
};

static string ToStringDecodeError(int error)
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
  return string {
           str_error + string {
             " ("
           } +to_string(error) + string {
             ")"
           }
  };
}

static ModuleInterface::ErrorType ToModuleError(int errorCode)
{
  switch(errorCode)
  {
  case AL_SUCCESS: return ModuleInterface::SUCCESS;
  case AL_ERR_CHAN_CREATION_NO_CHANNEL_AVAILABLE: return ModuleInterface::CHANNEL_CREATION_NO_CHANNEL_AVAILABLE;
  case AL_ERR_CHAN_CREATION_RESOURCE_UNAVAILABLE: return ModuleInterface::CHANNEL_CREATION_RESOURCE_UNAVAILABLE;
  case AL_ERR_CHAN_CREATION_NOT_ENOUGH_CORES: return ModuleInterface::CHANNEL_CREATION_RESOURCE_FRAGMENTED;
  case AL_ERR_REQUEST_MALFORMED: // fallthrough
  case AL_ERR_CMD_NOT_ALLOWED: // fallthrough
  case AL_ERR_INVALID_CMD_VALUE: return ModuleInterface::BAD_PARAMETER;
  case AL_ERR_NO_MEMORY: return ModuleInterface::NO_MEMORY;
  default: return ModuleInterface::UNDEFINED;
  }

  return ModuleInterface::UNDEFINED;
}

void DecModule::EndDecoding(AL_TBuffer* decodedFrame)
{
  if(!decodedFrame)
  {
    auto error = AL_Decoder_GetLastError(decoder);

    LOG_ERROR(ToStringDecodeError(error));

    if(AL_IS_ERROR_CODE(error))
      callbacks.event(Callbacks::Event::ERROR, (void*)ToModuleError(error));

    return;
  }

  auto handleOut = handles.Get(decodedFrame);
  assert(handleOut);

  bool isInputParsed;
  media->Get(SETTINGS_INDEX_INPUT_PARSED, &isInputParsed);

  if(!isInputParsed)
  {
    callbacks.associate(nullptr, handleOut);
    return;
  }

  auto handlesMeta = (AL_TBufHandleMetaData*)AL_Buffer_GetMetaData(decodedFrame, AL_META_TYPE_BUFHANDLE);

  if(!handlesMeta)
    assert(0 && "meta should be present");

  vector<AL_TSeiMetaData*> seis;

  for(int handle = 0; handle < handlesMeta->numHandles; handle++)
  {
    auto stream = handlesMeta->pHandles[handle];
    assert(stream);
    auto seiMeta = (AL_TSeiMetaData*)AL_Buffer_GetMetaData(stream, AL_META_TYPE_SEI);

    if(seiMeta != nullptr)
    {
      AL_Buffer_RemoveMetaData(stream, (AL_TMetaData*)seiMeta);
      seis.push_back(seiMeta);
    }
  }

  displaySeis.Add(decodedFrame, seis);

  auto handleIn = handles.Get(handlesMeta->pHandles[handlesMeta->numHandles - 1]);
  callbacks.associate(handleIn, handleOut);
}

void DecModule::ReleaseBufs(AL_TBuffer* frame)
{
  auto handleOut = handles.Pop(frame);
  dpb.Remove(handleOut->data);
  callbacks.release(false, handleOut);
  AL_Buffer_Unref(frame);
  return;
}

void DecModule::CopyIfRequired(AL_TBuffer* frameToDisplay, int size)
{
  if(!shouldBeCopied.Exist(frameToDisplay))
    return;
  auto buffer = (unsigned char*)(shouldBeCopied.Get(frameToDisplay));
  copy(AL_Buffer_GetData(frameToDisplay), AL_Buffer_GetData(frameToDisplay) + size, buffer);
}

void DecModule::Display(AL_TBuffer* frameToDisplay, AL_TInfoDecode* info)
{
  auto isRelease = (frameToDisplay && info == nullptr);

  if(isRelease)
    return ReleaseBufs(frameToDisplay);

  auto isEOS = (frameToDisplay == nullptr && info == nullptr);

  if(isEOS)
  {
    callbacks.filled(nullptr);
    return;
  }

  auto seis = displaySeis.Pop(frameToDisplay);

  if(!seis.empty())
  {
    for(auto const& sei: seis)
    {
      auto payload = sei->payload;

      for(int i = 0; i < sei->numPayload; ++i, ++payload)
      {
        if(payload->bPrefix)
          ParsedPrefixSei(payload->type, payload->pData, payload->size);
      }
    }
  }

  BufferSizes bufferSizes {};
  media->Get(SETTINGS_INDEX_BUFFER_SIZES, &bufferSizes);
  auto size = bufferSizes.output;
  CopyIfRequired(frameToDisplay, size);
  currentDisplayPictureInfo.type = info->ePicStruct;
  currentDisplayPictureInfo.concealed = (AL_Decoder_GetFrameError(decoder, frameToDisplay) == AL_WARN_CONCEAL_DETECT);
  auto handleOut = handles.Pop(frameToDisplay);
  handleOut->offset = 0;
  handleOut->payload = size;
  callbacks.filled(handleOut);

  if(!seis.empty())
  {
    for(auto const& sei: seis)
    {
      auto payload = sei->payload;

      for(int i = 0; i < sei->numPayload; ++i, ++payload)
      {
        if(!payload->bPrefix)
          ParsedSuffixSei(payload->type, payload->pData, payload->size);
      }

      AL_MetaData_Destroy((AL_TMetaData*)sei);
    }
  }

  currentDisplayPictureInfo.type = -1;
  currentDisplayPictureInfo.concealed = false;
}

void DecModule::ResolutionFound(int bufferNumber, int bufferSize, AL_TStreamSettings const& settings, AL_TCropInfo const& crop)
{
  (void)bufferNumber, (void)bufferSize, (void)crop;

  if(resolutionFoundAsBeenCalled)
  {
    /* TODO: implement dynamic resolution changes*/
    return;
  }
  resolutionFoundAsBeenCalled = true;
  media->settings.tStream = settings;

  StrideAlignments strideAlignments;
  media->Get(SETTINGS_INDEX_STRIDE_ALIGNMENTS, &strideAlignments);

  media->stride = RoundUp(static_cast<int>(AL_Decoder_GetMinPitch(settings.tDim.iWidth, settings.iBitDepth, media->settings.eFBStorageMode)), strideAlignments.horizontal);
  media->sliceHeight = RoundUp(static_cast<int>(AL_Decoder_GetMinStrideHeight(settings.tDim.iHeight)), strideAlignments.vertical);

  callbacks.event(Callbacks::Event::RESOLUTION_CHANGE, nullptr);
}

void DecModule::ParsedPrefixSei(int type, uint8_t* payload, int size)
{
  Sei sei {
    type, payload, size
  };
  callbacks.event(Callbacks::Event::SEI_PREFIX_PARSED, &sei);
}

void DecModule::ParsedSuffixSei(int type, uint8_t* payload, int size)
{
  Sei sei {
    type, payload, size
  };
  callbacks.event(Callbacks::Event::SEI_SUFFIX_PARSED, &sei);
}

ModuleInterface::ErrorType DecModule::CreateDecoder(bool shouldPrealloc)
{
  if(decoder)
  {
    LOG_ERROR("Decoder is ALREADY created");
    return UNDEFINED;
  }

  auto channel = device->Init();
  AL_TDecCallBacks decCallbacks {};
  decCallbacks.endDecodingCB = { RedirectionEndDecoding, this };
  decCallbacks.displayCB = { RedirectionDisplay, this };
  decCallbacks.resolutionFoundCB = { RedirectionResolutionFound, this };
  decCallbacks.parsedSeiCB = { RedirectionParsedSei, this };

  bool inputParsed;
  media->Get(SETTINGS_INDEX_INPUT_PARSED, &inputParsed);

  if(inputParsed)
    decCallbacks.parsedSeiCB = { nullptr, nullptr };

  auto errorCode = AL_Decoder_Create(&decoder, channel, allocator.get(), &media->settings, &decCallbacks);

  if(AL_IS_ERROR_CODE(errorCode))
  {
    LOG_ERROR(string { "Failed to create Decoder: " } +ToStringDecodeError(errorCode));
    return ToModuleError(errorCode);
  }

  if(shouldPrealloc)
  {
    if(!AL_Decoder_PreallocateBuffers(decoder))
    {
      auto errorCode = AL_Decoder_GetLastError(decoder);
      DestroyDecoder();
      return ToModuleError(errorCode);
    }
  }

  return SUCCESS;
}

bool DecModule::DestroyDecoder()
{
  if(!decoder)
  {
    LOG_ERROR("Decoder isn't created");
    return false;
  }

  AL_Decoder_Destroy(decoder);
  device->Deinit();
  decoder = nullptr;
  resolutionFoundAsBeenCalled = false;

  return true;
}

void DecModule::Free(void* buffer)
{
  if(!buffer)
    return;

  if(dpb.Exist((char*)buffer))
  {
    auto handle = dpb.Pop((char*)buffer);
    assert(!handles.Exist(handle));
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

  auto buffer = (char*)((intptr_t)fd);

  if(dpb.Exist(buffer))
  {
    auto handle = dpb.Pop(buffer);
    assert(!handles.Exist(handle));
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
    LOG_ERROR("No more memory");
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
    LOG_ERROR("No more memory");
    return -1;
  }

  auto fd = AL_LinuxDmaAllocator_GetFd((AL_TLinuxDmaAllocator*)allocator.get(), handle);
  allocatedDMA.Add(fd, handle);
  return fd;
}

static void StubCallbackEvent(Callbacks::Event, void*)
{
}

bool DecModule::SetCallbacks(Callbacks callbacks)
{
  if(!callbacks.emptied || !callbacks.associate || !callbacks.filled || !callbacks.release)
    return false;

  if(!callbacks.event)
    callbacks.event = &StubCallbackEvent;

  this->callbacks = callbacks;

  return true;
}

void DecModule::InputBufferDestroy(AL_TBuffer* input)
{
  auto hanleIn = handles.Pop(input);

  AL_Buffer_Destroy(input);

  hanleIn->offset = 0;
  hanleIn->payload = 0;
  callbacks.emptied(hanleIn);
}

void DecModule::InputBufferFreeWithoutDestroyingMemory(AL_TBuffer* input)
{
  auto hanleIn = handles.Pop(input);

  input->hBuf = nullptr;
  AL_Buffer_Destroy(input);

  hanleIn->offset = 0;
  hanleIn->payload = 0;
  callbacks.emptied(hanleIn);
}

static bool isFd(BufferHandleType type)
{
  return type == BufferHandleType::BUFFER_HANDLE_FD;
}

static bool isCharPtr(BufferHandleType type)
{
  return type == BufferHandleType::BUFFER_HANDLE_CHAR_PTR;
}

AL_TBuffer* DecModule::CreateInputBuffer(char* buffer, int size)
{
  AL_TBuffer* input {};
  BufferHandles bufferHandles {};
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &bufferHandles);

  if(isFd(bufferHandles.input))
  {
    auto fd = static_cast<int>((intptr_t)buffer);

    if(fd < 0)
      throw invalid_argument("fd");

    auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator.get(), fd);

    if(!dmaHandle)
    {
      LOG_ERROR(string { "Failed to import fd: " } +to_string(fd));
      return nullptr;
    }

    input = AL_Buffer_Create(allocator.get(), dmaHandle, size, RedirectionInputBufferDestroy);
  }

  if(isCharPtr(bufferHandles.input))
  {
    if(allocated.Exist(buffer))
      input = AL_Buffer_Create(allocator.get(), allocated.Get(buffer), size, RedirectionInputBufferFreeWithoutDestroyingMemory);
    else
    {
      bool isInputParsed;
      media->Get(SETTINGS_INDEX_INPUT_PARSED, &isInputParsed);

      if(isInputParsed)
      {
        input = AL_Buffer_Create_And_Allocate(allocator.get(), size, RedirectionInputBufferDestroy);
        copy(buffer, buffer + size, AL_Buffer_GetData(input));
      }
      else
        input = AL_Buffer_WrapData((uint8_t*)buffer, size, RedirectionInputBufferDestroy);
    }
  }

  if(input == nullptr)
    return nullptr;

  AL_Buffer_SetUserData(input, this);
  AL_Buffer_Ref(input);

  return input;
}

bool DecModule::CreateAndAttachStreamMeta(AL_TBuffer& buf)
{
  auto meta = (AL_TMetaData*)(AL_StreamMetaData_Create(1));

  if(!meta)
    return false;

  if(!AL_Buffer_AddMetaData(&buf, meta))
  {
    AL_MetaData_Destroy(meta);
    return false;
  }
  return true;
}

uint32_t ConvertModuleToSoftFlags(Flags flags)
{
  uint32_t softFlags = 0;

  if(flags.isSync)
    softFlags |= AL_SECTION_SYNC_FLAG;

  if(flags.isEndOfFrame)
    softFlags |= AL_SECTION_END_FRAME_FLAG;

  return softFlags;
}

bool DecModule::Empty(BufferHandleInterface* handle)
{
  if(!decoder)
    return false;

  auto eos = (!handle || handle->payload == 0);

  if(eos)
  {
    AL_Decoder_Flush(decoder);
    return true;
  }

  auto buffer = handle->data;
  AL_TBuffer* input = CreateInputBuffer(buffer, handle->payload);

  if(!input)
    return false;
  bool inputParsed = false;
  media->Get(SETTINGS_INDEX_INPUT_PARSED, &inputParsed);

  if(inputParsed)
  {
    if(!AL_Buffer_GetMetaData(input, AL_META_TYPE_STREAM))
    {
      if(!CreateAndAttachStreamMeta(*input))
        return false;
    }
    auto streamMeta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(input, AL_META_TYPE_STREAM));
    AL_StreamMetaData_ClearAllSections(streamMeta);
    AL_StreamMetaData_AddSection(streamMeta, 0, handle->payload, ConvertModuleToSoftFlags(currentFlags));

    if(!AL_Buffer_GetMetaData(input, AL_META_TYPE_SEI))
    {
      int maxSei = 32;
      int maxSeiBuf = 2 * 1024;
      auto pSeiMeta = AL_SeiMetaData_Create(maxSei, maxSeiBuf);

      if(!pSeiMeta)
        return false;
      AL_Buffer_AddMetaData(input, (AL_TMetaData*)pSeiMeta);
    }
  }

  handles.Add(input, handle);

  auto pushed = AL_Decoder_PushBuffer(decoder, input, handle->payload);
  AL_Buffer_Unref(input);

  return pushed;
}

static AL_TMetaData* CreateSourceMeta(AL_TStreamSettings const& streamSettings, Resolution resolution)
{
  auto picFormat = AL_GetDecPicFormat(streamSettings.eChroma, static_cast<uint8_t>(streamSettings.iBitDepth), AL_FB_RASTER, false);
  auto fourCC = AL_GetDecFourCC(picFormat);
  auto stride = resolution.stride.horizontal;
  auto sliceHeight = resolution.stride.vertical;
  AL_TPlane planeY = { 0, stride };
  AL_TPlane planeUV = { stride* sliceHeight, stride };
  return (AL_TMetaData*)(AL_SrcMetaData_Create({ resolution.width, resolution.height }, planeY, planeUV, fourCC));
}

void DecModule::OutputBufferDestroy(AL_TBuffer* output)
{
  output->hBuf = nullptr;
  AL_Buffer_Destroy(output);
}

void DecModule::OutputDmaBufferDestroy(AL_TBuffer* output)
{
  AL_Buffer_Destroy(output);
}

void DecModule::OutputBufferDestroyAndFree(AL_TBuffer* output)
{
  shouldBeCopied.Pop(output);
  AL_Buffer_Destroy(output);
}

AL_TBuffer* DecModule::CreateOutputBuffer(char* buffer, int size)
{
  Resolution resolution {};
  media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
  auto sourceMeta = CreateSourceMeta(media->settings.tStream, resolution);

  if(!sourceMeta)
    return nullptr;

  AL_TBuffer* output {};
  BufferHandles bufferHandles {};
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &bufferHandles);

  if(isFd(bufferHandles.output))
  {
    auto fd = static_cast<int>((intptr_t)buffer);

    if(fd < 0)
      throw invalid_argument("fd");

    auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator.get(), fd);

    if(!dmaHandle)
    {
      LOG_ERROR(string { "Failed to import fd: " } +to_string(fd));
      AL_MetaData_Destroy((AL_TMetaData*)sourceMeta);
      return nullptr;
    }

    output = AL_Buffer_Create(allocator.get(), dmaHandle, size, RedirectionOutputDmaBufferDestroy);
  }

  if(isCharPtr(bufferHandles.output))
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
    AL_MetaData_Destroy((AL_TMetaData*)sourceMeta);
    return nullptr;
  }

  auto attachedSourceMeta = AL_Buffer_AddMetaData(output, (AL_TMetaData*)sourceMeta);
  assert(attachedSourceMeta);

  AL_Buffer_SetUserData(output, this);
  dpb.Add(buffer, output);
  AL_Buffer_Ref(output);

  return output;
}

bool DecModule::Fill(BufferHandleInterface* handle)
{
  if(!decoder || !handle)
    return false;

  auto buffer = handle->data;

  auto output = dpb.Exist(buffer) ? dpb.Get(buffer) : CreateOutputBuffer(buffer, handle->size);

  if(!output)
    return false;

  handles.Add(output, handle);

  AL_Decoder_PutDisplayPicture(decoder, output);
  return true;
}

ModuleInterface::ErrorType DecModule::Start(bool shouldPrealloc)
{
  if(decoder)
  {
    LOG_ERROR("You can't call Start twice");
    return UNDEFINED;
  }

  return CreateDecoder(shouldPrealloc);
}

bool DecModule::Stop()
{
  if(!decoder)
    return false;

  DestroyDecoder();
  return true;
}

ModuleInterface::ErrorType DecModule::SetDynamic(std::string index, void const* param)
{
  if(index == "DYNAMIC_INDEX_STREAM_FLAGS")
  {
    currentFlags = *static_cast<Flags const*>(param);
    return SUCCESS;
  }

  return BAD_INDEX;
}

ModuleInterface::ErrorType DecModule::GetDynamic(std::string index, void* param)
{
  if(index == "DYNAMIC_INDEX_CURRENT_DISPLAY_PICTURE_INFO")
  {
    auto displayPictureInfo = static_cast<DisplayPictureInfo*>(param);
    displayPictureInfo->type = currentDisplayPictureInfo.type;
    return SUCCESS;
  }
  return BAD_INDEX;
}

