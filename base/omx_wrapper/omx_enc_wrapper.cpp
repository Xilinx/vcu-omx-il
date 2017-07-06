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
#include "base/omx_videocodec/omx_videocodec.h"
#include "base/omx_processtype/omx_enc_process.h"

#include "base/omx_codectype/omx_hevc_codec.h"
#include "base/omx_codectype/omx_avc_codec.h"

static ProcessType* CreateProcess(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName)
{
  if(!hComponent)
    return nullptr;

  if(!cComponentName)
    return nullptr;

  CodecType* pCodec = nullptr;

  if(!strncmp(cComponentName, "OMX.allegro.h265.encoder", strlen(cComponentName)))
    pCodec = new HEVCCodec();
  else if(!strncmp(cComponentName, "OMX.allegro.h264.encoder", strlen(cComponentName)))
    pCodec = new AVCCodec();
  else
    return nullptr;

  return new ProcessEncode(hComponent, pCodec);
}

static OMX_PTR GenerateProcess(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{
  auto pProcessType = CreateProcess(hComponent, cComponentName);

  if(!pProcessType)
    return nullptr;

  auto pThis = new OMXVideoCodec(hComponent, pProcessType, cComponentName, cRole);

  if(!pThis)
    delete pProcessType;

  auto w = new Wrapper;
  w->module = pProcessType;
  w->base = pThis;

  return w;
}

static void DestroyPrivate(Wrapper* w)
{
  if(!w)
    return;

  if(w->base)
    delete w->base;

  if(w->module)
    delete static_cast<ProcessType*>(w->module);

  delete w;
}

extern "C"
{
OMX_PTR CreateComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole)
{
  return GenerateProcess(hComponent, cComponentName, cRole);
}

void DestroyComponentPrivate(OMX_IN OMX_PTR pComponentPrivate)
{
  auto w = static_cast<Wrapper*>(pComponentPrivate);
  DestroyPrivate(w);
}
}

