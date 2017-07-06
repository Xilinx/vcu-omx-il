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
#include "base/omx_checker/omx_checker.h"

static OMXBase* GetThis(OMX_IN OMX_HANDLETYPE hComponent)
{
  if(!hComponent)
    return nullptr;

  auto c = (Wrapper*)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

  if(!c)
    return nullptr;

  if(!c->base)
    return nullptr;

  return c->base;
}

OMX_ERRORTYPE MySendCommand(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_COMMANDTYPE Cmd, OMX_IN OMX_U32 nParam1, OMX_IN OMX_PTR pCmdData)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;

  return pThis->SendCommand(Cmd, nParam1, pCmdData);
}

OMX_ERRORTYPE MyGetState(OMX_IN OMX_HANDLETYPE hComponent, OMX_OUT OMX_STATETYPE* pState)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->GetState(pState);
}

OMX_ERRORTYPE MySetCallbacks(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_CALLBACKTYPE* pCallbacks, OMX_IN OMX_PTR pAppData)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->SetCallbacks(pCallbacks, pAppData);
}

OMX_ERRORTYPE MyGetParameter(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_INDEXTYPE nParamIndex, OMX_INOUT OMX_PTR pParam)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->GetParameter(nParamIndex, pParam);
}

OMX_ERRORTYPE MySetParameter(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_INDEXTYPE nParamIndex, OMX_IN OMX_PTR pParam)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->SetParameter(nParamIndex, pParam);
}

OMX_ERRORTYPE MyUseBuffer(OMX_IN OMX_HANDLETYPE hComponent, OMX_OUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes, OMX_IN OMX_U8* pBuffer)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->UseBuffer(ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
}

OMX_ERRORTYPE MyAllocateBuffer(OMX_IN OMX_HANDLETYPE hComponent, OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->AllocateBuffer(ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes);
}

OMX_ERRORTYPE MyFreeBuffer(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->FreeBuffer(nPortIndex, pBufferHdr);
}

OMX_ERRORTYPE MyEmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->EmptyThisBuffer(pBufferHdr);
}

OMX_ERRORTYPE MyFillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->FillThisBuffer(pBufferHdr);
}

extern "C"
{
extern void DestroyComponentPrivate(OMX_IN OMX_PTR pComponent);
}
OMX_ERRORTYPE MyComponentDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;

  pThis->ComponentDeInit();

  auto pComp = (OMX_COMPONENTTYPE*)hComponent;
  DestroyComponentPrivate(pComp->pComponentPrivate);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE MyGetComponentVersion(OMX_IN OMX_HANDLETYPE hComponent, OMX_OUT OMX_STRING pComponentName, OMX_OUT OMX_VERSIONTYPE* pComponentVersion, OMX_OUT OMX_VERSIONTYPE* pSpecVersion, OMX_OUT OMX_UUIDTYPE[128] /*pComponentUUID */)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->GetComponentVersion(pComponentName, pComponentVersion, pSpecVersion);
}

OMX_ERRORTYPE MyGetConfig(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_INDEXTYPE nIndex, OMX_INOUT OMX_PTR pComponentConfigStructure)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->GetConfig(nIndex, pComponentConfigStructure);
}

OMX_ERRORTYPE MySetConfig(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_INDEXTYPE nIndex, OMX_IN OMX_PTR pComponentConfigStructure)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->SetConfig(nIndex, pComponentConfigStructure);
}

OMX_ERRORTYPE MyGetExtensionIndex(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cParameterName, OMX_OUT OMX_INDEXTYPE* pIndexType)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->GetExtensionIndex(cParameterName, pIndexType);
}

OMX_ERRORTYPE MyComponentTunnelRequest(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_U32 nPort, OMX_IN OMX_HANDLETYPE hTunneledComp, OMX_IN OMX_U32 nTunneledPort, OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->ComponentTunnelRequest(nPort, hTunneledComp, nTunneledPort, pTunnelSetup);
}

OMX_ERRORTYPE MyUseEGLImage(OMX_IN OMX_HANDLETYPE hComponent, OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN void* eglImage)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->UseEGLImage(ppBufferHdr, nPortIndex, pAppPrivate, eglImage);
}

OMX_ERRORTYPE MyComponentRoleEnum(OMX_IN OMX_HANDLETYPE hComponent, OMX_OUT OMX_U8* cRole, OMX_IN OMX_U32 nIndex)
{
  auto pThis = GetThis(hComponent);

  if(!pThis)
    return OMX_ErrorBadParameter;
  return pThis->ComponentRoleEnum(cRole, nIndex);
}

extern "C"
{
extern OMX_PTR CreateComponentPrivate(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole);
OMX_API OMX_ERRORTYPE CreateComponent(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_CALLBACKTYPE* pCallbacks)
{
  if(!hComponent)
    return OMX_ErrorBadParameter;

  OMX_PTR pThis = CreateComponentPrivate(hComponent, cComponentName, cRole);

  if(!pThis)
    return OMX_ErrorBadParameter;

  auto pComp = (OMX_COMPONENTTYPE*)hComponent;

  OMXChecker::SetHeaderVersion(*pComp);

  pComp->pComponentPrivate = pThis;
  pComp->pApplicationPrivate = pAppPrivate;

  pComp->SetCallbacks = MySetCallbacks;
  pComp->SendCommand = MySendCommand;
  pComp->GetParameter = MyGetParameter;
  pComp->SetParameter = MySetParameter;
  pComp->GetState = MyGetState;
  pComp->UseBuffer = MyUseBuffer;
  pComp->AllocateBuffer = MyAllocateBuffer;
  pComp->FreeBuffer = MyFreeBuffer;
  pComp->EmptyThisBuffer = MyEmptyThisBuffer;
  pComp->FillThisBuffer = MyFillThisBuffer;
  pComp->ComponentDeInit = MyComponentDeInit;
  pComp->GetComponentVersion = MyGetComponentVersion;
  pComp->GetConfig = MyGetConfig;
  pComp->SetConfig = MySetConfig;
  pComp->GetExtensionIndex = MyGetExtensionIndex;
  pComp->ComponentTunnelRequest = MyComponentTunnelRequest;
  pComp->UseEGLImage = MyUseEGLImage;
  pComp->ComponentRoleEnum = MyComponentRoleEnum;

  return MySetCallbacks(pComp, pCallbacks, pAppPrivate);
}
}

