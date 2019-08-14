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

#include "omx_component_structs.h"
#include "omx_component_interface.h"
#include "base/omx_module/module_interface.h"
#include "base/omx_module/mediatype_interface.h"
#include "base/omx_module/sync_ip_interface.h"
#include <utility/processor_fifo.h>
#include "omx_buffer_handle.h"
#include "omx_convert_omx_media.h"
#include "omx_component_getset.h"
#include "omx_expertise_interface.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <future>
#include <cassert>
#include <utility/logger.h>
#include <utility/omx_translate.h>

static OMX_U32 constexpr ALLEGRODVT_OMX_VERSION = 3;

#define OMX_TRY() \
  try \
  { \
    void FORCE_SEMICOLON()

#define OMX_CATCH_L(f) \
  } \
  catch(OMX_ERRORTYPE& e) \
  { \
    LOG_ERROR(ToStringOMXError(e)); \
    f(e); \
    return e; \
  } \
  void FORCE_SEMICOLON()

#define OMX_CATCH() \
  } \
  catch(OMX_ERRORTYPE& e) \
  { \
    LOG_ERROR(ToStringOMXError(e)); \
    return e; \
  } \
  void FORCE_SEMICOLON()

struct OMXSei
{
  OMX_ALG_VIDEO_CONFIG_SEI configSei;
  bool isPrefix;
};

struct Component : public OMXComponentInterface
{
  Component(OMX_HANDLETYPE component, std::shared_ptr<MediatypeInterface> media, std::unique_ptr<ModuleInterface>&& module, std::unique_ptr<ExpertiseInterface>&& expertise, std::shared_ptr<SyncIpInterface> syncIp, OMX_STRING name, OMX_STRING role);
  ~Component() override;
  OMX_ERRORTYPE SendCommand(OMX_IN OMX_COMMANDTYPE cmd, OMX_IN OMX_U32 param, OMX_IN OMX_PTR data) override;

  OMX_ERRORTYPE EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header) override;
  OMX_ERRORTYPE FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header) override;

  OMX_ERRORTYPE GetState(OMX_OUT OMX_STATETYPE* state) override;
  OMX_ERRORTYPE SetCallbacks(OMX_IN OMX_CALLBACKTYPE* callbacks, OMX_IN OMX_PTR app) override;

  virtual OMX_ERRORTYPE UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size, OMX_IN OMX_U8* buffer) override;
  virtual OMX_ERRORTYPE AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size) override;
  virtual OMX_ERRORTYPE FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header) override;
  void ComponentDeInit() override;

  OMX_ERRORTYPE GetComponentVersion(OMX_OUT OMX_STRING name, OMX_OUT OMX_VERSIONTYPE* version, OMX_OUT OMX_VERSIONTYPE* spec) override;

  OMX_ERRORTYPE GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param) override;
  OMX_ERRORTYPE SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param) override;
  OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index) override;
  OMX_ERRORTYPE GetConfig(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR config) override;
  OMX_ERRORTYPE SetConfig(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR config) override;

  OMX_ERRORTYPE ComponentTunnelRequest(OMX_IN OMX_U32 index, OMX_IN OMX_HANDLETYPE comp, OMX_IN OMX_U32 tunneledIndex, OMX_INOUT OMX_TUNNELSETUPTYPE* setup) override;
  OMX_ERRORTYPE UseEGLImage(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN void* eglImage) override;
  OMX_ERRORTYPE ComponentRoleEnum(OMX_OUT OMX_U8* role, OMX_IN OMX_U32 index) override;

protected:
  OMX_HANDLETYPE const component;
  std::shared_ptr<MediatypeInterface> media;
  std::unique_ptr<ModuleInterface> module;
  std::unique_ptr<ExpertiseInterface> expertise;
  std::shared_ptr<SyncIpInterface> syncIp;
  Port input;
  Port output;
  bool shouldPrealloc;
  bool shouldClearROI;
  bool shouldPushROI;
  bool shouldFireEventPortSettingsChanges;
  bool isSyncIpCreated;
  std::vector<OMXSei> tmpSeis;

  OMX_STRING name;
  OMX_STRING role;
  OMX_STATETYPE state;
  TransientState transientState;
  OMX_CALLBACKTYPE callbacks;
  OMX_PTR app;
  OMX_VERSIONTYPE version;
  OMX_VERSIONTYPE spec;
  OMX_PORT_PARAM_TYPE videoPortParams;
  std::queue<OMX_MARKTYPE*> marks;

  struct EOSHandles
  {
    BufferHandleInterface* input {};
    BufferHandleInterface* output {};
  };

  EOSHandles eosHandles;

  std::unique_ptr<ProcessorFifo<Task>> processorMain;
  std::unique_ptr<ProcessorFifo<Task>> processorEmpty;
  std::unique_ptr<ProcessorFifo<Task>> processorFill;
  std::shared_ptr<std::promise<void>> pauseFillPromise;
  std::shared_ptr<std::promise<void>> pauseEmptyPromise;
  void _ProcessMain(Task task);
  void _ProcessFillBuffer(Task task);
  void _ProcessEmptyBuffer(Task task);
  void _DeleteFillEmpty(Task task);

  void CreateName(OMX_STRING name);
  void CreateRole(OMX_STRING role);

  void CheckPortIndex(int index);
  Port* GetPort(int index);
  void PopulatingPorts();
  void UnpopulatingPorts();
  void FlushFillEmptyBuffers(bool fill, bool empty);
  void CleanFlushFillEmptyBuffers();
  void BlockFillEmptyBuffers(bool fill, bool empty);
  void UnblockFillEmptyBuffers();
  void FlushEosHandles();
  virtual void FlushComponent();

  void CreateCommand(OMX_COMMANDTYPE command, OMX_U32 param, OMX_PTR data);
  void TreatSetStateCommand(Task task);
  void TreatFlushCommand(Task task);
  void TreatDisablePortCommand(Task task);
  void TreatEnablePortCommand(Task task);
  void TreatMarkBufferCommand(Task task);
  virtual void TreatEmptyBufferCommand(Task* task);
  void TreatFillBufferCommand(Task task);
  void TreatDynamicCommand(Task task);
  void AttachMark(OMX_BUFFERHEADERTYPE* header);

  virtual void EmptyThisBufferCallBack(BufferHandleInterface* emptied);
  virtual void AssociateCallBack(BufferHandleInterface* empty, BufferHandleInterface* fill);
  virtual void FillThisBufferCallBack(BufferHandleInterface* filled);
  virtual void ReleaseCallBack(bool isInput, BufferHandleInterface* buf);
  virtual void EventCallBack(Callbacks::Event type, void* data);

  void ReturnFilledBuffer(OMX_BUFFERHEADERTYPE* filled, int offset, int size);
  void ReturnEmptiedBuffer(OMX_BUFFERHEADERTYPE* emptied);
};

static inline bool IsEOSDetected(OMX_U32 flags)
{
  return flags & OMX_BUFFERFLAG_EOS;
}

static inline bool IsCompMarked(OMX_HANDLETYPE mark, OMX_HANDLETYPE component)
{
  return mark == component;
}

static inline void PropagateHeaderData(OMX_BUFFERHEADERTYPE const& src, OMX_BUFFERHEADERTYPE& dst)
{
  assert(!dst.hMarkTargetComponent);
  assert(!dst.pMarkData);
  assert(!dst.nTimeStamp);
  dst.hMarkTargetComponent = src.hMarkTargetComponent;
  dst.pMarkData = src.pMarkData;
  dst.nTickCount = src.nTickCount;
  dst.nTimeStamp = src.nTimeStamp;
  dst.nFlags = src.nFlags;
}

static int constexpr VIDEO_START_PORT = 0;
static int constexpr VIDEO_PORTS_COUNT = 2;

static inline bool IsInputPort(int index)
{
  return index == VIDEO_START_PORT;
}

