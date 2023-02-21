// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "omx_component.h"
#include "module/module_enc.h"

struct EncComponent final : public Component
{
  EncComponent(OMX_HANDLETYPE component, std::shared_ptr<SettingsInterface> media, std::unique_ptr<EncModule>&& module, OMX_STRING name, OMX_STRING role, std::unique_ptr<ExpertiseInterface>&& expertise);
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
  locked_queue<uint8_t*> roiFreeBuffers;
  ThreadSafeMap<OMX_BUFFERHEADERTYPE*, uint8_t*> roiMap;
  ThreadSafeMap<OMX_BUFFERHEADERTYPE*, uint8_t*> roiDestroyMap;

  ThreadSafeMap<BufferHandleInterface*, std::vector<OMXSei>> seisMap;
};

