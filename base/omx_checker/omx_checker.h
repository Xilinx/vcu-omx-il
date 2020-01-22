/******************************************************************************
*
* Copyright (C) 2016-2020 Allegro DVT2.  All rights reserved.
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

