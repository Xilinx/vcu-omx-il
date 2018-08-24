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

#include "base/omx_module/omx_module_enc.h"
#include "base/omx_component/omx_component_enc.h"
#include "base/omx_component/omx_expertise_hevc.h"
#include "base/omx_mediatype/omx_mediatype_enc_hevc.h"

#if AL_ENABLE_SYNCIP
#include "base/omx_module/omx_sync_ip.h"
#else
#include "base/omx_module/null_sync_ip.h"
#endif



#include "base/omx_module/omx_device_enc_hardware_mcu.h"

#include <cstring>
#include <memory>
#include <functional>
#include <stdexcept>

using namespace std;

extern "C" {
#include <lib_fpga/DmaAlloc.h>
}

static SyncIpInterface* createSyncIp(shared_ptr<MediatypeInterface> media, shared_ptr<AL_TAllocator> allocator)
{
#if AL_ENABLE_SYNCIP
  return new OMXSyncIp(media, allocator);
#else
  (void)media, (void)allocator;
  return new NullSyncIp();
#endif
}

static AL_TAllocator* createDmaAlloc(string deviceName)
{
  auto alloc = AL_DmaAlloc_Create(deviceName.c_str());

  if(alloc == nullptr)
    throw runtime_error(string("Couldnt allocate dma allocator (tried using ") + deviceName + string(")"));
  return alloc;
}



#include "base/omx_component/omx_expertise_avc.h"
#include "base/omx_mediatype/omx_mediatype_enc_avc.h"


static EncComponent* GenerateAvcComponentHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole)
{
  shared_ptr<EncMediatypeAVC> media(new EncMediatypeAVC());
  shared_ptr<EncDeviceHardwareMcu> device(new EncDeviceHardwareMcu);
  shared_ptr<AL_TAllocator> allocator(createDmaAlloc("/dev/allegroIP"), [](AL_TAllocator* allocator) {
    AL_Allocator_Destroy(allocator);
  });
  unique_ptr<EncModule> module(new EncModule(media, device, allocator));
  unique_ptr<ExpertiseAVC> expertise(new ExpertiseAVC());
  shared_ptr<SyncIpInterface> syncIp(createSyncIp(media, allocator));
  return new EncComponent(hComponent, media, move(module), cComponentName, cRole, move(expertise), syncIp);
}


static EncComponent* GenerateHevcComponentHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole)
{
  shared_ptr<EncMediatypeHEVC> media(new EncMediatypeHEVC());
  shared_ptr<EncDeviceHardwareMcu> device(new EncDeviceHardwareMcu);
  shared_ptr<AL_TAllocator> allocator(createDmaAlloc("/dev/allegroIP"), [](AL_TAllocator* allocator) {
    AL_Allocator_Destroy(allocator);
  });
  unique_ptr<EncModule> module(new EncModule(media, device, allocator));
  unique_ptr<ExpertiseHEVC> expertise(new ExpertiseHEVC());
  shared_ptr<SyncIpInterface> syncIp(createSyncIp(media, allocator));
  return new EncComponent(hComponent, media, move(module), cComponentName, cRole, move(expertise), syncIp);
}


static OMX_PTR GenerateDefaultComponent(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{

  if(!strncmp(cComponentName, "OMX.allegro.h265.hardware.encoder", strlen(cComponentName)))
    return GenerateHevcComponentHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h265.encoder", strlen(cComponentName)))
    return GenerateHevcComponentHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h264.hardware.encoder", strlen(cComponentName)))
    return GenerateAvcComponentHardware(hComponent, cComponentName, cRole);

  if(!strncmp(cComponentName, "OMX.allegro.h264.encoder", strlen(cComponentName)))
    return GenerateAvcComponentHardware(hComponent, cComponentName, cRole);
  return nullptr;
}

extern "C"
{
OMX_PTR CreateComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{
  return GenerateDefaultComponent(hComponent, cComponentName, cRole);
}

void DestroyComponentPrivate(OMX_IN OMX_PTR pComponentPrivate)
{
  delete static_cast<EncComponent*>(pComponentPrivate);
}
}

