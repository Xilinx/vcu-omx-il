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
#include "base/omx_checker/omx_checker.h"
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

#include "omx_convert_module_soft_enc.h"
#include "omx_convert_module_soft.h"

static inline int GetBitdepthFromFormat(AL_EPicFormat const& format)
{
  return AL_GET_BITDEPTH(format);
}

static bool CheckValidity(AL_TEncSettings const& settings)
{
  auto err = AL_Settings_CheckValidity(const_cast<AL_TEncSettings*>(&settings), stderr);

  return (err == AL_SUCCESS);
}

static void LookCoherency(AL_TEncSettings& settings)
{
  auto const chan = settings.tChParam;
  auto const fourCC = AL_EncGetSrcFourCC({ AL_GET_CHROMA_MODE(chan.ePicFormat), AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER });
  AL_Settings_CheckCoherency(&settings, fourCC, stdout);
}

EncModule::EncModule(std::unique_ptr<EncMediatypeInterface>&& media, std::unique_ptr<EncDevice>&& device, deleted_unique_ptr<AL_TAllocator>&& allocator) :
  media(std::move(media)),
  device(std::move(device)),
  allocator(std::move(allocator))
{
  assert(this->media);
  assert(this->device);
  assert(this->allocator);
  encoder = nullptr;
  isCreated = false;
  ResetRequirements();
}

bool EncModule::CreateEncoder()
{
  if(encoder)
  {
    fprintf(stderr, "Encoder is ALREADY created\n");
    assert(0);
  }

  auto& settings = media->settings;
  scheduler = device->Init(settings, *allocator.get());
  auto errorCode = AL_Encoder_Create(&encoder, scheduler, allocator.get(), &media->settings, { EncModule::RedirectionEndEncoding, this });

  if(errorCode != AL_SUCCESS)
  {
    fprintf(stderr, "Failed to create Encoder: %d\n", errorCode);
    return false;
  }

  eosHandles.output = nullptr;

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

bool EncModule::Pause()
{
  if(!encoder)
    return false;

  if(!eosHandles.eosSent)
  {
    fprintf(stderr, "Passing in Pause without sending eos\n");
    assert(0);
    return false;
  }

  std::unique_lock<std::mutex> lock(eosHandles.mutex);
  eosHandles.cv.wait(lock, [&] { return eosHandles.eosReceive == true;
                     });
  return DestroyEncoder();
}

void EncModule::Stop()
{
  if(!encoder)
    return;

  if(!eosHandles.eosSent)
  {
    fprintf(stderr, "Passing in Stop without sending eos\n");
    assert(0);
    return;
  }

  std::unique_lock<std::mutex> lock(eosHandles.mutex);
  eosHandles.cv.wait(lock, [&] { return eosHandles.eosReceive == true;
                     });
  lock.unlock();
  DestroyEncoder();
}

void EncModule::ResetRequirements()
{
  media->Reset();
  auto& chan = media->settings.tChParam;
  chan.uWidth = 176;
  chan.uHeight = 144;
  chan.ePicFormat = AL_420_8BITS;

  auto& rateCtrl = chan.tRCParam;
  rateCtrl.eOptions = AL_RC_OPT_SCN_CHG_RES;
  fds.input = fds.output = false;
  media->settings.bEnableAUD = false;
}

BuffersRequirements EncModule::GetBuffersRequirements() const
{
  BuffersRequirements b;
  auto const chan = media->settings.tChParam;
  auto const gop = chan.tGopParam;
  auto& input = b.input;
  input.min = 1 + gop.uNumB;
  input.size = GetAllocSize_Src({ chan.uWidth, chan.uHeight }, AL_GET_BITDEPTH(chan.ePicFormat), AL_GET_CHROMA_MODE(chan.ePicFormat), chan.eSrcMode);
  input.bytesAlignment = device->GetAllocationRequirements().input.bytesAlignment;
  input.contiguous = device->GetAllocationRequirements().input.contiguous;

  auto& output = b.output;
  output.min = 2 + gop.uNumB + 1; // 1 for eos
  output.size = AL_GetMaxNalSize({ chan.uWidth, chan.uHeight }, AL_GET_CHROMA_MODE(chan.ePicFormat));
  output.bytesAlignment = device->GetAllocationRequirements().output.bytesAlignment;
  output.contiguous = device->GetAllocationRequirements().output.contiguous;

  return b;
}

int EncModule::GetLatency() const
{
  auto const chan = media->settings.tChParam;
  auto const gopParam = chan.tGopParam;
  auto const rateCtrl = chan.tRCParam;
  auto const bufsCount = gopParam.uNumB + 1;

  auto const realFramerate = (static_cast<double>(rateCtrl.uFrameRate * rateCtrl.uClkRatio) / 1000.0);

  auto timeInMilliseconds = (static_cast<double>(bufsCount * 1000.0) / realFramerate);

  if(chan.bSubframeLatency)
    timeInMilliseconds /= chan.uNumSlices;

  return std::ceil(timeInMilliseconds);
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

  auto handle = allocated.Get(buffer);
  assert(handle);

  AL_Allocator_Free(allocator.get(), handle);

  allocated.Remove(buffer);
}

void EncModule::FreeDMA(int fd)
{
  if(fd < 0)
    return;

  auto handle = allocatedDMA.Get(fd);
  assert(handle);

  AL_Allocator_Free(allocator.get(), handle);
  close(fd);

  allocatedDMA.Remove(fd);
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
    throw std::invalid_argument("handle");

  if(!buffer)
    throw std::invalid_argument("buffer");

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
    throw std::invalid_argument("handle");

  if(fd < 0)
    throw std::invalid_argument("fd");

  auto dmaHandle = AL_LinuxDmaAllocator_ImportFromFd((AL_TLinuxDmaAllocator*)allocator.get(), fd);

  if(!dmaHandle)
  {
    fprintf(stderr, "Failed to import fd : %i\n", fd);
    return false;
  }

  auto encoderBuffer = AL_Buffer_Create(allocator.get(), dmaHandle, size, AL_Buffer_Destroy);

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
    throw std::invalid_argument("handle");

  auto encoderBuffer = pool.Get(handle);
  assert(encoderBuffer);

  pool.Remove(handle);

  if(shouldBeCopied.Exist(encoderBuffer))
    shouldBeCopied.Remove(encoderBuffer);
  AL_Buffer_Unref(encoderBuffer);
}

void EncModule::UnuseDMA(void* handle)
{
  if(!handle)
    throw std::invalid_argument("handle");

  auto encoderBuffer = pool.Get(handle);
  assert(encoderBuffer);
  pool.Remove(handle);
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

bool EncModule::Empty(uint8_t* buffer, int offset, int size)
{
  (void)offset;

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
    if(!CreateAndAttachSourceMeta(*input, media->settings.tChParam, GetResolutions().input))
      return false;
  }

  translate.Add(input, buffer);

  auto const eos = (size == 0);

  if(eos)
  {
    eosHandles.input = input;
    eosHandles.eosSent = true;
    return AL_Encoder_Process(encoder, nullptr, nullptr);
  }

  eosHandles.input = nullptr;
  eosHandles.eosSent = false;

  if(shouldBeCopied.Exist(input))
  {
    auto buffer = shouldBeCopied.Get(input);
    memcpy(AL_Buffer_GetData(input), buffer, input->zSize);
  }

  return AL_Encoder_Process(encoder, input, nullptr);
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
  (void)offset;
  (void)size;

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
  auto const handle = translate.Get(buf);
  assert(handle);
  translate.Remove(buf);

  if(isDma)
    UnuseDMA(handle);
  else
    Unuse(handle);

  callbacks.release(isSrc, handle);
}

void EncModule::EndEncoding(AL_TBuffer* stream, AL_TBuffer const* source)
{
  auto const isStreamRelease = (stream && source == nullptr);
  auto const isSrcRelease = (stream == nullptr && source);

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

               translate.Remove(source);
               translate.Remove(stream);

               if(fds.input)
                 UnuseDMA(handleIn);
               else
                 Unuse(handleIn);

               callbacks.emptied(handleIn, 0, 0);

               if(fds.output)
                 UnuseDMA(handleOut);
               else
                 Unuse(handleOut);

               callbacks.filled(handleOut, 0, size);
             };

  if(isEOS)
  {
    end(eosHandles.input, eosHandles.output, 0);
    std::lock_guard<std::mutex> lock(eosHandles.mutex);
    eosHandles.eosReceive = true;
    eosHandles.cv.notify_one();
    return;
  }

  std::lock_guard<std::mutex> lock(eosHandles.mutex);
  eosHandles.eosReceive = false;
  eosHandles.cv.notify_one();

  auto const size = ReconstructStream(*stream);

  if(shouldBeCopied.Exist(stream))
  {
    auto buffer = shouldBeCopied.Get(stream);
    memcpy(buffer, AL_Buffer_GetData(stream), size);
  }

  end(const_cast<AL_TBuffer*>(source), stream, size);
}

#define ROUNDUP(n,width) (((n) + (width) - 1) & ~unsigned((width) - 1))

Resolutions EncModule::GetResolutions() const
{
  auto const chan = media->settings.tChParam;
  Resolutions resolutions;
  auto& inRes = resolutions.input;
  auto& outRes = resolutions.output;

  inRes.width = outRes.width = chan.uWidth;
  inRes.height = outRes.height = chan.uHeight;
  inRes.stride = outRes.stride = ROUNDUP(AL_CalculatePitchValue(chan.uWidth, AL_GET_BITDEPTH(chan.ePicFormat), AL_FB_RASTER), media->strideAlignment);
  inRes.sliceHeight = outRes.sliceHeight = ROUNDUP(chan.uHeight, media->sliceHeightAlignment);

  return resolutions;
}

Clocks EncModule::GetClocks() const
{
  auto const rateCtrl = media->settings.tChParam.tRCParam;
  Clocks clocks;

  auto& inClock = clocks.input;
  auto& outClock = clocks.output;

  inClock.framerate = outClock.framerate = rateCtrl.uFrameRate;
  inClock.clockratio = outClock.clockratio = rateCtrl.uClkRatio;
  return clocks;
}

Formats EncModule::GetFormats() const
{
  auto const chan = media->settings.tChParam;
  Formats formats;

  auto& inFormat = formats.input;
  inFormat.mime = "video/x-raw"; // NV12
  inFormat.compression = COMPRESSION_UNUSED; // NV12

  auto& outFormat = formats.output;
  outFormat.mime = media->Mime();
  outFormat.compression = media->Compression();

  inFormat.color = outFormat.color = ConvertToModuleColor(AL_GET_CHROMA_MODE(chan.ePicFormat));
  inFormat.bitdepth = outFormat.bitdepth = GetBitdepthFromFormat(chan.ePicFormat);

  return formats;
}

Bitrates EncModule::GetBitrates() const
{
  auto const rateCtrl = media->settings.tChParam.tRCParam;
  Bitrates bitrates;
  bitrates.target = rateCtrl.uTargetBitRate / 1000;
  bitrates.max = rateCtrl.uMaxBitRate / 1000;
  bitrates.cpb = rateCtrl.uCPBSize / 90;
  bitrates.ird = rateCtrl.uInitialRemDelay / 90;
  bitrates.mode = ConvertToModuleRateControl(rateCtrl.eRCMode);
  bitrates.option = ConvertToModuleRateControlOption(rateCtrl.eOptions);

  return bitrates;
}

Gop EncModule::GetGop() const
{
  auto const gopParam = media->settings.tChParam.tGopParam;
  Gop gop;
  gop.b = gopParam.uNumB;
  gop.length = gopParam.uGopLength;
  gop.idrFrequency = gopParam.uFreqIDR;
  gop.mode = ConvertToModuleGopControl(gopParam.eMode);
  gop.gdr = ConvertToModuleGdr(gopParam.eGdrMode);

  return gop;
}

QPs EncModule::GetQPs() const
{
  QPs qps;
  qps.mode = ConvertToModuleQPControl(media->settings.eQpCtrlMode);

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

std::vector<ProfileLevelType> EncModule::GetProfileLevelSupported() const
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
  return ConvertToModuleAspectRatio(media->settings.eAspectRatio);
}

bool EncModule::IsEnableLowBandwidth() const
{
  return media->IsEnableLowBandwidth();
}

int EncModule::GetPrefetchBufferSize() const
{
  return media->settings.iPrefetchLevel2 / 1024;
}

ScalingListType EncModule::GetScalingList() const
{
  return ConvertToModuleScalingList(media->settings.eScalingList);
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

bool EncModule::SetResolutions(Resolutions const& resolutions)
{
  if(resolutions.input != resolutions.output)
    return false;

  if((GetPow2MaxAlignment(8, resolutions.input.stride) == 0)||(GetPow2MaxAlignment(8, resolutions.input.sliceHeight) == 0))
    return false;

  auto& chan = media->settings.tChParam;
  chan.uWidth = resolutions.input.width;
  chan.uHeight = resolutions.input.height;
  media->strideAlignment = GetPow2MaxAlignment(8, resolutions.input.stride);
  media->sliceHeightAlignment = GetPow2MaxAlignment(8, resolutions.input.sliceHeight);
  return true;
}

bool EncModule::SetClocks(Clocks const& clocks)
{
  if(clocks.input != clocks.output)
    return false;

  auto& rateCtrl = media->settings.tChParam.tRCParam;
  rateCtrl.uFrameRate = clocks.input.framerate;
  rateCtrl.uClkRatio = clocks.input.clockratio;
  return true;
}

bool EncModule::SetFormats(Formats const& formats)
{
  if(formats.input.bitdepth != formats.output.bitdepth)
    return false;

  if(formats.input.color != formats.output.color)
    return false;
  auto& chan = media->settings.tChParam;
  AL_SET_BITDEPTH(chan.ePicFormat, formats.input.bitdepth);
  AL_SET_CHROMA_MODE(chan.ePicFormat, ConvertToSoftChroma(formats.input.color));
  return true;
}

bool EncModule::SetBitrates(Bitrates const& bitrates)
{
  if(bitrates.max < bitrates.target)
    return false;

  auto& rateCtrl = media->settings.tChParam.tRCParam;
  rateCtrl.uTargetBitRate = bitrates.target * 1000;
  rateCtrl.uMaxBitRate = bitrates.max * 1000;
  rateCtrl.uCPBSize = bitrates.cpb * 90;
  rateCtrl.uInitialRemDelay = bitrates.ird * 90;
  rateCtrl.eRCMode = ConvertToSoftRateControl(bitrates.mode);
  rateCtrl.eOptions = ConvertToSoftRateControlOption(bitrates.option);
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
  gopParam.eMode = ConvertToSoftGopControl(gop.mode);
  gopParam.eGdrMode = ConvertToSoftGdr(gop.gdr);
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
  media->settings.eQpCtrlMode = ConvertToSoftQPControl(qps.mode);

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

  media->settings.eAspectRatio = ConvertToSoftAspectRatio(aspectRatio);
  return true;
}

bool EncModule::SetEnableLowBandwidth(bool const& enableLowBandwidth)
{
  return media->SetEnableLowBandwidth(enableLowBandwidth);
}

bool EncModule::SetPrefetchBufferSize(int const& size)
{
  if(size < 0)
    return false;
  media->settings.iPrefetchLevel2 = size * 1024;
  return true;
}

bool EncModule::SetScalingList(ScalingListType const& scalingList)
{
  if(scalingList == SCALING_LIST_MAX)
    return false;
  media->settings.eScalingList = ConvertToSoftScalingList(scalingList);
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

Flags EncModule::GetFlags(void* handle)
{
  Flags flags;
  auto stream = pool.Get(handle);
  assert(stream);

  auto const meta = (AL_TStreamMetaData*)(AL_Buffer_GetMetaData(stream, AL_META_TYPE_STREAM));
  assert(meta);

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

