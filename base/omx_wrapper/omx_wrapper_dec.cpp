// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "base/omx_component/omx_component_dec.h"
#include "base/omx_component/omx_expertise_hevc.h"
#include "module/settings_dec_hevc.h"
#include "module/module_dec.h"

#include "module/device_dec_hardware_mcu.h"

#include <cstring>
#include <memory>
#include <functional>
#include <stdexcept>

using namespace std;

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
}

#include <cstdlib>

static char const* DEVICE_DEC_NAME()
{
  if(getenv("ALLEGRO_DEC_DEVICE_PATH"))
    return getenv("ALLEGRO_DEC_DEVICE_PATH");
  return "/dev/allegroDecodeIP";
}

static AL_TAllocator* createDmaAlloc(string const deviceName)
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
  false, true
};

static BufferBytesAlignments constexpr BUFFER_BYTES_ALIGNMENTS_HARDWARE {
  0, 32
};

static StrideAlignments constexpr STRIDE_ALIGNMENTS_HARDWARE
{
  64, 64
};

#include "base/omx_component/omx_expertise_avc.h"
#include "module/settings_dec_avc.h"

static DecComponent* GenerateAvcComponentHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole, OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_PTR pSettings)
{
  (void)nCoreParamIndex;
  (void)pSettings;
  shared_ptr<DecSettingsAVC> media {
    new DecSettingsAVC {
      BUFFER_CONTIGUITIES_HARDWARE, BUFFER_BYTES_ALIGNMENTS_HARDWARE, STRIDE_ALIGNMENTS_HARDWARE
    }
  };
  string deviceName = DEVICE_DEC_NAME();

  if(nCoreParamIndex == OMX_ALG_CoreIndexDevice)
    deviceName = ((OMX_ALG_CORE_DEVICE*)pSettings)->cDevice;

  shared_ptr<DecDeviceHardwareMcu> device {
    new DecDeviceHardwareMcu {
      deviceName
    }
  };
  shared_ptr<AL_TAllocator> allocator {
    createDmaAlloc(deviceName), [](AL_TAllocator* allocator) {
      AL_Allocator_Destroy(allocator);
    }
  };
  unique_ptr<DecModule> module {
    new DecModule {
      media, device, allocator
    }
  };
  unique_ptr<ExpertiseAVC> expertise {
    new ExpertiseAVC {}
  };
  return new DecComponent {
           hComponent, media, move(module), cComponentName, cRole, move(expertise)
  };
}

static DecComponent* GenerateHevcComponentHardware(OMX_HANDLETYPE hComponent, OMX_STRING cComponentName, OMX_STRING cRole, OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_PTR pSettings)
{
  (void)nCoreParamIndex;
  (void)pSettings;
  shared_ptr<DecSettingsHEVC> media {
    new DecSettingsHEVC {
      BUFFER_CONTIGUITIES_HARDWARE, BUFFER_BYTES_ALIGNMENTS_HARDWARE, STRIDE_ALIGNMENTS_HARDWARE
    }
  };
  string deviceName = DEVICE_DEC_NAME();

  if(nCoreParamIndex == OMX_ALG_CoreIndexDevice)
    deviceName = ((OMX_ALG_CORE_DEVICE*)pSettings)->cDevice;

  shared_ptr<DecDeviceHardwareMcu> device {
    new DecDeviceHardwareMcu {
      deviceName
    }
  };
  shared_ptr<AL_TAllocator> allocator {
    createDmaAlloc(deviceName), [](AL_TAllocator* allocator) {
      AL_Allocator_Destroy(allocator);
    }
  };
  unique_ptr<DecModule> module {
    new DecModule {
      media, device, allocator
    }
  };
  unique_ptr<ExpertiseHEVC> expertise {
    new ExpertiseHEVC {}
  };
  return new DecComponent {
           hComponent, media, move(module), cComponentName, cRole, move(expertise)
  };
}

static OMX_PTR GenerateDefaultComponent(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_IN OMX_PTR pSettings)
{

  if(!strncmp(cComponentName, "OMX.allegro.h265.hardware.decoder", strlen(cComponentName)))
    return GenerateHevcComponentHardware(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);

  if(!strncmp(cComponentName, "OMX.allegro.h265.decoder", strlen(cComponentName)))
    return GenerateHevcComponentHardware(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);

  if(!strncmp(cComponentName, "OMX.allegro.h264.hardware.decoder", strlen(cComponentName)))
    return GenerateAvcComponentHardware(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);

  if(!strncmp(cComponentName, "OMX.allegro.h264.decoder", strlen(cComponentName)))
    return GenerateAvcComponentHardware(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);
  return nullptr;
}

OMX_PTR CreateDecComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_IN OMX_PTR pSettings)
{
  return GenerateDefaultComponent(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);
}

void DestroyDecComponentPrivate(OMX_IN OMX_PTR pComponentPrivate)
{
  delete static_cast<DecComponent*>(pComponentPrivate);
}

