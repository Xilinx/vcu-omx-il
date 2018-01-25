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

#include "base/omx_module/omx_module_enc.h"
#include "base/omx_codec/omx_codec_enc.h"
#include "base/omx_codec/omx_expertise_enc_hevc.h"
#include "base/omx_mediatype/omx_mediatype_enc_hevc.h"



#include "base/omx_module/omx_device_enc_hardware_mcu.h"

#include <string.h>
#include <memory>
#include <functional>
#include <stdexcept>

using namespace std;

extern "C" {
#include "lib_fpga/DmaAlloc.h"
}

static AL_TAllocator* createDmaAlloc(string deviceName)
{
  auto alloc = DmaAlloc_Create(deviceName.c_str());

  if(alloc == nullptr)
    throw runtime_error(string("Couldnt allocate dma allocator (tried using ") + deviceName + string(")"));
  return alloc;
}


#include "base/omx_codec/omx_expertise_enc_avc.h"
#include "base/omx_mediatype/omx_mediatype_enc_avc.h"


static EncCodec* GenerateAvcCodecHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole)
{
  unique_ptr<EncMediatypeAVC> mediatype(new EncMediatypeAVC);
  unique_ptr<EncDeviceHardwareMcu> device(new EncDeviceHardwareMcu);
  deleted_unique_ptr<AL_TAllocator> allocator(createDmaAlloc("/dev/allegroIP"), [](AL_TAllocator* allocator) {
    AL_Allocator_Destroy(allocator);
  });
  unique_ptr<EncModule> module(new EncModule(move(mediatype), move(device), move(allocator)));
  unique_ptr<EncExpertiseAVC> expertise(new EncExpertiseAVC);
  return new EncCodec(hComponent, move(module), cComponentName, cRole, move(expertise));
}


static EncCodec* GenerateHevcCodecHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole)
{
  unique_ptr<EncMediatypeHEVC> mediatype(new EncMediatypeHEVC);
  unique_ptr<EncDeviceHardwareMcu> device(new EncDeviceHardwareMcu);
  deleted_unique_ptr<AL_TAllocator> allocator(createDmaAlloc("/dev/allegroIP"), [](AL_TAllocator* allocator) {
    AL_Allocator_Destroy(allocator);
  });
  unique_ptr<EncModule> module(new EncModule(move(mediatype), move(device), move(allocator)));
  unique_ptr<EncExpertiseHEVC> expertise(new EncExpertiseHEVC);
  return new EncCodec(hComponent, move(module), cComponentName, cRole, move(expertise));
}


static OMX_PTR GenerateDefaultCodec(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{
  OMX_PTR enc = nullptr;


  if(!strncmp(cComponentName, "OMX.allegro.h265.hardware.encoder", strlen(cComponentName)))
    enc = GenerateHevcCodecHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h265.encoder", strlen(cComponentName)))
    enc = GenerateHevcCodecHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h264.hardware.encoder", strlen(cComponentName)))
    enc = GenerateAvcCodecHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h264.encoder", strlen(cComponentName)))
    enc = GenerateAvcCodecHardware(hComponent, cComponentName, cRole);
  return enc;
}

extern "C"
{
OMX_PTR CreateComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{
  return GenerateDefaultCodec(hComponent, cComponentName, cRole);
}

void DestroyComponentPrivate(OMX_IN OMX_PTR pComponentPrivate)
{
  delete static_cast<EncCodec*>(pComponentPrivate);
}
}

