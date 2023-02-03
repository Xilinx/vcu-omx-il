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

#pragma once

#include <OMX_ComponentAlg.h> // buffer mode

#include "omx_component.h"
#include "module/module_dec.h"
#include <list>

struct DecComponent final : Component
{
  DecComponent(OMX_HANDLETYPE component, std::shared_ptr<SettingsInterface>, std::unique_ptr<DecModule>&& module, OMX_STRING name, OMX_STRING role, std::unique_ptr<ExpertiseInterface>&& expertise);
  ~DecComponent() override;
  OMX_ERRORTYPE AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size) override;
  OMX_ERRORTYPE FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header) override;

private:
  struct PropagatedData
  {
    PropagatedData(OMX_HANDLETYPE hMarkTargetComponent, OMX_PTR pMarkData, OMX_U32 nTickCount, OMX_TICKS nTimeStamp, OMX_U32 nFlags) :
      hMarkTargetComponent{hMarkTargetComponent},
      pMarkData{pMarkData},
      nTickCount{nTickCount},
      nTimeStamp{nTimeStamp},
      nFlags{nFlags}
    {
    };
    OMX_HANDLETYPE const hMarkTargetComponent;
    OMX_PTR const pMarkData;
    OMX_U32 const nTickCount;
    OMX_TICKS const nTimeStamp;
    OMX_U32 const nFlags;
  };
  void EmptyThisBufferCallBack(BufferHandleInterface* handle) override;
  void AssociateCallBack(BufferHandleInterface* empty, BufferHandleInterface* fill) override;
  void FillThisBufferCallBack(BufferHandleInterface* filled) override;
  void EventCallBack(Callbacks::Event type, void* data) override;
  void FlushComponent() override;

  void TreatEmptyBufferCommand(Task* task) override;
  std::list<PropagatedData> transmit;
  std::mutex mutex;
  OMX_TICKS oldTimeStamp;
  bool dataHasBeenPropagated;
};

