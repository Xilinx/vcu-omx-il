#include "module_enc.h"
#include "convert_module_soft_enc.h"
#include "convert_module_soft.h"
#include <cassert>
#include <cmath>
#include <unistd.h> // close fd
#include <algorithm>
#include <future>
#include <utility/logger.h>
#include <utility/round.h>
#include <string>

extern "C"
{
#include <lib_common/BufferPixMapMeta.h>
#include <lib_common/BufferStreamMeta.h>
#include <lib_common/BufferPictureMeta.h>
#include <lib_common/BufferLookAheadMeta.h>
#include <lib_common/StreamBuffer.h>
#include <lib_common/Profiles.h>

#include <lib_common_enc/IpEncFourCC.h>
#include <lib_common_enc/EncBuffers.h>

#include <lib_fpga/DmaAllocLinux.h>
}

using namespace std;

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

EncModule::EncModule(shared_ptr<EncMediatypeInterface> media, shared_ptr<EncDeviceInterface> device, shared_ptr<AL_TAllocator> allocator, shared_ptr<MemoryInterface> memory) :
  media{media},
  device{device},
  allocator{allocator},
  memory{memory}
{
  assert(this->media);
  assert(this->device);
  assert(this->allocator);
  encoders.clear();
  media->Reset();
  sem.reset();
  configHandle = nullptr;
  nextQPBuffer = nullptr;
  initialDimension = { -1, -1 };
  currentDimension = { -1, -1 };
  currentPictureType = AL_SLICE_MAX_ENUM;
  currentPictureIsSkipped = false;
}

EncModule::~EncModule()
{
  if(nextQPBuffer)
  {
    AL_Buffer_Unref(nextQPBuffer);
    nextQPBuffer = nullptr;
  }
}

static map<AL_ERR, string> MapToStringEncodeError =
{
  { AL_ERR_NO_MEMORY, "encoder: memory allocation failure (firmware or ctrlsw)" },
  { AL_ERR_STREAM_OVERFLOW, "encoder: stream overflow" },
  { AL_ERR_TOO_MANY_SLICES, "encoder: too many slices" },
  { AL_ERR_CHAN_CREATION_NO_CHANNEL_AVAILABLE, "encoder: no channel available on the hardware" },
  { AL_ERR_CHAN_CREATION_RESOURCE_UNAVAILABLE, "encoder: hardware doesn't have enough resources" },
  { AL_ERR_CHAN_CREATION_NOT_ENOUGH_CORES, "encoder, hardware doesn't have enough resources (fragmentation)" },
  { AL_ERR_REQUEST_MALFORMED, "encoder: request to hardware was malformed" },
  { AL_WARN_LCU_OVERFLOW, "encoder: lcu overflow" },
  { AL_WARN_NUM_SLICES_ADJUSTED, "encoder: num slices have been adjusted" }
};

static string ToStringEncodeError(int error)
{
  string str_error {};
  try
  {
    str_error = MapToStringEncodeError.at(error);
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

void EncModule::InitEncoders(int numPass)
{
  encoders.clear();

  for(int pass = 0; pass < numPass; pass++)
  {
    GenericEncoder encoderPass {
      pass
    };

    if(pass < (numPass - 1))
    {
      encoderPass.callbackParam = { this, pass };

      auto requiredBuffers = AL_IS_AVC(media->settings.tChParam[0].eProfile) ? 2 : 1;

      for(int i = 0; i < requiredBuffers; i++)
      {
        BufferSizes bufferSizes {};
        media->Get(SETTINGS_INDEX_BUFFER_SIZES, &bufferSizes);
        encoderPass.streamBuffers.push_back(AL_Buffer_Create_And_Allocate(allocator.get(), bufferSizes.output, AL_Buffer_Destroy));
        AL_Buffer_Ref(encoderPass.streamBuffers.back());
      }

      auto p = bind(&EncModule::_ProcessEmptyFifo, this, placeholders::_1);
      encoderPass.threadFifo.reset(new ProcessorFifo<EmptyFifoParam> { p, nullptr, "Engine - Enc" });
    }

    encoders.push_back(encoderPass);
  }
}

static TwoPassMngr* createTwoPassManager(shared_ptr<MediatypeInterface> media)
{
  TwoPass tp;
  media->Get(SETTINGS_INDEX_TWOPASS, &tp);
  Gop gop;
  media->Get(SETTINGS_INDEX_GROUP_OF_PICTURES, &gop);
  Clock ck;
  media->Get(SETTINGS_INDEX_CLOCK, &ck);
  Bitrate br;
  media->Get(SETTINGS_INDEX_BITRATE, &br);
  return new TwoPassMngr(tp.sLogFile, tp.nPass, false, gop.length, br.cpb, br.ird, ck.framerate);
}

static bool CreateAndAttachStreamMeta(AL_TBuffer& buf)
{
  auto meta = (AL_TMetaData*)(AL_StreamMetaData_Create(AL_MAX_SECTION));

  if(!meta)
    return false;

  if(!AL_Buffer_AddMetaData(&buf, meta))
  {
    AL_MetaData_Destroy(meta);
    return false;
  }
  return true;
}

ModuleInterface::ErrorType EncModule::CreateEncoder()
{
  if(encoders.size())
  {
    LOG_ERROR("Encoder is ALREADY created");
    return UNDEFINED;
  }

  twoPassMngr.reset(createTwoPassManager(media));

  Resolution resolution;
  media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
  initialDimension = { resolution.dimension.horizontal, resolution.dimension.vertical };
  currentDimension = { resolution.dimension.horizontal, resolution.dimension.vertical };
  roiCtx = AL_RoiMngr_Create(resolution.dimension.horizontal, resolution.dimension.vertical, media->settings.tChParam[0].eProfile, AL_ROI_QUALITY_MEDIUM, AL_ROI_INCOMING_ORDER);

  if(!roiCtx)
  {
    LOG_ERROR("Failed to create ROI manager");
    return BAD_PARAMETER;
  }

  auto scheduler = device->Init();
  auto numPass = 1;

  auto settings = media->settings;
  numPass = AL_TwoPassMngr_HasLookAhead(settings) ? 2 : 1;

  InitEncoders(numPass);

  for(auto pass = 0; pass < numPass; pass++)
  {
    auto settingsPass = media->settings;
    GenericEncoder& encoderPass = encoders[pass];
    AL_CB_EndEncoding callback = { EncModule::RedirectionEndEncoding, this };

    if(twoPassMngr && twoPassMngr->iPass == 1)
      AL_TwoPassMngr_SetPass1Settings(settingsPass);

    if((pass < (numPass - 1)) && (AL_TwoPassMngr_HasLookAhead(settings)))
    {
      AL_TwoPassMngr_SetPass1Settings(settingsPass);
      callback = { EncModule::RedirectionEndEncodingLookAhead, &(encoderPass.callbackParam) };
      LookAhead la;
      media->Get(SETTINGS_INDEX_LOOKAHEAD, &la);
      encoderPass.lookAheadMngr.reset(new LookAheadMngr(la.lookAhead, la.isFirstPassSceneChangeDetectionEnabled));
    }

    auto errorCode = AL_Encoder_Create(&encoderPass.enc, scheduler, allocator.get(), &settingsPass, callback);

    if(AL_IS_ERROR_CODE(errorCode))
    {
      LOG_ERROR(string { "Failed to create Encoder: " } +ToStringEncodeError(errorCode));
      return ToModuleError(errorCode);
    }

    if(AL_IS_WARNING_CODE(errorCode))
      LOG_WARNING(string { "Warining: " } +ToStringEncodeError(errorCode));

    for(int i = 0; i < (int)encoderPass.streamBuffers.size(); i++)
    {
      if(pass < numPass - 1 && encoderPass.streamBuffers[i])
      {
        CreateAndAttachStreamMeta(*encoderPass.streamBuffers[i]);
        AL_Encoder_PutStreamBuffer(encoderPass.enc, encoderPass.streamBuffers[i]);
      }
    }
  }

  encoders.front().nextQPBuffer = nextQPBuffer;

  return SUCCESS;
}

static bool isFd(BufferHandleType type)
{
  return type == BufferHandleType::BUFFER_HANDLE_FD;
}

static bool isCharPtr(BufferHandleType type)
{
  return type == BufferHandleType::BUFFER_HANDLE_CHAR_PTR;
}

bool EncModule::DestroyEncoder()
{
  if(!encoders.size())
  {
    LOG_ERROR("Encoder isn't created");
    return false;
  }

  if(configHandle)
  {
    BufferHandles bufferHandles;
    media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &bufferHandles);
    ReleaseBuf(configHandle, isFd(bufferHandles.output), false);
    configHandle = nullptr;
    sem.reset();
  }

  initialDimension = { -1, -1 };
  currentDimension = { -1, -1 };
  currentPictureType = AL_SLICE_MAX_ENUM;
  currentPictureIsSkipped = false;

  for(int pass = 0; pass < (int)encoders.size(); pass++)
  {
    GenericEncoder encoder = encoders[pass];

    AL_Encoder_Destroy(encoder.enc);

    if(encoder.nextQPBuffer != nullptr)
    {
      AL_Buffer_Unref(encoder.nextQPBuffer);
      encoder.nextQPBuffer = nullptr;
    }

    while(encoder.lookAheadMngr && !encoder.lookAheadMngr->m_fifo.empty())
    {
      auto src = encoder.lookAheadMngr->m_fifo.front();
      encoder.lookAheadMngr->m_fifo.pop_front();
      AL_Buffer_Unref(src);
    }

    for(int i = 0; i < (int)encoder.streamBuffers.size(); i++)
    {
      if(encoder.streamBuffers[i])
        AL_Buffer_Unref(encoder.streamBuffers[i]);
    }
  }

  encoders.clear();

  device->Deinit();

  if(!roiCtx)
  {
    LOG_ERROR("ROI manager isn't created");
    return false;
  }
  AL_RoiMngr_Destroy(roiCtx);

  return true;
}

ModuleInterface::ErrorType EncModule::Start(bool)
{
  if(encoders.size())
  {
    LOG_ERROR("You can't call Start twice");
    return UNDEFINED;
  }

  return CreateEncoder();
}

bool EncModule::Stop()
{
  if(nextQPBuffer)
  {
    AL_Buffer_Unref(nextQPBuffer);
    nextQPBuffer = nullptr;
  }

  if(!encoders.size())
    return false;

  DestroyEncoder();
  return true;
}

static void StubCallbackEvent(Callbacks::Event, void*)
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
    LOG_ERROR("No more memory");
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
    LOG_ERROR("No more memory");
    return -1;
  }

  auto fd = AL_LinuxDmaAllocator_GetFd((AL_TLinuxDmaAllocator*)allocator.get(), handle);
  allocatedDMA.Add(fd, handle);
  return fd;
}

static void FreeWithoutDestroyingMemory(AL_TBuffer* buffer)
{
  buffer->iChunkCnt = 0;
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
    encoderBuffer = AL_Buffer_Create(allocator.get(), nullptr, size, FreeWithoutDestroyingMemory);

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
    LOG_ERROR(string { "Failed to import fd: " } +to_string(fd));
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

static void UpdatePixMapMetaResolution(shared_ptr<MediatypeInterface> media, AL_TPixMapMetaData* pMeta)
{
  Resolution resolution;
  auto ret = media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
  assert(ret == MediatypeInterface::SUCCESS);
  pMeta->tDim.iWidth = resolution.dimension.horizontal;
  pMeta->tDim.iHeight = resolution.dimension.vertical;
}

static AL_TMetaData* CreatePixMapMeta(shared_ptr<MediatypeInterface> media)
{
  Format format {};
  media->Get(SETTINGS_INDEX_FORMAT, &format);
  auto picFormat = AL_EncGetSrcPicFormat(ConvertModuleToSoftChroma(format.color), static_cast<uint8_t>(format.bitdepth), AL_FB_RASTER, false);
  auto fourCC = AL_EncGetSrcFourCC(picFormat);
  Resolution resolution;
  auto ret = media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
  assert(ret == MediatypeInterface::SUCCESS);
  auto stride = resolution.stride.horizontal;
  auto sliceHeight = resolution.stride.vertical;
  AL_TPlane planeY = { 0, 0, stride };
  AL_TPlane planeUV = { 0, stride * sliceHeight, stride };
  return (AL_TMetaData*)(AL_PixMapMetaData_Create({ resolution.dimension.horizontal, resolution.dimension.vertical }, planeY, planeUV, fourCC));
}

static bool CreateAndAttachPixMapMeta(AL_TBuffer& buf, shared_ptr<MediatypeInterface> media)
{
  auto meta = CreatePixMapMeta(media);

  if(!meta)
    return false;

  if(!AL_Buffer_AddMetaData(&buf, meta))
  {
    AL_MetaData_Destroy(meta);
    return false;
  }
  return true;
}

bool EncModule::Empty(BufferHandleInterface* handle)
{
  if(!encoders.size())
    return false;

  GenericEncoder& currentEnc = encoders.front();
  AL_HEncoder encoder = currentEnc.enc;

  auto eos = (handle == nullptr || handle->payload == 0);

  if(eos)
  {
    auto bRet = AL_Encoder_Process(encoder, nullptr, nullptr);
    return bRet;
  }

  uint8_t* buffer = (uint8_t*)handle->data;

  BufferHandles bufferHandles {};
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &bufferHandles);

  if(isFd(bufferHandles.input))
    UseDMA(handle, static_cast<int>((intptr_t)buffer), handle->payload);

  if(isCharPtr(bufferHandles.input))
    Use(handle, buffer, handle->payload);

  auto input = pool.Get(handle);

  if(!input)
    return false;

  auto meta = AL_Buffer_GetMetaData(input, AL_META_TYPE_PIXMAP);

  if(!meta)
  {
    if(!CreateAndAttachPixMapMeta(*input, media))
      return false;
  }
  else
    UpdatePixMapMetaResolution(media, (AL_TPixMapMetaData*)meta);

  if(encoders.size() > 1)
    AL_TwoPassMngr_CreateAndAttachTwoPassMetaData(input);

  if(twoPassMngr->iPass)
  {
    auto pPictureMetaTP = AL_TwoPassMngr_CreateAndAttachTwoPassMetaData(input);

    if(twoPassMngr->iPass == 2)
      twoPassMngr->GetFrame(pPictureMetaTP);
  }

  handles.Add(input, handle);

  if(shouldBeCopied.Exist(input))
  {
    auto buffer = shouldBeCopied.Get(input);
    copy(buffer, buffer + input->zSizes[0], AL_Buffer_GetData(input));
  }

  if(currentEnc.nextQPBuffer == nullptr)
    return AL_Encoder_Process(encoder, input, nullptr);

  auto success = AL_Encoder_Process(encoder, input, currentEnc.nextQPBuffer);

  if(currentEnc.index != encoders.back().index)
    encoders[currentEnc.index + 1].nextQPBuffer = currentEnc.nextQPBuffer;
  else
  {
    AL_Buffer_Unref(currentEnc.nextQPBuffer);
    currentEnc.nextQPBuffer = nullptr;
  }

  return success;
}

static bool CreateAndAttachPictureMeta(AL_TBuffer& buf)
{
  auto meta = (AL_TMetaData*)(AL_PictureMetaData_Create());

  if(!meta)
    return false;

  if(!AL_Buffer_AddMetaData(&buf, meta))
  {
    AL_MetaData_Destroy(meta);
    return false;
  }
  return true;
}

bool EncModule::Fill(BufferHandleInterface* handle)
{
  if(!encoders.size() || !handle)
    return false;

  AL_HEncoder encoder = encoders.back().enc;

  auto buffer = (uint8_t*)handle->data;

  BufferHandles bufferHandles {};
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &bufferHandles);

  if(isFd(bufferHandles.output))
    UseDMA(handle, static_cast<int>((intptr_t)buffer), handle->size);

  if(isCharPtr(bufferHandles.output))
    Use(handle, buffer, handle->size);

  auto output = pool.Get(handle);

  if(!output)
    return false;

  if(!AL_Buffer_GetMetaData(output, AL_META_TYPE_STREAM))
  {
    if(!CreateAndAttachStreamMeta(*output))
      return false;
  }

  if(!AL_Buffer_GetMetaData(output, AL_META_TYPE_PICTURE))
  {
    if(!CreateAndAttachPictureMeta(*output))
      return false;
  }

  handles.Add(output, handle);

  bool isSeparateConfigurationFromDataEnabled = false;
  media->Get(SETTINGS_INDEX_SEPARATE_CONFIGURATION_FROM_DATA, &isSeparateConfigurationFromDataEnabled);

  if(!isSeparateConfigurationFromDataEnabled)
    return AL_Encoder_PutStreamBuffer(encoder, output);

  unique_lock<std::mutex> lock(mutex);

  if(!configHandle)
  {
    configHandle = output;
    sem.notify();
    return true;
  }
  lock.unlock();

  return AL_Encoder_PutStreamBuffer(encoder, output);
}

static int WriteFillerDataSection(shared_ptr<MemoryInterface> memory, AL_TBuffer* source, AL_TBuffer* destination, int offset, int numSection)
{
  auto meta = reinterpret_cast<AL_TStreamMetaData*>(AL_Buffer_GetMetaData(source, AL_META_TYPE_STREAM));
  auto& section = meta->pSections[numSection];

  auto src = AL_Buffer_GetData(source);
  auto dst = AL_Buffer_GetData(destination);
  auto srcOffset = section.uOffset;
  auto dstOffset = offset;
  auto length = section.uLength;

  while(--length && (src[srcOffset] != 0xFF))
  {
    dst[dstOffset++] = src[srcOffset++];
  }

  if(length > 0)
    memory->set(destination, dstOffset, 0xFF, length);

  assert(src[srcOffset + length] == 0x80);
  dst[dstOffset + length] = src[srcOffset + length];

  return section.uLength;
}

static int WriteOneSection(shared_ptr<MemoryInterface> memory, AL_TBuffer* source, AL_TBuffer* destination, int offset, int numSection)
{
  auto meta = reinterpret_cast<AL_TStreamMetaData*>(AL_Buffer_GetMetaData(source, AL_META_TYPE_STREAM));
  auto& section = meta->pSections[numSection];

  if(!section.uLength)
    return 0;

  auto size = source->zSizes[0] - section.uOffset;

  if(size < section.uLength)
  {
    memory->move(destination, offset, source, section.uOffset, size);
    memory->move(destination, offset, source, 0, section.uLength - size);
  }
  else
    memory->move(destination, offset, source, section.uOffset, section.uLength);

  return section.uLength;
}

static int ConstructConfigStream(shared_ptr<MemoryInterface> memory, AL_TBuffer* config, AL_TBuffer* stream, int& firstSection)
{
  auto size = 0;
  auto meta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(stream, AL_META_TYPE_STREAM));
  assert(meta);

  assert(firstSection <= meta->uNumSection);

  while(((meta->pSections[firstSection].eFlags & AL_SECTION_CONFIG_FLAG) != 0) && (firstSection < meta->uNumSection))
  {
    if(meta->pSections[firstSection].eFlags & AL_SECTION_APP_FILLER_FLAG)
      size += WriteFillerDataSection(memory, stream, config, size, firstSection);
    else
      size += WriteOneSection(memory, stream, config, size, firstSection);
    firstSection++;
  }

  return size;
}

static int ReconstructStream(shared_ptr<MemoryInterface> memory, AL_TBuffer* stream, int firstSection)
{
  auto size = 0;
  auto meta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(stream, AL_META_TYPE_STREAM));
  assert(meta);

  assert(firstSection <= meta->uNumSection);

  for(int i = firstSection; i < meta->uNumSection; i++)
  {
    if(meta->pSections[i].eFlags & AL_SECTION_APP_FILLER_FLAG)
      size += WriteFillerDataSection(memory, stream, stream, size, i);
    else
      size += WriteOneSection(memory, stream, stream, size, i);
  }

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

static Flags GetCurrentFlags(AL_TBuffer* stream, int firstSection)
{
  auto meta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(stream, AL_META_TYPE_STREAM));
  assert(meta);

  Flags flags {};

  for(int i = firstSection; i < meta->uNumSection; i++)
  {
    if(meta->pSections[i].eFlags & AL_SECTION_SYNC_FLAG)
      flags.isSync = true;

    if(!(meta->pSections[i].eFlags & AL_SECTION_CONFIG_FLAG))
      flags.isEndOfSlice = true;

    if(meta->pSections[i].eFlags & AL_SECTION_END_FRAME_FLAG)
      flags.isEndOfFrame = true;

    if(meta->pSections[i].eFlags & AL_SECTION_CONFIG_FLAG)
      flags.isConfig = true;
  }

  return flags;
}

bool EncModule::isEndOfFrame(AL_TBuffer* stream)
{
  auto flags = GetCurrentFlags(stream, 0);
  return flags.isEndOfFrame;
}

void EncModule::EndEncoding(AL_TBuffer* stream, AL_TBuffer const* source)
{
  AL_HEncoder encoder = encoders.back().enc;

  auto errorCode = AL_Encoder_GetLastError(encoder);

  if(AL_IS_ERROR_CODE(errorCode))
  {
    LOG_ERROR(ToStringEncodeError(errorCode));
    callbacks.event(Callbacks::Event::ERROR, (void*)ToModuleError(errorCode));
  }

  if(AL_IS_WARNING_CODE(errorCode))
    LOG_WARNING(ToStringEncodeError(errorCode));

  BufferHandles bufferHandles;
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &bufferHandles);

  auto isSrcRelease = ((stream == nullptr) && source);

  if(isSrcRelease)
  {
    ReleaseBuf(source, isFd(bufferHandles.input), true);
    return;
  }

  auto isStreamRelease = (stream && (source == nullptr));

  if(isStreamRelease)
  {
    ReleaseBuf(stream, isFd(bufferHandles.output), false);
    return;
  }

  auto isEOS = ((stream == nullptr) && (source == nullptr));

  if(twoPassMngr->iPass == 1)
  {
    if(isEOS)
      twoPassMngr->Flush();
    else
    {
      auto pPictureMetaTP = (AL_TLookAheadMetaData*)AL_Buffer_GetMetaData(source, AL_META_TYPE_LOOKAHEAD);
      twoPassMngr->AddFrame(pPictureMetaTP);
    }
  }

  if(isEOS)
  {
    callbacks.filled(nullptr);
    return;
  }

  AL_TPixMapMetaData* pixMapMeta = (AL_TPixMapMetaData*)AL_Buffer_GetMetaData(source, AL_META_TYPE_PIXMAP);

  int frameWidth = pixMapMeta->tDim.iWidth;
  int frameHeigth = pixMapMeta->tDim.iHeight;

  if((frameWidth != currentDimension.horizontal) || (frameHeigth != currentDimension.vertical))
  {
    Dimension<int> dimension {};
    dimension.horizontal = frameWidth;
    dimension.vertical = frameHeigth;

    callbacks.event(Callbacks::Event::RESOLUTION_CHANGED, &dimension);
  }

  currentDimension.horizontal = frameWidth;
  currentDimension.vertical = frameHeigth;

  auto rhandleIn = handles.Get(source);
  assert(rhandleIn->data);

  auto rhandleOut = handles.Get(stream);
  assert(rhandleOut->data);

  currentOutputedStreamForSei = stream;
  AL_TStreamMetaData* streamMeta = (AL_TStreamMetaData*)AL_Buffer_GetMetaData(stream, AL_META_TYPE_STREAM);
  assert(streamMeta);
  currentTemporalId = streamMeta->uTemporalID;
  int firstSection = 0;
  currentFlags = GetCurrentFlags(stream, firstSection);

  bool isSeparateConfigurationFromDataEnabled = false;
  media->Get(SETTINGS_INDEX_SEPARATE_CONFIGURATION_FROM_DATA, &isSeparateConfigurationFromDataEnabled);

  if(currentFlags.isConfig && isSeparateConfigurationFromDataEnabled)
  {
    currentFlags.isEndOfFrame = false;
    currentFlags.isEndOfSlice = false;

    unique_lock<std::mutex> lock(mutex);
    sem.wait();
    AL_TBuffer* config = configHandle;
    configHandle = nullptr;
    auto configHandleOut = handles.Pop(config);
    lock.unlock();
    assert(configHandleOut->data);
    callbacks.associate(rhandleIn, configHandleOut);
    auto async_config_reconstruction = [&]() -> int
                                       {
                                         int size = ConstructConfigStream(memory, config, stream, firstSection);

                                         if(!shouldBeCopied.Exist(config))
                                           return size;
                                         auto buffer = shouldBeCopied.Get(config);
                                         copy(AL_Buffer_GetData(config), AL_Buffer_GetData(config) + size, buffer);

                                         return size;
                                       };

    auto async_config_reconstruction_handle = async(async_config_reconstruction);

    auto size = async_config_reconstruction_handle.get();

    if(isFd(bufferHandles.output))
      UnuseDMA(configHandleOut);

    if(isCharPtr(bufferHandles.output))
      Unuse(configHandleOut);

    configHandleOut->offset = 0;
    configHandleOut->payload = size;
    callbacks.filled(configHandleOut);
  }

  currentFlags = GetCurrentFlags(stream, firstSection);

  callbacks.associate(rhandleIn, rhandleOut);

  auto async_stream_reconstruction = [&]() -> int
                                     {
                                       int size = ReconstructStream(memory, stream, firstSection);

                                       if(!shouldBeCopied.Exist(stream))
                                         return size;
                                       auto buffer = shouldBeCopied.Get(stream);
                                       copy(AL_Buffer_GetData(stream), AL_Buffer_GetData(stream) + size, buffer);
                                       return size;
                                     };

  auto async_stream_reconstruction_handle = async(async_stream_reconstruction);

  if(isEndOfFrame(stream))
    handles.Remove(source);

  handles.Remove(stream);

  if(isEndOfFrame(stream))
  {
    if(isFd(bufferHandles.input))
      UnuseDMA(rhandleIn);

    if(isCharPtr(bufferHandles.input))
      Unuse(rhandleIn);

    rhandleIn->offset = 0;
    rhandleIn->payload = 0;
    callbacks.emptied(rhandleIn);
  }

  auto size = async_stream_reconstruction_handle.get();

  AL_TPictureMetaData* pictureMeta = (AL_TPictureMetaData*)AL_Buffer_GetMetaData(stream, AL_META_TYPE_PICTURE);
  currentPictureType = pictureMeta->eType;
  currentPictureIsSkipped = pictureMeta->bSkipped;

  if(isFd(bufferHandles.output))
    UnuseDMA(rhandleOut);

  if(isCharPtr(bufferHandles.output))
    Unuse(rhandleOut);

  rhandleOut->offset = 0;
  rhandleOut->payload = size;
  callbacks.filled(rhandleOut);
}

void EncModule::EndEncodingLookAhead(AL_TBuffer* stream, AL_TBuffer const* source, int index)
{
  assert(index < (int)encoders.size() - 1);

  auto isStreamRelease = (stream && source == nullptr);
  auto isSrcRelease = (stream == nullptr && source);
  auto isEOS = (stream == nullptr && source == nullptr);

  GenericEncoder& encoder = encoders[index];

  auto errorCode = AL_Encoder_GetLastError(encoder.enc);

  if(AL_IS_ERROR_CODE(errorCode))
  {
    LOG_ERROR(ToStringEncodeError(errorCode));
    callbacks.event(Callbacks::Event::ERROR, (void*)ToModuleError(errorCode));
  }

  if(AL_IS_WARNING_CODE(errorCode))
    LOG_WARNING(ToStringEncodeError(errorCode));

  if(isEOS)
  {
    AddFifo(encoder, (AL_TBuffer*)source);
    return;
  }

  BufferHandles bufferHandles;
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &bufferHandles);

  if(isSrcRelease)
  {
    ReleaseBuf(source, isFd(bufferHandles.input), true);
    return;
  }

  if(isStreamRelease)
    return;

  if(isEndOfFrame(stream))
  {
    AddFifo(encoder, (AL_TBuffer*)source);
    AL_Encoder_PutStreamBuffer(encoder.enc, stream);
  }
}

void EncModule::AddFifo(GenericEncoder& encoder, AL_TBuffer* src)
{
  bool isEOS = (src == nullptr);

  if(!isEOS)
  {
    AL_Buffer_Ref(src);
    encoder.lookAheadMngr->m_fifo.push_back(src);
  }
  EmptyFifoParam param;
  param.encoder = &encoder;
  param.isEOS = isEOS;
  encoder.threadFifo->queue(param);
}

void EncModule::EmptyFifo(GenericEncoder& encoder, bool isEOS)
{
  assert(encoder.index < (int)encoders.size() - 1);

  GenericEncoder nextEnc = encoders[encoder.index + 1];

  if(isEOS && encoder.lookAheadMngr->m_fifo.size() == 0)
  {
    AL_Encoder_Process(nextEnc.enc, nullptr, nullptr);
    return;
  }

  if(!(isEOS || (int)encoder.lookAheadMngr->m_fifo.size() == encoder.lookAheadMngr->uLookAheadSize))
    return;

  encoder.lookAheadMngr->ProcessLookAheadParams();
  auto src = encoder.lookAheadMngr->m_fifo.front();
  encoder.lookAheadMngr->m_fifo.pop_front();

  AL_TBuffer* qpBuffer = nextEnc.nextQPBuffer;

  if(nextEnc.nextQPBuffer != nullptr)
    nextEnc.nextQPBuffer = nullptr;

  AL_Encoder_Process(nextEnc.enc, src, qpBuffer);

  if(qpBuffer)
  {
    if(nextEnc.index != encoders.back().index)
      encoders[nextEnc.index + 1].nextQPBuffer = qpBuffer;
    else
      AL_Buffer_Unref(qpBuffer);
  }

  AL_Buffer_Unref(src);

  if(isEOS)
    EmptyFifo(encoder, isEOS);
}

ModuleInterface::ErrorType EncModule::SetDynamic(std::string index, void const* param)
{
  auto createQPTable = [&](unsigned char const* bufferToCopy) -> AL_TBuffer*
                       {
                         Resolution resolution;
                         auto ret = media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
                         assert(ret == MediatypeInterface::SUCCESS);
                         AL_TDimension tDim {
                           resolution.dimension.horizontal, resolution.dimension.vertical
                         };
                         auto size = AL_GetAllocSizeEP2(tDim, static_cast<AL_ECodec>(AL_GET_CODEC(media->settings.tChParam[0].eProfile)));
                         auto qpTable = AL_Buffer_Create_And_Allocate(allocator.get(), size, AL_Buffer_Destroy);
                         copy(bufferToCopy, bufferToCopy + size, AL_Buffer_GetData(qpTable));
                         return qpTable;
                       };

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_EMPTY")
  {
    auto roiBuffer = createQPTable(static_cast<unsigned char const*>(param));
    AL_Buffer_Ref(roiBuffer);

    if(encoders.empty())
    {
      if(nextQPBuffer)
        AL_Buffer_Unref(nextQPBuffer);
      nextQPBuffer = roiBuffer;
      return SUCCESS;
    }

    if(encoders.front().nextQPBuffer)
      AL_Buffer_Unref(encoders.front().nextQPBuffer);
    encoders.front().nextQPBuffer = roiBuffer;
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_INSERT_QUANTIZATION_PARAMETER_BUFFER")
  {
    auto qpTable = createQPTable(static_cast<unsigned char const*>(param));
    AL_Buffer_Ref(qpTable);

    if(encoders.empty())
    {
      if(nextQPBuffer)
        AL_Buffer_Unref(nextQPBuffer);
      nextQPBuffer = qpTable;
      return SUCCESS;
    }

    if(encoders.front().nextQPBuffer)
      AL_Buffer_Unref(encoders.front().nextQPBuffer);
    encoders.front().nextQPBuffer = qpTable;
    return SUCCESS;
  }

  if(!encoders.size())
    return UNDEFINED;

  AL_HEncoder encoder = encoders.back().enc;

  if(index == "DYNAMIC_INDEX_CLOCK")
  {
    auto clock = static_cast<Clock const*>(param);

    if(!AL_Encoder_SetFrameRate(encoder, clock->framerate, clock->clockratio))
    {
      assert(0);
      return BAD_PARAMETER;
    }
    auto ret = media->Set(SETTINGS_INDEX_CLOCK, clock);
    assert(ret == MediatypeInterface::SUCCESS);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_BITRATE")
  {
    auto bitrate = static_cast<int>((intptr_t)param);
    Bitrate mediaBitrate;
    media->Get(SETTINGS_INDEX_BITRATE, &mediaBitrate);
    mediaBitrate.target = bitrate;

    if(mediaBitrate.rateControl.mode != RateControlType::RATE_CONTROL_VARIABLE_BITRATE)
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
    assert(ret == MediatypeInterface::SUCCESS);
    AL_Encoder_SetGopNumB(encoder, gop->b);
    AL_Encoder_SetGopLength(encoder, gop->length);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD")
  {
    assert(roiCtx);
    auto roi = static_cast<RegionQuality const*>(param);
    auto ret = AL_RoiMngr_AddROI(roiCtx, roi->region.point.x, roi->region.point.y, roi->region.dimension.horizontal, roi->region.dimension.vertical, static_cast<AL_ERoiQuality>(roi->quality.byValue));
    assert(ret);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_CLEAR")
  {
    assert(roiCtx);
    AL_RoiMngr_Clear(roiCtx);
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

  if(index == "DYNAMIC_INDEX_INSERT_PREFIX_SEI")
  {
    auto sei = static_cast<Sei const*>(param);

    if(AL_Encoder_AddSei(encoder, currentOutputedStreamForSei, true, sei->type, sei->data, sei->payload, currentTemporalId) < 0)
    {
      assert(0);
      return BAD_PARAMETER;
    }
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_INSERT_SUFFIX_SEI")
  {
    auto sei = static_cast<Sei const*>(param);

    if(AL_Encoder_AddSei(encoder, currentOutputedStreamForSei, false, sei->type, sei->data, sei->payload, currentTemporalId) < 0)
    {
      assert(0);
      return BAD_PARAMETER;
    }
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_HIGH_DYNAMIC_RANGE_SEIS")
  {
    auto hdrSEIS = ConvertModuleToSoftHDRSEIs(*static_cast<HighDynamicRangeSeis const*>(param));

    if(!AL_Encoder_SetHDRSEIs(encoder, &hdrSEIS))
    {
      assert(0);
      return BAD_PARAMETER;
    }
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_RESOLUTION")
  {
    auto resolution = static_cast<Resolution const*>(param);
    AL_TDimension dimension {
      resolution->dimension.horizontal, resolution->dimension.vertical
    };

    if(!AL_Encoder_SetInputResolution(encoder, dimension))
    {
      assert(0);
      return BAD_PARAMETER;
    }
    auto ret = media->Set(SETTINGS_INDEX_RESOLUTION, resolution);
    assert(ret == MediatypeInterface::SUCCESS);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_LOOP_FILTER_BETA")
  {
    auto beta = static_cast<int>((intptr_t)param);

    if(AL_Encoder_SetLoopFilterBetaOffset(encoder, beta))
      return BAD_PARAMETER;
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_LOOP_FILTER_TC")
  {
    auto tc = static_cast<int>((intptr_t)param);

    if(AL_Encoder_SetLoopFilterTcOffset(encoder, tc))
      return BAD_PARAMETER;
    return SUCCESS;
  }
  return BAD_INDEX;
}

ModuleInterface::ErrorType EncModule::GetDynamic(std::string index, void* param)
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
    auto bufferToFill = static_cast<unsigned char*>(param);
    AL_RoiMngr_FillBuff(roiCtx, 1, 1, bufferToFill + EP2_BUF_QP_BY_MB.Offset);
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_SIZE")
  {
    Resolution resolution {};
    auto ret = media->Get(SETTINGS_INDEX_RESOLUTION, &resolution);
    assert(ret == MediatypeInterface::SUCCESS);
    AL_TDimension tDim {
      resolution.dimension.horizontal, resolution.dimension.vertical
    };
    *static_cast<int*>(param) = AL_GetAllocSizeEP2(tDim, static_cast<AL_ECodec>(AL_GET_CODEC(media->settings.tChParam[0].eProfile)));
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_STREAM_FLAGS")
  {
    *static_cast<Flags*>(param) = this->currentFlags;
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_MAX_RESOLUTION_CHANGE_SUPPORTED")
  {
    auto dimension = static_cast<Dimension<int>*>(param);
    dimension->horizontal = initialDimension.horizontal;
    dimension->vertical = initialDimension.vertical;
    return SUCCESS;
  }

  if(index == "DYNAMIC_INDEX_SKIP_PICTURE")
  {
    *static_cast<bool*>(param) = currentPictureIsSkipped;
    return SUCCESS;
  }

  return BAD_INDEX;
}

void EncModule::_ProcessEmptyFifo(EmptyFifoParam param)
{
  assert(param.encoder);
  GenericEncoder& encoder = *(param.encoder);
  EmptyFifo(encoder, param.isEOS);
}
