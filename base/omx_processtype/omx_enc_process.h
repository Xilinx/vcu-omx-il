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
#include "omx_enc_ipdevice.h"
#include "base/omx_codectype/omx_enc_param.h"
#include "base/omx_utils/threadsafe_map.h"

class ProcessEncode;

struct EncodeParam
{
  ProcessEncode* pProcess;
};

struct EncBufferUserData
{
  ProcessEncode* pProcess;
};

class ProcessEncode : public ProcessType
{
public:
  ProcessEncode(OMX_HANDLETYPE /* hComponent */, CodecType* /* pCodec */);
  ~ProcessEncode();

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
  AL_HANDLE m_hEncoder;
  EncBufferUserData m_tUserData;
  OMX_BUFFERHEADERTYPE* m_pEOS;
  std::unique_ptr<CIpDevice> m_IpDevice;
  AL_TEncSettings m_EncSettings;
  OMX_U32 m_EOSFlags;
  EncodingParameters m_VideoParameters {};
  EncodeParam m_tEncodeParam;
  bool m_bSubframe;

  void SetStandardPortParameter();
  void SetStandardInParameterPortDefinition();
  void SetStandardOutParameterPortDefinition();
  void SetStandardInParameterBufferSupplier();
  void SetStandardOutParameterBufferSupplier();
  void SetStandardYUVVideoPortDefinition();
  void SetStandardVideoPortDefinition();
  void ResetEncodingParameters();

  OMX_PORT_PARAM_TYPE GetPortParameter();

  OMX_ERRORTYPE SetInParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE);
  OMX_ERRORTYPE SetOutParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE);
  OMX_ERRORTYPE SetYUVVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE);
  OMX_ERRORTYPE SetVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE);
  OMX_ERRORTYPE SetStandardParameterComponentRole(OMX_PARAM_COMPONENTROLETYPE);
  OMX_ERRORTYPE SetVideoParameterRateControl(OMX_VIDEO_PARAM_BITRATETYPE);
  OMX_ERRORTYPE SetVideoParameterProfileLevel(OMX_VIDEO_PARAM_PROFILELEVELTYPE);

  AL_ERateCtrlMode ConvertRCMode(OMX_VIDEO_CONTROLRATETYPE mode);
  OMX_VIDEO_CONTROLRATETYPE ConvertRCMode(AL_ERateCtrlMode mode);

  void SendTask(ComponentThreadTask*);
  ComponentThreadTask* ReceiveTask();

  OMX_ERRORTYPE SetupIpDevice();

  void DestroyEncoder();
  bool CreateEncoder();
  void SetEncoderSettings();
  void SendToEncoder(OMX_BUFFERHEADERTYPE* /* pInputBuf */);
  void FlushEncoder(OMX_U32 /* nFlags */);
  AL_EPicFormat GetPictureFormat();
  OMX_U32 GetSizeYUV(OMX_U32 /* stride */, OMX_U32 /* height */, OMX_COLOR_FORMATTYPE /* color */);
  void ThreadComponent();
  void UpdateOutputPortDefinition();
  OMX_U32 inline ComputeLatency();
  void BufferHeaderUpdate(OMX_BUFFERHEADERTYPE* /* pHeader */, OMX_U32 /* nPortIndex */, OMX_PTR /* pAppPrivate */, OMX_U32 /* nSizeBytes */);
  int GetOutputBufferSize(AL_EPicFormat format);
  int GetOutputBufferCount();

  friend void RedirectionSourceBufferIsDone(AL_TBuffer* pBuf);
  friend void RedirectionStreamBufferIsDone(AL_TBuffer* pBuf);

  void SourceBufferIsDone(OMX_BUFFERHEADERTYPE* pBufHdr);
  void StreamBufferIsDone(OMX_BUFFERHEADERTYPE* pBufHdr);

  friend void RedirectionFrameEncode(void* pUserParam, AL_TBuffer* pStream, AL_TBuffer const* const pSrc);
  void FrameEncode(AL_TBuffer* pStream, AL_TBuffer const* const pSrc);
};

