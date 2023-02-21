// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <OMX_Core.h>
#include <OMX_Component.h>

struct OMXComponentInterface
{
  virtual ~OMXComponentInterface() = 0;

  virtual OMX_ERRORTYPE SendCommand(OMX_IN OMX_COMMANDTYPE Cmd, OMX_IN OMX_U32 nParam1, OMX_IN OMX_PTR pCmdData) = 0;
  virtual OMX_ERRORTYPE GetState(OMX_OUT OMX_STATETYPE* pState) = 0;
  virtual OMX_ERRORTYPE SetCallbacks(OMX_IN OMX_CALLBACKTYPE* pCallbacks, OMX_IN OMX_PTR pAppData) = 0;
  virtual OMX_ERRORTYPE GetParameter(OMX_IN OMX_INDEXTYPE nParamIndex, OMX_INOUT OMX_PTR pParam) = 0;
  virtual OMX_ERRORTYPE SetParameter(OMX_IN OMX_INDEXTYPE nIndex, OMX_IN OMX_PTR pParam) = 0;
  virtual OMX_ERRORTYPE UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes, OMX_IN OMX_U8* pBuffer) = 0;
  virtual OMX_ERRORTYPE AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes) = 0;
  virtual OMX_ERRORTYPE FreeBuffer(OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr) = 0;
  virtual OMX_ERRORTYPE EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* pInputBuf) = 0;
  virtual OMX_ERRORTYPE FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr) = 0;
  virtual void ComponentDeInit() = 0;
  virtual OMX_ERRORTYPE GetComponentVersion(OMX_OUT OMX_STRING pComponentName, OMX_OUT OMX_VERSIONTYPE* pComponentVersion, OMX_OUT OMX_VERSIONTYPE* pSpecVersion) = 0;
  virtual OMX_ERRORTYPE GetConfig(OMX_IN OMX_INDEXTYPE nIndex, OMX_INOUT OMX_PTR pComponentConfigStructure) = 0;
  virtual OMX_ERRORTYPE SetConfig(OMX_IN OMX_INDEXTYPE nIndex, OMX_IN OMX_PTR pComponentConfigStructure) = 0;
  virtual OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_STRING cParameterName, OMX_OUT OMX_INDEXTYPE* pIndexType) = 0;
  virtual OMX_ERRORTYPE ComponentTunnelRequest(OMX_IN OMX_U32 nPort, OMX_IN OMX_HANDLETYPE hTunneledComp, OMX_IN OMX_U32 nTunneledPort, OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup) = 0;
  virtual OMX_ERRORTYPE UseEGLImage(OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN void* eglImage) = 0;
  virtual OMX_ERRORTYPE ComponentRoleEnum(OMX_OUT OMX_U8* cRole, OMX_IN OMX_U32 nIndex) = 0;
};

