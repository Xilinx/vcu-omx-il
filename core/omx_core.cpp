// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include <cstdlib>
#include <cstring>
#include <string>
#include <dlfcn.h>
#include <algorithm>
#include <type_traits>
#include <utility/logger.h>
#include <utility/omx_translate.h>

#include "omx_core.h"
#include <OMX_Component.h>
#include <stdexcept>

using namespace std;

static int constexpr OMX_MAX_COMP_ROLES = 1;
static int constexpr NB_OF_COMP = sizeof(AL_COMP_LIST) / sizeof(omx_comp_type);

static omx_comp_type const* getComp(char* cComponentName)
{
  if(!cComponentName)
    return nullptr;

  for(int i = 0; i < NB_OF_COMP; i++)
  {
    if(strncmp(cComponentName, AL_COMP_LIST[i].name, strlen(AL_COMP_LIST[i].name)) == 0)
      return &AL_COMP_LIST[i];
  }

  return nullptr;
}

OMX_ERRORTYPE OMX_APIENTRY OMX_Init(void)
{
  LOG_IMPORTANT();
  string env("/usr/lib");

  if(getenv("OMX_ALLEGRO_PATH"))
    env = getenv("OMX_ALLEGRO_PATH");

  string path = env + "/";

#if defined(ANDROID) || defined(__ANDROID_API__)
  path = "/system/lib/";
#endif /* ANDROID */

  int libraryLoaded = 0;

  for(int i = 0; i < NB_OF_COMP; i++)
  {
    string cCodecName = path + AL_COMP_LIST[i].pSoLibName + "." + to_string(OMX_VERSION_MAJOR);
    AL_COMP_LIST[i].pLibHandle = ::dlopen(cCodecName.c_str(), RTLD_LAZY);

    if(AL_COMP_LIST[i].pLibHandle == nullptr)
      LOG_ERROR(dlerror());
    else
      libraryLoaded++;
  }

  if(libraryLoaded == 0)
  {
    LOG_ERROR("No library found! Did you set OMX_ALLEGRO_PATH?");
    return OMX_ErrorUndefined;
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_APIENTRY OMX_Deinit(void)
{
  LOG_IMPORTANT();

  for(int i = 0; i < NB_OF_COMP; i++)
  {
    if(AL_COMP_LIST[i].pLibHandle == nullptr)
      continue;

    if(::dlclose(AL_COMP_LIST[i].pLibHandle))
      LOG_ERROR(dlerror());
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_APIENTRY OMX_ComponentNameEnum(OMX_OUT OMX_STRING cComponentName, OMX_IN OMX_U32 nNameLength, OMX_IN OMX_U32 nIndex)
{
  LOG_IMPORTANT(string { "cComponentName: " } +ToStringAddr(cComponentName) + string { ", nNameLength: " } +to_string(nNameLength) + string { ", nIndex: " } +to_string(nIndex));

  if(!cComponentName)
    return OMX_ErrorBadParameter;

  if(nIndex >= NB_OF_COMP)
    return OMX_ErrorNoMore;

  strncpy(cComponentName, AL_COMP_LIST[nIndex].name, nNameLength);

  return OMX_ErrorNone;
}

static OMX_HANDLETYPE CreateComponent(const omx_comp_type* pComponent, char const* cFunctionName, OMX_PTR pAppData, OMX_CALLBACKTYPE* pCallBacks, OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_PTR pSettings)
{
  dlerror();

  using CreateComponentFuncPtr = add_pointer<OMX_ERRORTYPE(OMX_IN OMX_HANDLETYPE, OMX_IN OMX_STRING, OMX_IN OMX_STRING, OMX_IN OMX_PTR, OMX_IN OMX_CALLBACKTYPE*, OMX_IN OMX_ALG_COREINDEXTYPE, OMX_IN OMX_PTR)>::type;
  auto createFunction = reinterpret_cast<CreateComponentFuncPtr>(reinterpret_cast<uintptr_t>(::dlsym(pComponent->pLibHandle, cFunctionName)));
  auto pErr = dlerror();

  if(pErr)
  {
    LOG_ERROR(pErr);
    return nullptr;
  }

  auto pMyComponent = new OMX_COMPONENTTYPE;
  auto eRet = createFunction(pMyComponent, (OMX_STRING)pComponent->name, (OMX_STRING)pComponent->role, pAppData, pCallBacks, nCoreParamIndex, pSettings);

  if(eRet != OMX_ErrorNone)
  {
    delete pMyComponent;
    return nullptr;
  }

  return pMyComponent;
}

OMX_ERRORTYPE OMX_APIENTRY OMX_GetHandle(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_PTR pAppData, OMX_IN OMX_CALLBACKTYPE* pCallBacks)
{
  return OMX_ALG_GetHandle(pHandle, cComponentName, pAppData, pCallBacks, OMX_ALG_CoreIndexUnused, NULL);
}

OMX_ERRORTYPE OMX_APIENTRY OMX_ALG_GetHandle(OMX_OUT OMX_HANDLETYPE* pHandle, OMX_IN OMX_STRING cComponentName, OMX_IN OMX_PTR pAppData, OMX_IN OMX_CALLBACKTYPE* pCallBacks, OMX_IN OMX_ALG_COREINDEXTYPE nCoreParamIndex, OMX_IN OMX_PTR pSettings)
{
  LOG_IMPORTANT(
    string("pHandle: ") + ToStringAddr(pHandle) +
    string(", cComponentName: ") + cComponentName +
    string(", pAppData: ") + ToStringAddr(pAppData) +
    string(", pCallBacks: ") + ToStringAddr(pCallBacks) +
    string(", nCoreParamIndex: ") + ToStringOMXALGCoreIndex(nCoreParamIndex) +
    string(", pSettings: ") + ToStringAddr(pSettings));

  if(!pHandle)
    return OMX_ErrorBadParameter;

  auto pComponent = getComp(cComponentName);

  if(!pComponent)
    return OMX_ErrorComponentNotFound;

  try
  {
    *pHandle = CreateComponent(pComponent, "CreateComponent", pAppData, pCallBacks, nCoreParamIndex, pSettings);
  }
  catch(runtime_error const& e)
  {
    LOG_ERROR(e.what());
    return OMX_ErrorUndefined;
  }

  return *pHandle ? OMX_ErrorNone : OMX_ErrorUndefined;
}

OMX_ERRORTYPE OMX_APIENTRY OMX_FreeHandle(OMX_IN OMX_HANDLETYPE hComponent)
{
  LOG_IMPORTANT(string { "hComponent: " } +ToStringAddr(hComponent));
  auto pMyComponent = static_cast<OMX_COMPONENTTYPE*>(hComponent);
  auto eRet = pMyComponent->ComponentDeInit(hComponent);

  delete pMyComponent;

  return eRet;
}

OMX_ERRORTYPE OMX_GetComponentsOfRole(OMX_IN OMX_STRING role, OMX_INOUT OMX_U32* pNumComps, OMX_INOUT OMX_U8** compNames)
{
  LOG_IMPORTANT(string { "role: " } +role + string { ", pNumComps: " } +ToStringAddr(pNumComps) + string { ", compNames: " } +ToStringAddr(compNames));

  if((!role) || (!pNumComps))
    return OMX_ErrorBadParameter;

  if(!compNames)
  {
    *pNumComps = 0;

    for(int i = 0; i < NB_OF_COMP; i++)
    {
      if(strncmp(AL_COMP_LIST[i].role, role, strlen(role)) == 0)
        (*pNumComps)++;
    }

    return OMX_ErrorNone;
  }

  if(*pNumComps == 0)
    return OMX_ErrorBadParameter;

  int compNamesIndex = 0;

  for(int i = 0; i < NB_OF_COMP; i++)
  {
    if(strncmp(AL_COMP_LIST[i].role, role, strlen(role)) == 0)
    {
      strncpy((char*)compNames[compNamesIndex++], AL_COMP_LIST[i].name, OMX_MAX_STRINGNAME_SIZE);

      if(static_cast<OMX_U32>(compNamesIndex) >= *pNumComps)
        return OMX_ErrorNone;
    }
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_GetRolesOfComponent(OMX_IN OMX_STRING compName, OMX_INOUT OMX_U32* pNumRoles, OMX_OUT OMX_U8** roles)
{
  LOG_IMPORTANT(string { "compName: " } +compName + string { ", pNumRoles: " } +ToStringAddr(pNumRoles) + string { ", roles: " } +ToStringAddr(roles));
  auto pComponent = getComp(compName);

  if(!pComponent)
    return OMX_ErrorComponentNotFound;

  if(!roles && !pNumRoles)
    return OMX_ErrorBadParameter;

  if(roles && !pNumRoles)
    return OMX_ErrorBadParameter;

  if(roles && pNumRoles && (*pNumRoles == 0))
    return OMX_ErrorBadParameter;

  if(!roles && pNumRoles)
  {
    *pNumRoles = static_cast<OMX_U32>(OMX_MAX_COMP_ROLES);
    return OMX_ErrorNone;
  }

  auto minNumRoles = min(*pNumRoles, static_cast<OMX_U32>(OMX_MAX_COMP_ROLES));
  *pNumRoles = minNumRoles;

  for(OMX_U32 i = 0; i < minNumRoles; i++)
    strncpy((char*)roles[i], (char*)pComponent->role, OMX_MAX_STRINGNAME_SIZE);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_APIENTRY OMX_SetupTunnel(OMX_IN OMX_HANDLETYPE hOutput, OMX_IN OMX_U32 nPortOutput, OMX_IN OMX_HANDLETYPE hInput, OMX_IN OMX_U32 nPortInput)
{
  LOG_IMPORTANT(string { "hOutput: " } +ToStringAddr(hOutput) + string { ", nPortOutput: " } +to_string(nPortOutput) + string { ", hInput: " } +ToStringAddr(hInput) + string { ", nPortInput: " } +to_string(nPortInput));
  auto pMyComponent = static_cast<OMX_COMPONENTTYPE*>(hOutput);

  if(!pMyComponent)
    return OMX_ErrorInvalidComponent;

  return pMyComponent->ComponentTunnelRequest(hOutput, nPortOutput, hInput, nPortInput, nullptr);
}

OMX_ERRORTYPE OMX_GetContentPipe(OMX_OUT OMX_HANDLETYPE* hPipe, OMX_IN OMX_STRING szURI)
{
  LOG_IMPORTANT(string { "hPipe: " } +ToStringAddr(hPipe) + string { ", szURI: " } +szURI);
  return OMX_ErrorNotImplemented;
}

