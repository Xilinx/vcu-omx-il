// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include <utility/logger.h>
#include "base/omx_component/omx_component_interface.h"
#include "base/omx_checker/omx_checker.h"
#include "omx_wrapper_codec_entry_point.cpp"
#include "omx_wrapper_dec.h"

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
  DestroyDecComponentPrivate(pComp->pComponentPrivate);

  return OMX_ErrorNone;
}

extern "C"
{
OMX_API OMX_ERRORTYPE CreateComponent(OMX_IN OMX_HANDLETYPE hComponent, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_STRING cRole, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_CALLBACKTYPE* pCallbacks, OMX_IN OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_IN OMX_PTR pSettings)
{
  if(!hComponent)
    return OMX_ErrorBadParameter;

  OMX_PTR pThis = CreateDecComponentPrivate(hComponent, cComponentName, cRole, nCoreParamIndex, pSettings);

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

