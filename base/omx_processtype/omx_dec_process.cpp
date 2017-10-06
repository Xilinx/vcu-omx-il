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

#include "omx_dec_process.h"
#include "omx_schedulertype.h"

extern "C"
{
#include "lib_common/BufferSrcMeta.h"
#include "lib_fpga/DmaAllocLinux.h"
}

#include "base/omx_utils/omx_log.h"
#include "base/omx_utils/omx_setup.h"
#include "base/omx_buffer/omx_buffer_meta.h"

#include <OMX_ComponentExt.h>

#include <assert.h>
#include <string>
#include <sstream>
#include <cmath>


static CustomParam AL_CustomParam[] =
{
};

static bool isCopyNeeded(bool isFileDescriptorUsed)
{
  return !isFileDescriptorUsed;
}

static void copy(char* source, size_t sourceOffset, char* dest, size_t destOffset, size_t size)
{
  if(source != dest)
    memcpy(dest + destOffset, source + sourceOffset, size);
}

static OMX_COLOR_FORMATTYPE getColorFormat(TFourCC const tFourCC)
{
  switch(AL_GetBitDepth(tFourCC))
  {
  case 8:
  {
    if(AL_GetChromaMode(tFourCC) == CHROMA_4_2_0)
      return OMX_COLOR_FormatYUV420SemiPlanar;

    if(AL_GetChromaMode(tFourCC) == CHROMA_4_2_2)
      return OMX_COLOR_FormatYUV422SemiPlanar;
    break;
  }
  case 10:
  {
    if(AL_GetChromaMode(tFourCC) == CHROMA_4_2_0)
      return (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked;

    if(AL_GetChromaMode(tFourCC) == CHROMA_4_2_2)
      return (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked;
    break;
  }
  default:
    LOGE("0x%.8x is an unknown FOURCC", tFourCC);
    assert(0);
  }

  return OMX_COLOR_FormatUnused;
}

static int getBitDepth(OMX_COLOR_FORMATTYPE format)
{
  OMX_U32 extendedFormat = format;

  if(extendedFormat == OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked || extendedFormat == OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked)
    return 10;
  return 8;
}

static TFourCC getFourCC(OMX_COLOR_FORMATTYPE format)
{
  switch(static_cast<OMX_U32>(format))
  {
  case OMX_COLOR_FormatYUV420SemiPlanar: return FOURCC(NV12);
  case OMX_COLOR_FormatYUV422SemiPlanar: return FOURCC(NV16);
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked: return FOURCC(RX0A);
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked: return FOURCC(RX2A);
  default: assert(0);
  }

  return 0;
}

void RedirectionStreamBufferIsDone(AL_TBuffer* pBuf)
{
  auto const pUserData = static_cast<DecBufferUserData*>(AL_Buffer_GetUserData(pBuf));

  if(!pUserData)
  {
    LOGE("No OMX-IL callback found");
    return;
  }

  auto const pThis = pUserData->pProcess;

  if(!pThis)
  {
    LOGE("No process found");
    return;
  }

  auto const pHeader = pThis->GetOMXBufferHeader(pBuf);

  if(!pHeader)
  {
    LOGE("No OMX header found");
    return;
  }

  pThis->StreamBufferIsDone(pHeader);
}

void RedirectionSourceBufferIsDone(AL_TBuffer* pBuf)
{
  auto const pUserData = static_cast<DecBufferUserData*>(AL_Buffer_GetUserData(pBuf));

  if(!pUserData)
  {
    LOGE("No OMX-IL callback found");
    return;
  }

  auto const pThis = pUserData->pProcess;

  if(!pThis)
  {
    LOGE("No process found");
    return;
  }

  auto const pHeader = pThis->GetOMXBufferHeader(pBuf);

  if(!pHeader)
  {
    LOGE("No OMX header found");
    return;
  }

  pThis->SourceBufferIsDone(pHeader);
}

void ProcessDecode::BufferHeaderUpdate(OMX_BUFFERHEADERTYPE* pHeader, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, OMX_U32 nSizeBytes)
{
  pHeader->pAppPrivate = pAppPrivate;
  pHeader->nAllocLen = nSizeBytes;

  if(nPortIndex == inPort.getDefinition().nPortIndex)
  {
    pHeader->nInputPortIndex = nPortIndex;
    pHeader->nOutputPortIndex = 0xFFFFFFFE;
    pHeader->pInputPortPrivate = pAppPrivate;
    pHeader->pOutputPortPrivate = nullptr;
  }
  else
  {
    pHeader->nInputPortIndex = 0xFFFFFFFE;
    pHeader->nOutputPortIndex = nPortIndex;
    pHeader->pInputPortPrivate = nullptr;
    pHeader->pOutputPortPrivate = pAppPrivate;
  }

  auto const pOMXMeta = (AL_TMetaData*)AL_OMXMetaData_Create(pHeader);
  assert(pOMXMeta);
  AL_Buffer_AddMetaData((AL_TBuffer*)pHeader->pPlatformPrivate, pOMXMeta);
}

void RedirectionFrameDisplay(AL_TBuffer* pDisplayedFrame, AL_TInfoDecode const tInfo, void* pParam)
{
  auto displayParam = static_cast<DisplayParam*>(pParam);

  assert(displayParam);

  auto processDecode = static_cast<ProcessDecode*>(displayParam->pProcess);

  assert(processDecode);

  processDecode->FrameDisplay(pDisplayedFrame, tInfo);
}

#include <stdlib.h> // for exit
void RedirectionFrameDecode(AL_TBuffer* pDecodedFrame, void* pParam)
{
  /* TODO handle this error */
  if(!pDecodedFrame)
    exit(99);

  auto decodeParam = static_cast<DecodeParam*>(pParam);
  assert(decodeParam);

  auto processDecode = static_cast<ProcessDecode*>(decodeParam->pProcess);
  assert(processDecode);

  processDecode->FrameDecode(pDecodedFrame);
}

void RedirectionResolutionFound(int const iBufferNeeded, int const iBufferSize, int const iWidth, int const iHeight, AL_TCropInfo const tCropInfo, TFourCC const tFourCC, void* pParam)
{
  auto resolutionFoundParam = static_cast<ResolutionFoundParam*>(pParam);

  assert(resolutionFoundParam);

  auto processDecode = static_cast<ProcessDecode*>(resolutionFoundParam->pProcess);

  assert(processDecode);

  processDecode->ResolutionFound(iBufferNeeded, iBufferSize, iWidth, iHeight, tCropInfo, tFourCC);
}

static void initSettings(AL_TDecSettings& settings)
{
  memset(&settings, 0, sizeof(AL_TDecSettings));
  settings.iStackSize = 5;
  settings.uDDRWidth = 32;
  settings.eDecUnit = AL_AU_UNIT;
}

ProcessDecode::ProcessDecode(OMX_HANDLETYPE hComponent, CodecType* pCodec)
{
  // Check Parameters
  assert(hComponent);
  assert(pCodec);

  m_hComponent = hComponent;
  m_pCodec = pCodec;

  // Init Private Variables
  m_hDecoder = NULL;
  m_iFramesDisplayed = 0;
  m_iFramesSend = 0;
  m_tFourCC = FOURCC(NV12);
  m_iDPBBuffersCount = 0;
  m_iFramesDecoded = 0;
  m_bEOSReceived = false;
  m_bForceStop = false;
  m_bRunning = false;
  m_tUserData = { this };
  initSettings(m_eSettings);
  m_bIsDPBFull = false;
  m_pEOSHeader = nullptr;
  m_bUseVCU = AL_USE_VCU;
  m_iSchedulerType = AL_USE_MCU ? SCHEDULER_MCU : SCHEDULER_CPU;

  m_bUseInputFileDescriptor = false;
  m_bUseOutputFileDescriptor = false;

  InitCallBacks();

  SetStandardPortParameter();
  SetStandardInParameterPortDefinition();
  SetStandardOutParameterPortDefinition();
  SetStandardYUVVideoPortDefinition();
  SetStandardVideoPortDefinition();
  SetStandardInParameterBufferSupplier();
  SetStandardOutParameterBufferSupplier();

  m_hFinished = Rtos_CreateEvent(false);

  m_ThreadComponent = std::thread(&ProcessDecode::ThreadComponent, this);
  m_ThreadProcess = std::thread(&ProcessDecode::ThreadProcess, this);

  m_state = OMX_StateLoaded;
}

void ProcessDecode::ComponentDeInit()
{
  LOGV("_ %s _ ", __func__);
  auto StopComponent = new ComponentThreadTask;

  SendTask(StopComponent);
  m_ThreadComponent.join();

  auto StopProcess = new ProcessThreadTask;
  SendProcessTask(StopProcess);
  m_ThreadProcess.join();

  Rtos_DeleteEvent(m_hFinished);
  delete m_pCodec;
}

ProcessDecode::~ProcessDecode()
{
}

OMX_STATETYPE ProcessDecode::GetState()
{
  return m_state;
}

OMX_STRING ProcessDecode::GetType()
{
  return (OMX_STRING)"decoder";
}

OMX_STRING ProcessDecode::GetRole()
{
  return (OMX_STRING)"video_decoder";
}

void ProcessDecode::SetCallBack(OMX_CALLBACKTYPE* pCallback)
{
  m_pCallback = pCallback;
}

void ProcessDecode::SetAppData(OMX_PTR pAppData)
{
  m_pAppData = pAppData;
}

OMX_ERRORTYPE ProcessDecode::GetParameter(OMX_IN OMX_INDEXTYPE nParamIndex, OMX_INOUT OMX_PTR pParam)
{
  try
  {
    if(!pParam)
      return OMX_ErrorBadParameter;

    if((*((OMX_U32*)pParam) / sizeof(pParam) < 1))
      return OMX_ErrorBadParameter;

    OMXChecker::CheckHeaderVersion(*(((OMX_VERSIONTYPE*)pParam) + 1));
    auto eRet = OMX_ErrorNone;

    auto const nIndex = static_cast<OMX_U32>(nParamIndex);

    if(nIndex == OMX_IndexParamVideoInit)
    {
      LOGV("_ %s _ : OMX_IndexParamVideoInit", __func__);
      auto port = (OMX_PORT_PARAM_TYPE*)pParam;

      if(eRet == OMX_ErrorNone)
        *port = GetPortParameter();

      return eRet;
    }
    else if(nIndex == OMX_ALG_IndexParamReportedLatency)
    {
      LOGV("_ %s _: OMX_IndexParamLatency", __func__);
      auto port = (OMX_ALG_PARAM_REPORTED_LATENCY*)pParam;
      port->nLatency = ComputeLatency();
      return OMX_ErrorNone;
    }

    OMX_U32 const nPortIndex = *(((OMX_U32*)pParam) + 2);
    auto port = GetPort(nPortIndex);

    if(!port)
      return OMX_ErrorBadPortIndex;
    switch((OMX_U32)nIndex)
    {
    case OMX_IndexParamPortDefinition:
    {
      LOGV("_ %s _ : OMX_IndexParamPortDefinition (%u)", __func__, nPortIndex);
      auto param = (OMX_PARAM_PORTDEFINITIONTYPE*)pParam;
      *param = port->getDefinition();
      break;
    }
    case OMX_IndexParamCompBufferSupplier:
    {
      LOGV("_ %s _ : OMX_IndexParamCompBufferSupplier (%u)", __func__, nPortIndex);
      auto param = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParam;
      *param = port->getSupplier();
      break;
    }
    case OMX_IndexParamVideoPortFormat:
    {
      LOGV("_ %s _ : OMX_IndexParamVideoPortFormat (%u)", __func__, nPortIndex);
      auto param = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)pParam;

      if(param->nIndex == 0)
        *param = port->getVideoFormat();
      else
        eRet = OMX_ErrorNoMore;
      break;
    }
    case OMX_IndexParamVideoProfileLevelCurrent:
    {
      LOGV("_ %s _ : OMX_IndexParamVideoProfileLevelCurrent (%u)", __func__, nPortIndex);
      auto param = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)pParam;

      if(param->nPortIndex == outPort.getDefinition().nPortIndex)
        eRet = OMX_ErrorBadPortIndex;
      else
      {
        param->eProfile = m_pCodec->GetCurProfile();
        param->eLevel = m_pCodec->GetCurLevel();
      }
      break;
    }
    case OMX_IndexParamVideoProfileLevelQuerySupported:
    {
      LOGV("_ %s _ : OMX_IndexParamVideoProfileLevelQuerySupported (%u)", __func__, nPortIndex);
      auto param = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)pParam;

      if(param->nPortIndex == outPort.getDefinition().nPortIndex)
        eRet = OMX_ErrorBadPortIndex;
      else
      {
        if(param->nProfileIndex < m_pCodec->GetSupportedProfileLevelSize())
        {
          param->eProfile = m_pCodec->GeteProfile(param->nProfileIndex);
          param->eLevel = m_pCodec->GeteLevel(param->nProfileIndex);
        }
        else
          eRet = OMX_ErrorNoMore;
      }
      break;
    }
    default:
    {
      eRet = OMX_ErrorUnsupportedIndex;
    }
    }

    return eRet;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE ProcessDecode::SetParameter(OMX_IN OMX_INDEXTYPE nIndex, OMX_IN OMX_PTR pParam)
{
  try
  {
    if(!pParam)
      return OMX_ErrorBadParameter;

    if((*((OMX_U32*)pParam) / sizeof(pParam) < 1))
      return OMX_ErrorBadParameter;

    OMXChecker::CheckHeaderVersion(*(((OMX_VERSIONTYPE*)pParam) + 1));

    auto eRet = OMX_ErrorNone;

    if(nIndex == OMX_IndexParamStandardComponentRole)
    {
      LOGV("_ %s _ : OMX_IndexParamStandardComponentRole", __func__);
      auto param = (OMX_PARAM_COMPONENTROLETYPE*)pParam;
      LOGV("Role: %s", (OMX_STRING)param->cRole);
      char role[OMX_MAX_STRINGNAME_SIZE] =
      {
        0
      };

      if(strlen(GetRole()) + strlen(".") + strlen(m_pCodec->GetRole()) > OMX_MAX_STRINGNAME_SIZE)
        return OMX_ErrorOverflow;

      strncat(role, GetRole(), strlen(GetRole()));
      strncat(role, ".", strlen("."));
      strncat(role, m_pCodec->GetRole(), strlen(m_pCodec->GetRole()));

      if(!strncmp((OMX_STRING)param->cRole, role, OMX_MAX_STRINGNAME_SIZE))
        return SetStandardParameterComponentRole(*param);
      else
        return OMX_ErrorBadParameter;
    }


    OMX_U32 const nPortIndex = *(((OMX_U32*)pParam) + 2);
    auto port = GetPort(nPortIndex);

    if(!port)
      return OMX_ErrorBadPortIndex;
    switch((OMX_U32)nIndex)
    {
    case OMX_IndexParamPortDefinition:
    {
      LOGV("_ %s _ : OMX_IndexParamPortDefinition (%u)", __func__, nPortIndex);
      auto port = (OMX_PARAM_PORTDEFINITIONTYPE*)pParam;

      if(eRet == OMX_ErrorNone)
      {
        if(port->nPortIndex == inPort.getDefinition().nPortIndex)
          eRet = SetInParameterPortDefinition(*port);
        else
          eRet = SetOutParameterPortDefinition(*port);
      }
      break;
    }
    case OMX_IndexParamCompBufferSupplier:
    {
      LOGV("_ %s _ : OMX_IndexParamCompBufferSupplier (%u)", __func__, nPortIndex);
      auto param = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParam;
      eRet = port->setSupplier(*param);
      break;
    }
    case OMX_IndexParamVideoPortFormat:
    {
      LOGV("_ %s _ : OMX_IndexParamVideoPortFormat (%u)", __func__, nPortIndex);
      auto port = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)pParam;

      if(port->nPortIndex == inPort.getDefinition().nPortIndex)
        eRet = SetVideoParameterPortFormat(*port);
      else
        eRet = SetYUVVideoParameterPortFormat(*port);
      break;
    }
    case OMX_IndexParamVideoProfileLevelCurrent:
    {
      LOGV("_ %s _ : OMX_IndexParamVideoProfileLevelCurrent (%u)", __func__, nPortIndex);
      auto param = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)pParam;

      if(param->nPortIndex == outPort.getDefinition().nPortIndex)
        eRet = OMX_ErrorBadPortIndex;
      else
        m_pCodec->SetProfileLevel({ param->eProfile, param->eLevel });
      break;
    }
    case OMX_ALG_IndexPortParamBufferMode:
    {
      auto bufMode = (OMX_ALG_PORT_PARAM_BUFFER_MODE*)pParam;

      if(bufMode->nPortIndex == inPort.getDefinition().nPortIndex)
        m_bUseInputFileDescriptor = (bufMode->eMode == OMX_ALG_BUF_DMA);
      else
        m_bUseOutputFileDescriptor = (bufMode->eMode == OMX_ALG_BUF_DMA);
      break;
    }
    case OMX_ALG_IndexParamVideoDecodedPictureBuffer:
    {
      auto dpb = (OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER*)pParam;

      if(dpb->nPortIndex == inPort.getDefinition().nPortIndex)
        m_eSettings.eDpbMode = (AL_EDpbMode)dpb->eDecodedPictureBufferMode;
      else
        eRet = OMX_ErrorBadPortIndex;
      break;
    }
    case OMX_ALG_IndexParamVideoInternalEntropyBuffers:
    {
      auto ieb = (OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS*)pParam;

      if(ieb->nPortIndex == inPort.getDefinition().nPortIndex)
        m_eSettings.iStackSize = ieb->nNumInternalEntropyBuffers;
      else
        eRet = OMX_ErrorBadPortIndex;
      break;
    }
    case OMX_ALG_IndexParamVideoSubframe:
    {
      auto sub = (OMX_ALG_VIDEO_PARAM_SUBFRAME*)pParam;

      if(sub->nPortIndex == inPort.getDefinition().nPortIndex)
        m_eSettings.eDecUnit = AL_AU_UNIT;
      else
        eRet = OMX_ErrorBadPortIndex;

      break;
    }

    default:
    {
      eRet = OMX_ErrorUnsupportedIndex;
    }
    }

    return eRet;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

void ProcessDecode::SetStandardPortParameter()
{
  OMXChecker::SetHeaderVersion(m_PortParameter);
  m_PortParameter.nPorts = 0x2;
  m_PortParameter.nStartPortNumber = 0x0;
}

void ProcessDecode::SetStandardInParameterPortDefinition()
{
  auto& def = inPort.getDefinition();
  OMXChecker::SetHeaderVersion(def);

  def.nPortIndex = 0x0;
  def.bEnabled = OMX_TRUE;
  def.bPopulated = OMX_FALSE;
  def.eDomain = OMX_PortDomainVideo;
  def.eDir = OMX_DirInput;
  def.nBufferCountMin = 0x2;
  def.nBufferCountActual = 0x2;
  assert(def.nBufferCountActual >= def.nBufferCountMin);
  def.nBufferSize = 8 * 1024 * 1024;
  def.bBuffersContiguous = OMX_TRUE;
  def.nBufferAlignment = 64;
}

void ProcessDecode::SetStandardOutParameterPortDefinition()
{
  auto& def = outPort.getDefinition();
  OMXChecker::SetHeaderVersion(def);

  def.nPortIndex = 0x1;
  def.bEnabled = OMX_TRUE;
  def.bPopulated = OMX_FALSE;
  def.eDomain = OMX_PortDomainVideo;
  def.eDir = OMX_DirOutput;
  def.nBufferCountMin = 0;
  def.nBufferCountActual = 0;
  assert(def.nBufferCountActual >= def.nBufferCountMin);

  def.nBufferSize = 0;
  def.bBuffersContiguous = OMX_TRUE;
  def.nBufferAlignment = AL_ALIGN_FRM_BUF;
}

void ProcessDecode::SetStandardInParameterBufferSupplier()
{
  OMX_PARAM_BUFFERSUPPLIERTYPE supp;
  OMXChecker::SetHeaderVersion(supp);
  supp.nPortIndex = 0;
  supp.eBufferSupplier = OMX_BufferSupplyOutput;

  inPort.setSupplier(supp);
}

void ProcessDecode::SetStandardOutParameterBufferSupplier()
{
  OMX_PARAM_BUFFERSUPPLIERTYPE supp;
  OMXChecker::SetHeaderVersion(supp);
  supp.nPortIndex = 1;
  supp.eBufferSupplier = OMX_BufferSupplyOutput;

  outPort.setSupplier(supp);
}

void ProcessDecode::SetStandardYUVVideoPortDefinition()
{
  auto& videoDef = outPort.getVideoDefinition();
  videoDef.cMIMEType = (OMX_STRING)"video/yuv";
  videoDef.pNativeRender = NULL;
  videoDef.nFrameWidth = WIDTH;
  videoDef.nFrameHeight = HEIGHT;
  videoDef.nSliceHeight = RndHeight(videoDef.nFrameHeight);
  videoDef.nBitrate = 0;
  videoDef.xFramerate = FRAMERATE;
  videoDef.bFlagErrorConcealment = OMX_FALSE;
  videoDef.eCompressionFormat = OMX_VIDEO_CodingUnused;
  videoDef.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  videoDef.nStride = RndPitch(videoDef.nFrameWidth, getBitDepth(videoDef.eColorFormat));
  videoDef.pNativeWindow = NULL;
  m_tFourCC = getFourCC(videoDef.eColorFormat);
}

void ProcessDecode::SetStandardVideoPortDefinition()
{
  auto& videoDef = inPort.getVideoDefinition();
  videoDef.cMIMEType = m_pCodec->GetMIME();
  videoDef.pNativeRender = NULL;
  videoDef.nFrameWidth = WIDTH;
  videoDef.nFrameHeight = HEIGHT;
  videoDef.nStride = STRIDE;
  videoDef.nSliceHeight = HEIGHT;
  videoDef.nBitrate = 0;
  videoDef.xFramerate = FRAMERATE;
  videoDef.bFlagErrorConcealment = OMX_FALSE;
  videoDef.eCompressionFormat = m_pCodec->GetCompressionFormat();
  videoDef.eColorFormat = OMX_COLOR_FormatUnused;
  videoDef.pNativeWindow = NULL;
}

OMX_PORT_PARAM_TYPE ProcessDecode::GetPortParameter()
{
  return m_PortParameter;
}

OMX_ERRORTYPE ProcessDecode::SetInParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE param)
{
  auto& videoParam = param.format.video;

  if(((videoParam.nStride % AL_ALIGN_PITCH) != 0) || videoParam.nStride == 0)
  {
    OMX_S32 newStride = RndPitch(videoParam.nFrameWidth, getBitDepth(videoParam.eColorFormat));
    LOGI("Changing input port stride (%i) to %i (port width is %i)", videoParam.nStride, newStride, videoParam.nFrameWidth);
    videoParam.nStride = newStride;
  }

  if(((videoParam.nSliceHeight % AL_ALIGN_HEIGHT) != 0) ||
     (videoParam.nSliceHeight == 0) ||
     ((int)videoParam.nSliceHeight < (int)videoParam.nFrameHeight))
  {
    OMX_S32 newSliceHeight = RndHeight(videoParam.nFrameHeight);
    LOGI("Changing input port slice height (%i) to %i (port height is %i)", videoParam.nSliceHeight, newSliceHeight, videoParam.nFrameHeight);
    videoParam.nSliceHeight = newSliceHeight;
  }

  inPort.setDefinition(param);

  // Update Output port Definition
  auto& videoDef = outPort.getVideoDefinition();
  videoDef.nFrameWidth = videoParam.nFrameWidth;
  videoDef.nFrameHeight = videoParam.nFrameHeight;
  videoDef.nStride = RndPitch(videoParam.nFrameWidth, getBitDepth(videoDef.eColorFormat));
  videoDef.nSliceHeight = RndHeight(videoParam.nFrameHeight);

  auto& def = outPort.getDefinition();
  def.nBufferSize = AL_GetAllocSize_Frame({ (int)videoDef.nFrameWidth, (int)videoDef.nFrameHeight }, AL_GetChromaMode(m_tFourCC), AL_GetBitDepth(m_tFourCC), false, AL_FB_RASTER);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessDecode::SetOutParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE param)
{
  auto eRet = OMX_ErrorNone;
  auto& videoParam = param.format.video;

  if(((videoParam.nStride % AL_ALIGN_PITCH) != 0) || videoParam.nStride == 0)
  {
    OMX_S32 newStride = RndPitch(videoParam.nFrameWidth, getBitDepth(videoParam.eColorFormat));
    LOGI("Changing output port stride (%i) to %i (port width is %i)", videoParam.nStride, newStride, videoParam.nFrameWidth);
    videoParam.nStride = newStride;
  }

  if(((videoParam.nSliceHeight % AL_ALIGN_HEIGHT) != 0) ||
     (videoParam.nSliceHeight == 0) ||
     ((int)videoParam.nSliceHeight < (int)videoParam.nFrameHeight))
  {
    OMX_S32 newSliceHeight = RndHeight(videoParam.nFrameHeight);
    LOGI("Changing output port slice height (%i) to %i (port height is %i)", videoParam.nSliceHeight, newSliceHeight, videoParam.nFrameHeight);
    videoParam.nSliceHeight = newSliceHeight;
  }

  m_tFourCC = getFourCC(videoParam.eColorFormat);
  param.nBufferSize = AL_GetAllocSize_Frame({ (int)videoParam.nFrameWidth, (int)videoParam.nFrameHeight }, AL_GetChromaMode(m_tFourCC), AL_GetBitDepth(m_tFourCC), false, AL_FB_RASTER);
  outPort.setDefinition(param);

  return eRet;
}

OMX_ERRORTYPE ProcessDecode::SetYUVVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE format)
{
  auto& videoDef = outPort.getVideoDefinition();
  videoDef.eCompressionFormat = format.eCompressionFormat;
  videoDef.eColorFormat = format.eColorFormat;
  videoDef.xFramerate = format.xFramerate;
  m_tFourCC = getFourCC(videoDef.eColorFormat);

  auto& def = outPort.getDefinition();
  def.nBufferSize = AL_GetAllocSize_Frame({ (int)videoDef.nFrameWidth, (int)videoDef.nFrameHeight }, AL_GetChromaMode(m_tFourCC), AL_GetBitDepth(m_tFourCC), false, AL_FB_RASTER);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessDecode::SetVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE format)
{
  auto& videoDef = inPort.getVideoDefinition();
  videoDef.eCompressionFormat = format.eCompressionFormat;
  videoDef.eColorFormat = format.eColorFormat;
  videoDef.xFramerate = format.xFramerate;

  auto& def = inPort.getDefinition();
  def.nBufferSize = 0; // XXX
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessDecode::SetStandardParameterComponentRole(OMX_PARAM_COMPONENTROLETYPE)
{
  auto eRet = OMX_ErrorNone;

  SetStandardPortParameter();
  SetStandardInParameterPortDefinition();
  SetStandardOutParameterPortDefinition();
  SetStandardYUVVideoPortDefinition();
  SetStandardVideoPortDefinition();
  SetStandardInParameterBufferSupplier();
  SetStandardOutParameterBufferSupplier();

  return eRet;
}

OMX_ERRORTYPE ProcessDecode::SendCommand(OMX_COMMANDTYPE Cmd, OMX_U32 nPortIndex, OMX_PTR pCmdData)
{
  auto eRet = OMX_ErrorNone;

  // StateSet is a special command

  if((nPortIndex == inPort.getDefinition().nPortIndex) ||
     (nPortIndex == outPort.getDefinition().nPortIndex) ||
     (nPortIndex == OMX_ALL))
  {
    auto ThreadTask = new ComponentThreadTask;
    ThreadTask->data = reinterpret_cast<uintptr_t*>(nPortIndex);
    ThreadTask->opt = pCmdData;
    switch(Cmd)
    {
    case OMX_CommandFlush:
    {
      ThreadTask->cmd = ProcessFlush;
      break;
    }
    case OMX_CommandPortDisable:
    {
      delete ThreadTask;
      auto port = GetPort(nPortIndex);
      auto& def = port->getDefinition();
      def.bEnabled = OMX_FALSE;
      def.bPopulated = OMX_FALSE;
      m_pCallback->EventHandler(m_hComponent, m_pAppData, OMX_EventCmdComplete, OMX_CommandPortDisable, def.nPortIndex, nullptr);
      return OMX_ErrorNone;
    }
    case OMX_CommandPortEnable:
    {
      delete ThreadTask;
      auto port = GetPort(nPortIndex);
      auto& def = port->getDefinition();
      def.bEnabled = OMX_TRUE;
      m_pCallback->EventHandler(m_hComponent, m_pAppData, OMX_EventCmdComplete, OMX_CommandPortEnable, def.nPortIndex, nullptr);
      return OMX_ErrorNone;
    }
    case OMX_CommandMarkBuffer:
    {
      ThreadTask->cmd = ProcessMarkBuffer;
      break;
    }
    default:
    {
      delete ThreadTask;
      return OMX_ErrorBadParameter;
    }
    }

    SendTask(ThreadTask);
  }
  else
    return OMX_ErrorBadPortIndex;

  return eRet;
}

OMX_ERRORTYPE ProcessDecode::SetState(OMX_U32 newState)
{
  auto eRet = OMX_ErrorNone;
  auto ThreadTask = new ComponentThreadTask;

  ThreadTask->cmd = ProcessSetComponentState;
  ThreadTask->data = reinterpret_cast<uintptr_t*>(newState);
  ThreadTask->opt = nullptr;

  SendTask(ThreadTask);

  return eRet;
}

void ProcessDecode::SendProcessTask(ProcessThreadTask* ThreadTask)
{
  m_ProcessTaskQueue.push(ThreadTask);
}

void ProcessDecode::SendTask(ComponentThreadTask* ThreadTask)
{
  m_ComponentTaskQueue.push(ThreadTask);
}

OMX_ERRORTYPE ProcessDecode::UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes, OMX_IN OMX_U8* pBuffer)
{
  LOGV("_ %s _,\nnPortIndex = %u\npAppPrivate = %p\nnSizeBytes = %u\npBuffer = %p", __func__,
       nPortIndex,
       pAppPrivate,
       nSizeBytes,
       pBuffer
       );

  auto port = GetPort(nPortIndex);

  if(!port)
    return OMX_ErrorBadPortIndex;

  // Create IP device
  // We suppose all settings are done
  auto eRet = SetupIpDevice();

  if(eRet != OMX_ErrorNone)
    return eRet;

  if(port->getDefinition().bPopulated)
    return OMX_ErrorBadParameter;

  if(nSizeBytes != port->getDefinition().nBufferSize)
    return OMX_ErrorBadParameter;

  auto BufferHeader = port->useBuffer(pBuffer, nSizeBytes, &m_tUserData);

  if(!BufferHeader)
    return OMX_ErrorInsufficientResources;

  BufferHeaderUpdate(BufferHeader, nPortIndex, pAppPrivate, nSizeBytes);

  *ppBufferHdr = BufferHeader;

  return eRet;
}

OMX_ERRORTYPE ProcessDecode::AllocateBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes)
{
  LOGV("_ %s _ \nnPortIndex = %u\npAppPrivate = %p\nnSizeBytes = %u", __func__,
       nPortIndex,
       pAppPrivate,
       nSizeBytes
       );

  auto port = GetPort(nPortIndex);

  if(!port)
    return OMX_ErrorBadPortIndex;

  // Create IP device
  // We suppose all settings are done
  auto eRet = SetupIpDevice();

  if(eRet != OMX_ErrorNone)
    return eRet;

  if(port->getDefinition().bPopulated)
    return OMX_ErrorBadParameter;

  if(nSizeBytes != port->getDefinition().nBufferSize)
    return OMX_ErrorBadParameter;

  auto BufferHeader = port->allocateBuffer(nSizeBytes, &m_tUserData);

  if(!BufferHeader)
    return OMX_ErrorInsufficientResources;

  BufferHeaderUpdate(BufferHeader, nPortIndex, pAppPrivate, nSizeBytes);

  *ppBufferHdr = BufferHeader;

  return eRet;
}

ComponentThreadTask* ProcessDecode::ReceiveTask()
{
  return m_ComponentTaskQueue.pop();
}

ProcessThreadTask* ProcessDecode::ReceiveProcessTask()
{
  return m_ProcessTaskQueue.pop();
}

void ProcessDecode::ThreadProcess()
{
  while(true)
  {
    auto ThreadTask = ReceiveProcessTask();
    switch(ThreadTask->cmd)
    {
    case Process:
    {
      assert(m_hDecoder);
      auto pInputBuf = inPort.getBuffer();
      SendToDecoder(pInputBuf);
      delete ThreadTask;
      break;
    }
    case Close:
      delete ThreadTask;
      return;
    }
  }
}

void ProcessDecode::ThreadComponent()
{
  while(true)
  {
    // Blocking call
    auto ThreadTask = ReceiveTask();
    switch(ThreadTask->cmd)
    {
    case ProcessSetComponentState:
    {
      OMX_STATETYPE newState = (OMX_STATETYPE)((uintptr_t)(ThreadTask->data));

      // Delete Decoder
      if((newState == OMX_StateIdle) &&
         (m_state == OMX_StateExecuting))
        DestroyDecoder();

      // Create Decoder
      if((newState == OMX_StateExecuting) &&
         (m_state == OMX_StateIdle))
        CreateDecoder();

      m_state = newState;
      m_pCallback->EventHandler(m_hComponent, m_pAppData, OMX_EventCmdComplete, OMX_CommandStateSet, newState, nullptr);

      break;
    }
    case ProcessFlush:
    {
      auto nPortIndex = (OMX_U32)((uintptr_t)(ThreadTask->data));

      if(!m_bEOSReceived && !m_bForceStop && m_bRunning)
      {
        LOGV("ForceStop %u", (OMX_U32)nPortIndex);
        AL_Decoder_ForceStop(m_hDecoder);
        Rtos_WaitEvent(m_hFinished, AL_WAIT_FOREVER);
        m_bForceStop = true;
      }

      m_pCallback->EventHandler(m_hComponent, m_pAppData, OMX_EventCmdComplete, OMX_CommandFlush, nPortIndex, nullptr);
      break;
    }
    case ProcessStopPort: break;
    case ProcessRestartPort: break;
    case ProcessStop:
    {
      delete ThreadTask;
      return;
    }
    case ProcessProcessInput:
    {
      break;
    }
    case ProcessMarkBuffer:
    {
      LOGV("Mark Buffer");
      break;
    }
    default:
      assert(0);
    }

    delete ThreadTask;
  }
}

void ProcessDecode::DestroyDecoder()
{
  AL_Decoder_Destroy(m_hDecoder);
  DeinitCallBacksParam();
}

void ProcessDecode::DeinitCallBacksParam()
{
}

void ProcessDecode::InitCallBacksParam()
{
  m_tFrameDecodeParam.pProcess = this;
  m_tFrameDisplayParam.pProcess = this;
  m_tResolutionFoundParam.pProcess = this;
}

void ProcessDecode::InitCallBacks()
{
  memset(&m_CallBacks, 0, sizeof(AL_TDecCallBacks));
  InitCallBacksParam();
  m_CallBacks.endDecodingCB.func = &RedirectionFrameDecode;
  m_CallBacks.endDecodingCB.userParam = &m_tFrameDecodeParam;
  m_CallBacks.displayCB.func = &RedirectionFrameDisplay;
  m_CallBacks.displayCB.userParam = &m_tFrameDisplayParam;
  m_CallBacks.resolutionFoundCB.func = &RedirectionResolutionFound;
  m_CallBacks.resolutionFoundCB.userParam = &m_tResolutionFoundParam;
}

OMX_ERRORTYPE ProcessDecode::SetupIpDevice()
{
  std::lock_guard<std::mutex> lock(m_MutexIpDevice);

  if(!m_IpDevice)
  {
    m_IpDevice = CreateIpDevice(m_bUseVCU, m_iSchedulerType, m_eSettings.eDecUnit);
    LOGI("Create IP Device");

    if(!m_IpDevice)
    {
      LOGE("Failed to create IP Device");
      return OMX_ErrorUndefined;
    }
    else
    {
      inPort.createAllocator(m_IpDevice->m_pAllocator, m_bUseInputFileDescriptor, &RedirectionSourceBufferIsDone);
      outPort.createAllocator(m_IpDevice->m_pAllocator, m_bUseOutputFileDescriptor, &RedirectionStreamBufferIsDone);
    }
  }
  return OMX_ErrorNone;
}

void ProcessDecode::CreateDecoder()
{
  // Set Decoder Settings

  if(SetupIpDevice() != OMX_ErrorNone)
    throw std::runtime_error("Failed to create IpDevice");

  SetSettings(m_eSettings);

  LOGI("\nCreate Decoder : \n\tStack size : %i\n\tBitDepth : %i\n\tFramerate : %u\n\tClockRatio : %u\n\tCodec : %s",
       m_eSettings.iStackSize,
       m_eSettings.iBitDepth,
       m_eSettings.uFrameRate,
       m_eSettings.uClkRatio,
       (m_eSettings.bIsAvc) ? "AVC" : "HEVC"
       );

  AL_Decoder_Create(&m_hDecoder, m_IpDevice->m_pScheduler, m_IpDevice->m_pAllocator, &m_eSettings, &m_CallBacks);

  if(!m_hDecoder)
    throw std::runtime_error("Can't create AL_Decoder");
}

void ProcessDecode::UpdateDPB()
{
  int const iDPBBufferNeeded = outPort.getDefinition().nBufferCountActual - 1;

  if(m_iDPBBuffersCount < iDPBBufferNeeded)
  {
    m_iDPBBuffersCount++;

    if(m_iDPBBuffersCount == iDPBBufferNeeded)
    {
      m_bIsDPBFull = true;
      m_SemaphoreDPBisFilled.notify();
    }
  }
}

void ProcessDecode::AddDPBMetaData(OMX_BUFFERHEADERTYPE* pBufferHeader)
{
  auto const pBuffer = static_cast<AL_TBuffer*>(pBufferHeader->pPlatformPrivate);
  assert(pBuffer);
  auto const videoDef = outPort.getVideoDefinition();
  AL_TPitches tPitches =
  {
    0, 0
  };
  AL_TOffsetYC tOffsetYC =
  {
    0, 0
  };
  auto const pSrcMeta = (AL_TMetaData*)AL_SrcMetaData_Create(videoDef.nFrameWidth, videoDef.nFrameHeight, tPitches, tOffsetYC, m_tFourCC);
  assert(pSrcMeta);
  AL_Buffer_AddMetaData(pBuffer, pSrcMeta);

  auto const pOMXMeta = (AL_TMetaData*)AL_OMXMetaData_Create(pBufferHeader);
  assert(pOMXMeta);
  AL_Buffer_AddMetaData(pBuffer, pOMXMeta);
}

OMX_ERRORTYPE ProcessDecode::FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  assert(pBufferHdr->nFilledLen == 0);

  if(!m_pEOSHeader)
  {
    m_pEOSHeader = pBufferHdr;
    return OMX_ErrorNone;
  }

  auto const pEOSBuffer = static_cast<AL_TBuffer*>(m_pEOSHeader->pPlatformPrivate);
  auto const pBuffer = static_cast<AL_TBuffer*>(pBufferHdr->pPlatformPrivate);
  assert(pBuffer);

  if(pBuffer == pEOSBuffer)
    return OMX_ErrorNone;

  if(m_bIsDPBFull)
    AL_Decoder_ReleaseDecPict(m_hDecoder, pBuffer);
  else
  {
    AddDPBMetaData(pBufferHdr);
    outPort.putBuffer(pBufferHdr);
    UpdateDPB();
  }

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessDecode::EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  auto ProcessTask = new ProcessThreadTask;
  ProcessTask->cmd = Process;
  inPort.putBuffer(pBufferHdr);
  SendProcessTask(ProcessTask);

  return OMX_ErrorNone;
}

void ProcessDecode::SetSettings(AL_TDecSettings& settings)
{
  auto framerate = inPort.getVideoDefinition().xFramerate;
  auto const f = std::ceil(framerate / 65536.0);
  auto const clkRatio = std::rint((f * 1000.0 * 65536.0) / framerate);


  if(!strncmp("avc", m_pCodec->GetRole(), OMX_MAX_STRINGNAME_SIZE))
    settings.bIsAvc = true;

  settings.iBitDepth = HW_IP_BIT_DEPTH;
  settings.uFrameRate = f;
  settings.uClkRatio = clkRatio;

  if(m_eSettings.iStackSize < 1)
  {
    LOGI("Set Buffering to 5");
    settings.iStackSize = 5;
  }
}

static inline std::string FourCCToString(TFourCC tFourCC)
{
  std::stringstream ss;
  ss << static_cast<char>(tFourCC & 0xFF) << static_cast<char>((tFourCC & 0xFF00) >> 8) << static_cast<char>((tFourCC & 0xFF0000) >> 16) << static_cast<char>((tFourCC & 0xFF000000) >> 24);
  return ss.str();
}

static void DisplayResolution(int const iWidth, int const iHeight, AL_TCropInfo const tCropInfo, TFourCC const tFourCC, int const iLatency)
{
  int const uCropWidth = tCropInfo.uCropOffsetLeft + tCropInfo.uCropOffsetRight;
  int const uCropHeight = tCropInfo.uCropOffsetTop + tCropInfo.uCropOffsetBottom;
  auto const tFourCCInString = FourCCToString(tFourCC).c_str();

  LOGI("\n"
       "Input Resolution   : %i x %i\n"
       "Crop top     : %u\n"
       "Crop bottom  : %u\n"
       "Crop left    : %u\n"
       "Crop right   : %u\n"
       "Display Resolution : %i x %i\n"
       "FourCC : %s\n"
       "Latency : %u ms",
       iWidth, iHeight,
       tCropInfo.uCropOffsetTop,
       tCropInfo.uCropOffsetBottom,
       tCropInfo.uCropOffsetLeft,
       tCropInfo.uCropOffsetRight,
       iWidth - uCropWidth, iHeight - uCropHeight,
       tFourCCInString,
       iLatency
       );
}

static void DisplayBuffersNeeded(unsigned int const iBuffersNeeded, size_t const zBufferSize)
{
  LOGI("\n"
       "Buffer needed: %u\n"
       "Buffer size  : %zu",
       iBuffersNeeded,
       zBufferSize
       );
}

void ProcessDecode::ResolutionFound(int const iBuffersNeeded, int const zBufferSize, int const iWidth, int const iHeight, AL_TCropInfo const tCropInfo, TFourCC const tFourCC)
{
  std::lock_guard<std::mutex> lock(m_MutexResolutionFound);
  m_tFourCC = tFourCC;
  auto iTotalBuffersNeeded = iBuffersNeeded + 1;

  DisplayResolution(iWidth, iHeight, tCropInfo, m_tFourCC, ComputeLatency());
  DisplayBuffersNeeded(iTotalBuffersNeeded, zBufferSize);

  auto& def = outPort.getDefinition();
  def.nBufferCountMin = iTotalBuffersNeeded;
  def.nBufferCountActual = iTotalBuffersNeeded;
  def.nBufferSize = zBufferSize;
  auto& videoDef = outPort.getVideoDefinition();
  videoDef.eColorFormat = getColorFormat(tFourCC);
  videoDef.nStride = RndPitch(iWidth, getBitDepth(videoDef.eColorFormat));
  videoDef.nSliceHeight = RndHeight(iHeight);

  int const iCropWidth = tCropInfo.uCropOffsetLeft + tCropInfo.uCropOffsetRight;
  int const iCropHeight = tCropInfo.uCropOffsetTop + tCropInfo.uCropOffsetBottom;
  videoDef.nFrameWidth = iWidth - iCropWidth;
  videoDef.nFrameHeight = iHeight - iCropHeight;

  m_pCallback->EventHandler(m_hComponent, m_pAppData, OMX_EventPortSettingsChanged, 1, 0, nullptr);

  m_SemaphoreDPBisFilled.wait();

  for(auto i = 0; i < m_iDPBBuffersCount; ++i)
  {
    auto pBufferHeader = outPort.getBuffer();
    assert(pBufferHeader);
    auto pDPBBuffer = static_cast<AL_TBuffer*>(pBufferHeader->pPlatformPrivate);
    assert(pDPBBuffer);
    AL_Decoder_PutDecPict(m_hDecoder, pDPBBuffer);
  }

  LOGV("END Resolution");
  m_bRunning = true;
}

void ProcessDecode::SetPropagatingData(OMX_BUFFERHEADERTYPE* const pBufferHeader)
{
  if(m_PropagatingDataQueue.size() > 0)
  {
    auto PropData = m_PropagatingDataQueue.front();
    pBufferHeader->hMarkTargetComponent = PropData->hMarkTargetComponent;
    pBufferHeader->pMarkData = PropData->pMarkData;
    pBufferHeader->nTimeStamp = PropData->nTimeStamp;
    pBufferHeader->nFlags = PropData->nFlags & ~OMX_BUFFERFLAG_EOS;
    m_PropagatingDataQueue.pop();
    delete PropData;
  }
}

OMX_BUFFERHEADERTYPE* ProcessDecode::GetOMXBufferHeader(AL_TBuffer const* const pBuf)
{
  auto tOMXMeta = (AL_TOMXMetaData*)AL_Buffer_GetMetaData(pBuf, AL_META_TYPE_OMX);
  assert(tOMXMeta);

  auto pOMXBufferHeader = tOMXMeta->pBufHdr;
  assert(pOMXBufferHeader);

  return pOMXBufferHeader;
}

void ProcessDecode::FrameDecode(AL_TBuffer* pDecodedFrame)
{
  std::lock_guard<std::mutex> lock(m_MutexDecode);
  LOGV("Decoded picture %i", m_iFramesDecoded);
  m_iFramesDecoded++;

  auto pOMXBufferHeader = GetOMXBufferHeader(pDecodedFrame);
  SetPropagatingData(pOMXBufferHeader);
}

void ProcessDecode::FrameDisplay(AL_TBuffer* pDisplayedFrame, AL_TInfoDecode const)
{
  std::lock_guard<std::mutex> lock(m_MutexDisplay);

  if(!pDisplayedFrame)
  {
    LOGI("Complete");
    Rtos_SetEvent(m_hFinished);
    m_pEOSHeader->nFilledLen = 0;
    m_pEOSHeader->nFlags = OMX_BUFFERFLAG_EOS;
    m_pCallback->FillBufferDone(m_hComponent, m_pAppData, m_pEOSHeader);
    m_pCallback->EventHandler(m_hComponent, m_pAppData, OMX_EventBufferFlag, outPort.getDefinition().nPortIndex, OMX_BUFFERFLAG_EOS, nullptr);
    return;
  }
  LOGV("Displayed picture %i", m_iFramesDisplayed);
  m_iFramesDisplayed++;
  auto pOutputBuf = GetOMXBufferHeader(pDisplayedFrame);
  assert(pOutputBuf);
  auto const videoParam = outPort.getVideoDefinition();
  pOutputBuf->nFilledLen = AL_GetAllocSize_Frame({ (int)videoParam.nFrameWidth, (int)videoParam.nFrameHeight }, AL_GetChromaMode(m_tFourCC), AL_GetBitDepth(m_tFourCC), false, AL_FB_RASTER);

  if(isCopyNeeded(outPort.useFileDescriptor()))
    copy((char*)AL_Buffer_GetData(pDisplayedFrame), 0, (char*)pOutputBuf->pBuffer, pOutputBuf->nOffset, pDisplayedFrame->zSize);

  m_pCallback->FillBufferDone(m_hComponent, m_pAppData, pOutputBuf);
}

void ProcessDecode::SendToDecoder(OMX_BUFFERHEADERTYPE* pInputBuf)
{
  auto pBufStream = static_cast<AL_TBuffer*>(pInputBuf->pPlatformPrivate);

  assert(pBufStream);

  auto PropData = new PropagatingData
  {
    pInputBuf->hMarkTargetComponent,
    pInputBuf->pMarkData,
    pInputBuf->nTimeStamp,
    pInputBuf->nFlags,
  };

  m_PropagatingDataQueue.push(PropData);

  if(isCopyNeeded(inPort.useFileDescriptor()))
    copy((char*)pInputBuf->pBuffer, pInputBuf->nOffset, (char*)AL_Buffer_GetData(pBufStream), 0, pInputBuf->nFilledLen);

  AL_Buffer_Ref(pBufStream);
  auto bRet = AL_Decoder_PushBuffer(m_hDecoder, pBufStream, pInputBuf->nFilledLen, AL_BUF_MODE_BLOCK);
  assert(bRet);
  LOGV("Frame %i send", m_iFramesSend);
  ++m_iFramesSend;
  AL_Buffer_Unref(pBufStream);

  if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
  {
    LOGI("Flush");
    m_bEOSReceived = true;
    AL_Decoder_Flush(m_hDecoder);
  }
}

OMX_ERRORTYPE ProcessDecode::GetExtensionIndex(OMX_IN OMX_STRING cParameterName, OMX_OUT OMX_INDEXTYPE* pIndexType)
{
  auto eRet = OMX_ErrorNoMore;
  unsigned int index = 0;

  while(index < ARRAYSIZE(AL_CustomParam))
  {
    if(!strncmp((OMX_STRING)AL_CustomParam[index].cName, cParameterName, OMX_MAX_STRINGNAME_SIZE))
    {
      *pIndexType = static_cast<OMX_INDEXTYPE>(AL_CustomParam[index].nIndexType);
      return OMX_ErrorNone;
    }
    index++;
  }

  return eRet;
}

OMX_ERRORTYPE ProcessDecode::ComponentTunnelRequest(OMX_IN OMX_U32 nPort, OMX_IN OMX_HANDLETYPE hTunneledComp, OMX_IN OMX_U32 nTunneledPort, OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
  auto eRet = OMX_ErrorNone;

  if(nPort == inPort.getDefinition().nPortIndex)
  {
    if(pTunnelSetup->eSupplier != inPort.getSupplier().eBufferSupplier)
    {
      OMX_PARAM_BUFFERSUPPLIERTYPE OutputPortTunneled;
      OMXChecker::SetHeaderVersion(OutputPortTunneled);
      OutputPortTunneled.nPortIndex = nTunneledPort;
      eRet = OMX_GetParameter(hTunneledComp, OMX_IndexParamCompBufferSupplier, &OutputPortTunneled);

      if(eRet != OMX_ErrorNone)
        return eRet;
      OutputPortTunneled.eBufferSupplier = inPort.getSupplier().eBufferSupplier;
      eRet = OMX_SetParameter(hTunneledComp, OMX_IndexParamCompBufferSupplier, &OutputPortTunneled);

      if(eRet != OMX_ErrorNone)
        eRet = OMX_ErrorPortsNotCompatible;
    }
  }
  else if(nPort == outPort.getDefinition().nPortIndex)
    pTunnelSetup->eSupplier = outPort.getSupplier().eBufferSupplier;
  else
    eRet = OMX_ErrorBadPortIndex;
  return eRet;
}

void ProcessDecode::SourceBufferIsDone(OMX_BUFFERHEADERTYPE* pBufHdr)
{
  pBufHdr->nFilledLen = 0;
  m_pCallback->EmptyBufferDone(m_hComponent, m_pAppData, pBufHdr);
}

void ProcessDecode::StreamBufferIsDone(OMX_BUFFERHEADERTYPE* pBufHdr)
{
  m_pCallback->FillBufferDone(m_hComponent, m_pAppData, pBufHdr);
}

OMX_U32 inline ProcessDecode::ComputeLatency()
{
  auto const videoDef = inPort.getVideoDefinition();
  double bufsCount = 0;

  if(m_eSettings.eDpbMode == AL_DPB_LOW_REF)
    bufsCount = 1;
  else
    bufsCount = m_eSettings.iStackSize + m_pCodec->DPBSize(videoDef.nFrameWidth, videoDef.nFrameHeight, (OMX_ALG_EDpbMode)m_eSettings.eDpbMode);
  auto const f = std::ceil(videoDef.xFramerate / 65536.0);
  return std::ceil((bufsCount * 1000.0) / f);
}

