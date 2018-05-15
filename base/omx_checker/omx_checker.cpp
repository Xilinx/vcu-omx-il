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

#include "omx_checker.h"

static inline bool isStateLoaded(OMX_STATETYPE const curState)
{
  return curState == OMX_StateLoaded;
}

static inline bool isStateIdle(OMX_STATETYPE const curState)
{
  return curState == OMX_StateIdle;
}

static inline bool isStateExecuting(OMX_STATETYPE const curState)
{
  return curState == OMX_StateExecuting;
}

static inline bool isStatePause(OMX_STATETYPE const curState)
{
  return curState == OMX_StatePause;
}

static inline bool isStateWaitForResources(OMX_STATETYPE const curState)
{
  return curState == OMX_StateWaitForResources;
}

static inline bool isStateInvalid(OMX_STATETYPE const curState)
{
  return curState == OMX_StateInvalid;
}

void OMXChecker::CheckStateOperation(AL_ComponentMethods const methodName, OMX_STATETYPE const curState)
{
  switch(methodName)
  {
  case AL_GetComponentVersion:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_SendCommand:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_GetParameter:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_SetParameter:
  {
    if(isStateIdle(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateExecuting(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStatePause(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_GetConfig:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_SetConfig:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_GetExtensionIndex:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_GetState:
  {
    break;
  }
  case AL_ComponentTunnelRequest:
  {
    if(isStateIdle(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateExecuting(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStatePause(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateWaitForResources(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_UseBuffer:
  {
    // if(isStateIdle(curState))
    // throw OMX_ErrorIncorrectStateOperation;

    // if(isStateExecuting(curState))
    // throw OMX_ErrorIncorrectStateOperation;

    if(isStatePause(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_AllocateBuffer:
  {
    // if(isStateIdle(curState))
    // throw OMX_ErrorIncorrectStateOperation;

    // if(isStateExecuting(curState))
    // throw OMX_ErrorIncorrectStateOperation;

    if(isStatePause(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_FreeBuffer:
  {
    break;
  }
  case AL_EmptyThisBuffer:
  {
    if(isStateLoaded(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateWaitForResources(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_FillThisBuffer:
  {
    if(isStateLoaded(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateWaitForResources(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_SetCallbacks:
  {
    if(isStateIdle(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateExecuting(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStatePause(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateWaitForResources(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case AL_ComponentDeinit:
  {
    break;
  }
  case AL_UseEGLImage:
  {
    break;
  }
  case AL_ComponentRoleEnum:
  {
    break;
  }
  default:
    throw OMX_ErrorUndefined;
  }
}

void OMXChecker::CheckStateTransition(OMX_STATETYPE const curState, OMX_STATETYPE const newState)
{
    if(isStateInvalid(newState))
      throw OMX_ErrorInvalidState;

  if(newState == curState)
    throw OMX_ErrorSameState;

  switch(curState)
  {
  case OMX_StateLoaded:
  {
    if(isStateExecuting(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStatePause(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStateInvalid(newState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case OMX_StateWaitForResources:
  {
    if(isStateExecuting(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStatePause(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStateInvalid(newState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case OMX_StateIdle:
  {
    if(isStateWaitForResources(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStateInvalid(newState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case OMX_StateExecuting:
  {
    if(isStateWaitForResources(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStateLoaded(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStateInvalid(newState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case OMX_StatePause:
  {
    if(isStateWaitForResources(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStateLoaded(newState))
      throw OMX_ErrorIncorrectStateTransition;

    if(isStateInvalid(newState))
      throw OMX_ErrorInvalidState;
    break;
  }
  default:
    throw OMX_ErrorUndefined;
  }
}

void OMXChecker::CheckStateExistance(const OMX_STATETYPE state)
{
  switch(state)
  {
  case OMX_StateLoaded:
  case OMX_StateIdle:
  case OMX_StateWaitForResources:
  case OMX_StateExecuting:
  case OMX_StatePause:
  case OMX_StateInvalid:
    return;
  default:
    throw OMX_ErrorBadParameter;
  }
}

