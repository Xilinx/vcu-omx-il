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

#include "module/mediatype_enc_hevc.h"
#include "module/module_enc.h"
#include "module/cpp_memory.h"

#include "base/omx_component/omx_component_enc.h"
#include "base/omx_component/omx_expertise_hevc.h"

#if AL_ENABLE_DMA_COPY_ENC
#include "module/dma_memory.h"
#endif

#include "module/device_enc_hardware_mcu.h"

#include <cstring>
#include <memory>
#include <functional>
#include <stdexcept>

using namespace std;

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
}

static int constexpr HORIZONTAL_STRIDE_ALIGNMENTS = 64;
static int constexpr VERCTICAL_STRIDE_ALIGNMENTS_HEVC = 32;

static StrideAlignments constexpr STRIDE_ALIGNMENTS_HEVC {
  HORIZONTAL_STRIDE_ALIGNMENTS, VERCTICAL_STRIDE_ALIGNMENTS_HEVC
};

static int constexpr VERCTICAL_STRIDE_ALIGNMENTS_AVC = 16;

static StrideAlignments constexpr STRIDE_ALIGNMENTS_AVC {
  HORIZONTAL_STRIDE_ALIGNMENTS, VERCTICAL_STRIDE_ALIGNMENTS_AVC
};

#if defined(ANDROID) || defined(__ANDROID_API__)
static bool constexpr IS_SEPARATE_CONFIGURATION_FROM_DATA_ENABLED = true;
#else
static bool constexpr IS_SEPARATE_CONFIGURATION_FROM_DATA_ENABLED = false;
#endif

static MemoryInterface* createMemory()
{
#if AL_ENABLE_DMA_COPY_ENC
  char const* device = "/dev/dmaproxy";
  return new DMAMemory {
           device
  };
#else
  return new CPPMemory {};
#endif
}

static char const* ALLOC_DEVICE_ENC_NAME = "/dev/allegroIP";

static AL_TAllocator* createDmaAlloc(string deviceName)
{
  auto alloc = AL_DmaAlloc_Create(deviceName.c_str());

  if(alloc == nullptr)
    throw runtime_error {
            string {
              "Couldnt allocate dma allocator (tried using "
            } +deviceName + string {
              ")"
            }
    };
  return alloc;
}

static BufferContiguities constexpr BUFFER_CONTIGUITIES_HARDWARE {
  true, true
};

static BufferBytesAlignments constexpr BUFFER_BYTES_ALIGNMENTS_HARDWARE {
  HORIZONTAL_STRIDE_ALIGNMENTS, HORIZONTAL_STRIDE_ALIGNMENTS
};

#include "base/omx_component/omx_expertise_avc.h"
#include "module/mediatype_enc_avc.h"

static EncComponent* GenerateAvcComponentHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole)
{
  shared_ptr<EncMediatypeAVC> media {
    new EncMediatypeAVC {
      BUFFER_CONTIGUITIES_HARDWARE, BUFFER_BYTES_ALIGNMENTS_HARDWARE, STRIDE_ALIGNMENTS_AVC, IS_SEPARATE_CONFIGURATION_FROM_DATA_ENABLED
    }
  };
  shared_ptr<AL_TAllocator> allocator {
    createDmaAlloc(ALLOC_DEVICE_ENC_NAME), [](AL_TAllocator* allocator) {
      AL_Allocator_Destroy(allocator);
    }
  };
  shared_ptr<EncDeviceHardwareMcu> device {
    new EncDeviceHardwareMcu {
      allocator
    }
  };
  shared_ptr<MemoryInterface> memory {
    createMemory()
  };
  unique_ptr<EncModule> module {
    new EncModule {
      media, device, allocator, memory
    }
  };
  unique_ptr<ExpertiseAVC> expertise {
    new ExpertiseAVC {}
  };
  return new EncComponent {
           hComponent, media, move(module), cComponentName, cRole, move(expertise)
  };
}

static EncComponent* GenerateHevcComponentHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole)
{
  shared_ptr<EncMediatypeHEVC> media {
    new EncMediatypeHEVC {
      BUFFER_CONTIGUITIES_HARDWARE, BUFFER_BYTES_ALIGNMENTS_HARDWARE, STRIDE_ALIGNMENTS_HEVC, IS_SEPARATE_CONFIGURATION_FROM_DATA_ENABLED
    }
  };
  shared_ptr<AL_TAllocator> allocator {
    createDmaAlloc(ALLOC_DEVICE_ENC_NAME), [](AL_TAllocator* allocator) {
      AL_Allocator_Destroy(allocator);
    }
  };
  shared_ptr<EncDeviceHardwareMcu> device {
    new EncDeviceHardwareMcu {
      allocator
    }
  };
  shared_ptr<MemoryInterface> memory {
    createMemory()
  };
  unique_ptr<EncModule> module {
    new EncModule {
      media, device, allocator, memory
    }
  };
  unique_ptr<ExpertiseHEVC> expertise {
    new ExpertiseHEVC {}
  };
  return new EncComponent {
           hComponent, media, move(module), cComponentName, cRole, move(expertise)
  };
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

OMX_PTR CreateEncComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{
  return GenerateDefaultComponent(hComponent, cComponentName, cRole);
}

void DestroyEncComponentPrivate(OMX_IN OMX_PTR pComponentPrivate)
{
  delete static_cast<EncComponent*>(pComponentPrivate);
}

