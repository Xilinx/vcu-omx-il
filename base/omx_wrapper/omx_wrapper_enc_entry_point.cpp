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

#include <utility/logger.h>
#include "base/omx_component/omx_component_interface.h"
#include "base/omx_checker/omx_checker.h"
#include "omx_wrapper_codec_entry_point.cpp"
#include "omx_wrapper_enc.h"

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <string>

using namespace std;

static OMX_ERRORTYPE ComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
  LOG_IMPORTANT(string { "hComponent: " } +ToStringAddr(hComponent));
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;

  pThis->ComponentDeInit();

  auto pComp = static_cast<OMX_COMPONENTTYPE*>(hComponent);
  DestroyEncComponentPrivate(pComp->pComponentPrivate);

  return OMX_ErrorNone;
}

extern "C"
{
OMX_API OMX_ERRORTYPE CreateComponent(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_CALLBACKTYPE* pCallbacks)
{
  if(!hComponent)
    return OMX_ErrorBadParameter;

  OMX_PTR pThis = CreateEncComponentPrivate(hComponent, cComponentName, cRole);

  if(!pThis)
    return OMX_ErrorBadParameter;

  auto pComp = (OMX_COMPONENTTYPE*)hComponent;

  OMXChecker::SetHeaderVersion(*pComp);

  pComp->pComponentPrivate = pThis;
  pComp->pApplicationPrivate = pAppPrivate;

  pComp->SetCallbacks = SetCallbacks;
  pComp->SendCommand = SendCommand;
  pComp->GetParameter = GetParameter;
  pComp->SetParameter = SetParameter;
  pComp->GetState = GetState;
  pComp->UseBuffer = UseBuffer;
  pComp->AllocateBuffer = AllocateBuffer;
  pComp->FreeBuffer = FreeBuffer;
  pComp->EmptyThisBuffer = EmptyThisBuffer;
  pComp->FillThisBuffer = FillThisBuffer;
  pComp->ComponentDeInit = ComponentDeInit;
  pComp->GetComponentVersion = GetComponentVersion;
  pComp->GetConfig = GetConfig;
  pComp->SetConfig = SetConfig;
  pComp->GetExtensionIndex = GetExtensionIndex;
  pComp->ComponentTunnelRequest = ComponentTunnelRequest;
  pComp->UseEGLImage = UseEGLImage;
  pComp->ComponentRoleEnum = ComponentRoleEnum;

  return pComp->SetCallbacks(pComp, pCallbacks, pAppPrivate);
}
}

