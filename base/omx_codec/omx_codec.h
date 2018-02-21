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

#pragma once

#include "base/omx_base/omx_base.h"
#include "base/omx_module/omx_module_interface.h"
#include "base/omx_utils/processor_fifo.h"
#include "base/omx_utils/threadsafe_map.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <memory>

#define ALLEGRODVT_OMX_VERSION 3

enum Command
{
  SetState,
  Flush,
  DisablePort,
  EnablePort,
  MarkBuffer,
  EmptyBuffer,
  FillBuffer,
  SetDynamic,
  Fence,
  RemoveFence,
  Max,
};

enum TransientState
{
  TransientInvalid,
  TransientLoadedToIdle,
  TransientIdleToPause,
  TransientPauseToExecuting,
  TransientExecutingToIdle,
  TransientExecutingToPause,
  TransientPauseToIdle,
  TransientIdleToLoaded,
  TransientMax,
};

struct Task
{
  Task() :
    cmd(), data(nullptr), opt(nullptr)
  {
  }

  Command cmd;
  void* data;
  std::shared_ptr<void> opt;
};

struct Port
{
  Port(int const& index, int const& expected) :
    index(index), enable(true), playable(false), isTransientToEnable(false), isTransientToDisable(false), expected(expected)
  {
  };

  int const index;
  bool enable;
  bool playable;
  bool isTransientToEnable;
  bool isTransientToDisable;
  size_t expected;

  void Add(OMX_BUFFERHEADERTYPE* header)
  {
    std::lock_guard<std::mutex> lock(mutex);
    buffers.push_back(header);

    if(buffers.size() < expected)
      return;

    playable = true;
    cv_full.notify_one();
  }

  void Remove(OMX_BUFFERHEADERTYPE* header)
  {
    std::lock_guard<std::mutex> lock(mutex);
    buffers.erase(std::remove(buffers.begin(), buffers.end(), header), buffers.end());

    if((buffers.size() > 0))
      return;

    playable = false;
    cv_empty.notify_one();
  }

  void WaitEmpty()
  {
    std::unique_lock<std::mutex> lck(mutex);
    cv_empty.wait(lck, [&] {
      return playable == false;
    });
  }

  void WaitFull()
  {
    std::unique_lock<std::mutex> lck(mutex);
    cv_full.wait(lck, [&] {
      return playable == true;
    });
  }

private:
  std::mutex mutex;
  std::vector<OMX_BUFFERHEADERTYPE*> buffers;
  std::condition_variable cv_full;
  std::condition_variable cv_empty;
};

class Codec : public OMXBase
{
public:
  Codec(OMX_HANDLETYPE component, std::unique_ptr<ModuleInterface>&& module, OMX_STRING name, OMX_STRING role);
  ~Codec();
  OMX_ERRORTYPE SendCommand(OMX_IN OMX_COMMANDTYPE cmd, OMX_IN OMX_U32 param, OMX_IN OMX_PTR data);

  OMX_ERRORTYPE EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header);
  OMX_ERRORTYPE FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header);

  OMX_ERRORTYPE GetState(OMX_OUT OMX_STATETYPE* state);
  OMX_ERRORTYPE SetCallbacks(OMX_IN OMX_CALLBACKTYPE* callbacks, OMX_IN OMX_PTR app);
  virtual OMX_ERRORTYPE UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size, OMX_IN OMX_U8* buffer);
  virtual OMX_ERRORTYPE AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size);
  virtual OMX_ERRORTYPE FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header);
  void ComponentDeInit();
  OMX_ERRORTYPE GetComponentVersion(OMX_OUT OMX_STRING name, OMX_OUT OMX_VERSIONTYPE* version, OMX_OUT OMX_VERSIONTYPE* spec);
  virtual OMX_ERRORTYPE GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param);
  virtual OMX_ERRORTYPE SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param);
  virtual OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index);
  OMX_ERRORTYPE GetConfig(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR config);
  OMX_ERRORTYPE SetConfig(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR config);
  OMX_ERRORTYPE ComponentTunnelRequest(OMX_IN OMX_U32 index, OMX_IN OMX_HANDLETYPE comp, OMX_IN OMX_U32 tunneledIndex, OMX_INOUT OMX_TUNNELSETUPTYPE* setup);
  OMX_ERRORTYPE UseEGLImage(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN void* eglImage);
  OMX_ERRORTYPE ComponentRoleEnum(OMX_OUT OMX_U8* role, OMX_IN OMX_U32 index);

protected:
  OMX_HANDLETYPE const component;
  std::unique_ptr<ModuleInterface> module;
  Port input;
  Port output;
  bool shouldPrealloc;
  bool shouldClearROI;
  bool shouldPushROI;

  OMX_STRING name;
  OMX_STRING role;
  OMX_STATETYPE state;
  TransientState transientState;
  OMX_CALLBACKTYPE callbacks;
  OMX_PTR app;
  OMX_VERSIONTYPE version;
  OMX_VERSIONTYPE spec;
  OMX_PORT_PARAM_TYPE ports;
  ThreadSafeMap<uint8_t*, OMX_BUFFERHEADERTYPE*> map;
  std::queue<OMX_MARKTYPE*> marks;
  std::mutex moduleMutex {};

  std::unique_ptr<ProcessorFifo> processor;
  std::unique_ptr<ProcessorFifo> processorFill;
  void _Process(void* data);
  void _ProcessFillBuffer(void* data);
  void _Delete(void* data);

  void CreateName(OMX_STRING name);
  void CreateRole(OMX_STRING role);

  void CheckPortIndex(int index);
  Port* GetPort(int index);
  inline void PopulatingPorts();
  inline void UnpopulatingPorts();

  void CreateCommand(OMX_COMMANDTYPE command, OMX_U32 param, OMX_PTR data);
  void TreatSetStateCommand(Task* task);
  void TreatFlushCommand(Task* task);
  void TreatDisablePortCommand(Task* task);
  void TreatEnablePortCommand(Task* task);
  void TreatMarkBufferCommand(Task* task);
  virtual void TreatEmptyBufferCommand(Task* task);
  void TreatFillBufferCommand(Task* task);
  void TreatDynamicCommand(Task* task);
  void AttachMark(OMX_BUFFERHEADERTYPE* header);

  virtual void EmptyThisBufferCallBack(uint8_t* emptied, int offset, int size, void* handle);
  virtual void AssociateCallBack(uint8_t* empty, uint8_t* fill);
  virtual void FillThisBufferCallBack(uint8_t* filled, int offset, int size);
  virtual void ReleaseCallBack(bool isInput, uint8_t* buf);
  virtual void EventCallBack(CallbackEventType type, void* data);

private:
  void ReturnFilledBuffer(uint8_t* filled, int offset, int size);
  void ReturnEmptiedBuffer(uint8_t* emptied);
};

inline bool IsEOSDetected(OMX_U32 flags)
{
  return flags & OMX_BUFFERFLAG_EOS;
}

inline void ClearPropagatedData(OMX_BUFFERHEADERTYPE* header)
{
  header->hMarkTargetComponent = NULL;
  header->pMarkData = NULL;
  header->nFilledLen = 0;
  header->nFlags = 0;
}

inline bool IsCompMarked(OMX_HANDLETYPE mark, OMX_HANDLETYPE component)
{
  return mark == component;
}

inline void PropagateHeaderData(OMX_BUFFERHEADERTYPE const* src, OMX_BUFFERHEADERTYPE* dst)
{
  assert(src);
  assert(dst);
  assert(!dst->hMarkTargetComponent);
  assert(!dst->pMarkData);
  assert(!dst->nTimeStamp);
  dst->hMarkTargetComponent = src->hMarkTargetComponent;
  dst->pMarkData = src->pMarkData;
  dst->nTimeStamp = src->nTimeStamp;
  dst->nFlags = src->nFlags;
}

#define START_PORT 0
#define PORTS_COUNT 2

inline void SetPortsParam(OMX_PORT_PARAM_TYPE& port)
{
  port.nPorts = PORTS_COUNT;
  port.nStartPortNumber = START_PORT;
}

inline bool IsInputPort(int index)
{
  return index == START_PORT;
}

static inline void CheckVersionExistance(OMX_PTR ptr)
{
  auto const size = *static_cast<OMX_U32*>(ptr);

  if(size < (sizeof(OMX_U32) + sizeof(OMX_VERSIONTYPE)))
    throw OMX_ErrorBadParameter;
}

inline OMX_VERSIONTYPE GetVersion(OMX_PTR ptr)
{
  CheckVersionExistance(ptr);
  auto tmp = ptr;
  tmp = static_cast<OMX_U32*>(tmp) + 1; // nVersion is always after nSize

  return *static_cast<OMX_VERSIONTYPE*>(tmp);
}

