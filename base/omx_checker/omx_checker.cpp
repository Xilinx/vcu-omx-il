/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#include "omx_checker.h"

static inline bool isStateLoaded(OMX_STATETYPE curState)
{
  return curState == OMX_StateLoaded;
}

static inline bool isStateIdle(OMX_STATETYPE curState)
{
  return curState == OMX_StateIdle;
}

static inline bool isStateExecuting(OMX_STATETYPE curState)
{
  return curState == OMX_StateExecuting;
}

static inline bool isStatePause(OMX_STATETYPE curState)
{
  return curState == OMX_StatePause;
}

static inline bool isStateWaitForResources(OMX_STATETYPE curState)
{
  return curState == OMX_StateWaitForResources;
}

static inline bool isStateInvalid(OMX_STATETYPE curState)
{
  return curState == OMX_StateInvalid;
}

void OMXChecker::CheckStateOperation(ComponentMethods methodName, OMX_STATETYPE curState)
{
  switch(methodName)
  {
  case ComponentMethods::GetComponentVersion:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case ComponentMethods::SendCommand:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case ComponentMethods::GetParameter:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case ComponentMethods::SetParameter:
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
  case ComponentMethods::GetConfig:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case ComponentMethods::SetConfig:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case ComponentMethods::GetExtensionIndex:
  {
    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case ComponentMethods::GetState:
  {
    break;
  }
  case ComponentMethods::ComponentTunnelRequest:
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
  case ComponentMethods::UseBuffer:
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
  case ComponentMethods::AllocateBuffer:
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
  case ComponentMethods::FreeBuffer:
  {
    break;
  }
  case ComponentMethods::EmptyThisBuffer:
  {
    if(isStateLoaded(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateWaitForResources(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case ComponentMethods::FillThisBuffer:
  {
    if(isStateLoaded(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateWaitForResources(curState))
      throw OMX_ErrorIncorrectStateOperation;

    if(isStateInvalid(curState))
      throw OMX_ErrorInvalidState;
    break;
  }
  case ComponentMethods::SetCallbacks:
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
  case ComponentMethods::ComponentDeinit:
  {
    break;
  }
  case ComponentMethods::UseEGLImage:
  {
    break;
  }
  case ComponentMethods::ComponentRoleEnum:
  {
    break;
  }
  default:
    throw OMX_ErrorUndefined;
  }
}

void OMXChecker::CheckStateTransition(OMX_STATETYPE curState, OMX_STATETYPE newState)
{
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

void OMXChecker::CheckStateExistance(OMX_STATETYPE state)
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

