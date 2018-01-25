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

#include "getters.h"
#include "helpers.h"

#include <assert.h>
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
  initHeader(lat);

  auto err = OMX_GetParameter(*component, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamReportedLatency), &lat);

  if(err != OMX_ErrorNone)
    return -1;

  return lat.nLatency;
}

int Getters::GetBuffersSize(int const& index)
{
  OMX_PARAM_PORTDEFINITIONTYPE p;
  initHeader(p);
  p.nPortIndex = index;
  OMX_GetParameter(*component, OMX_IndexParamPortDefinition, &p);
  return p.nBufferSize;
}

int Getters::GetBuffersCount(int const& index)
{
  OMX_PARAM_PORTDEFINITIONTYPE p;
  initHeader(p);
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

bool Getters::IsComponentSupplier(int const& index)
{
  OMX_PARAM_PORTDEFINITIONTYPE p;
  initHeader(p);
  p.nPortIndex = index;
  OMX_GetParameter(*component, OMX_IndexParamPortDefinition, &p);
  auto dir = p.eDir;
  OMX_PARAM_BUFFERSUPPLIERTYPE s;
  initHeader(s);
  s.nPortIndex = index;
  OMX_GetParameter(*component, OMX_IndexParamCompBufferSupplier, &s);

  if(IsInputSupplier(dir, s.eBufferSupplier) || IsOutputSupplier(dir, s.eBufferSupplier))
    return true;

  return false;
}

