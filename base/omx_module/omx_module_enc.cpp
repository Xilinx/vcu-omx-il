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
#include <assert.h>
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
#include "base/omx_utils/roundup.h"

using namespace std;

static inline int GetBitdepthFromFormat(AL_EPicFormat const& format)
{
  return AL_GET_BITDEPTH(format);
}

static bool CheckValidity(AL_TEncSettings const& settings)
{
  auto err = AL_Settings_CheckValidity(const_cast<AL_TEncSettings*>(&settings), stderr);

  return err == AL_SUCCESS;
}

static void LookCoherency(AL_TEncSettings& settings)
{
  auto const chan = settings.tChParam;
  auto const fourCC = AL_EncGetSrcFourCC({ AL_GET_CHROMA_MODE(chan.ePicFormat), AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER });
  AL_Settings_CheckCoherency(&settings, fourCC, stdout);
}

EncModule::EncModule(unique_ptr<EncMediatypeInterface>&& media, unique_ptr<EncDevice>&& device, deleted_unique_ptr<AL_TAllocator>&& allocator) :
  media(move(media)),
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

map<int, string> MapToStringEncodeError =
{
  { AL_ERR_INIT_FAILED, "encoder: initialization failure" },
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

bool EncModule::CreateEncoder()
{
  if(encoder)
  {
    fprintf(stderr, "Encoder is ALREADY created\n");
    assert(0);
  }

  auto const chan = media->settings.tChParam;
  roiCtx = AL_RoiMngr_Create(chan.uWidth, chan.uHeight, chan.eProfile, AL_ROI_QUALITY_MEDIUM, AL_ROI_INCOMING_ORDER);

  if(!roiCtx)
  {
    fprintf(stderr, "Failed to create ROI manager:\n");
    return false;
  }

  auto& settings = media->settings;
  scheduler = device->Init(settings, *allocator.get());
  auto errorCode = AL_Encoder_Create(&encoder, scheduler, allocator.get(), &media->settings, { EncModule::RedirectionEndEncoding, this });

  if(errorCode != AL_SUCCESS)
  {
    fprintf(stderr, "Failed to create Encoder:\n");
    fprintf(stderr, "/!\\ %s (%d)\n", ToStringEncodeError(errorCode).c_str(), errorCode);
    return false;
  }

  eosHandles.output = nullptr;
  eosHandles.input = nullptr;

  return true;
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
    assert(0);
    return false;
  }
  isCreated = true;

  return true;
}

void EncModule::Destroy()
{
  if(encoder)
  {
    fprintf(stderr, "Encoder should ALREADY be destroyed\n");
    assert(0);
  }
  isCreated = false;
}

bool EncModule::Run(bool)
{
  if(encoder)
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

static int RawAllocationSize(int width, int widthAlignment, int height,  int heightAlignment, int bitdepth, AL_EChromaMode eChromaMode)
{
  auto const IP_WIDTH_ALIGNMENT = 32;
  auto const IP_HEIGHT_ALIGNMENT = 8;
  assert(widthAlignment % IP_WIDTH_ALIGNMENT == 0); // IP requirements
  assert(heightAlignment % IP_HEIGHT_ALIGNMENT == 0); // IP requirements
  auto const adjustedWidthAlignment = widthAlignment > IP_WIDTH_ALIGNMENT ? widthAlignment : IP_WIDTH_ALIGNMENT;
  int const adjustedHeightAlignment = heightAlignment > IP_HEIGHT_ALIGNMENT ? heightAlignment : IP_HEIGHT_ALIGNMENT;

  auto const bitdepthWidth = bitdepth == 8 ? width : (width + 2) / 3 * 4;
  auto const adjustedWidth = RoundUp(bitdepthWidth, adjustedWidthAlignment);
  auto const adjustedHeight = RoundUp(height, adjustedHeightAlignment);

  auto size = adjustedWidth * adjustedHeight;

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


BufferRequirements EncModule::GetBufferRequirements() const
{
  BufferRequirements b;
  auto const chan = media->settings.tChParam;
  auto const gop = chan.tGopParam;
  auto& input = b.input;
  input.min = 1 + gop.uNumB;
  input.size = RawAllocationSize(chan.uWidth, media->strideAlignment, chan.uHeight, media->sliceHeightAlignment, AL_GET_BITDEPTH(chan.ePicFormat), AL_GET_CHROMA_MODE(chan.ePicFormat));
  input.bytesAlignment = device->GetAllocationRequirements().input.bytesAlignment;
  input.contiguous = device->GetAllocationRequirements().input.contiguous;

  auto& output = b.output;
  output.min = 2 + gop.uNumB + 1; // 1 for eos
  output.size = AL_GetMaxNalSize({ chan.uWidth, chan.uHeight }, AL_GET_CHROMA_MODE(chan.ePicFormat), AL_GET_BITDEPTH(chan.ePicFormat));
  output.bytesAlignment = device->GetAllocationRequirements().output.bytesAlignment;
  output.contiguous = device->GetAllocationRequirements().output.contiguous;

  if(chan.bSubframeLatency)
  {
    auto const numSlices = chan.uNumSlices;
    auto& size = output.size;
    size /= numSlices;
    size += 4095 * 2; /* we need space for the headers on each slice */
    size = (size + 31) & ~31; /* stream size is required to be 32 bits aligned */

    output.min *= numSlices;
  }

  return b;
}

int EncModule::GetLatency() const
{
  auto const chan = media->settings.tChParam;
  auto const gopParam = chan.tGopParam;
  auto const rateCtrl = chan.tRCParam;
  auto bufsCount = gopParam.uNumB + 1;

  if(AL_IS_AVC(chan.eProfile))
    bufsCount += 1; // intermediate

  auto const realFramerate = (static_cast<double>(rateCtrl.uFrameRate * rateCtrl.uClkRatio) / 1000.0);

  auto timeInMilliseconds = (static_cast<double>(bufsCount * 1000.0) / realFramerate);

  if(chan.bSubframeLatency)
  {
    timeInMilliseconds /= chan.uNumSlices;
    timeInMilliseconds += 1.0; // overhead
  }

  return ceil(timeInMilliseconds);
}

bool EncModule::SetCallbacks(Callbacks callbacks)
{
  if(!callbacks.emptied || !callbacks.associate || !callbacks.filled || !callbacks.release)
    return false;

  this->callbacks = callbacks;
  return true;
}

bool EncModule::Flush()
{
  if(!encoder)
    return false;

  Stop();
  return Run(true);
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

  auto fd = AL_LinuxDmaAllocator_ExportToFd((AL_TLinuxDmaAllocator*)allocator.get(), handle);
  allocatedDMA.Add(fd, handle);
  return fd;
}

static void MyFree(AL_TBuffer* buffer)
{
  AL_Allocator_Free(buffer->pAllocator, buffer->hBuf);
  AL_Buffer_Destroy(buffer);
}

bool EncModule::Use(void* handle, unsigned char* buffer, int size)
{
  if(!handle)
    throw invalid_argument("handle");

  if(!buffer)
    throw invalid_argument("buffer");

  AL_TBuffer* encoderBuffer = nullptr;

  if(allocated.Exist(buffer))
  {
    encoderBuffer = AL_Buffer_Create(allocator.get(), allocated.Get(buffer), size, AL_Buffer_Destroy);
  }
  else if(size)
  {
    encoderBuffer = AL_Buffer_Create_And_Allocate(allocator.get(), size, MyFree);
    shouldBeCopied.Add(encoderBuffer, buffer);
  }
  else
    encoderBuffer = AL_Buffer_Create(allocator.get(), NULL, size, AL_Buffer_Destroy);

  if(!encoderBuffer)
    return false;

  if(pool.Exist(handle))
    assert(0);

  AL_Buffer_Ref(encoderBuffer);
  pool.Add(handle, encoderBuffer);

  return true;
}

bool EncModule::UseDMA(void* handle, int fd, int size)
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

  auto encoderBuffer = AL_Buffer_Create(allocator.get(), dmaHandle, size, MyFree);

  if(!encoderBuffer)
    return false;

  if(pool.Exist(handle))
    assert(0);

  pool.Add(handle, encoderBuffer);
  AL_Buffer_Ref(encoderBuffer);

  return true;
}

void EncModule::Unuse(void* handle)
{
  if(!handle)
    throw invalid_argument("handle");

  auto encoderBuffer = pool.Pop(handle);

  if(shouldBeCopied.Exist(encoderBuffer))
    shouldBeCopied.Remove(encoderBuffer);
  AL_Buffer_Unref(encoderBuffer);
}

void EncModule::UnuseDMA(void* handle)
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

bool EncModule::Empty(uint8_t* buffer, int offset, int size, void* handle)
{
  (void)offset; (void) handle;

  if(!encoder)
    return false;

  if(fds.input)
    UseDMA(buffer, static_cast<int>((intptr_t)buffer), size);
  else
    Use(buffer, buffer, size);

  auto input = pool.Get(buffer);

  if(!input)
    return false;

  if(!AL_Buffer_GetMetaData(input, AL_META_TYPE_SOURCE))
  {
    if(!CreateAndAttachSourceMeta(*input, media->settings.tChParam, GetResolution()))
      return false;
  }

  translate.Add(input, buffer);

  auto const eos = (size == 0);

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

  return AL_Encoder_Process(encoder, input, roiBuffer);
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

bool EncModule::Fill(uint8_t* buffer, int offset, int size)
{
  (void)offset, (void)size;

  if(!encoder)
    return false;

  if(fds.output)
    UseDMA(buffer, static_cast<int>((intptr_t)buffer), size);
  else
    Use(buffer, buffer, size);

  auto output = pool.Get(buffer);

  if(!output)
    return false;

  if(!AL_Buffer_GetMetaData(output, AL_META_TYPE_STREAM))
  {
    if(!CreateAndAttachStreamMeta(*output))
      return false;
  }

  translate.Add(output, buffer);

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
  auto const handle = translate.Pop(buf);

  if(isDma)
    UnuseDMA(handle);
  else
    Unuse(handle);

  callbacks.release(isSrc, handle);
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

    if((errorCode & AL_ERROR) && (errorCode != AL_ERR_STREAM_OVERFLOW) && callbacks.event)
      callbacks.event(CALLBACK_EVENT_ERROR, nullptr);
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
               auto const handleIn = translate.Get(source);
               assert(handleIn);
               auto const handleOut = translate.Get(stream);
               assert(handleOut);

               callbacks.associate(handleIn, handleOut);

               if(isEndOfFrame(stream) || isEOS)
                 translate.Remove(source);

               translate.Remove(stream);

               if(isEndOfFrame(stream) || isEOS)
               {
                 if(fds.input)
                   UnuseDMA(handleIn);
                 else
                   Unuse(handleIn);
                 callbacks.emptied(handleIn, 0, 0, 0); //TODO handle
               }

               if(fds.output)
                 UnuseDMA(handleOut);
               else
                 Unuse(handleOut);

               callbacks.filled(handleOut, 0, size);
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
  auto const chan = media->settings.tChParam;
  Resolution resolution;
  resolution.width = chan.uWidth;
  resolution.height = chan.uHeight;
  resolution.stride = RoundUp(AL_CalculatePitchValue(chan.uWidth, AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER), media->strideAlignment);
  resolution.sliceHeight = RoundUp(chan.uHeight, media->sliceHeightAlignment);

  return resolution;
}

Clock EncModule::GetClock() const
{
  auto const rateCtrl = media->settings.tChParam.tRCParam;
  Clock clock;
  clock.framerate = rateCtrl.uFrameRate;
  clock.clockratio = rateCtrl.uClkRatio;
  return clock;
}

Mimes EncModule::GetMimes() const
{
  Mimes mimes;
  auto& inMime = mimes.input;
  inMime.mime = "video/x-raw"; // NV12
  inMime.compression = COMPRESSION_UNUSED; // NV12

  auto& outMime = mimes.output;
  outMime.mime = media->Mime();
  outMime.compression = media->Compression();

  return mimes;
}

Format EncModule::GetFormat() const
{
  Format format;
  auto const chan = media->settings.tChParam;
  format.color = ConvertSoftToModuleColor(AL_GET_CHROMA_MODE(chan.ePicFormat));
  format.bitdepth = GetBitdepthFromFormat(chan.ePicFormat);

  return format;
}

Bitrates EncModule::GetBitrates() const
{
  auto const rateCtrl = media->settings.tChParam.tRCParam;
  Bitrates bitrates;
  bitrates.target = rateCtrl.uTargetBitRate / 1000;
  bitrates.max = rateCtrl.uMaxBitRate / 1000;
  bitrates.cpb = rateCtrl.uCPBSize / 90;
  bitrates.ird = rateCtrl.uInitialRemDelay / 90;
  bitrates.mode = ConvertSoftToModuleRateControl(rateCtrl.eRCMode);
  bitrates.option = ConvertSoftToModuleRateControlOption(rateCtrl.eOptions);

  return bitrates;
}

Gop EncModule::GetGop() const
{
  auto const gopParam = media->settings.tChParam.tGopParam;
  Gop gop;
  gop.b = gopParam.uNumB;
  gop.length = gopParam.uGopLength;
  gop.idrFrequency = gopParam.uFreqIDR;
  gop.mode = ConvertSoftToModuleGopControl(gopParam.eMode);
  gop.gdr = ConvertSoftToModuleGdr(gopParam.eGdrMode);

  return gop;
}

QPs EncModule::GetQPs() const
{
  QPs qps;
  qps.mode = ConvertSoftToModuleQPControl(media->settings.eQpCtrlMode);

  auto const rateCtrl = media->settings.tChParam.tRCParam;
  qps.initial = rateCtrl.iInitialQP;
  qps.deltaIP = rateCtrl.uIPDelta;
  qps.deltaPB = rateCtrl.uPBDelta;
  qps.min = rateCtrl.iMinQP;
  qps.max = rateCtrl.iMaxQP;
  return qps;
}

ProfileLevelType EncModule::GetProfileLevel() const
{
  return media->ProfileLevel();
}

vector<Format> EncModule::GetFormatsSupported() const
{
  return media->FormatsSupported();
}

vector<ProfileLevelType> EncModule::GetProfileLevelSupported() const
{
  return media->ProfileLevelSupported();
}

EntropyCodingType EncModule::GetEntropyCoding() const
{
  return media->EntropyCoding();
}

bool EncModule::IsConstrainedIntraPrediction() const
{
  return media->IsConstrainedIntraPrediction();
}

LoopFilterType EncModule::GetLoopFilter() const
{
  return media->LoopFilter();
}

AspectRatioType EncModule::GetAspectRatio() const
{
  return ConvertSoftToModuleAspectRatio(media->settings.eAspectRatio);
}

bool EncModule::IsEnableLowBandwidth() const
{
  return media->IsEnableLowBandwidth();
}

bool EncModule::IsEnablePrefetchBuffer() const
{
  return media->settings.iPrefetchLevel2 != 0;
}

ScalingListType EncModule::GetScalingList() const
{
  return ConvertSoftToModuleScalingList(media->settings.eScalingList);
}

bool EncModule::IsEnableFillerData() const
{
  return media->settings.bEnableFillerData;
}

Slices EncModule::GetSlices() const
{
  Slices slices;
  slices.dependent = media->settings.bDependentSlice;

  auto const chan = media->settings.tChParam;
  slices.num = chan.uNumSlices;
  slices.size = chan.uSliceSize;

  return slices;
}

bool EncModule::IsEnableSubframe() const
{
  auto const chan = media->settings.tChParam;
  return chan.bSubframeLatency;
}

FileDescriptors EncModule::GetFileDescriptors() const
{
  return fds;
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

bool EncModule::SetResolution(Resolution const& resolution)
{
  if((resolution.width % 8) != 0)
    return false;

  if((resolution.height % 8) != 0)
    return false;

  if(resolution.stride != 0)
  {
    int const align = GetPow2MaxAlignment(8, resolution.stride);

    if(align == 0)
      return false;

    if(align > media->strideAlignment)
      media->strideAlignment = align;
  }

  if(resolution.sliceHeight != 0)
  {
    int const align = GetPow2MaxAlignment(8, resolution.sliceHeight);

    if(align == 0)
      return false;

    if(align > media->sliceHeightAlignment)
      media->sliceHeightAlignment = align;
  }

  auto& chan = media->settings.tChParam;
  chan.uWidth = resolution.width;
  chan.uHeight = resolution.height;
  return true;
}

bool EncModule::SetClock(Clock const& clock)
{
  auto& rateCtrl = media->settings.tChParam.tRCParam;
  rateCtrl.uFrameRate = clock.framerate;
  rateCtrl.uClkRatio = clock.clockratio;
  return true;
}

bool EncModule::SetFormat(Format const& format)
{
  auto& chan = media->settings.tChParam;
  AL_SET_BITDEPTH(chan.ePicFormat, format.bitdepth);
  AL_SET_CHROMA_MODE(chan.ePicFormat, ConvertModuleToSoftChroma(format.color));
  return true;
}

bool EncModule::SetBitrates(Bitrates const& bitrates)
{
  if(bitrates.mode >= RATE_CONTROL_MAX)
    return false;

  if(bitrates.max < bitrates.target)
    return false;

  auto& rateCtrl = media->settings.tChParam.tRCParam;
  rateCtrl.uTargetBitRate = bitrates.target * 1000;
  rateCtrl.uMaxBitRate = bitrates.max * 1000;
  rateCtrl.uCPBSize = bitrates.cpb * 90;
  rateCtrl.uInitialRemDelay = bitrates.ird * 90;
  rateCtrl.eRCMode = ConvertModuleToSoftRateControl(bitrates.mode);
  rateCtrl.eOptions = ConvertModuleToSoftRateControlOption(bitrates.option);
  return true;
}

bool EncModule::SetGop(Gop const& gop)
{
  if(gop.b < 0)
    return false;

  if(gop.length <= gop.b)
    return false;

  auto& gopParam = media->settings.tChParam.tGopParam;
  gopParam.uNumB = gop.b;
  gopParam.uGopLength = gop.length;
  gopParam.uFreqIDR = gop.idrFrequency;
  gopParam.eMode = ConvertModuleToSoftGopControl(gop.mode);
  gopParam.eGdrMode = ConvertModuleToSoftGdr(gop.gdr);
  return true;
}

bool EncModule::SetProfileLevel(ProfileLevelType const& profileLevel)
{
  return media->SetProfileLevel(profileLevel);
}

bool EncModule::SetEntropyCoding(EntropyCodingType const& entropyCoding)
{
  return media->SetEntropyCoding(entropyCoding);
}

bool EncModule::SetConstrainedIntraPrediction(bool const& constrainedIntraPrediction)
{
  return media->SetConstrainedIntraPrediction(constrainedIntraPrediction);
}

bool EncModule::SetLoopFilter(LoopFilterType const& loopFilter)
{
  return media->SetLoopFilter(loopFilter);
}

bool EncModule::SetQPs(QPs const& qps)
{
  // TODO check on qp
  media->settings.eQpCtrlMode = ConvertModuleToSoftQPControl(qps.mode);

  auto& rateCtrl = media->settings.tChParam.tRCParam;
  rateCtrl.iInitialQP = qps.initial;
  rateCtrl.uIPDelta = qps.deltaIP;
  rateCtrl.uPBDelta = qps.deltaPB;
  rateCtrl.iMinQP = qps.min;
  rateCtrl.iMaxQP = qps.max;

  return true;
}

bool EncModule::SetAspectRatio(AspectRatioType const& aspectRatio)
{
  if(aspectRatio == ASPECT_RATIO_MAX)
    return false;

  media->settings.eAspectRatio = ConvertModuleToSoftAspectRatio(aspectRatio);
  return true;
}

bool EncModule::SetEnableLowBandwidth(bool const& enableLowBandwidth)
{
  return media->SetEnableLowBandwidth(enableLowBandwidth);
}

bool EncModule::SetEnablePrefetchBuffer(bool const& enablePrefetchBuffer)
{
  media->settings.iPrefetchLevel2 = enablePrefetchBuffer ? !0 : 0;
  return true;
}

bool EncModule::SetScalingList(ScalingListType const& scalingList)
{
  if(scalingList == SCALING_LIST_MAX)
    return false;
  media->settings.eScalingList = ConvertModuleToSoftScalingList(scalingList);
  return true;
}

bool EncModule::SetEnableFillerData(bool const& enableFillerData)
{
  // TODO Check if CBR ?
  media->settings.bEnableFillerData = enableFillerData;
  return true;
}

bool EncModule::SetSlices(Slices const& slices)
{
  // TODO Check coherency ?
  media->settings.bDependentSlice = slices.dependent;
  auto& chan = media->settings.tChParam;
  chan.uNumSlices = slices.num;
  chan.uSliceSize = slices.size;
  return true;
}

bool EncModule::SetEnableSubframe(bool const& enableSubframe)
{
  // TODO Check slices ?
  auto& chan = media->settings.tChParam;
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

Flags EncModule::GetFlags(void* handle)
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
    auto& rateCtrl = media->settings.tChParam.tRCParam;
    rateCtrl.uTargetBitRate = bitrate;

    if(ConvertSoftToModuleRateControl(rateCtrl.eRCMode) != RATE_CONTROL_VARIABLE_BITRATE)
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
    auto const chan = media->settings.tChParam;
    AL_TDimension tDim = { chan.uWidth, chan.uHeight };
    auto const size = GetAllocSizeEP2(tDim, chan.uMaxCuSize);
    auto roiBuffer = AL_Buffer_Create_And_Allocate(allocator.get(), size, MyFree);
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
    auto const rateCtrl = media->settings.tChParam.tRCParam;
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
    auto const chan = media->settings.tChParam;
    AL_TDimension tDim = { chan.uWidth, chan.uHeight };
    *size = GetAllocSizeEP2(tDim, chan.uMaxCuSize);
    return SUCCESS;
  }

  return ERROR_NOT_IMPLEMENTED;
}

