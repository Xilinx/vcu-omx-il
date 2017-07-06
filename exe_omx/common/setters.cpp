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

#include "setters.h"
#include "helpers.h"

#include <assert.h>
#include <OMX_Component.h>

Setters::Setters(OMX_HANDLETYPE* component) : component(component)
{
  assert(component);
}

bool Setters::SetBoardMode(OMX_HWMODE const mode)
{
  switch(mode)
  {
  case OMX_HW_UNUSED: break;
  case OMX_HW_MCU: break;
  default:
    return false;
  }

  OMX_INDEXTYPE type = OMX_IndexComponentStartUnused;
  auto err = OMX_GetExtensionIndex(*component, (OMX_STRING)"OMX.allegro.board", &type);

  if(err != OMX_ErrorNone)
    return false;

  OMX_PARAM_BOARD param;
  initHeader(param);
  param.eMode = mode;

  err = OMX_SetParameter(*component, type, &param);

  if(err != OMX_ErrorNone)
    return false;

  return true;
}

bool Setters::SetBufferMode(OMX_U32 const port, OMX_BUFFERMODE const mode)
{
  switch(mode)
  {
  case OMX_BUF_NORMAL: break;
  case OMX_BUF_DMA: break;
  default:
    return false;
  }

  OMX_INDEXTYPE type = OMX_IndexComponentStartUnused;
  auto err = OMX_GetExtensionIndex(*component, (OMX_STRING)"OMX.allegro.bufferMode", &type);

  if(err != OMX_ErrorNone)
    return false;

  OMX_PORT_PARAM_BUFFERMODE param;
  initHeader(param);
  param.nPortIndex = port;
  param.eMode = mode;

  err = OMX_SetParameter(*component, type, &param);

  if(err != OMX_ErrorNone)
    return false;

  return true;
}

