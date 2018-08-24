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

#include "omx_module_enc.h"
#include "omx_convert_module_soft_roi.h"
#include <cassert>
#include <cmath>
#include <unistd.h> // close fd
#include <cstring> // memcpy

extern "C"
{
#include <lib_common/BufferSrcMeta.h>
#include <lib_common/BufferStreamMeta.h>
#include <lib_common/BufferLookAheadMeta.h>
#include <lib_common/StreamBuffer.h>

#include <lib_common_enc/IpEncFourCC.h>
#include <lib_common_enc/EncBuffers.h>

#include <lib_fpga/DmaAllocLinux.h>
}

#include "base/omx_checker/omx_checker.h"
#include "base/omx_mediatype/omx_convert_module_soft_enc.h"
#include "base/omx_mediatype/omx_convert_module_soft.h"
#include "base/omx_utils/round.h"

using namespace std;

static ErrorType ToModuleError(int errorCode)
{
  switch(errorCode)
  {
  case AL_SUCCESS:
    return SUCCESS;
  case AL_ERR_CHAN_CREATION_NO_CHANNEL_AVAILABLE:
    return ERROR_CHAN_CREATION_NO_CHANNEL_AVAILABLE;
  case AL_ERR_CHAN_CREATION_RESOURCE_UNAVAILABLE:
    return ERROR_CHAN_CREATION_RESOURCE_UNAVAILABLE;
  case AL_ERR_CHAN_CREATION_NOT_ENOUGH_CORES:
    return ERROR_CHAN_CREATION_RESOURCE_FRAGMENTED;
  case AL_ERR_REQUEST_MALFORMED: // fallthrough
  case AL_ERR_CMD_NOT_ALLOWED: // fallthrough
  case AL_ERR_INVALID_CMD_VALUE:
    return ERROR_BAD_PARAMETER;
  case AL_ERR_NO_MEMORY:
    return ERROR_NO_MEMORY;
  default:
    return ERROR_UNDEFINED;
  }
}

static bool CheckValidity(AL_TEncSettings const& settings)
{
  auto errCount = AL_Settings_CheckValidity(const_cast<AL_TEncSettings*>(&settings), const_cast<AL_TEncChanParam*>(&settings.tChParam[0]), stderr);
  return errCount == 0;
}

static void LookCoherency(AL_TEncSettings& settings)
{
  auto chan = settings.tChParam[0];
  auto picFormat = AL_EncGetSrcPicFormat(AL_GET_CHROMA_MODE(chan.ePicFormat), AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER, false);
  auto fourCC = AL_EncGetSrcFourCC(picFormat);
  assert(AL_GET_BITDEPTH(chan.ePicFormat) == chan.uSrcBitDepth);
  AL_Settings_CheckCoherency(&settings, &settings.tChParam[0], fourCC, stdout);
}

EncModule::EncModule(shared_ptr<EncMediatypeInterface> media, shared_ptr<EncDevice> device, shared_ptr<AL_TAllocator> allocator) :
  media(media),
  device(device),
  allocator(allocator)
{
  assert(this->media);
  assert(this->device);
  assert(this->allocator);
  encoders.clear();
  isCreated = false;
  ResetRequirements();
}

EncModule::~EncModule() = default;

map<AL_ERR, string> MapToStringEncodeError =
{
  { AL_ERR_NO_MEMORY, "encoder: memory allocation failure (firmware or ctrlsw)" },
  { AL_ERR_STREAM_OVERFLOW, "encoder: stream overflow" },
  { AL_ERR_TOO_MANY_SLICES, "encoder: too many slices" },
  { AL_ERR_CHAN_CREATION_NO_CHANNEL_AVAILABLE, "encoder: no channel available on the hardware" },
  { AL_ERR_CHAN_CREATION_RESOURCE_UNAVAILABLE, "encoder: hardware doesn't have enough resources" },
  { AL_ERR_CHAN_CREATION_NOT_ENOUGH_CORES, "encoder, hardware doesn't have enough resources (fragmentation)" },
  { AL_ERR_REQUEST_MALFORMED, "encoder: request to hardware was malformed" },
  { AL_WARN_LCU_OVERFLOW, "encoder: lcu overflow" }
};

string ToStringEncodeError(int error)
{
  string str_error = "";
  try
  {
    str_error = MapToStringEncodeError.at(error);
  }
  catch(out_of_range& e)
  {
    str_error = "unknown error";
  }
  return str_error;
}

void EncModule::InitEncoders(int numPass)
{
  encoders.clear();

  for(auto pass = 0; pass < numPass; pass++)
  {
    PassEncoder encoderPass;
    encoderPass.index = pass;
    encoderPass.streamBuffer = nullptr;
    encoderPass.lookAheadParams.fifoSize = 0;
    encoderPass.EOSFinished = new promise<int>();

    if(pass < numPass - 1)
    {
      encoderPass.lookAheadParams.callbackParam = { this, pass };
      encoderPass.streamBuffer = AL_Buffer_Create_And_Allocate(allocator.get(), GetBufferRequirements().output.size, AL_Buffer_Destroy);
      AL_Buffer_Ref(encoderPass.streamBuffer);
    }

    encoders.push_back(encoderPass);
  }
}

ErrorType EncModule::CreateEncoder()
{
  if(encoders.size())
  {
    fprintf(stderr, "Encoder is ALREADY created\n");
    return ERROR_UNDEFINED;
  }

  auto chan = media->settings.tChParam[0];
  roiCtx = AL_RoiMngr_Create(chan.uWidth, chan.uHeight, chan.eProfile, AL_ROI_QUALITY_MEDIUM, AL_ROI_INCOMING_ORDER);

  if(!roiCtx)
  {
    fprintf(stderr, "Failed to create ROI manager:\n");
    return ERROR_BAD_PARAMETER;
  }

  auto settings = media->settings;
  scheduler = device->Init(settings, *allocator.get());
  auto numPass = 1;

#if AL_ENABLE_TWOPASS
  numPass = AL_TwoPassMngr_HasLookAhead(settings) ? 2 : 1;
#endif

  InitEncoders(numPass);

  for(auto pass = 0; pass < numPass; pass++)
  {
    auto settingsPass = media->settings;
    PassEncoder& encoderPass = encoders[pass];
    AL_CB_EndEncoding callback = { EncModule::RedirectionEndEncoding, this };

#if AL_ENABLE_TWOPASS

    if(pass < numPass - 1 && AL_TwoPassMngr_HasLookAhead(settings))
    {
      AL_TwoPassMngr_SetPass1Settings(settingsPass);
      callback = { EncModule::RedirectionEndEncodingLookAhead, &(encoderPass.lookAheadParams.callbackParam) };
      encoderPass.lookAheadParams.fifoSize = settings.LookAhead;
    }
#endif

    auto errorCode = AL_Encoder_Create(&encoderPass.enc, scheduler, allocator.get(), &settingsPass, callback);

    if(errorCode != AL_SUCCESS)
    {
      fprintf(stderr, "Failed to create first pass Encoder:\n");
      fprintf(stderr, "/!\\ %s (%d)\n", ToStringEncodeError(errorCode).c_str(), errorCode);
      return ToModuleError(errorCode);
    }

    if(pass < numPass - 1 && encoderPass.streamBuffer)
    {
      CreateAndAttachStreamMeta(*encoderPass.streamBuffer);
      AL_Encoder_PutStreamBuffer(encoderPass.enc, encoderPass.streamBuffer);
    }
  }

  eosHandles.output = nullptr;
  eosHandles.input = nullptr;

  return SUCCESS;
}

bool EncModule::DestroyEncoder()
{
  if(!encoders.size())
  {
    fprintf(stderr, "Encoder isn't created\n");
    return false;
  }

  for(uint8_t pass = 0; pass < encoders.size(); pass++)
  {
    PassEncoder encoder = encoders[pass];

    AL_Encoder_Destroy(encoder.enc);

    while(!encoder.roiBuffers.empty())
    {
      auto roiBuffer = encoder.roiBuffers.front();
      encoder.roiBuffers.pop_front();
      AL_Buffer_Unref(roiBuffer);
    }

    while(!encoder.fifo.empty())
    {
      auto src = encoder.fifo.front();
      encoder.fifo.pop_front();
      AL_Buffer_Unref(src);
    }

    if(encoder.streamBuffer)
      AL_Buffer_Unref(encoder.streamBuffer);

    delete encoder.EOSFinished;
  }

  encoders.clear();

  device->Deinit(scheduler);
  scheduler = nullptr;

  if(!roiCtx)
  {
    fprintf(stderr, "ROI manager isn't created\n");
    return false;
  }
  AL_RoiMngr_Destroy(roiCtx);

  return true;
}

bool EncModule::CheckParam()
{
  auto& settings = media->settings;

  if(!CheckValidity(settings))
    return false;

  LookCoherency(settings);

  return true;
}

bool EncModule::Create()
{
  if(encoders.size())
  {
    fprintf(stderr, "Encoder should NOT be created\n");
    return false;
  }
  isCreated = true;

  return true;
}

void EncModule::Destroy()
{
  assert(!encoders.size() && "Encoder should ALREADY be destroyed");
  isCreated = false;
}

ErrorType EncModule::Run(bool)
{
  if(encoders.size())
  {
    fprintf(stderr, "You can't call Run twice\n");
    return ERROR_UNDEFINED;
  }

  if(!isCreated)
  {
    fprintf(stderr, "You should call Create before Run\n");
    return ERROR_UNDEFINED;
  }

  return CreateEncoder();
}

void EncModule::FlushEosHandles()
{
  if(eosHandles.input)
    callbacks.release(true, eosHandles.input);

  if(eosHandles.output)
    callbacks.release(false, eosHandles.output);

  eosHandles.input = nullptr;
  eosHandles.output = nullptr;
}

bool EncModule::Pause()
{
  if(!encoders.size())
    return false;

  auto ret = DestroyEncoder();
  FlushEosHandles();
  return ret;
}

void EncModule::Stop()
{
  if(!encoders.size())
    return;

  DestroyEncoder();
  FlushEosHandles();
}

void EncModule::ResetRequirements()
{
  media->Reset();
}

static int RawAllocationSize(int stride, int sliceHeight, AL_EChromaMode eChromaMode)
{
  auto IP_WIDTH_ALIGNMENT = 32;
  auto IP_HEIGHT_ALIGNMENT = 8;
  assert(stride % IP_WIDTH_ALIGNMENT == 0); // IP requirements
  assert(sliceHeight % IP_HEIGHT_ALIGNMENT == 0); // IP requirements
  auto size = stride * sliceHeight;
  switch(eChromaMode)
  {
  case CHROMA_MONO: return size;
  case CHROMA_4_2_0: return (3 * size) / 2;
  case CHROMA_4_2_2: return 2 * size;
  default: return -1;
  }
}

BufferRequirements EncModule::GetBufferRequirements() const
{
  BufferRequirements b;
  BufferCounts bufferCounts;
  media->Get(SETTINGS_INDEX_BUFFER_COUNTS, &bufferCounts);
  auto chan = media->settings.tChParam[0];
  auto& input = b.input;
  input.min = bufferCounts.input;
  input.size = RawAllocationSize(media->stride, media->sliceHeight, AL_GET_CHROMA_MODE(chan.ePicFormat));
  input.bytesAlignment = device->GetBufferBytesAlignments().input;
  input.contiguous = device->GetBufferContiguities().input;

  auto& output = b.output;
  output.min = bufferCounts.output;
  output.min += 1; // for eos
  output.size = AL_GetMitigatedMaxNalSize({ chan.uWidth, chan.uHeight }, AL_GET_CHROMA_MODE(chan.ePicFormat), AL_GET_BITDEPTH(chan.ePicFormat));
  output.bytesAlignment = device->GetBufferBytesAlignments().output;
  output.contiguous = device->GetBufferContiguities().output;

  if(chan.bSubframeLatency)
  {
    Slices slices;
    media->Get(SETTINGS_INDEX_SLICE_PARAMETER, &slices);
    auto& size = output.size;
    size /= slices.num;
    size += 4095 * 2; /* we need space for the headers on each slice */
    size = RoundUp(size, 32); /* stream size is required to be 32 bits aligned */
  }

  return b;
}

static void StubCallbackEvent(CallbackEventType, void*)
{
}

bool EncModule::SetCallbacks(Callbacks callbacks)
{
  if(!callbacks.emptied || !callbacks.associate || !callbacks.filled || !callbacks.release)
    return false;

  if(!callbacks.event)
    callbacks.event = &StubCallbackEvent;

  this->callbacks = callbacks;

  return true;
}

bool EncModule::Flush()
{
  if(!encoders.size())
    return false;

  Stop();
  return Run(true) == SUCCESS;
}

void EncModule::Free(void* buffer)
{
  if(!buffer)
    return;

  auto handle = allocated.Pop(buffer);
  AL_Allocator_Free(allocator.get(), handle);
}

void EncModule::FreeDMA(int fd)
{
  if(fd < 0)
    return;

  auto handle = allocatedDMA.Pop(fd);
  AL_Allocator_Free(allocator.get(), handle);
  close(fd);
}

void* EncModule::Allocate(size_t size)
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

int EncModule::AllocateDMA(int size)
{
  auto handle = AL_Allocator_Alloc(allocator.get(), size);

  if(!handle)
  {
    fprintf(stderr, "No more memory\n");
    return -1;
  }

  auto fd = AL_LinuxDmaAllocator_GetFd((AL_TLinuxDmaAllocator*)allocator.get(), handle);
  allocatedDMA.Add(fd, handle);
  return fd;
}

static void FreeWithoutDestroyingMemory(AL_TBuffer* buffer)
{
  buffer->hBuf = NULL;
  AL_Buffer_Destroy(buffer);
}

bool EncModule::Use(BufferHandleInterface* handle, unsigned char* buffer, int size)
{
  if(!handle)
    throw invalid_argument("handle");

  if(!buffer)
    throw invalid_argument("buffer");

  AL_TBuffer* encoderBuffer = nullptr;

  if(allocated.Exist(buffer))
  {
    encoderBuffer = AL_Buffer_Create(allocator.get(), allocated.Get(buffer), size, FreeWithoutDestroyingMemory);
  }
  else if(size)
  {
    encoderBuffer = AL_Buffer_Create_And_Allocate(allocator.get(), size, AL_Buffer_Destroy);
    shouldBeCopied.Add(encoderBuffer, buffer);
  }
  else
    encoderBuffer = AL_Buffer_Create(allocator.get(), NULL, size, FreeWithoutDestroyingMemory);

  if(!encoderBuffer)
    return false;

  assert(!pool.Exist(handle));

  AL_Buffer_Ref(encoderBuffer);
  pool.Add(handle, encoderBuffer);

  return true;
}

bool EncModule::UseDMA(BufferHandleInterface* handle, int fd, int size)
{
  if(!handle)
    throw invalid_argument("handle");

  if(fd < 0)
    throw invalid_argument("fd");

  auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator.get(), fd);

  if(!dmaHandle)
  {
    fprintf(stderr, "Failed to import fd : %i\n", fd);
    return false;
  }

  auto encoderBuffer = AL_Buffer_Create(allocator.get(), dmaHandle, size, AL_Buffer_Destroy);

  if(!encoderBuffer)
    return false;

  assert(!pool.Exist(handle));

  AL_Buffer_Ref(encoderBuffer);
  pool.Add(handle, encoderBuffer);

  return true;
}

void EncModule::Unuse(BufferHandleInterface* handle)
{
  if(!handle)
    throw invalid_argument("handle");

  auto encoderBuffer = pool.Pop(handle);

  if(shouldBeCopied.Exist(encoderBuffer))
    shouldBeCopied.Remove(encoderBuffer);
  AL_Buffer_Unref(encoderBuffer);
}

void EncModule::UnuseDMA(BufferHandleInterface* handle)
{
  if(!handle)
    throw invalid_argument("handle");

  auto encoderBuffer = pool.Pop(handle);
  AL_Buffer_Unref(encoderBuffer);
}

static AL_TMetaData* CreateSourceMeta(shared_ptr<MediatypeInterface> media, Resolution const& resolution)
{
  Format format {};
  media->Get(SETTINGS_INDEX_FORMAT, &format);
  auto picFormat = AL_EncGetSrcPicFormat(ConvertModuleToSoftChroma(format.color), static_cast<uint8_t>(format.bitdepth), AL_FB_RASTER, false);
  auto fourCC = AL_EncGetSrcFourCC(picFormat);
  auto stride = resolution.stride.widthStride;
  auto sliceHeight = resolution.stride.heightStride;
  AL_TPitches const pitches = { stride, stride };
  AL_TOffsetYC const offsetYC = { 0, stride * sliceHeight };
  return (AL_TMetaData*)(AL_SrcMetaData_Create({ resolution.width, resolution.height }, pitches, offsetYC, fourCC));
}

static bool CreateAndAttachSourceMeta(AL_TBuffer& buf, shared_ptr<MediatypeInterface> media, Resolution const& resolution)
{
  auto meta = CreateSourceMeta(media, resolution);

  if(!meta)
    return false;

  if(!AL_Buffer_AddMetaData(&buf, meta))
  {
    meta->MetaDestroy(meta);
    return false;
  }
  return true;
}

#if AL_ENABLE_TWOPASS
static bool CreateAndAttachLookAheadMeta(AL_TBuffer& buf)
{
  auto meta = (AL_TLookAheadMetaData*)AL_Buffer_GetMetaData(&buf, AL_META_TYPE_LOOKAHEAD);

  if(!meta)
  {
    meta = AL_LookAheadMetaData_Create();

    if(!AL_Buffer_AddMetaData(&buf, (AL_TMetaData*)meta))
    {
      meta->tMeta.MetaDestroy((AL_TMetaData*)meta);
      return false;
    }
  }
  AL_LookAheadMetaData_Reset(meta);
  return true;
}

#endif

bool EncModule::Empty(BufferHandleInterface* handle)
{
  if(!encoders.size())
    return false;

  PassEncoder& currentEnc = encoders.front();
  AL_HEncoder encoder = currentEnc.enc;

  auto eos = (handle->payload == 0);

  if(eos)
  {
    eosHandles.input = handle;
    auto bRet = AL_Encoder_Process(encoder, nullptr, nullptr);

    if(bRet && (int)encoders.size() != 1)
    {
      currentEnc.EOSFinished->get_future().wait();
      EmptyFifo(currentEnc, true);
    }
    return bRet;
  }

  eosHandles.input = nullptr;

  uint8_t* buffer = (uint8_t*)handle->data;

  BufferHandles bufferHandles = GetBufferHandles();

  if(bufferHandles.input == BufferHandleType::BUFFER_HANDLE_FD)
    UseDMA(handle, static_cast<int>((intptr_t)buffer), handle->payload);
  else
    Use(handle, buffer, handle->payload);

  auto input = pool.Get(handle);

  if(!input)
    return false;

  if(!AL_Buffer_GetMetaData(input, AL_META_TYPE_SOURCE))
  {
    if(!CreateAndAttachSourceMeta(*input, media, GetResolution()))
      return false;
  }

#if AL_ENABLE_TWOPASS

  if(encoders.size() > 1)
    if(!CreateAndAttachLookAheadMeta(*input))
      return false;
#endif

  handles.Add(input, handle);

  if(shouldBeCopied.Exist(input))
  {
    auto buffer = shouldBeCopied.Get(input);
    memcpy(AL_Buffer_GetData(input), buffer, input->zSize);
  }

  if(currentEnc.roiBuffers.empty())
    return AL_Encoder_Process(encoder, input, nullptr);

  auto roiBuffer = currentEnc.roiBuffers.front();
  currentEnc.roiBuffers.pop_front();
  auto success = AL_Encoder_Process(encoder, input, roiBuffer);

  if(currentEnc.index != encoders.back().index)
    encoders[currentEnc.index + 1].roiBuffers.push_back(roiBuffer);
  else
    AL_Buffer_Unref(roiBuffer);

  return success;
}

bool EncModule::CreateAndAttachStreamMeta(AL_TBuffer& buf)
{
  auto meta = (AL_TMetaData*)(AL_StreamMetaData_Create(AL_MAX_SECTION));

  if(!meta)
    return false;

  if(!AL_Buffer_AddMetaData(&buf, meta))
  {
    meta->MetaDestroy(meta);
    return false;
  }
  return true;
}

bool EncModule::Fill(BufferHandleInterface* handle)
{
  if(!encoders.size())
    return false;

  AL_HEncoder encoder = encoders.back().enc;

  if(!eosHandles.output)
  {
    eosHandles.output = handle;
    return true;
  }

  auto buffer = (uint8_t*)handle->data;

  BufferHandles bufferHandles = GetBufferHandles();

  if(bufferHandles.output == BufferHandleType::BUFFER_HANDLE_FD)
    UseDMA(handle, static_cast<int>((intptr_t)buffer), handle->size);
  else
    Use(handle, buffer, handle->size);

  auto output = pool.Get(handle);

  if(!output)
    return false;

  if(!AL_Buffer_GetMetaData(output, AL_META_TYPE_STREAM))
  {
    if(!CreateAndAttachStreamMeta(*output))
      return false;
  }

  handles.Add(output, handle);

  return AL_Encoder_PutStreamBuffer(encoder, output);
}

static void AppendBuffer(uint8_t*& dst, uint8_t const* src, size_t len)
{
  memmove(dst, src, len);
  dst += len;
}

static int WriteOneSection(uint8_t*& dst, AL_TBuffer& stream, int numSection)
{
  auto meta = (AL_TStreamMetaData*)AL_Buffer_GetMetaData(&stream, AL_META_TYPE_STREAM);

  if(!meta->pSections[numSection].uLength)
    return 0;

  auto size = stream.zSize - meta->pSections[numSection].uOffset;

  if(size < (meta->pSections[numSection]).uLength)
  {
    AppendBuffer(dst, (AL_Buffer_GetData(&stream) + meta->pSections[numSection].uOffset), size);
    AppendBuffer(dst, AL_Buffer_GetData(&stream), (meta->pSections[numSection]).uLength - size);
  }
  else
    AppendBuffer(dst, (AL_Buffer_GetData(&stream) + meta->pSections[numSection].uOffset), meta->pSections[numSection].uLength);

  return meta->pSections[numSection].uLength;
}

static int ReconstructStream(AL_TBuffer& stream)
{
  auto origin = AL_Buffer_GetData(&stream);
  auto size = 0;

  auto meta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(&stream, AL_META_TYPE_STREAM));
  assert(meta);

  for(auto i = 0; i < meta->uNumSection; i++)
    size += WriteOneSection(origin, stream, i);

  return size;
}

void EncModule::ReleaseBuf(AL_TBuffer const* buf, bool isDma, bool isSrc)
{
  auto rhandle = handles.Pop(buf);

  if(isDma)
    UnuseDMA(rhandle);
  else
    Unuse(rhandle);

  callbacks.release(isSrc, rhandle);
}

bool EncModule::isEndOfFrame(AL_TBuffer* stream)
{
  auto flags = GetFlags(stream);
  return flags.isEndOfFrame;
}

void EncModule::EndEncoding(AL_TBuffer* stream, AL_TBuffer const* source)
{
  AL_HEncoder encoder = encoders.back().enc;

  auto errorCode = AL_Encoder_GetLastError(encoder);

  if(errorCode != AL_SUCCESS)
  {
    fprintf(stderr, "/!\\ %s (%d)\n", ToStringEncodeError(errorCode).c_str(), errorCode);

    if((errorCode & AL_ERROR) && (errorCode != AL_ERR_STREAM_OVERFLOW))
      callbacks.event(CALLBACK_EVENT_ERROR, (void*)ToModuleError(errorCode));
  }

  BufferHandles bufferHandles = GetBufferHandles();

  auto isSrcRelease = (stream == nullptr && source);

  if(isSrcRelease)
  {
    ReleaseBuf(source, bufferHandles.input == BufferHandleType::BUFFER_HANDLE_FD, true);
    return;
  }

  auto isStreamRelease = (stream && source == nullptr);

  if(isStreamRelease)
  {
    ReleaseBuf(stream, bufferHandles.output == BufferHandleType::BUFFER_HANDLE_FD, false);
    return;
  }

  auto isEOS = (stream == nullptr && source == nullptr);

  if(isEOS)
  {
    callbacks.associate(eosHandles.input, eosHandles.output);
    eosHandles.input->offset = 0;
    eosHandles.input->payload = 0;
    callbacks.emptied(eosHandles.input);
    eosHandles.output->offset = 0;
    eosHandles.output->payload = 0;
    callbacks.filled(eosHandles.output, eosHandles.output->offset, eosHandles.output->payload);
    eosHandles.input = nullptr;
    eosHandles.output = nullptr;
    return;
  }

  auto size = ReconstructStream(*stream);

  if(shouldBeCopied.Exist(stream))
  {
    auto buffer = shouldBeCopied.Get(stream);
    memcpy(buffer, AL_Buffer_GetData(stream), size);
  }
  auto rhandleIn = handles.Get(source);
  assert(rhandleIn->data);

  auto rhandleOut = handles.Get(stream);
  assert(rhandleOut->data);

  callbacks.associate(rhandleIn, rhandleOut);

  if(isEndOfFrame(stream))
    handles.Remove(source);

  handles.Remove(stream);

  if(isEndOfFrame(stream))
  {
    if(bufferHandles.input == BufferHandleType::BUFFER_HANDLE_FD)
      UnuseDMA(rhandleIn);
    else
      Unuse(rhandleIn);

    rhandleIn->offset = 0;
    rhandleIn->payload = 0;
    callbacks.emptied(rhandleIn);
  }

  if(bufferHandles.output == BufferHandleType::BUFFER_HANDLE_FD)
    UnuseDMA(rhandleOut);
  else
    Unuse(rhandleOut);

  rhandleOut->offset = 0;
  rhandleOut->payload = size;
  callbacks.filled(rhandleOut, rhandleOut->offset, rhandleOut->payload);
}

void EncModule::EndEncodingLookAhead(AL_TBuffer* stream, AL_TBuffer const* source, int index)
{
  assert(index < (int)encoders.size() - 1);

  auto isStreamRelease = (stream && source == nullptr);
  auto isSrcRelease = (stream == nullptr && source);
  auto isEOS = (stream == nullptr && source == nullptr);

  PassEncoder& encoder = encoders[index];

  auto errorCode = AL_Encoder_GetLastError(encoder.enc);

  if(errorCode != AL_SUCCESS)
  {
    fprintf(stderr, "/!\\ %s (%d)\n", ToStringEncodeError(errorCode).c_str(), errorCode);

    if((errorCode & AL_ERROR) && (errorCode != AL_ERR_STREAM_OVERFLOW))
      callbacks.event(CALLBACK_EVENT_ERROR, (void*)ToModuleError(errorCode));
  }

  BufferHandles bufferHandles = GetBufferHandles();

  if(isSrcRelease)
  {
    ReleaseBuf(source, bufferHandles.input == BufferHandleType::BUFFER_HANDLE_FD, true);
    return;
  }

  if(isStreamRelease)
    return;

  if(isEOS || isEndOfFrame(stream))
  {
    AddFifo(encoder, (AL_TBuffer*)source);

    if(encoder.streamBuffer)
      AL_Encoder_PutStreamBuffer(encoder.enc, encoder.streamBuffer);

    return;
  }
}

void EncModule::AddFifo(PassEncoder& encoder, AL_TBuffer* src)
{
  bool isEOS = (src == nullptr);

  if(!isEOS)
  {
    AL_Buffer_Ref(src);
    encoder.fifo.push_back(src);
    EmptyFifo(encoder, isEOS);
  }
  else
    encoder.EOSFinished->set_value(0);
}

void EncModule::EmptyFifo(PassEncoder& encoder, bool isEOS)
{
  assert(encoder.index < (int)encoders.size() - 1);

  PassEncoder nextEnc = encoders[encoder.index + 1];

#if AL_ENABLE_TWOPASS

  if(encoder.lookAheadParams.fifoSize >= 10)
    encoder.ComputeComplexity(isEOS);
#endif

  if(isEOS && encoder.fifo.size() == 0)
    AL_Encoder_Process(nextEnc.enc, nullptr, nullptr);

  else if(isEOS || (int)encoder.fifo.size() == encoder.lookAheadParams.fifoSize)
  {
    auto src = encoder.fifo.front();
    encoder.fifo.pop_front();

#if AL_ENABLE_TWOPASS
    encoder.ProcessLookAheadParams(src);
#endif

    AL_TBuffer* roiBuffer = nullptr;

    if(!nextEnc.roiBuffers.empty())
    {
      roiBuffer = nextEnc.roiBuffers.front();
      nextEnc.roiBuffers.pop_front();
    }

    AL_Encoder_Process(nextEnc.enc, src, roiBuffer);

    if(roiBuffer)
    {
      if(nextEnc.index != encoders.back().index)
        encoders[nextEnc.index + 1].roiBuffers.push_back(roiBuffer);
      else
        AL_Buffer_Unref(roiBuffer);
    }

    AL_Buffer_Unref(src);

    if(isEOS)
      EmptyFifo(encoder, isEOS);
  }
}

Resolution EncModule::GetResolution() const
{
  Resolution resolution;
  media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
  return resolution;
}

BufferHandles EncModule::GetBufferHandles() const
{
  BufferHandles handles;
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &handles);
  return handles;
}

Flags EncModule::GetFlags(AL_TBuffer* stream)
{
  auto meta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(stream, AL_META_TYPE_STREAM));
  assert(meta);

  Flags flags {};

  for(auto i = 0; i < meta->uNumSection; i++)
  {
    if(meta->pSections[i].uFlags & SECTION_SYNC_FLAG)
      flags.isSync = true;

    if(!(meta->pSections[i].uFlags & SECTION_CONFIG_FLAG))
      flags.isEndOfSlice = true;

    if(meta->pSections[i].uFlags & SECTION_END_FRAME_FLAG)
      flags.isEndOfFrame = true;
  }

  return flags;
}

Flags EncModule::GetFlags(BufferHandleInterface* handle)
{
  AL_TBuffer* stream = nullptr;

  if(pool.Exist(handle))
    stream = pool.Get(handle);

  if(!stream)
    return Flags {};

  return GetFlags(stream);
}

ErrorType EncModule::SetDynamic(std::string index, void const* param)
{
  if(!encoders.size())
    return ERROR_UNDEFINED;

  AL_HEncoder encoder = encoders.back().enc;

  if(index == "DYNAMIC_INDEX_CLOCK")
  {
    auto clock = static_cast<Clock const*>(param);
    auto ret = media->Set(SETTINGS_INDEX_CLOCK, clock);
    assert(ret == MediatypeInterface::ERROR_SETTINGS_NONE);
    AL_Encoder_SetFrameRate(encoder, clock->framerate, clock->clockratio);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_BITRATE")
  {
    auto bitrate = static_cast<int>((intptr_t)param);
    Bitrate mediaBitrate;
    media->Get(SETTINGS_INDEX_BITRATE, &mediaBitrate);
    mediaBitrate.target = bitrate;

    if(mediaBitrate.mode != RateControlType::RATE_CONTROL_VARIABLE_BITRATE)
      mediaBitrate.max = mediaBitrate.target;
    media->Set(SETTINGS_INDEX_BITRATE, &mediaBitrate);
    AL_Encoder_SetBitRate(encoder, bitrate * 1000);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_INSERT_IDR")
  {
    AL_Encoder_RestartGop(encoder);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_GOP")
  {
    auto gop = static_cast<Gop const*>(param);
    auto ret = media->Set(SETTINGS_INDEX_GROUP_OF_PICTURES, gop);
    assert(ret == MediatypeInterface::ERROR_SETTINGS_NONE);
    AL_Encoder_SetGopNumB(encoder, gop->b);
    AL_Encoder_SetGopLength(encoder, gop->length);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD")
  {
    assert(roiCtx);
    auto roi = static_cast<RegionQuality const*>(param);
    auto ret = AL_RoiMngr_AddROI(roiCtx, roi->region.x, roi->region.y, roi->region.width, roi->region.height, ConvertModuleToSoftQuality(roi->quality));
    assert(ret);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_CLEAR")
  {
    assert(roiCtx);
    AL_RoiMngr_Clear(roiCtx);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_EMPTY")
  {
    assert(roiCtx);
    auto bufferToEmpty = static_cast<char const*>(param);
    auto chan = media->settings.tChParam[0];
    AL_TDimension tDim = { chan.uWidth, chan.uHeight };
    auto size = AL_GetAllocSizeEP2(tDim, chan.uMaxCuSize);
    auto roiBuffer = AL_Buffer_Create_And_Allocate(allocator.get(), size, AL_Buffer_Destroy);
    AL_Buffer_Ref(roiBuffer);
    memcpy(AL_Buffer_GetData(roiBuffer), bufferToEmpty, size);
    encoders.front().roiBuffers.push_back(roiBuffer);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_NOTIFY_SCENE_CHANGE")
  {
    auto lookAhead = static_cast<int>((intptr_t)param);
    AL_Encoder_NotifySceneChange(encoder, lookAhead);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_IS_LONG_TERM")
  {
    AL_Encoder_NotifyIsLongTerm(encoder);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_USE_LONG_TERM")
  {
    AL_Encoder_NotifyUseLongTerm(encoder);
    return SUCCESS;
  }

  return ERROR_NOT_IMPLEMENTED;
}

ErrorType EncModule::GetDynamic(std::string index, void* param)
{
  if(index == "DYNAMIC_INDEX_CLOCK")
  {
    media->Get(SETTINGS_INDEX_CLOCK, static_cast<Clock*>(param));
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_BITRATE")
  {
    Bitrate mediaBitrate;
    media->Get(SETTINGS_INDEX_BITRATE, &mediaBitrate);
    *static_cast<int*>(param) = mediaBitrate.target;
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_GOP")
  {
    media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, static_cast<Gop*>(param));
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_FILL")
  {
    assert(roiCtx);
    uint8_t* bufferToFill = static_cast<uint8_t*>(param);
    AL_RoiMngr_FillBuff(roiCtx, 1, 1, bufferToFill + EP2_BUF_QP_BY_MB.Offset);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_SIZE")
  {
    Resolution mediaResolution;
    media->Get(SETTINGS_INDEX_RESOLUTION, &mediaResolution);
    AL_TDimension tDim = { mediaResolution.width, mediaResolution.height };
    *static_cast<int*>(param) = AL_GetAllocSizeEP2(tDim, media->settings.tChParam[0].uMaxCuSize);
    return SUCCESS;
  }

  return ERROR_NOT_IMPLEMENTED;
}

#if AL_ENABLE_TWOPASS

void PassEncoder::ProcessLookAheadParams(AL_TBuffer* src)
{
  auto pPictureMetaLA = (AL_TLookAheadMetaData*)AL_Buffer_GetMetaData(src, AL_META_TYPE_LOOKAHEAD);
  auto fifoSize = (int)fifo.size();

  if(pPictureMetaLA)
  {
    if(lookAheadParams.fifoSize >= 10)
      pPictureMetaLA->iComplexity = lookAheadParams.complexity;

    if(fifoSize >= 1)
    {
      pPictureMetaLA->bNextSceneChange = AL_TwoPassMngr_SceneChangeDetected(src, fifo[0]);
      pPictureMetaLA->iIPRatio = AL_TwoPassMngr_GetIPRatio(src, fifo[0]);

      for(int i = 1; (i < std::min(fifoSize, 3)); i++)
        pPictureMetaLA->iIPRatio = std::min(pPictureMetaLA->iIPRatio, AL_TwoPassMngr_GetIPRatio(src, fifo[i]));
    }
  }
}

void PassEncoder::ComputeComplexity(bool isEOS)
{
  lookAheadParams.complexityCount++;
  int fifoSize = static_cast<int>(fifo.size());

  if(lookAheadParams.complexityCount >= 5 && (isEOS || fifoSize == lookAheadParams.fifoSize))
  {
    lookAheadParams.complexityCount = 0;
    lookAheadParams.complexity = 1000;

    if(fifoSize >= 5 && AL_Buffer_GetMetaData(fifo.front(), AL_META_TYPE_LOOKAHEAD))
    {
      int iComp[2] = { 0, 0 };

      for(int i = 0; i < fifoSize; i++)
      {
        auto pPictureMetaLA = (AL_TLookAheadMetaData*)AL_Buffer_GetMetaData(fifo[i], AL_META_TYPE_LOOKAHEAD);
        iComp[(i < 5) ? 0 : 1] += pPictureMetaLA->iPictureSize;
      }

      lookAheadParams.complexity = ((1000 * fifoSize / 5) + lookAheadParams.complexityDiff) * iComp[0] / (iComp[0] + iComp[1]);
      lookAheadParams.complexityDiff += (1000 - lookAheadParams.complexity);
    }
  }
}

#endif

