// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "getters.h"

#include "helpers.h"
#include <cassert>
#include <OMX_Component.h>
#include <OMX_ComponentExt.h>
#include <OMX_IndexExt.h>

Getters::Getters(OMX_HANDLETYPE* component) :
  component(component)
{
  assert(component);
}

int Getters::GetLatency()
{
  OMX_ALG_PARAM_REPORTED_LATENCY lat;
  InitHeader(lat);

  auto err = OMX_GetParameter(*component, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamReportedLatency), &lat);

  if(err != OMX_ErrorNone)
    return -1;

  return lat.nLatency;
}

int Getters::GetBuffersSize(int index)
{
  OMX_PARAM_PORTDEFINITIONTYPE p;
  InitHeader(p);
  p.nPortIndex = index;
  OMX_GetParameter(*component, OMX_IndexParamPortDefinition, &p);
  return p.nBufferSize;
}

int Getters::GetBuffersCount(int index)
{
  OMX_PARAM_PORTDEFINITIONTYPE p;
  InitHeader(p);
  p.nPortIndex = index;
  OMX_GetParameter(*component, OMX_IndexParamPortDefinition, &p);
  return p.nBufferCountActual;
}

static inline bool IsInputSupplier(OMX_DIRTYPE const& dir, OMX_BUFFERSUPPLIERTYPE const& sup)
{
  return (dir == OMX_DirInput) && (sup == OMX_BufferSupplyInput);
}

static inline bool IsOutputSupplier(OMX_DIRTYPE const& dir, OMX_BUFFERSUPPLIERTYPE const& sup)
{
  return (dir == OMX_DirOutput) && (sup == OMX_BufferSupplyOutput);
}

bool Getters::IsComponentSupplier(int index)
{
  OMX_PARAM_PORTDEFINITIONTYPE p;
  InitHeader(p);
  p.nPortIndex = index;
  OMX_GetParameter(*component, OMX_IndexParamPortDefinition, &p);
  auto dir = p.eDir;
  OMX_PARAM_BUFFERSUPPLIERTYPE s;
  InitHeader(s);
  s.nPortIndex = index;
  OMX_GetParameter(*component, OMX_IndexParamCompBufferSupplier, &s);

  if(IsInputSupplier(dir, s.eBufferSupplier) || IsOutputSupplier(dir, s.eBufferSupplier))
    return true;

  return false;
}

