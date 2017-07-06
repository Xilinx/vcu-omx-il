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

#include <thread>
#include "base/omx_utils/locked_queue.h"
#include "base/omx_codectype/omx_codectype.h"
#include "base/omx_checker/omx_checker.h"
#include "base/omx_port/omx_port.h"
#include <OMX_IndexExt.h>
#include <OMX_VideoExt.h>
extern "C"
{
#include "lib_common/FourCC.h"
#include "lib_fpga/DmaAlloc.h"
}

#define CUSTOMPARAM_BOARD "OMX.allegro.board"
#define CUSTOMPARAM_BUFFERMODE "OMX.allegro.bufferMode"

struct CustomParam
{
  OMX_U8 cName[OMX_MAX_STRINGNAME_SIZE];
  OMX_INDEXEXTTYPE nIndexType;
};

enum ComponentThreadCommand
{
  ProcessSetComponentState = 100,
  ProcessFlush,
  ProcessStopPort,
  ProcessRestartPort,
  ProcessStop,
  ProcessProcessInput,
  ProcessMarkBuffer
};

struct ComponentThreadTask
{
  ComponentThreadTask() : cmd(ProcessStop), data(nullptr), opt(nullptr)
  {
  }

  ComponentThreadCommand cmd;
  void* data;
  void* opt;
};

class ProcessType
{
public:
  /* Variables */

  /* Methods */
  virtual ~ProcessType()
  {
  };

  virtual OMX_STATETYPE GetState() = 0;
  virtual OMX_STRING GetType() = 0;
  virtual OMX_STRING GetRole() = 0;
  virtual void SetCallBack(OMX_CALLBACKTYPE* /* pCallback */) = 0;
  virtual void SetAppData(OMX_PTR /* pAppData */) = 0;
  virtual OMX_ERRORTYPE GetParameter(OMX_IN OMX_INDEXTYPE /* nParamIndex */, OMX_INOUT OMX_PTR /* pParam */) = 0;
  virtual OMX_ERRORTYPE SetParameter(OMX_IN OMX_INDEXTYPE /* nIndex */, OMX_IN OMX_PTR /* pParam */) = 0;
  virtual OMX_ERRORTYPE SendCommand(OMX_IN OMX_COMMANDTYPE /* Cmd */, OMX_IN OMX_U32 /* nPortIndex */, OMX_IN OMX_PTR /* pCmdData */) = 0;
  virtual OMX_ERRORTYPE SetState(OMX_IN OMX_U32 /* newState */) = 0;
  virtual OMX_ERRORTYPE UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** /* ppBufferHdr */, OMX_IN OMX_U32 /* nPortIndex */, OMX_IN OMX_PTR /* pAppPrivate */, OMX_IN OMX_U32 /* nSizeBytes */, OMX_IN OMX_U8* /* pBuffer */) = 0;
  virtual OMX_ERRORTYPE FreeBuffer(OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr);
  virtual OMX_ERRORTYPE AllocateBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** /* ppBufferHdr */, OMX_IN OMX_U32 /* nPortIndex */, OMX_IN OMX_PTR /* pAppPrivate */, OMX_IN OMX_U32 /* nSizeBytes */) = 0;
  virtual OMX_ERRORTYPE FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* /* pBufferHdr */) = 0;
  virtual OMX_ERRORTYPE EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* /* pBufferHdr */) = 0;
  virtual OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_STRING /* cParameterName */, OMX_OUT OMX_INDEXTYPE* /* pIndexType */) = 0;
  virtual OMX_ERRORTYPE ComponentTunnelRequest(OMX_IN OMX_U32 /* nPort */, OMX_IN OMX_HANDLETYPE /* hTunneledComp */, OMX_IN OMX_U32 /* nTunneledPort */, OMX_INOUT OMX_TUNNELSETUPTYPE* /* pTunnelSetup */) = 0;
  virtual void ComponentDeInit() = 0;

protected:
  /* Variables */
  OMX_HANDLETYPE m_hComponent;
  bool m_bUseVCU;
  OMX_STATETYPE m_state;
  OMX_CALLBACKTYPE* m_pCallback;
  CodecType* m_pCodec;
  OMX_PTR m_pAppData;
  int m_iSchedulerType;
  locked_queue<ComponentThreadTask*> m_ComponentTaskQueue;
  std::mutex m_MutexState;
  std::mutex m_MutexIpDevice;
  std::thread m_ThreadComponent;
  TFourCC m_tFourCC;
  bool m_bUseInputFileDescriptor;
  bool m_bUseOutputFileDescriptor;

  Port inPort;
  Port outPort;

  OMX_PORT_PARAM_TYPE m_PortParameter;

  /* Methods */
  virtual Port* GetPort(OMX_U32 const nPortIndex);
  virtual void SetStandardPortParameter() = 0;
  virtual void SetStandardInParameterPortDefinition() = 0;
  virtual void SetStandardOutParameterPortDefinition() = 0;
  virtual void SetStandardInParameterBufferSupplier() = 0;
  virtual void SetStandardOutParameterBufferSupplier() = 0;
  virtual void SetStandardYUVVideoPortDefinition() = 0;

  virtual OMX_PORT_PARAM_TYPE GetPortParameter() = 0;
  virtual OMX_ERRORTYPE SetInParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE) = 0;
  virtual OMX_ERRORTYPE SetOutParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE) = 0;
  virtual OMX_ERRORTYPE SetYUVVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE) = 0;
  virtual OMX_ERRORTYPE SetVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE) = 0;
  virtual OMX_ERRORTYPE SetStandardParameterComponentRole(OMX_PARAM_COMPONENTROLETYPE) = 0;

  virtual void SendTask(ComponentThreadTask*) = 0;
  virtual ComponentThreadTask* ReceiveTask() = 0;

  virtual void ThreadComponent() = 0;
  virtual OMX_ERRORTYPE SetupIpDevice() = 0;

private:
  /* Variables */

  /* Methods */
};

