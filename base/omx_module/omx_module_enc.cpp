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

#include "omx_module_enc.h"
#include "omx_convert_module_soft_roi.h"
#include <cassert>
#include <unistd.h>
#include <cmath>

extern "C"
{
#include <lib_common/BufferSrcMeta.h>
#include <lib_common/BufferStreamMeta.h>
#include <lib_common/StreamBuffer.h>

#include <lib_common_enc/IpEncFourCC.h>
#include <lib_common_enc/EncBuffers.h>

#include <lib_fpga/DmaAllocLinux.h>
}

#include "base/omx_checker/omx_checker.h"
#include "base/omx_settings/omx_convert_module_soft_enc.h"
#include "base/omx_settings/omx_convert_module_soft.h"
#include "base/omx_settings/omx_settings_structs.h"
#include "base/omx_utils/roundup.h"

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
  auto err = AL_Settings_CheckValidity(const_cast<AL_TEncSettings*>(&settings), const_cast<AL_TEncChanParam*>(&settings.tChParam[0]), stderr);

  return err == AL_SUCCESS;
}

static void LookCoherency(AL_TEncSettings& settings)
{
  auto const chan = settings.tChParam[0];
  auto const fourCC = AL_EncGetSrcFourCC({ AL_GET_CHROMA_MODE(chan.ePicFormat), AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER });
  assert(AL_GET_BITDEPTH(chan.ePicFormat) == chan.uEncodingBitDepth);
  AL_Settings_CheckCoherency(&settings, &settings.tChParam[0], fourCC, stdout);
}

EncModule::EncModule(shared_ptr<EncMediatypeInterface> media, unique_ptr<EncDevice>&& device, deleted_unique_ptr<AL_TAllocator>&& allocator) :
  media(media),
  device(move(device)),
  allocator(move(allocator))
{
  assert(this->media);
  assert(this->device);
  assert(this->allocator);
  encoder = nullptr;
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

ErrorType EncModule::CreateEncoder()
{
  if(encoder)
  {
    fprintf(stderr, "Encoder is ALREADY created\n");
    return ERROR_UNDEFINED;
  }

  auto const chan = media->settings.tChParam[0];
  roiCtx = AL_RoiMngr_Create(chan.uWidth, chan.uHeight, chan.eProfile, AL_ROI_QUALITY_MEDIUM, AL_ROI_INCOMING_ORDER);

  if(!roiCtx)
  {
    fprintf(stderr, "Failed to create ROI manager:\n");
    return ERROR_BAD_PARAMETER;
  }

  auto& settings = media->settings;
  scheduler = device->Init(settings, *allocator.get());
  auto errorCode = AL_Encoder_Create(&encoder, scheduler, allocator.get(), &media->settings, { EncModule::RedirectionEndEncoding, this });

  if(errorCode != AL_SUCCESS)
  {
    fprintf(stderr, "Failed to create Encoder:\n");
    fprintf(stderr, "/!\\ %s (%d)\n", ToStringEncodeError(errorCode).c_str(), errorCode);
    return ToModuleError(errorCode);
  }

  eosHandles.output = nullptr;
  eosHandles.input = nullptr;

  return SUCCESS;
}

bool EncModule::DestroyEncoder()
{
  if(!encoder)
  {
    fprintf(stderr, "Encoder isn't created\n");
    return false;
  }

  AL_Encoder_Destroy(encoder);
  encoder = NULL;

  device->Deinit(scheduler);
  scheduler = nullptr;

  if(!roiCtx)
  {
    fprintf(stderr, "ROI manager isn't created\n");
    return false;
  }
  AL_RoiMngr_Destroy(roiCtx);

  while(!roiBuffers.empty())
  {
    auto roiBuffer = roiBuffers.front();
    roiBuffers.pop_front();
    AL_Buffer_Unref(roiBuffer);
  }

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
  if(encoder)
  {
    fprintf(stderr, "Encoder should NOT be created\n");
    return false;
  }
  isCreated = true;

  return true;
}

void EncModule::Destroy()
{
  assert(!encoder && "Encoder should ALREADY be destroyed");
  isCreated = false;
}

ErrorType EncModule::Run(bool)
{
  if(encoder)
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
    ReleaseBuf(eosHandles.input, fds.input, true);

  if(eosHandles.output)
    ReleaseBuf(eosHandles.output, fds.output, false);

  eosHandles.input = nullptr;
  eosHandles.output = nullptr;
}

bool EncModule::Pause()
{
  if(!encoder)
    return false;

  auto ret = DestroyEncoder();
  FlushEosHandles();
  return ret;
}

void EncModule::Stop()
{
  if(!encoder)
    return;

  DestroyEncoder();
  FlushEosHandles();
}

void EncModule::ResetRequirements()
{
  media->Reset();
  fds.input = fds.output = false;
}

static int RawAllocationSize(int stride, int sliceHeight, AL_EChromaMode eChromaMode)
{
  auto const IP_WIDTH_ALIGNMENT = 32;
  auto const IP_HEIGHT_ALIGNMENT = 8;
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
  auto const chan = media->settings.tChParam[0];
  auto& input = b.input;
  input.min = bufferCounts.input;
  input.size = RawAllocationSize(media->stride, media->sliceHeight, AL_GET_CHROMA_MODE(chan.ePicFormat));
  input.bytesAlignment = device->GetAllocationRequirements().input.bytesAlignment;
  input.contiguous = device->GetAllocationRequirements().input.contiguous;

  auto& output = b.output;
  output.min = bufferCounts.output;
  output.min += 1; // for eos
  output.size = AL_GetMaxNalSize({ chan.uWidth, chan.uHeight }, AL_GET_CHROMA_MODE(chan.ePicFormat), AL_GET_BITDEPTH(chan.ePicFormat));
  output.bytesAlignment = device->GetAllocationRequirements().output.bytesAlignment;
  output.contiguous = device->GetAllocationRequirements().output.contiguous;

  if(chan.bSubframeLatency)
  {
    Slices slices;
    media->Get(SETTINGS_INDEX_SLICE_PARAMETER, &slices);
    auto& size = output.size;
    size /= slices.num;
    size += 4095 * 2; /* we need space for the headers on each slice */
    size = (size + 31) & ~31; /* stream size is required to be 32 bits aligned */
  }

  return b;
}

int EncModule::GetLatency() const
{
  int latency;
  media->Get(SETTINGS_INDEX_LATENCY, &latency);
  return latency;
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
  if(!encoder)
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

static AL_TMetaData* CreateSourceMeta(AL_TEncChanParam const& chan, Resolution const& resolution)
{
  auto const fourCC = AL_EncGetSrcFourCC({ AL_GET_CHROMA_MODE(chan.ePicFormat), AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER });
  auto const stride = resolution.stride;
  auto const sliceHeight = resolution.sliceHeight;
  AL_TPitches const pitches = { stride, stride };
  AL_TOffsetYC const offsetYC = { 0, stride * sliceHeight };
  return (AL_TMetaData*)(AL_SrcMetaData_Create({ chan.uWidth, chan.uHeight }, pitches, offsetYC, fourCC));
}

static bool CreateAndAttachSourceMeta(AL_TBuffer& buf, AL_TEncChanParam const& chan, Resolution const& resolution)
{
  auto const meta = CreateSourceMeta(chan, resolution);

  if(!meta)
    return false;

  if(!AL_Buffer_AddMetaData(&buf, meta))
  {
    meta->MetaDestroy(meta);
    return false;
  }
  return true;
}

bool EncModule::Empty(BufferHandleInterface* handle)
{
  if(!encoder)
    return false;

  uint8_t* buffer = (uint8_t*)handle->data;

  if(fds.input)
    UseDMA(handle, static_cast<int>((intptr_t)buffer), handle->payload);
  else
    Use(handle, buffer, handle->payload);

  auto input = pool.Get(handle);

  if(!input)
    return false;

  if(!AL_Buffer_GetMetaData(input, AL_META_TYPE_SOURCE))
  {
    if(!CreateAndAttachSourceMeta(*input, media->settings.tChParam[0], GetResolution()))
      return false;
  }

  handles.Add(input, handle);

  auto const eos = (handle->payload == 0);

  if(eos)
  {
    eosHandles.input = input;
    return AL_Encoder_Process(encoder, nullptr, nullptr);
  }

  eosHandles.input = nullptr;

  if(shouldBeCopied.Exist(input))
  {
    auto buffer = shouldBeCopied.Get(input);
    memcpy(AL_Buffer_GetData(input), buffer, input->zSize);
  }

  AL_TBuffer* roiBuffer = nullptr;

  if(!roiBuffers.empty())
  {
    roiBuffer = roiBuffers.front();
    roiBuffers.pop_front();
  }

  auto success = AL_Encoder_Process(encoder, input, roiBuffer);

  if(roiBuffer)
    AL_Buffer_Unref(roiBuffer);
  return success;
}

static bool CreateAndAttachStreamMeta(AL_TBuffer& buf)
{
  auto const meta = (AL_TMetaData*)(AL_StreamMetaData_Create(AL_MAX_SECTION));

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
  if(!encoder)
    return false;

  auto buffer = (uint8_t*)handle->data;

  if(fds.output)
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

  if(!eosHandles.output)
  {
    eosHandles.output = output;
    return true;
  }

  return AL_Encoder_PutStreamBuffer(encoder, output);
}

static void AppendBuffer(uint8_t*& dst, uint8_t const* src, size_t len)
{
  memmove(dst, src, len);
  dst += len;
}

static int WriteOneSection(uint8_t*& dst, AL_TBuffer& stream, int numSection)
{
  auto const meta = (AL_TStreamMetaData*)AL_Buffer_GetMetaData(&stream, AL_META_TYPE_STREAM);

  if(!meta->pSections[numSection].uLength)
    return 0;

  auto const size = stream.zSize - meta->pSections[numSection].uOffset;

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

  auto const meta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(&stream, AL_META_TYPE_STREAM));
  assert(meta);

  for(auto i = 0; i < meta->uNumSection; i++)
    size += WriteOneSection(origin, stream, i);

  return size;
}

void EncModule::ReleaseBuf(AL_TBuffer const* buf, bool isDma, bool isSrc)
{
  auto const rhandle = handles.Pop(buf);

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
  auto const isStreamRelease = (stream && source == nullptr);
  auto const isSrcRelease = (stream == nullptr && source);

  auto errorCode = AL_Encoder_GetLastError(encoder);

  if(errorCode != AL_SUCCESS)
  {
    fprintf(stderr, "/!\\ %s (%d)\n", ToStringEncodeError(errorCode).c_str(), errorCode);

    if((errorCode & AL_ERROR) && (errorCode != AL_ERR_STREAM_OVERFLOW))
      callbacks.event(CALLBACK_EVENT_ERROR, (void*)ToModuleError(errorCode));
  }

  if(isSrcRelease)
  {
    ReleaseBuf(source, fds.input, true);
    return;
  }

  if(isStreamRelease)
  {
    ReleaseBuf(stream, fds.output, false);
    return;
  }

  auto const isEOS = (stream == nullptr && source == nullptr);

  auto end = [&](AL_TBuffer* source, AL_TBuffer* stream, int size)
             {
               assert(source);
               assert(stream);
               auto const rhandleIn = handles.Get(source);
               assert(rhandleIn->data);

               auto const rhandleOut = handles.Get(stream);
               assert(rhandleOut->data);

               callbacks.associate(rhandleIn, rhandleOut);

               if(isEndOfFrame(stream) || isEOS)
                 handles.Remove(source);

               handles.Remove(stream);

               if(isEndOfFrame(stream) || isEOS)
               {
                 if(fds.input)
                   UnuseDMA(rhandleIn);
                 else
                   Unuse(rhandleIn);

                 rhandleIn->offset = 0;
                 rhandleIn->payload = 0;
                 callbacks.emptied(rhandleIn);
               }

               if(fds.output)
                 UnuseDMA(rhandleOut);
               else
                 Unuse(rhandleOut);

               rhandleOut->offset = 0;
               rhandleOut->payload = size;
               callbacks.filled(rhandleOut, rhandleOut->offset, rhandleOut->payload);
             };

  if(isEOS)
  {
    end(eosHandles.input, eosHandles.output, 0);
    eosHandles.input = nullptr;
    eosHandles.output = nullptr;
    return;
  }

  auto const size = ReconstructStream(*stream);

  if(shouldBeCopied.Exist(stream))
  {
    auto buffer = shouldBeCopied.Get(stream);
    memcpy(buffer, AL_Buffer_GetData(stream), size);
  }

  end(const_cast<AL_TBuffer*>(source), stream, size);
}

Resolution EncModule::GetResolution() const
{
  auto const chan = media->settings.tChParam[0];
  Resolution resolution;
  resolution.width = chan.uWidth;
  resolution.height = chan.uHeight;
  resolution.stride = media->stride;
  resolution.sliceHeight = media->sliceHeight;

  return resolution;
}

Clock EncModule::GetClock() const
{
  Clock clock;
  media->Get(SETTINGS_INDEX_CLOCK, &clock);
  return clock;
}

Mimes EncModule::GetMimes() const
{
  Mimes mimes;
  media->Get(SETTINGS_INDEX_MIMES, &mimes);
  return mimes;
}

Format EncModule::GetFormat() const
{
  Format format;
  auto const chan = media->settings.tChParam[0];
  format.color = ConvertSoftToModuleColor(AL_GET_CHROMA_MODE(chan.ePicFormat));
  assert(chan.uEncodingBitDepth == AL_GET_BITDEPTH(chan.ePicFormat));
  format.bitdepth = AL_GET_BITDEPTH(chan.ePicFormat);
  return format;
}

Bitrate EncModule::GetBitrate() const
{
  Bitrate bitrate;
  media->Get(SETTINGS_INDEX_BITRATE, &bitrate);
  return bitrate;
}

Gop EncModule::GetGop() const
{
  Gop gop;
  media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  return gop;
}

QPs EncModule::GetQPs() const
{
  QPs qps;
  media->Get(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &qps);
  return qps;
}

ProfileLevelType EncModule::GetProfileLevel() const
{
  return media->ProfileLevel();
}

vector<Format> EncModule::GetFormatsSupported() const
{
  vector<Format> formats;
  media->Get(SETTINGS_INDEX_FORMATS_SUPPORTED, &formats);
  return formats;
}

vector<VideoModeType> EncModule::GetVideoModesSupported() const
{
  vector<VideoModeType> videoModesSupported;
  media->Get(SETTINGS_INDEX_VIDEO_MODES_SUPPORTED, &videoModesSupported);
  return videoModesSupported;
}

VideoModeType EncModule::GetVideoMode() const
{
  VideoModeType videoMode;
  media->Get(SETTINGS_INDEX_VIDEO_MODE, &videoMode);
  return videoMode;
}

bool EncModule::SetVideoMode(VideoModeType const& videoMode)
{
  auto ret = media->Set(SETTINGS_INDEX_VIDEO_MODE, &videoMode);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

vector<ProfileLevelType> EncModule::GetProfileLevelSupported() const
{
  vector<ProfileLevelType> profileslevels;
  media->Get(SETTINGS_INDEX_PROFILES_LEVELS_SUPPORTED, &profileslevels);
  return profileslevels;
}

EntropyCodingType EncModule::GetEntropyCoding() const
{
  EntropyCodingType entropyCoding;
  media->Get(SETTINGS_INDEX_ENTROPY_CODING, &entropyCoding);
  return entropyCoding;
}

bool EncModule::IsConstrainedIntraPrediction() const
{
  bool isConstrainedIntraPrediction;
  media->Get(SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION, &isConstrainedIntraPrediction);
  return isConstrainedIntraPrediction;
}

LoopFilterType EncModule::GetLoopFilter() const
{
  LoopFilterType loopFilter;
  media->Get(SETTINGS_INDEX_LOOP_FILTER, &loopFilter);
  return loopFilter;
}

AspectRatioType EncModule::GetAspectRatio() const
{
  AspectRatioType aspectRatio;
  media->Get(SETTINGS_INDEX_ASPECT_RATIO, &aspectRatio);
  return aspectRatio;
}

bool EncModule::IsEnableLowBandwidth() const
{
  bool isLowBandwidthEnabled;
  media->Get(SETTINGS_INDEX_LOW_BANDWIDTH, &isLowBandwidthEnabled);
  return isLowBandwidthEnabled;
}

bool EncModule::IsEnablePrefetchBuffer() const
{
  bool isCacheLevel2Enabled;
  media->Get(SETTINGS_INDEX_CACHE_LEVEL2, &isCacheLevel2Enabled);
  return isCacheLevel2Enabled;
}

ScalingListType EncModule::GetScalingList() const
{
  ScalingListType scalingList;
  media->Get(SETTINGS_INDEX_SCALING_LIST, &scalingList);
  return scalingList;
}

bool EncModule::IsEnableFillerData() const
{
  bool isFillerDataEnabled;
  media->Get(SETTINGS_INDEX_FILLER_DATA, &isFillerDataEnabled);
  return isFillerDataEnabled;
}

Slices EncModule::GetSlices() const
{
  Slices slices;
  media->Get(SETTINGS_INDEX_SLICE_PARAMETER, &slices);
  return slices;
}

bool EncModule::IsEnableSubframe() const
{
  auto const chan = media->settings.tChParam[0];
  return chan.bSubframeLatency;
}

FileDescriptors EncModule::GetFileDescriptors() const
{
  return fds;
}

bool EncModule::SetResolution(Resolution const& resolution)
{
  if((resolution.width % 2) != 0)
    return false;

  if((resolution.height % 2) != 0)
    return false;

  auto& chan = media->settings.tChParam[0];
  chan.uWidth = resolution.width;
  chan.uHeight = resolution.height;

  auto minStride = (int)RoundUp(AL_EncGetMinPitch(chan.uWidth, AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER), media->strideAlignment);
  media->stride = max(minStride, RoundUp(resolution.stride, media->strideAlignment));

  auto minSliceHeight = (int)RoundUp(chan.uHeight, media->sliceHeightAlignment);
  media->sliceHeight = max(minSliceHeight, RoundUp(resolution.sliceHeight, media->sliceHeightAlignment));

  return true;
}

bool EncModule::SetClock(Clock const& clock)
{
  auto ret = media->Set(SETTINGS_INDEX_CLOCK, &clock);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetFormat(Format const& format)
{
  auto& chan = media->settings.tChParam[0];
  AL_SET_BITDEPTH(chan.ePicFormat, format.bitdepth);
  chan.uEncodingBitDepth = format.bitdepth;
  assert(chan.uEncodingBitDepth == AL_GET_BITDEPTH(chan.ePicFormat));
  AL_SET_CHROMA_MODE(chan.ePicFormat, ConvertModuleToSoftChroma(format.color));

  auto minStride = RoundUp(AL_EncGetMinPitch(chan.uWidth, AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER), media->strideAlignment);
  media->stride = max(minStride, media->stride);

  return true;
}

bool EncModule::SetBitrate(Bitrate const& bitrates)
{
  auto ret = media->Set(SETTINGS_INDEX_BITRATE, &bitrates);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetGop(Gop const& gop)
{
  auto ret = media->Set(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetProfileLevel(ProfileLevelType const& profileLevel)
{
  return media->SetProfileLevel(profileLevel);
}

bool EncModule::SetEntropyCoding(EntropyCodingType const& entropyCoding)
{
  auto ret = media->Set(SETTINGS_INDEX_ENTROPY_CODING, &entropyCoding);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetConstrainedIntraPrediction(bool constrainedIntraPrediction)
{
  auto ret = media->Set(SETTINGS_INDEX_CONSTRAINED_INTRA_PREDICTION, &constrainedIntraPrediction);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetLoopFilter(LoopFilterType const& loopFilter)
{
  auto ret = media->Set(SETTINGS_INDEX_LOOP_FILTER, &loopFilter);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetQPs(QPs const& qps)
{
  auto ret = media->Set(SETTINGS_INDEX_QUANTIZATION_PARAMETER, &qps);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetAspectRatio(AspectRatioType const& aspectRatio)
{
  auto ret = media->Set(SETTINGS_INDEX_ASPECT_RATIO, &aspectRatio);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetEnableLowBandwidth(bool enableLowBandwidth)
{
  auto ret = media->Set(SETTINGS_INDEX_LOW_BANDWIDTH, &enableLowBandwidth);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetEnablePrefetchBuffer(bool enablePrefetchBuffer)
{
  auto ret = media->Set(SETTINGS_INDEX_CACHE_LEVEL2, &enablePrefetchBuffer);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetScalingList(ScalingListType const& scalingList)
{
  auto ret = media->Set(SETTINGS_INDEX_SCALING_LIST, &scalingList);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetEnableFillerData(bool enableFillerData)
{
  auto ret = media->Set(SETTINGS_INDEX_FILLER_DATA, &enableFillerData);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetSlices(Slices const& slices)
{
  auto ret = media->Set(SETTINGS_INDEX_SLICE_PARAMETER, &slices);
  return ret == MediatypeInterface::ERROR_SETTINGS_NONE;
}

bool EncModule::SetEnableSubframe(bool enableSubframe)
{
  // TODO Check slices ?
  auto& chan = media->settings.tChParam[0];
  chan.bSubframeLatency = enableSubframe;
  return true;
}

bool EncModule::SetFileDescriptors(FileDescriptors const& fds)
{
  // TODO Check fds ?
  this->fds = fds;
  return true;
}

Flags EncModule::GetFlags(AL_TBuffer* stream)
{
  auto const meta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(stream, AL_META_TYPE_STREAM));
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
  AL_TBuffer* stream = pool.Get(handle);
  assert(stream);

  return GetFlags(stream);
}

ErrorType EncModule::SetDynamic(std::string index, void const* param)
{
  if(!encoder)
    return ERROR_UNDEFINED;

  if(index == "DYNAMIC_INDEX_CLOCK")
  {
    auto const clock = static_cast<Clock const*>(param);

    if(!SetClock(*clock))
      assert(0);

    AL_Encoder_SetFrameRate(encoder, clock->framerate, clock->clockratio);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_BITRATE")
  {
    auto const bitrate = static_cast<int>((intptr_t)param) * 1000;
    AL_Encoder_SetBitRate(encoder, bitrate);
    auto& rateCtrl = media->settings.tChParam[0].tRCParam;
    rateCtrl.uTargetBitRate = bitrate;

    if(ConvertSoftToModuleRateControl(rateCtrl.eRCMode) != RateControlType::RATE_CONTROL_VARIABLE_BITRATE)
      rateCtrl.uMaxBitRate = rateCtrl.uTargetBitRate;

    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_INSERT_IDR")
  {
    AL_Encoder_RestartGop(encoder);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_GOP")
  {
    auto const gop = static_cast<Gop const*>(param);

    if(!SetGop(*gop))
      assert(0);
    AL_Encoder_SetGopNumB(encoder, gop->b);
    AL_Encoder_SetGopLength(encoder, gop->length);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD")
  {
    assert(roiCtx);
    auto const roi = static_cast<RegionQuality const*>(param);
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
    auto const chan = media->settings.tChParam[0];
    AL_TDimension tDim = { chan.uWidth, chan.uHeight };
    auto const size = AL_GetAllocSizeEP2(tDim, chan.uMaxCuSize);
    auto roiBuffer = AL_Buffer_Create_And_Allocate(allocator.get(), size, AL_Buffer_Destroy);
    AL_Buffer_Ref(roiBuffer);
    memcpy(AL_Buffer_GetData(roiBuffer), bufferToEmpty, size);
    roiBuffers.push_back(roiBuffer);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_NOTIFY_SCENE_CHANGE")
  {
    auto const lookAhead = static_cast<int>((intptr_t)param);
    AL_Encoder_NotifySceneChange(encoder, lookAhead);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_IS_LONG_TERM")
  {
    AL_Encoder_NotifyIsLongTerm(encoder);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_IS_LONG_TERM")
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
    auto framerate = static_cast<Clock*>(param);
    auto const f = GetClock();
    *framerate = f;
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_BITRATE")
  {
    auto bitrate = (int*)param;
    auto const rateCtrl = media->settings.tChParam[0].tRCParam;
    *bitrate = rateCtrl.uTargetBitRate / 1000;
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_GOP")
  {
    auto gop = static_cast<Gop*>(param);
    *gop = GetGop();
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
    auto size = (int*)param;
    auto const chan = media->settings.tChParam[0];
    AL_TDimension tDim = { chan.uWidth, chan.uHeight };
    *size = AL_GetAllocSizeEP2(tDim, chan.uMaxCuSize);
    return SUCCESS;
  }

  return ERROR_NOT_IMPLEMENTED;
}

