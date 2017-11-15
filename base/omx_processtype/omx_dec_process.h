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
#include "omx_processtype.h"
#include "omx_dec_ipdevice.h"
extern "C"
{
#include "lib_decode/lib_decode.h"
}

class ProcessDecode;

enum ProcessThreadCommand
{
  Process,
  Close
};

struct ProcessThreadTask
{
  ProcessThreadTask() : cmd(Close), data(nullptr)
  {
  }

  ProcessThreadCommand cmd;
  void* data;
};

struct DecBufferUserData
{
  ProcessDecode* pProcess;
};

struct DecodeParam
{
  ProcessDecode* pProcess;
};

struct DisplayParam
{
  ProcessDecode* pProcess;
};

struct ResolutionFoundParam
{
  ProcessDecode* pProcess;
};

struct PropagatingData
{
  OMX_HANDLETYPE hMarkTargetComponent;
  OMX_PTR pMarkData;
  OMX_TICKS nTimeStamp;
  OMX_U32 nFlags;
};

enum DisplayCommand
{
  StopDisplay,
  Display
};

struct DisplayTask
{
  DisplayTask() : cmd(StopDisplay), data(nullptr)
  {
  }

  DisplayCommand cmd;
  void* data;
};

class ProcessDecode : public ProcessType
{
public:
  ProcessDecode(OMX_HANDLETYPE /* hComponent */, CodecType* /* pCodec */);
  ~ProcessDecode();

  OMX_STATETYPE GetState();
  OMX_STRING GetType();
  OMX_STRING GetRole();
  void SetCallBack(OMX_CALLBACKTYPE* /* pCallback */);
  void SetAppData(OMX_PTR /* pAppData */);
  OMX_ERRORTYPE GetParameter(OMX_IN OMX_INDEXTYPE /* nParamIndex */, OMX_INOUT OMX_PTR /* pParam */);
  OMX_ERRORTYPE SetParameter(OMX_IN OMX_INDEXTYPE /* nIndex */, OMX_IN OMX_PTR /* pParam */);
  OMX_ERRORTYPE SendCommand(OMX_IN OMX_COMMANDTYPE /* Cmd */, OMX_IN OMX_U32 /* nParam1 */, OMX_IN OMX_PTR /* pCmdData */);
  OMX_ERRORTYPE SetState(OMX_IN OMX_U32 /* newState */);
  OMX_ERRORTYPE UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** /* ppBufferHdr */, OMX_IN OMX_U32 /* nPortIndex */, OMX_IN OMX_PTR /* pAppPrivate */, OMX_IN OMX_U32 /* nSizeBytes */, OMX_IN OMX_U8* /* pBuffer */);
  OMX_ERRORTYPE AllocateBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** /* ppBufferHdr */, OMX_IN OMX_U32 /* nPortIndex */, OMX_IN OMX_PTR /* pAppPrivate */, OMX_IN OMX_U32 /* nSizeBytes */);
  OMX_ERRORTYPE FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* /* pBufferHdr */);
  OMX_ERRORTYPE EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* /* pBufferHdr */);
  OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_STRING /* cParameterName */, OMX_OUT OMX_INDEXTYPE* /* pIndexType */);
  OMX_ERRORTYPE ComponentTunnelRequest(OMX_IN OMX_U32 /* nPort */, OMX_IN OMX_HANDLETYPE /* hTunneledComp */, OMX_IN OMX_U32 /* nTunneledPort */, OMX_INOUT OMX_TUNNELSETUPTYPE* /* pTunnelSetup */);
  void ComponentDeInit();

private:
  AL_HANDLE m_hDecoder;
  std::unique_ptr<CIpDevice> m_IpDevice;
  DecodeParam m_tFrameDecodeParam;
  DisplayParam m_tFrameDisplayParam;
  ResolutionFoundParam m_tResolutionFoundParam;
  std::mutex m_MutexDecode;
  std::mutex m_MutexDisplay;
  std::mutex m_MutexResolutionFound;
  semaphore m_SemaphoreDPBisFilled;
  AL_TDecCallBacks m_CallBacks;
  int m_iFramesDisplayed;
  int m_iFramesDecoded;
  int m_iFramesSend;
  int m_iDPBBuffersCount;
  bool m_bIsDPBFull;
  DecBufferUserData m_tUserData;
  OMX_BUFFERHEADERTYPE* m_pEOSHeader;
  std::queue<PropagatingData*> m_PropagatingDataQueue;
  locked_queue<ProcessThreadTask*> m_ProcessTaskQueue;
  std::thread m_ThreadProcess;
  AL_TDecSettings m_eSettings;
  bool m_bEOSReceived;
  bool m_bForceStop;
  bool m_bRunning;
  AL_EVENT m_hFinished;

  void SetStandardPortParameter();
  void SetStandardInParameterPortDefinition();
  void SetStandardOutParameterPortDefinition();
  void SetStandardInParameterBufferSupplier();
  void SetStandardOutParameterBufferSupplier();
  void SetStandardYUVVideoPortDefinition();
  void SetStandardVideoPortDefinition();

  OMX_PORT_PARAM_TYPE GetPortParameter();

  OMX_ERRORTYPE SetInParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE);
  OMX_ERRORTYPE SetOutParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE);
  OMX_ERRORTYPE SetYUVVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE);
  OMX_ERRORTYPE SetVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE);
  OMX_ERRORTYPE SetStandardParameterComponentRole(OMX_PARAM_COMPONENTROLETYPE);

  void SendTask(ComponentThreadTask*);
  ComponentThreadTask* ReceiveTask();

  OMX_ERRORTYPE SetupIpDevice();

  void DestroyDecoder();
  void CreateDecoder();
  void SetSettings(AL_TDecSettings& settings);
  void SendToDecoder(OMX_BUFFERHEADERTYPE* pInputBuf);
  void DeinitCallBacksParam();
  void InitCallBacksParam();
  void InitCallBacks();
  void AddDPBMetaData(OMX_BUFFERHEADERTYPE* pBufferHeader);
  void UpdateDPB();
  OMX_U32 inline ComputeLatency();
  void SetPropagatingData(OMX_BUFFERHEADERTYPE* const pBufferHeader);
  OMX_BUFFERHEADERTYPE* GetOMXBufferHeader(AL_TBuffer const* const pBuf);
  void BufferHeaderUpdate(OMX_BUFFERHEADERTYPE* /* pHeader */, OMX_U32 /* nPortIndex */, OMX_PTR /* pAppPrivate */, OMX_U32 /* nSizeBytes */);

  void SendProcessTask(ProcessThreadTask* ThreadTask);
  ProcessThreadTask* ReceiveProcessTask();
  void ThreadProcess();
  void ThreadComponent();
  void ThreadDisplay();

  friend void RedirectionSourceBufferIsDone(AL_TBuffer* pBuf);
  friend void RedirectionStreamBufferIsDone(AL_TBuffer* pBuf);
  void SourceBufferIsDone(OMX_BUFFERHEADERTYPE* pBufHdr);
  void StreamBufferIsDone(OMX_BUFFERHEADERTYPE* pBufHdr);

  friend void RedirectionFrameDecode(AL_TBuffer* pDecodedFrame, void* pParam);
  void FrameDecode(AL_TBuffer* pDecodedFrame);

  friend void RedirectionFrameDisplay(AL_TBuffer* pDisplayedFrame, AL_TInfoDecode const tInfo, void* pParam);
  void FrameDisplay(AL_TBuffer* pDisplayedFrame, AL_TInfoDecode const tInfo);

  friend void RedirectionResolutionFound(int const iBuffersNeeded, int const iBufferSize, AL_TDimension const tDim, AL_TCropInfo const tCropInfo, TFourCC const tFourCC, void* pParam);
  void ResolutionFound(int const BufferNumber, int const BufferSize, int const iWidth, int const iHeight, AL_TCropInfo const tCropInfo, TFourCC const tFourCC);
};

