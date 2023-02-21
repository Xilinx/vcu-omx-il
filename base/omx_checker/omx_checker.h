// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <OMX_Types.h>
#include <OMX_Core.h>
#include <cstring>

/*****************************************************************************/
struct OMXChecker
{
  enum class ComponentMethods
  {
    GetComponentVersion,
    SendCommand,
    GetParameter,
    SetParameter,
    GetConfig,
    SetConfig,
    GetExtensionIndex,
    GetState,
    ComponentTunnelRequest,
    UseBuffer,
    AllocateBuffer,
    FreeBuffer,
    EmptyThisBuffer,
    FillThisBuffer,
    SetCallbacks,
    ComponentDeinit,
    UseEGLImage,
    ComponentRoleEnum,
  };

  /*************************************************************************//*!
     \brief  The CheckStateOperation method test if the OMX_Method can be called in the
            current state
     \param[in]  methodName Name of the OMX_Method
     \param[in]  curState Current component's state
     \throw  OMX_ErrorInvalidState if the current state is invalid
             OMX_ErrorIncorrectStateOperation otherwise
  *****************************************************************************/
  static void CheckStateOperation(ComponentMethods methodName, OMX_STATETYPE curState);

  /*************************************************************************//*!
     \brief  The CheckStateExistance method test if the state exist
     \param[in]  state
     \throw  OMX_ErrorBadParameter if the state doesn't exist
  *****************************************************************************/
  static void CheckStateExistance(OMX_STATETYPE state);

  /*************************************************************************//*!
     \brief  The CheckStateTransition method test if the transition between old state
            and new state can be done
     \param[in]  curState Current component's state
     \param[in]  newState New component's state
     \throw OMX_ErrorSameState if the new state equal the current state
            OMX_ErrorIncorrectStateTransition otherwise
  *****************************************************************************/
  static void CheckStateTransition(OMX_STATETYPE curState, OMX_STATETYPE newState);

  /*************************************************************************//*!
     \brief  The CheckNotNull method test if the parameter send is not null
     \param[in]  ptr Pointer
     \throw OMX_ErrorBadParameter if ptr is null
  *****************************************************************************/
  template<typename T>
  static
  inline
  void CheckNotNull(T const ptr)
  {
    if(!ptr)
      throw OMX_ErrorBadParameter;
  }

  /*************************************************************************//*!
     \brief  The CheckNull method test if the parameter send is null
     \param[in]  ptr Pointer
     \throw OMX_ErrorBadParameter if ptr is not null
  *****************************************************************************/
  template<typename T>
  static
  inline
  void CheckNull(T const ptr)
  {
    if(ptr)
      throw OMX_ErrorBadParameter;
  }

  /*************************************************************************//*!
     \brief  The SetHeaderVersion method set the omx-il version header used. It also set to 0 everything else.
     \param[in]  header
  *****************************************************************************/
  template<typename T>
  static
  inline
  void SetHeaderVersion(T& header)
  {
    memset(&header, 0x0, sizeof(T));
    header.nSize = sizeof(header);
    header.nVersion.nVersion = OMX_VERSION;
  }

  /*************************************************************************//*!
     \brief  The CheckHeader method check if the header is at the omx-il version used
     \param[in]  version
     \throw OMX_ErrorVersionMismatch if version != OMX_VERSION
  *****************************************************************************/
  static
  inline
  void CheckHeaderVersion(OMX_VERSIONTYPE version)
  {
    if(version.nVersion != OMX_VERSION)
      throw OMX_ErrorVersionMismatch;
  };
};

