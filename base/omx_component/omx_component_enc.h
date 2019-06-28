/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

#include <vector>

#include "omx_component.h"
#include "base/omx_module/module_enc.h"
#include "base/omx_module/sync_ip_interface.h"

struct EncComponent final : public Component
{
  EncComponent(OMX_HANDLETYPE component, std::shared_ptr<MediatypeInterface> media, std::unique_ptr<EncModule>&& module, OMX_STRING name, OMX_STRING role, std::unique_ptr<ExpertiseInterface>&& expertise, std::shared_ptr<SyncIpInterface> syncIp);
  ~EncComponent() override;
  OMX_ERRORTYPE AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size) override;
  OMX_ERRORTYPE UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size, OMX_IN OMX_U8* buffer) override;
  OMX_ERRORTYPE FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header) override;

private:
  uint8_t* AllocateROIBuffer();
  void DestroyROIBuffer(uint8_t* roiBuffer);
  void EmptyThisBufferCallBack(BufferHandleInterface* handle) override;
  void AssociateCallBack(BufferHandleInterface* empty, BufferHandleInterface* fill) override;
  void FillThisBufferCallBack(BufferHandleInterface* filled) override;
  void TreatEmptyBufferCommand(Task* task) override;
  std::shared_ptr<SyncIpInterface> syncIp;
  locked_queue<uint8_t*> roiFreeBuffers;
  ThreadSafeMap<OMX_BUFFERHEADERTYPE*, uint8_t*> roiMap;
  ThreadSafeMap<OMX_BUFFERHEADERTYPE*, uint8_t*> roiDestroyMap;

  ThreadSafeMap<BufferHandleInterface*, std::vector<OMXSei>> seisMap;
};

