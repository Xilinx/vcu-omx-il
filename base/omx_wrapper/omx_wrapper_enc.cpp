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

#include "omx_wrapper.h"

#include "base/omx_module/omx_module_enc.h"
#include "base/omx_codec/omx_codec_enc.h"
#include "base/omx_codec/omx_expertise_enc_hevc.h"
#include "base/omx_mediatype/omx_mediatype_enc_hevc.h"



#include "base/omx_module/omx_device_enc_hardware_mcu.h"

#include <string.h>
#include <memory>
#include <functional>

extern "C" {
#include "lib_fpga/DmaAlloc.h"
}

#include "base/omx_codec/omx_expertise_enc_avc.h"
#include "base/omx_mediatype/omx_mediatype_enc_avc.h"

static EncCodec* GenerateAvcCodecHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole)
{
  std::unique_ptr<EncMediatypeAVC> mediatype(new EncMediatypeAVC);
  std::unique_ptr<EncDeviceHardwareMcu> device(new EncDeviceHardwareMcu);
  deleted_unique_ptr<AL_TAllocator> allocator(DmaAlloc_Create("/dev/allegroIP"), [](AL_TAllocator* allocator) {
    AL_Allocator_Destroy(allocator);
  });
  std::unique_ptr<EncModule> module(new EncModule(std::move(mediatype), std::move(device), std::move(allocator)));
  std::unique_ptr<EncExpertiseAVC> expertise(new EncExpertiseAVC);
  return new EncCodec(hComponent, std::move(module), cComponentName, cRole, std::move(expertise));
}


static EncCodec* GenerateHevcCodecHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole)
{
  std::unique_ptr<EncMediatypeHEVC> mediatype(new EncMediatypeHEVC);
  std::unique_ptr<EncDeviceHardwareMcu> device(new EncDeviceHardwareMcu);
  deleted_unique_ptr<AL_TAllocator> allocator(DmaAlloc_Create("/dev/allegroIP"), [](AL_TAllocator* allocator) {
    AL_Allocator_Destroy(allocator);
  });
  std::unique_ptr<EncModule> module(new EncModule(std::move(mediatype), std::move(device), std::move(allocator)));
  std::unique_ptr<EncExpertiseHEVC> expertise(new EncExpertiseHEVC);
  return new EncCodec(hComponent, std::move(module), cComponentName, cRole, std::move(expertise));
}


static OMX_PTR GenerateDefaultCodec(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{
  auto w = new Wrapper;
  w->base = nullptr;


  if(!strncmp(cComponentName, "OMX.allegro.h265.hardware.encoder", strlen(cComponentName)))
    w->base = GenerateHevcCodecHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h265.encoder", strlen(cComponentName)))
    w->base = GenerateHevcCodecHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h264.hardware.encoder", strlen(cComponentName)))
    w->base = GenerateAvcCodecHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h264.encoder", strlen(cComponentName)))
    w->base = GenerateAvcCodecHardware(hComponent, cComponentName, cRole);
  return w;
}

static void DestroyPrivate(Wrapper* w)
{
  if(!w)
    return;

  if(w->base)
    delete w->base;

  delete w;
}

extern "C"
{
OMX_PTR CreateComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{
  return GenerateDefaultCodec(hComponent, cComponentName, cRole);
}

void DestroyComponentPrivate(OMX_IN OMX_PTR pComponentPrivate)
{
  auto w = static_cast<Wrapper*>(pComponentPrivate);
  DestroyPrivate(w);
}
}

