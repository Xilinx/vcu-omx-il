// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

extern "C"
{
}

#include <stdexcept>

extern "C"
{
#include "lib_common/Allocator.h"
#include "lib_common_enc/Settings.h"
#include "lib_common_enc/EncChanParam.h"
}

struct RCPlugin
{
  uint32_t capacity;
  uint32_t qpFifo[32];
  uint32_t head;
  uint32_t tail;
  uint32_t curQp;
};

static inline void RCPlugin_SetNextFrameQP(RCPlugin* rc)
{
  rc->qpFifo[rc->head] = rc->curQp;
  rc->head = (rc->head + 1) % rc->capacity;

  ++rc->curQp;

  if(rc->curQp > 51)
    rc->curQp = 30;
}

static inline void RCPlugin_SetNextFrameQP(AL_TEncSettings const* pSettings, AL_TAllocator* pDmaAllocator)
{
  if(pSettings->hRcPluginDmaContext == NULL)
    throw std::runtime_error("RC Context isn't allocated");

  auto rc = (RCPlugin*)AL_Allocator_GetVirtualAddr(pDmaAllocator, pSettings->hRcPluginDmaContext);

  if(rc == NULL)
    throw std::runtime_error("RC Context isn't correctly defined");

  RCPlugin_SetNextFrameQP(rc);
}

static inline void RCPlugin_Init(RCPlugin* rc)
{
  rc->head = 0;
  rc->tail = 0;
  rc->capacity = 32;
  rc->curQp = 30;

  for(uint32_t i = 0; i < rc->capacity; ++i)
    rc->qpFifo[i] = 0;
}

static inline void RCPlugin_Init(AL_TEncSettings* pSettings, AL_TEncChanParam* pChParam, AL_TAllocator* pDmaAllocator)
{
  pSettings->hRcPluginDmaContext = NULL;
  pChParam->pRcPluginDmaContext = 0;
  pChParam->zRcPluginDmaSize = 0;

  if(pChParam->tRCParam.eRCMode == AL_RC_PLUGIN)
  {
    pChParam->zRcPluginDmaSize = sizeof(struct RCPlugin);
    pSettings->hRcPluginDmaContext = AL_Allocator_Alloc(pDmaAllocator, pChParam->zRcPluginDmaSize);

    if(pSettings->hRcPluginDmaContext == NULL)
      throw std::runtime_error("Couldn't allocate RC Plugin Context");

    auto rc = (RCPlugin*)AL_Allocator_GetVirtualAddr(pDmaAllocator, pSettings->hRcPluginDmaContext);
    RCPlugin_Init(rc);
  }
}
