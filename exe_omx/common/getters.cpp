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

