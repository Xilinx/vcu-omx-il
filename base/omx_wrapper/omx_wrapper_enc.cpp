/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#include "module/settings_enc_hevc.h"
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
  return new DMAMemory(device);
#else
  return new CPPMemory();
#endif
}

#include <cstdlib>

static char const* DEVICE_ENC_NAME()
{
  if(getenv("ALLEGRO_ENC_DEVICE_PATH"))
    return getenv("ALLEGRO_ENC_DEVICE_PATH");
  return "/dev/allegroIP";
}

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
#include "module/settings_enc_avc.h"

static EncComponent* GenerateAvcComponentHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole, OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_PTR pSettings)
{
  (void)nCoreParamIndex;
  (void)pSettings;
  string deviceName = DEVICE_ENC_NAME();

  if(nCoreParamIndex == OMX_ALG_CoreIndexDevice)
    deviceName = ((OMX_ALG_CORE_DEVICE*)pSettings)->cDevice;

  shared_ptr<AL_TAllocator> allocator {
    createDmaAlloc(deviceName), [](AL_TAllocator* allocator) {
      AL_Allocator_Destroy(allocator);
    }
  };
  shared_ptr<EncSettingsAVC> media {
    new EncSettingsAVC {
      BUFFER_CONTIGUITIES_HARDWARE, BUFFER_BYTES_ALIGNMENTS_HARDWARE, STRIDE_ALIGNMENTS_AVC, IS_SEPARATE_CONFIGURATION_FROM_DATA_ENABLED, allocator
    }
  };
  shared_ptr<EncDeviceHardwareMcu> device {
    new EncDeviceHardwareMcu {
      deviceName,
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

static EncComponent* GenerateHevcComponentHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole, OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_PTR pSettings)
{
  (void)nCoreParamIndex;
  (void)pSettings;
  string deviceName = DEVICE_ENC_NAME();

  if(nCoreParamIndex == OMX_ALG_CoreIndexDevice)
    deviceName = ((OMX_ALG_CORE_DEVICE*)pSettings)->cDevice;

  shared_ptr<AL_TAllocator> allocator {
    createDmaAlloc(deviceName), [](AL_TAllocator* allocator) {
      AL_Allocator_Destroy(allocator);
    }
  };
  shared_ptr<EncSettingsHEVC> media {
    new EncSettingsHEVC {
      BUFFER_CONTIGUITIES_HARDWARE, BUFFER_BYTES_ALIGNMENTS_HARDWARE, STRIDE_ALIGNMENTS_HEVC, IS_SEPARATE_CONFIGURATION_FROM_DATA_ENABLED, allocator
    }
  };
  shared_ptr<EncDeviceHardwareMcu> device {
    new EncDeviceHardwareMcu {
      deviceName,
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

static OMX_PTR GenerateDefaultComponent(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_IN OMX_PTR pSettings)
{

  if(!strncmp(cComponentName, "OMX.allegro.h265.hardware.encoder", strlen(cComponentName)))
    return GenerateHevcComponentHardware(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);

  if(!strncmp(cComponentName, "OMX.allegro.h265.encoder", strlen(cComponentName)))
    return GenerateHevcComponentHardware(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);

  if(!strncmp(cComponentName, "OMX.allegro.h264.hardware.encoder", strlen(cComponentName)))
    return GenerateAvcComponentHardware(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);

  if(!strncmp(cComponentName, "OMX.allegro.h264.encoder", strlen(cComponentName)))
    return GenerateAvcComponentHardware(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);
  return nullptr;
}

OMX_PTR CreateEncComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_IN OMX_PTR pSettings)
{
  return GenerateDefaultComponent(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);
}

void DestroyEncComponentPrivate(OMX_IN OMX_PTR pComponentPrivate)
{
  delete static_cast<EncComponent*>(pComponentPrivate);
}

