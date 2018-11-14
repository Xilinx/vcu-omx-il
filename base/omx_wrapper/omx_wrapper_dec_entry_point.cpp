/******************************************************************************
*
* Copyright (C) 2018 Allegro DVT2.  All rights reserved.
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

#include "base/omx_component/omx_component_interface.h"
#include "base/omx_checker/omx_checker.h"
#include "omx_wrapper_common_entry_point.cpp"
#include "omx_wrapper_dec.h"

#include <OMX_Core.h>
#include <OMX_Component.h>

static OMX_ERRORTYPE WrapComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;

  pThis->ComponentDeInit();

  auto pComp = (OMX_COMPONENTTYPE*)hComponent;
  DestroyDecComponentPrivate(pComp->pComponentPrivate);

  return OMX_ErrorNone;
}

extern "C"
{
OMX_API OMX_ERRORTYPE CreateComponent(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_CALLBACKTYPE* pCallbacks)
{
  if(!hComponent)
    return OMX_ErrorBadParameter;

  OMX_PTR pThis = CreateDecComponentPrivate(hComponent, cComponentName, cRole);

  if(!pThis)
    return OMX_ErrorBadParameter;

  auto pComp = (OMX_COMPONENTTYPE*)hComponent;

  OMXChecker::SetHeaderVersion(*pComp);

  pComp->pComponentPrivate = pThis;
  pComp->pApplicationPrivate = pAppPrivate;

  pComp->SetCallbacks = WrapSetCallbacks;
  pComp->SendCommand = WrapSendCommand;
  pComp->GetParameter = WrapGetParameter;
  pComp->SetParameter = WrapSetParameter;
  pComp->GetState = WrapGetState;
  pComp->UseBuffer = WrapUseBuffer;
  pComp->AllocateBuffer = WrapAllocateBuffer;
  pComp->FreeBuffer = WrapFreeBuffer;
  pComp->EmptyThisBuffer = WrapEmptyThisBuffer;
  pComp->FillThisBuffer = WrapFillThisBuffer;
  pComp->ComponentDeInit = WrapComponentDeInit;
  pComp->GetComponentVersion = WrapGetComponentVersion;
  pComp->GetConfig = WrapGetConfig;
  pComp->SetConfig = WrapSetConfig;
  pComp->GetExtensionIndex = WrapGetExtensionIndex;
  pComp->ComponentTunnelRequest = WrapComponentTunnelRequest;
  pComp->UseEGLImage = WrapUseEGLImage;
  pComp->ComponentRoleEnum = WrapComponentRoleEnum;

  return pComp->SetCallbacks(pComp, pCallbacks, pAppPrivate);
}
}

