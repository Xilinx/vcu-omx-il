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

#include "omx_component_dec.h"

#include <OMX_VideoExt.h>
#include <OMX_ComponentAlg.h>
#include <OMX_IVCommonAlg.h>

#include <cmath>

#include "base/omx_checker/omx_checker.h"
#include "base/omx_utils/omx_log.h"
#include "base/omx_utils/omx_translate.h"

#include "omx_component_getset.h"

using namespace std;

static DecModule& ToDecModule(ModuleInterface& module)
{
  return dynamic_cast<DecModule &>(module);
}

DecComponent::DecComponent(OMX_HANDLETYPE component, shared_ptr<MediatypeInterface> media, std::unique_ptr<DecModule>&& module, OMX_STRING name, OMX_STRING role, std::unique_ptr<Expertise>&& expertise) :
  Component(component, media, std::move(module), std::move(expertise), name, role)
{
}

DecComponent::~DecComponent() = default;

void DecComponent::EmptyThisBufferCallBack(BufferHandleInterface* handle)
{
  auto header = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)handle)->header;
  delete handle;
  ClearPropagatedData(header);

  if(callbacks.EmptyBufferDone)
    callbacks.EmptyBufferDone(component, app, header);
}

void DecComponent::AssociateCallBack(BufferHandleInterface*, BufferHandleInterface* fill)
{
  std::lock_guard<std::mutex> lock(mutex);

  if(transmit.empty())
    return;

  auto emptyHeader = transmit.front();
  auto fillHeader = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)fill)->header;
  assert(fillHeader);
  fillHeader->hMarkTargetComponent = emptyHeader.hMarkTargetComponent;
  fillHeader->pMarkData = emptyHeader.pMarkData;
  fillHeader->nTimeStamp = emptyHeader.nTimeStamp;
  transmit.pop_front();

  if(IsEOSDetected(emptyHeader.nFlags))
  {
    callbacks.EventHandler(component, app, OMX_EventBufferFlag, output.index, emptyHeader.nFlags, nullptr);
    transmit.clear();
  }

  if(IsCompMarked(emptyHeader.hMarkTargetComponent, component))
    callbacks.EventHandler(component, app, OMX_EventMark, 0, 0, emptyHeader.pMarkData);
}

void DecComponent::FillThisBufferCallBack(BufferHandleInterface* filled, int offset, int size)
{
  assert(filled);
  auto header = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)filled)->header;
  delete filled;

  header->nOffset = offset;
  header->nFilledLen = size;
  switch(ToDecModule(*module).GetDisplayPictureType())
  {
  case 0:
  {
    // Do nothing : PROGRESSIVE
    break;
  }
  case 1:
  {
    header->nFlags |= OMX_ALG_BUFFERFLAG_TOP_FIELD;
    break;
  }
  case 2:
  {
    header->nFlags |= OMX_ALG_BUFFERFLAG_BOT_FIELD;
    break;
  }
  default: break;
  }

  if(offset == 0 && size == 0)
    header->nFlags = OMX_BUFFERFLAG_EOS;

  if(callbacks.FillBufferDone)
    callbacks.FillBufferDone(component, app, header);
}

void DecComponent::EventCallBack(CallbackEventType type, void* data)
{
  assert(type <= CALLBACK_EVENT_MAX);
  switch(type)
  {
  case CALLBACK_EVENT_RESOLUTION_CHANGE:
  {
    LOGI("%s", ToStringCallbackEvent.at(type));

    auto port = GetPort(1);

    if(port->isTransientToEnable || !port->enable)
      callbacks.EventHandler(component, app, OMX_EventPortSettingsChanged, 1, 0, nullptr);
    break;
  }
  default:
    Component::EventCallBack(type, data);
    break;
  }
}

OMX_ERRORTYPE DecComponent::GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(name);
  OMXChecker::CheckNotNull(index);
  OMXChecker::CheckStateOperation(AL_GetExtensionIndex, state);
  return OMX_ErrorNoMore;
  OMX_CATCH();
}

static OMX_BUFFERHEADERTYPE* AllocateHeader(OMX_PTR app, int size, OMX_U8* buffer, bool isBufferAllocatedByModule, int index)
{
  auto header = new OMX_BUFFERHEADERTYPE;
  OMXChecker::SetHeaderVersion(*header);
  header->pBuffer = buffer;
  header->nAllocLen = size;
  header->pAppPrivate = app;
  header->pInputPortPrivate = new bool(isBufferAllocatedByModule);
  header->pOutputPortPrivate = new bool(isBufferAllocatedByModule);
  auto& p = IsInputPort(index) ? header->nInputPortIndex : header->nOutputPortIndex;
  p = index;

  return header;
}

static inline bool isBufferAllocatedByModule(OMX_BUFFERHEADERTYPE const* header)
{
  if(!header->pInputPortPrivate || !header->pOutputPortPrivate)
    return false;

  auto isInputAllocated = *(static_cast<bool*>(header->pInputPortPrivate));
  auto isOutputAllocated = *(static_cast<bool*>(header->pOutputPortPrivate));

  return isInputAllocated || isOutputAllocated;
}

static void DeleteHeader(OMX_BUFFERHEADERTYPE* header)
{
  delete static_cast<bool*>(header->pInputPortPrivate);
  delete static_cast<bool*>(header->pOutputPortPrivate);
  delete header;
}

OMX_ERRORTYPE DecComponent::AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);
  OMXChecker::CheckNotNull(size);
  CheckPortIndex(index);

  auto port = GetPort(index);

  if(transientState != TransientLoadedToIdle && !(port->isTransientToEnable))
    throw OMX_ErrorIncorrectStateOperation;

  BufferHandles handles;
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &handles);
  auto bufferHandlePort = IsInputPort(index) ? handles.input : handles.output;
  bool dmaOnPort = (bufferHandlePort == BufferHandleType::BUFFER_HANDLE_FD);
  auto buffer = dmaOnPort ? reinterpret_cast<OMX_U8*>(ToDecModule(*module).AllocateDMA(size * sizeof(OMX_U8))) : static_cast<OMX_U8*>(module->Allocate(size * sizeof(OMX_U8)));

  if(dmaOnPort ? (static_cast<int>((intptr_t)buffer) < 0) : !buffer)
    throw OMX_ErrorInsufficientResources;

  *header = AllocateHeader(app, size, buffer, true, index);
  assert(*header);
  port->Add(*header);

  return OMX_ErrorNone;
  OMX_CATCH_L([&](OMX_ERRORTYPE& e)
  {
    if(e != OMX_ErrorBadPortIndex)
      GetPort(index)->ErrorOccured();
  });
}

OMX_ERRORTYPE DecComponent::FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  OMX_TRY();
  OMXChecker::CheckNotNull(header);

  CheckPortIndex(index);
  auto port = GetPort(index);

  if((transientState != TransientIdleToLoaded) && (!port->isTransientToDisable))
    callbacks.EventHandler(component, app, OMX_EventError, OMX_ErrorPortUnpopulated, 0, nullptr);

  BufferHandles handles;
  media->Get(SETTINGS_INDEX_BUFFER_HANDLES, &handles);
  auto bufferHandlePort = IsInputPort(index) ? handles.input : handles.output;
  bool dmaOnPort = (bufferHandlePort == BufferHandleType::BUFFER_HANDLE_FD);
  dmaOnPort ? ToDecModule(*module).FreeDMA(static_cast<int>((intptr_t)header->pBuffer)) : module->Free(header->pBuffer);

  port->Remove(header);
  DeleteHeader(header);

  return OMX_ErrorNone;
  OMX_CATCH_L([&](OMX_ERRORTYPE& e)
  {
    if(e != OMX_ErrorBadPortIndex)
      GetPort(index)->ErrorOccured();
  });
}

void DecComponent::TreatEmptyBufferCommand(Task* task)
{
  std::lock_guard<std::mutex> lock(mutex);
  assert(task);
  assert(task->cmd == EmptyBuffer);
  assert(static_cast<int>((intptr_t)task->data) == input.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task->opt.get());
  assert(header);

  if(state == OMX_StateInvalid)
  {
    callbacks.EmptyBufferDone(component, app, header);
    return;
  }

  AttachMark(header);

  if(header->nFlags & OMX_BUFFERFLAG_ENDOFFRAME)
    transmit.push_back(PropagatedData(header->hMarkTargetComponent, header->pMarkData, header->nTimeStamp, header->nFlags));

  auto handle = new OMXBufferHandle(header);
  auto success = module->Empty(handle);
  assert(success);
}

