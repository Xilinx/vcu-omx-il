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

#include "omx_component_dec.h"
#include "omx_component_getset.h"

#include <OMX_VideoExt.h>
#include <OMX_ComponentAlg.h>
#include <OMX_IVCommonAlg.h>
#include <OMX_CoreAlg.h>

#include <cmath>

#include <utility/omx_translate.h>

#include "base/omx_checker/omx_checker.h"

using namespace std;

static DecModule& ToDecModule(ModuleInterface& module)
{
  return dynamic_cast<DecModule &>(module);
}

DecComponent::DecComponent(OMX_HANDLETYPE component, shared_ptr<MediatypeInterface> media, std::unique_ptr<DecModule>&& module, OMX_STRING name, OMX_STRING role, std::unique_ptr<ExpertiseInterface>&& expertise) :
  Component{component, media, std::move(module), std::move(expertise), name, role},
  shouldPropagateData{true}
{
}

DecComponent::~DecComponent() = default;

void DecComponent::EmptyThisBufferCallBack(BufferHandleInterface* handle)
{
  assert(handle);
  auto header = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)handle)->header;
  ReturnEmptiedBuffer(header);
  delete handle;
}

void DecComponent::FlushComponent()
{
  FlushFillEmptyBuffers(true, true);
  std::unique_lock<std::mutex> lock(mutex);
  transmit.clear();
  lock.unlock();
  shouldPropagateData = true;
}

void DecComponent::AssociateCallBack(BufferHandleInterface* empty_, BufferHandleInterface* fill_)
{
  std::lock_guard<std::mutex> lock(mutex);

  if(!empty_)
  {
    if(transmit.empty())
      return;

    auto emptyHeader = transmit.front();
    auto fill = (OMXBufferHandle*)(fill_);
    auto fillHeader = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)fill)->header;
    assert(fillHeader);
    fillHeader->hMarkTargetComponent = emptyHeader.hMarkTargetComponent;
    fillHeader->pMarkData = emptyHeader.pMarkData;
    fillHeader->nTickCount = emptyHeader.nTickCount;
    fillHeader->nTimeStamp = emptyHeader.nTimeStamp;
    transmit.pop_front();

    if(IsEOSDetected(emptyHeader.nFlags))
    {
      callbacks.EventHandler(component, app, OMX_EventBufferFlag, output.index, emptyHeader.nFlags, nullptr);
      transmit.clear();
      shouldPropagateData = true;
    }

    if(IsCompMarked(emptyHeader.hMarkTargetComponent, component))
      callbacks.EventHandler(component, app, OMX_EventMark, 0, 0, emptyHeader.pMarkData);
    return;
  }

  auto empty = (OMXBufferHandle*)(empty_);
  auto fill = (OMXBufferHandle*)(fill_);
  auto emptyHeader = empty->header;
  auto fillHeader = fill->header;
  PropagateHeaderData(*emptyHeader, *fillHeader);

  if(IsEOSDetected(emptyHeader->nFlags))
    callbacks.EventHandler(component, app, OMX_EventBufferFlag, output.index, emptyHeader->nFlags, nullptr);

  if(IsCompMarked(emptyHeader->hMarkTargetComponent, component))
    callbacks.EventHandler(component, app, OMX_EventMark, 0, 0, emptyHeader->pMarkData);
}

void DecComponent::FillThisBufferCallBack(BufferHandleInterface* filled)
{
  if(!filled)
  {
    if(eosHandles.input && eosHandles.output)
      AssociateCallBack(eosHandles.input, eosHandles.output);

    if(eosHandles.input)
      EmptyThisBufferCallBack(eosHandles.input);

    if(eosHandles.output)
      FillThisBufferCallBack(eosHandles.output);
    eosHandles.input = nullptr;
    eosHandles.output = nullptr;
    return;
  }

  assert(filled);
  auto header = (OMX_BUFFERHEADERTYPE*)((OMXBufferHandle*)filled)->header;
  auto offset = ((OMXBufferHandle*)filled)->offset;
  auto payload = ((OMXBufferHandle*)filled)->payload;
  DisplayPictureInfo displayPictureInfo {};
  auto err = module->GetDynamic(DYNAMIC_INDEX_CURRENT_DISPLAY_PICTURE_INFO, &displayPictureInfo);
  assert(err == ModuleInterface::SUCCESS);

  if(displayPictureInfo.concealed)
    header->nFlags |= OMX_BUFFERFLAG_DATACORRUPT;
  switch(displayPictureInfo.type)
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

  if(offset == 0 && payload == 0)
    header->nFlags = OMX_BUFFERFLAG_EOS;

  delete filled;

  ReturnFilledBuffer(header, offset, payload);
}

void DecComponent::EventCallBack(Callbacks::Event type, void* data)
{
  assert(type < Callbacks::Event::MAX);
  switch(type)
  {
  case Callbacks::Event::SEI_PREFIX_PARSED:
  {
    LOG_IMPORTANT(ToStringCallbackEvent.at(type));

    auto sei = static_cast<Sei*>(data);
    callbacks.EventHandler(component, app, static_cast<OMX_EVENTTYPE>(OMX_ALG_EventSEIPrefixParsed), sei->type, sei->payload, sei->data);
    break;
  }
  case Callbacks::Event::SEI_SUFFIX_PARSED:
  {
    LOG_IMPORTANT(ToStringCallbackEvent.at(type));

    auto sei = static_cast<Sei*>(data);
    callbacks.EventHandler(component, app, static_cast<OMX_EVENTTYPE>(OMX_ALG_EventSEISuffixParsed), sei->type, sei->payload, sei->data);
    break;
  }
  default:
    Component::EventCallBack(type, data);
    break;
  }
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

  if(transientState != TransientState::LoadedToIdle && !(port->isTransientToEnable))
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

  if((transientState != TransientState::IdleToLoaded) && (!port->isTransientToDisable))
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

static Flags CreateFlags(OMX_U32 nFlags)
{
  Flags flags {};

  if(nFlags & OMX_BUFFERFLAG_ENDOFFRAME)
    flags.isEndOfFrame = true;

  if(nFlags & OMX_BUFFERFLAG_ENDOFSUBFRAME)
    flags.isEndOfSlice = true;

  if(nFlags & OMX_BUFFERFLAG_SYNCFRAME)
    flags.isSync = true;

  return flags;
}

void DecComponent::TreatEmptyBufferCommand(Task* task)
{
  std::lock_guard<std::mutex> lock(mutex);
  assert(task);
  assert(task->cmd == Command::EmptyBuffer);
  assert(static_cast<int>((intptr_t)task->data) == input.index);
  auto header = static_cast<OMX_BUFFERHEADERTYPE*>(task->opt.get());
  assert(header);

  if(state == OMX_StateInvalid)
  {
    callbacks.EmptyBufferDone(component, app, header);
    return;
  }

  AttachMark(header);

  if(header->nFilledLen == 0)
  {
    if(header->nFlags & OMX_BUFFERFLAG_EOS)
    {
      auto handle = new OMXBufferHandle(header);
      eosHandles.input = handle;
      auto success = module->Empty(handle);
      assert(success);
      return;
    }
    callbacks.EmptyBufferDone(component, app, header);
    return;
  }

  bool isInputParsed;
  media->Get(SETTINGS_INDEX_INPUT_PARSED, &isInputParsed);

  bool isEarlyCallbackUsed;
  media->Get(SETTINGS_INDEX_LLP2_EARLY_CB, &isEarlyCallbackUsed);

  if(!isInputParsed || isEarlyCallbackUsed)
  {
    bool isEndOfFrameFlagRaised = (header->nFlags & OMX_BUFFERFLAG_ENDOFFRAME);

    if(isEndOfFrameFlagRaised && !isEarlyCallbackUsed)
      transmit.push_back(PropagatedData { header->hMarkTargetComponent, header->pMarkData, header->nTickCount, header->nTimeStamp, header->nFlags });
    else
    {
      if(shouldPropagateData)
        transmit.push_back(PropagatedData { header->hMarkTargetComponent, header->pMarkData, header->nTickCount, header->nTimeStamp, header->nFlags });

      shouldPropagateData = isEndOfFrameFlagRaised;
    }
  }

  auto flags = CreateFlags(header->nFlags);
  module->SetDynamic(DYNAMIC_INDEX_STREAM_FLAGS, &flags);
  auto handle = new OMXBufferHandle(header);

  auto success = module->Empty(handle);
  assert(success);

  if(header->nFlags & OMX_BUFFERFLAG_EOS)
  {
    success = module->Empty(nullptr);
    assert(success);
    return;
  }
}

