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

#include "omx_enc_process.h"
#include <assert.h>

#include <OMX_ComponentExt.h>
#include <OMX_VideoExt.h>

#include "base/omx_utils/omx_log.h"
#include "base/omx_utils/omx_setup.h"
#include "base/omx_buffer/omx_buffer_meta.h"
#include "omx_schedulertype.h"

extern "C"
{
#include "lib_encode/lib_encoder.h"

#include "lib_common/BufferStreamMeta.h"
#include "lib_common/BufferSrcMeta.h"
#include "lib_common_enc/EncBuffers.h"
#include "lib_common_enc/IpEncFourCC.h"
#include "lib_fpga/DmaAllocLinux.h"
}

static int RoundUp(int iVal, int iRnd)
{
  return (iVal + iRnd - 1) & (~(iRnd - 1));
}

#define CUSTOMPARAM_GOP_CONTROL "OMX.allegro.encoder.gopControl"
#define CUSTOMPARAM_CHANNEL "OMX.allegro.encoder.channel"

static CustomParam AL_CustomParam[] =
{
  { CUSTOMPARAM_BOARD, OMX_IndexParamBoard },
  { CUSTOMPARAM_BUFFERMODE, OMX_IndexPortParamBufferMode },
  { CUSTOMPARAM_GOP_CONTROL, OMX_IndexParamVideoGopControl },
  { CUSTOMPARAM_CHANNEL, OMX_IndexParamVideoEncoderChannel },
};

static void ReplaceMetaData(AL_TBuffer* pBuf, AL_TMetaData* pMeta, AL_EMetaType eType)
{
  AL_TMetaData* pOldMeta = AL_Buffer_GetMetaData(pBuf, eType);

  if(pOldMeta)
  {
    AL_Buffer_RemoveMetaData(pBuf, pOldMeta);
    pOldMeta->MetaDestroy(pOldMeta);
  }

  AL_Buffer_AddMetaData(pBuf, pMeta);
}

static OMX_BUFFERHEADERTYPE* GetOMXBufferHeader(AL_TBuffer const* const pBuf)
{
  auto tOMXMeta = (AL_TOMXMetaData*)AL_Buffer_GetMetaData(pBuf, AL_META_TYPE_OMX);
  assert(tOMXMeta);

  auto pOMXBufferHeader = tOMXMeta->pBufHdr;
  assert(pOMXBufferHeader);

  return pOMXBufferHeader;
}

static void AppendBuffer(uint8_t*& dst, uint8_t const* src, size_t len)
{
  memmove(dst, src, len);
  dst += len;
}

static void WriteOneSection(uint8_t*& dst, AL_TBuffer* bitstream, int iSection)
{
  AL_TStreamMetaData* pStreamMeta = (AL_TStreamMetaData*)AL_Buffer_GetMetaData(bitstream, AL_META_TYPE_STREAM);

  if((pStreamMeta->pSections[iSection]).uLength)
  {
    auto const uRemSize = bitstream->zSize - (pStreamMeta->pSections[iSection]).uOffset;

    if(uRemSize < (pStreamMeta->pSections[iSection]).uLength)
    {
      AppendBuffer(dst, (AL_Buffer_GetBufferData(bitstream) + (pStreamMeta->pSections[iSection]).uOffset), uRemSize);
      AppendBuffer(dst, AL_Buffer_GetBufferData(bitstream), (pStreamMeta->pSections[iSection]).uLength - uRemSize);
    }
    else
    {
      AppendBuffer(dst, (AL_Buffer_GetBufferData(bitstream) + (pStreamMeta->pSections[iSection]).uOffset), (pStreamMeta->pSections[iSection]).uLength);
    }
  }
}

static void WriteStream(OMX_BUFFERHEADERTYPE* pOutputBuf, AL_TBuffer* bitstream)
{
  auto const pBitstreamOrigData = AL_Buffer_GetBufferData(bitstream);
  auto pBitstreamIndexData = pBitstreamOrigData;
  int iNumSectionWritten = 0;
  static unsigned int uNumFrame = 0;
  unsigned i = 0; // For now we skip filler

  AL_TStreamMetaData* pStreamMeta = (AL_TStreamMetaData*)AL_Buffer_GetMetaData(bitstream, AL_META_TYPE_STREAM);

  LOGV("Frame (%u)", uNumFrame);

  while(i < pStreamMeta->uNumSection && ((pStreamMeta->pSections[i]).uFlags & SECTION_COMPLETE_FLAG))
  {
    if((pStreamMeta->pSections[i]).uFlags & SECTION_END_FRAME_FLAG)
    {
      pOutputBuf->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
      ++uNumFrame;
    }

    if((pStreamMeta->pSections[i]).uFlags & SECTION_SYNC_FLAG)
    {
      pOutputBuf->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
    }
    {
      auto start = pBitstreamIndexData;
      auto end = start;

      WriteOneSection(end, bitstream, i);
      pOutputBuf->nFilledLen += (OMX_U32)(end - start);
      LOGV("Section[%u] -- Length : %lu -- Flags : %.8X", i, (OMX_U32)(end - start), (pStreamMeta->pSections[i]).uFlags);
      pBitstreamIndexData = end;
      ++iNumSectionWritten;
      ++i;
    }
  }
}

static inline bool is422Format(OMX_COLOR_FORMATTYPE format)
{
  OMX_U32 extendedFormat = format;
  return extendedFormat == OMX_COLOR_FormatYUV422SemiPlanar || extendedFormat == OMX_COLOR_FormatYUV422SemiPlanar10bitPacked;
}

void RedirectionFrameEncode(void* pUserParam, AL_TBuffer* pStream, AL_TBuffer const* const pSrc)
{
  auto encodeParam = static_cast<EncodeParam*>(pUserParam);

  assert(encodeParam);

  auto processEncode = static_cast<ProcessEncode*>(encodeParam->pProcess);

  assert(processEncode);

  processEncode->FrameEncode(pStream, pSrc);
}

void ProcessEncode::FrameEncode(AL_TBuffer* pStream, AL_TBuffer const* const pSrc)
{
  if(!pStream)
  {
    OMX_BUFFERHEADERTYPE* out = nullptr;

    while(1)
    {
      out = outPort.getBuffer();

      if(!out)
        continue;

      if(map.Exist(out))
        break;
    }

    map.Remove(out);
    out->nFilledLen = 0;
    out->nFlags = m_EOSFlags;

    m_pCallback->FillBufferDone(m_hComponent, m_pAppData, out);

    FlushEncoder(m_EOSFlags);
    return;
  }

  auto pHeader = GetOMXBufferHeader(pStream);
  WriteStream(pHeader, pStream);

  auto const pHeaderSrc = GetOMXBufferHeader(pSrc);

  pHeader->hMarkTargetComponent = pHeaderSrc->hMarkTargetComponent;
  pHeader->pMarkData = pHeaderSrc->pMarkData;
  pHeader->nTimeStamp = pHeaderSrc->nTimeStamp;
  pHeader->nFlags = pHeaderSrc->nFlags;

  // UseBuffer when (AL_TBuffer*)pPlatformPrivate->pData != pBuffer;
  if(!outPort.useFileDescriptor())
  {
    if(AL_Buffer_GetBufferData(pStream) != pHeader->pBuffer)
      memcpy(pHeader->pBuffer, (AL_Buffer_GetBufferData(pStream) + pHeader->nOffset), pHeader->nFilledLen);
  }

  auto const pHeaderFlags = pHeader->nFlags;

  map.Remove(pHeader);
  AL_Buffer_Unref(pStream);

  if(pHeaderFlags & OMX_BUFFERFLAG_EOS)
    FlushEncoder(pHeaderFlags);
}

void RedirectionStreamBufferIsDone(AL_TBuffer* pBuf)
{
  auto const pUserData = static_cast<EncBufferUserData*>(AL_Buffer_GetUserData(pBuf));

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

  auto const pHeader = GetOMXBufferHeader(pBuf);

  if(!pHeader)
  {
    LOGE("No OMX header found");
    return;
  }

  pThis->StreamBufferIsDone(pHeader);
}

void RedirectionSourceBufferIsDone(AL_TBuffer* pBuf)
{
  auto const pUserData = static_cast<EncBufferUserData*>(AL_Buffer_GetUserData(pBuf));

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

  auto const pHeader = GetOMXBufferHeader(pBuf);

  if(!pHeader)
  {
    LOGE("No OMX header found");
    return;
  }

  pThis->SourceBufferIsDone(pHeader);
}

OMX_ERRORTYPE ProcessEncode::SetupIpDevice()
{
  std::lock_guard<std::mutex> lock(m_MutexIpDevice);

  if(!m_IpDevice)
  {
    // Update Info & Encoder Settings;
    SetEncoderSettings();

    if((AL_Settings_CheckValidity(&m_EncSettings, stderr)))
    {
      LOGE("Encoder settings are not valid !!");
      return OMX_ErrorUndefined;
    }

    AL_Settings_CheckCoherency(&m_EncSettings, m_tFourCC, stderr);

    m_IpDevice = CreateIpDevice(m_bUseVCU, m_iSchedulerType, m_EncSettings);

    LOGI("Create IP Device");

    if(!m_IpDevice)
    {
      LOGE("Failed to create IP Device");
      return OMX_ErrorUndefined;
    }

    inPort.createAllocator(m_IpDevice->m_pAllocator, m_bUseInputFileDescriptor, &RedirectionSourceBufferIsDone);
    outPort.createAllocator(m_IpDevice->m_pAllocator, m_bUseOutputFileDescriptor, &RedirectionStreamBufferIsDone);
  }
  return OMX_ErrorNone;
}

ProcessEncode::ProcessEncode(OMX_HANDLETYPE hComponent, CodecType* pCodec)
{
  assert(hComponent);
  assert(pCodec);

  m_hComponent = hComponent;
  m_pCodec = pCodec;

  m_hEncoder = NULL;
  m_EOSFlags = 0;
  m_bUseVCU = AL_USE_VCU;
  m_bUseInputFileDescriptor = false;
  m_bUseOutputFileDescriptor = false;
  m_tUserData =
  {
    this
  };

  m_iSchedulerType = AL_USE_MCU ? SCHEDULER_TYPE_MCU : SCHEDULER_TYPE_CPU;

  SetStandardPortParameter();
  SetStandardInParameterPortDefinition();
  SetStandardOutParameterPortDefinition();
  SetStandardYUVVideoPortDefinition();
  SetStandardVideoPortDefinition();
  SetStandardInParameterBufferSupplier();
  SetStandardOutParameterBufferSupplier();
  ResetEncodingParameters();

  m_ThreadComponent = std::thread(&ProcessEncode::ThreadComponent, this);

  m_state = OMX_StateLoaded;
}

void ProcessEncode::ComponentDeInit()
{
  LOGV("_ %s _ ", __func__);
  auto StopTask = new ComponentThreadTask;

  SendTask(StopTask);
  m_ThreadComponent.join();

  delete m_pCodec;
}

ProcessEncode::~ProcessEncode()
{
}

OMX_STATETYPE ProcessEncode::GetState()
{
  std::lock_guard<std::mutex> lock(m_MutexState);
  return m_state;
}

OMX_STRING ProcessEncode::GetType()
{
  return (OMX_STRING)"encoder";
}

OMX_STRING ProcessEncode::GetRole()
{
  return (OMX_STRING)"video_encoder";
}

void ProcessEncode::SetCallBack(OMX_CALLBACKTYPE* pCallback)
{
  LOGV("_ %s _ ", __func__);
  m_pCallback = pCallback;
}

void ProcessEncode::SetAppData(OMX_PTR pAppData)
{
  LOGV("_ %s _ ", __func__);
  m_pAppData = pAppData;
}

OMX_U32 inline ProcessEncode::ComputeLatency()
{
  auto const b = (m_pCodec->GetCodecBFrames() / (m_pCodec->GetCodecPFrames() + 1));
  return b + 2;
}

OMX_ERRORTYPE ProcessEncode::GetParameter(OMX_IN OMX_INDEXTYPE nParamIndex, OMX_INOUT OMX_PTR pParam)
{
  if(!pParam && (*((OMX_U32*)pParam) / sizeof(pParam) < 2))
    return OMX_ErrorBadParameter;

  auto eRet = OMXChecker::CheckHeaderVersion(*(((OMX_VERSIONTYPE*)pParam) + 1));

  if(eRet != OMX_ErrorNone)
    return eRet;

  auto const nIndex = static_cast<OMX_U32>(nParamIndex);

  if(nIndex == OMX_IndexParamVideoInit)
  {
    LOGV("_ %s _ : OMX_IndexParamVideoInit", __func__);
    auto port = (OMX_PORT_PARAM_TYPE*)pParam;
    *port = GetPortParameter();
    return OMX_ErrorNone;
  }
  else if(nIndex == OMX_IndexParamLatency)
  {
    LOGV("_ %s _: OMX_IndexParamLatency", __func__);
    auto port = (OMX_PARAM_LATENCY*)pParam;
    port->nBuffersLatency = ComputeLatency();
    return OMX_ErrorNone;
  }

  auto const nPortIndex = *(((OMX_U32*)pParam) + 2);
  auto port = GetPort(nPortIndex);

  if(!port)
    return OMX_ErrorBadPortIndex;
  switch(nIndex)
  {
  case OMX_IndexParamPortDefinition:
  {
    LOGV("_ %s _ : OMX_IndexParamPortDefinition (%lu)", __func__, nPortIndex);
    auto param = (OMX_PARAM_PORTDEFINITIONTYPE*)pParam;
    *param = port->getDefinition();
    break;
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    LOGV("_ %s _ : OMX_IndexParamCompBufferSupplier (%lu)", __func__, nPortIndex);
    auto param = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParam;
    *param = port->getSupplier();
    break;
  }
  case OMX_IndexParamVideoPortFormat:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoPortFormat (%lu)", __func__, nPortIndex);
    auto param = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)pParam;

    if(param->nIndex == 0)
      *param = port->getVideoFormat();
    else
      eRet = OMX_ErrorNoMore;
    break;
  }
  case OMX_IndexParamVideoProfileLevelQuerySupported:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoProfileLevelQuerySupported (%lu)", __func__, nPortIndex);
    auto param = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)pParam;

    if(param->nPortIndex == inPort.getDefinition().nPortIndex)
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
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoProfileLevelCurrent (%lu)", __func__, nPortIndex);
    auto param = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)pParam;

    if(param->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = OMX_ErrorBadPortIndex;
    else
    {
      OMX_VIDEO_PARAM_PROFILELEVELTYPE profLvl;
      OMXChecker::SetHeaderVersion(profLvl);
      profLvl.nPortIndex = param->nPortIndex;
      profLvl.eProfile = m_pCodec->ConvertProfile(m_VideoParameters.getProfile());
      profLvl.eLevel = m_pCodec->ConvertLevel(m_VideoParameters.getLevel());
      // profLvl.nProfileIndex is ignored here
      *param = profLvl;
    }
    break;
  }
  case OMX_IndexParamVideoQuantization:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoQuantization (%lu)", __func__, nPortIndex);
    auto param = (OMX_VIDEO_PARAM_QUANTIZATIONTYPE*)pParam;

    if(param->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = OMX_ErrorBadPortIndex;
    else
    {
      OMX_VIDEO_PARAM_QUANTIZATIONTYPE quant;
      OMXChecker::SetHeaderVersion(quant);
      quant.nPortIndex = param->nPortIndex;
      auto const qp = m_VideoParameters.getQuantization();
      quant.nQpI = qp.initialQP;
      quant.nQpP = qp.initialQP + qp.IPDeltaQP;
      quant.nQpB = qp.initialQP + qp.PBDeltaQP;
      *param = quant;
    }
    break;
  }
  case OMX_IndexParamVideoBitrate:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoBitrate (%lu)", __func__, nPortIndex);
    auto param = (OMX_VIDEO_PARAM_BITRATETYPE*)pParam;

    if(param->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = OMX_ErrorBadPortIndex;
    else
    {
      OMX_VIDEO_PARAM_BITRATETYPE bitrate;
      OMXChecker::SetHeaderVersion(bitrate);
      bitrate.nPortIndex = param->nPortIndex;
      bitrate.nTargetBitrate = m_VideoParameters.getBitrate();
      bitrate.eControlRate = ConvertRCMode(m_VideoParameters.getRCMode());
      *param = bitrate;
    }
    break;
  }

  default:
  {
    LOGV("_ %s _ : default", __func__);

    if(m_pCodec->CheckIndexParamVideoCodec(nParamIndex))
      eRet = m_pCodec->GetIndexParamVideoCodec(pParam, outPort.getDefinition());
    else
      eRet = OMX_ErrorUnsupportedIndex;
  }
  }

  return eRet;
}

OMX_ERRORTYPE ProcessEncode::SetParameter(OMX_IN OMX_INDEXTYPE nIndex, OMX_IN OMX_PTR pParam)
{
  if(!pParam && (*((OMX_U32*)pParam) / sizeof(pParam) < 2))
    return OMX_ErrorBadParameter;

  auto eRet = OMXChecker::CheckHeaderVersion(*(((OMX_VERSIONTYPE*)pParam) + 1));

  if(eRet != OMX_ErrorNone)
    return eRet;

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

  if(nIndex == (OMX_U32)OMX_IndexParamBoard)
  {
    LOGV("_ %s _ : OMX_IndexParamBoard", __func__);
    auto port = (OMX_PARAM_BOARD*)pParam;
    switch(port->eMode)
    {
    case OMX_HW_MCU:
    {
      LOGV("Use MCU");
      m_bUseVCU = true;
      m_iSchedulerType = SCHEDULER_TYPE_MCU;
      break;
    }
    default:
      return OMX_ErrorBadParameter;
    }

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
    LOGV("_ %s _ : OMX_IndexParamPortDefinition (%lu)", __func__, nPortIndex);
    auto port = (OMX_PARAM_PORTDEFINITIONTYPE*)pParam;

    if(port->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = SetInParameterPortDefinition(*port);
    else
      eRet = SetOutParameterPortDefinition(*port);
    break;
  }
  case OMX_IndexParamCompBufferSupplier:
  {
    LOGV("_ %s _ : OMX_IndexParamCompBufferSupplier (%lu)", __func__, nPortIndex);
    auto param = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParam;
    eRet = port->setSupplier(*param);
    break;
  }
  case OMX_IndexParamVideoPortFormat:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoPortFormat (%lu)", __func__, nPortIndex);
    auto port = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)pParam;

    if(port->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = SetYUVVideoParameterPortFormat(*port);
    else
      eRet = SetVideoParameterPortFormat(*port);
    break;
  }
  case OMX_IndexParamVideoProfileLevelCurrent:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoProfileLevelCurrent (%lu)", __func__, nPortIndex);
    auto port = (OMX_VIDEO_PARAM_PROFILELEVELTYPE*)pParam;

    if(port->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = OMX_ErrorBadPortIndex;
    else
      eRet = SetVideoParameterProfileLevel(*port);
    break;
  }
  case OMX_IndexParamVideoQuantization:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoQuantization (%lu)", __func__, nPortIndex);
    auto port = (OMX_VIDEO_PARAM_QUANTIZATIONTYPE*)pParam;

    if(port->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = OMX_ErrorBadPortIndex;
    else
    {
      int const initialQP = port->nQpI;
      int const IPDeltaQP = port->nQpP - port->nQpI;
      int const PBDeltaQP = port->nQpB - port->nQpP;
      Quantization quant =
      {
        initialQP, IPDeltaQP, PBDeltaQP
      };

      m_VideoParameters.setQuantization(quant);
    }
    break;
  }
  case OMX_IndexParamVideoBitrate:
  {
    LOGV("_ %s _ : OMX_IndexParamVideoBitrate (%lu)", __func__, nPortIndex);
    auto port = (OMX_VIDEO_PARAM_BITRATETYPE*)pParam;

    if(port->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = OMX_ErrorBadPortIndex;
    else
      eRet = SetVideoParameterRateControl(*port);
    break;
  }

  case OMX_IndexPortParamBufferMode:
  {
    auto bufMode = (OMX_PORT_PARAM_BUFFERMODE*)pParam;

    if(bufMode->nPortIndex == inPort.getDefinition().nPortIndex)
      m_bUseInputFileDescriptor = (bufMode->eMode == OMX_BUF_DMA);
    else
      m_bUseOutputFileDescriptor = (bufMode->eMode == OMX_BUF_DMA);
    break;
  }
  case OMX_IndexParamVideoGopControl:
  {
    auto gop = (OMX_VIDEO_PARAM_GOPCONTROL*)pParam;

    if(gop->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = OMX_ErrorBadPortIndex;
    else
    {
      m_VideoParameters.setGopMode(gop->eGopControlMode);
      m_VideoParameters.setGdrMode(gop->eGdrMode);
    }
    break;
  }
  case OMX_IndexParamVideoEncoderChannel:
  {
    auto chan = (OMX_VIDEO_PARAM_ENCODER_CHANNEL*)pParam;

    if(chan->nPortIndex == inPort.getDefinition().nPortIndex)
      eRet = OMX_ErrorBadPortIndex;
    else
    {
      m_VideoParameters.setNumSlices(chan->nNumSlices);
      m_VideoParameters.setL2CacheSize(chan->nL2CacheSize);
      m_VideoParameters.setQpMode(chan->eQpControlMode);
      m_VideoParameters.setCPBSize(chan->nCodedPictureBufferSize);
      m_VideoParameters.setInitialRemovalDelay(chan->nInitialRemovalDelay);
      m_VideoParameters.setScalingListMode(chan->eScalingListMode);

      if(chan->bDisableSceneChangeVersatility)
        m_VideoParameters.setRCOptions(AL_RC_OPT_NONE);
    }
    break;
  }

  default:
  {
    LOGV("_ %s _ : default", __func__);

    if(m_pCodec->CheckIndexParamVideoCodec(nIndex))
      eRet = m_pCodec->SetIndexParamVideoCodec(pParam, outPort.getDefinition(), const_cast<OMX_PARAM_PORTDEFINITIONTYPE &>(inPort.getDefinition()));
    else
      eRet = OMX_ErrorUnsupportedIndex;
  }
  }

  return eRet;
}

void ProcessEncode::SetStandardPortParameter()
{
  OMXChecker::SetHeaderVersion(m_PortParameter);
  m_PortParameter.nPorts = 0x2;
  m_PortParameter.nStartPortNumber = 0x0;
}

OMX_U32 ProcessEncode::GetSizeYUV(OMX_U32 stride, OMX_U32 height, OMX_COLOR_FORMATTYPE format)
{
  auto size = stride * height;
  auto is422 = is422Format(format);

  if(is422)
    size *= 2;
  else
  {
    size *= 3;
    size /= 2;
  }

  return size;
}

void ProcessEncode::SetStandardInParameterPortDefinition()
{
  auto& def = inPort.getDefinition();
  OMXChecker::SetHeaderVersion(def);

  def.nPortIndex = 0;
  def.bEnabled = OMX_TRUE;
  def.bPopulated = OMX_FALSE;
  def.eDomain = OMX_PortDomainVideo;
  def.eDir = OMX_DirInput;
  def.nBufferCountMin = 2;
  def.nBufferCountActual = 2;
  assert(def.nBufferCountActual >= def.nBufferCountMin);

  // Fill In Parameter Port Definition video format
  auto const videoDef = inPort.getVideoDefinition();
  def.nBufferSize = GetSizeYUV(videoDef.nStride, videoDef.nSliceHeight, videoDef.eColorFormat);

  def.bBuffersContiguous = OMX_TRUE;
  def.nBufferAlignment = 32;
}

void ProcessEncode::SetStandardOutParameterPortDefinition()
{
  auto& def = outPort.getDefinition();
  OMXChecker::SetHeaderVersion(def);

  def.nPortIndex = 1;
  def.bEnabled = OMX_TRUE;
  def.bPopulated = OMX_FALSE;
  def.eDomain = OMX_PortDomainVideo;
  def.eDir = OMX_DirOutput;
  def.nBufferCountMin = 4;
  def.nBufferCountActual = 4;
  assert(def.nBufferCountActual >= def.nBufferCountMin);

  auto const videoDef = outPort.getVideoDefinition();
  def.nBufferSize = GetMaxNalSize(videoDef.nFrameWidth, videoDef.nFrameHeight, AL_GET_CHROMA_MODE(AL_420_8BITS));

  def.bBuffersContiguous = OMX_TRUE;
  def.nBufferAlignment = 32;
}

void ProcessEncode::SetStandardInParameterBufferSupplier()
{
  OMX_PARAM_BUFFERSUPPLIERTYPE supp;
  OMXChecker::SetHeaderVersion(supp);
  supp.nPortIndex = 0;
  supp.eBufferSupplier = OMX_BufferSupplyOutput;

  inPort.setSupplier(supp);
}

void ProcessEncode::SetStandardOutParameterBufferSupplier()
{
  OMX_PARAM_BUFFERSUPPLIERTYPE supp;
  OMXChecker::SetHeaderVersion(supp);
  supp.nPortIndex = 1;
  supp.eBufferSupplier = OMX_BufferSupplyInput;

  outPort.setSupplier(supp);
}

void ProcessEncode::SetStandardYUVVideoPortDefinition()
{
  auto& videoDef = inPort.getVideoDefinition();
  videoDef.cMIMEType = (OMX_STRING)"video/yuv";
  videoDef.pNativeRender = NULL;
  videoDef.nFrameWidth = WIDTH;
  videoDef.nFrameHeight = HEIGHT;
  videoDef.nStride = STRIDE;
  videoDef.nSliceHeight = HEIGHT;
  videoDef.nBitrate = 0;
  videoDef.xFramerate = FRAMERATE;
  videoDef.bFlagErrorConcealment = OMX_FALSE;
  videoDef.eCompressionFormat = OMX_VIDEO_CodingUnused;
  videoDef.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  videoDef.pNativeWindow = NULL;
}

void ProcessEncode::SetStandardVideoPortDefinition()
{
  auto& videoDef = outPort.getVideoDefinition();
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

void ProcessEncode::ResetEncodingParameters()
{
  m_VideoParameters.setGopMode(OMX_AL_GOP_MODE_DEFAULT);
  m_VideoParameters.setGdrMode(OMX_AL_GDR_OFF);

  Quantization quant =
  {
    -1, -1, -1
  };

  m_VideoParameters.setQuantization(quant);
  m_VideoParameters.setQpMode(OMX_AUTO_QP);
  m_VideoParameters.setBitrate(0);

  auto& videoDef = outPort.getVideoDefinition();
  videoDef.nBitrate = m_VideoParameters.getBitrate();

  m_VideoParameters.setNumSlices(1);
  m_VideoParameters.setL2CacheSize(0);

  m_VideoParameters.setRCMode(AL_RC_CBR);
  m_VideoParameters.setRCOptions(AL_RC_OPT_SCN_CHG_RES);

  m_VideoParameters.setProfile(m_pCodec->ConvertProfile(m_pCodec->GetCurProfile()));
  m_VideoParameters.setLevel(m_pCodec->ConvertLevel(m_pCodec->GetCurLevel()));

  m_VideoParameters.setCPBSize(0);
  m_VideoParameters.setInitialRemovalDelay(0);

  m_VideoParameters.setScalingListMode(OMX_AL_DEFAULT);
}

OMX_PORT_PARAM_TYPE ProcessEncode::GetPortParameter()
{
  return m_PortParameter;
}

OMX_ERRORTYPE ProcessEncode::SetInParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE param)
{
  auto& videoParam = param.format.video;

  if(((videoParam.nStride % 32) != 0) ||
     (videoParam.nStride == 0) ||
     ((int)videoParam.nStride < (int)videoParam.nFrameWidth))
  {
    OMX_S32 newStride = RoundUp((int)videoParam.nFrameWidth, 32);
    LOGI("Changing input port stride (%li) to %li (port width is %li)", videoParam.nStride, newStride, videoParam.nFrameWidth);
    videoParam.nStride = newStride;
  }

  if((videoParam.nSliceHeight == 0) ||
     ((int)videoParam.nSliceHeight < (int)videoParam.nFrameHeight))
  {
    OMX_S32 newSliceHeight = videoParam.nFrameHeight;
    LOGI("Changing input port slice height (%li) to %li (port height is %li)", videoParam.nSliceHeight, newSliceHeight, videoParam.nFrameHeight);
    videoParam.nSliceHeight = newSliceHeight;
  }

  param.nBufferSize = GetSizeYUV(videoParam.nStride, videoParam.nSliceHeight, videoParam.eColorFormat);
  inPort.setDefinition(param);

  auto& outVideoDef = outPort.getVideoDefinition();
  outVideoDef.nFrameWidth = videoParam.nFrameWidth;
  outVideoDef.nFrameHeight = videoParam.nFrameHeight;
  outVideoDef.nStride = videoParam.nStride;
  outVideoDef.nSliceHeight = videoParam.nSliceHeight;

  auto& def = outPort.getDefinition();
  def.nBufferSize = GetMaxNalSize(videoParam.nFrameWidth, videoParam.nFrameHeight, AL_GET_CHROMA_MODE(GetPictureFormat()));

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessEncode::SetOutParameterPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE param)
{
  auto& videoParam = param.format.video;

  if(((videoParam.nStride % 32) != 0) ||
     (videoParam.nStride == 0) ||
     ((int)videoParam.nStride < (int)videoParam.nFrameWidth))
  {
    OMX_S32 newStride = RoundUp((int)videoParam.nFrameWidth, 32);
    LOGI("Changing output port stride (%li) to %li (port width is %li)", videoParam.nStride, newStride, videoParam.nFrameWidth);
    videoParam.nStride = newStride;
  }

  if((videoParam.nSliceHeight == 0) ||
     ((int)videoParam.nSliceHeight < (int)videoParam.nFrameHeight))
  {
    OMX_S32 newSliceHeight = videoParam.nFrameHeight;
    LOGI("Changing output port slice height (%li) to %li (port height is %li)", videoParam.nSliceHeight, newSliceHeight, videoParam.nFrameHeight);
    videoParam.nSliceHeight = newSliceHeight;
  }

  param.nBufferSize = GetMaxNalSize(videoParam.nFrameWidth, videoParam.nFrameHeight, AL_GET_CHROMA_MODE(GetPictureFormat()));
  outPort.setDefinition(param);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessEncode::SetYUVVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE format)
{
  auto& videoDef = inPort.getVideoDefinition();
  videoDef.eCompressionFormat = format.eCompressionFormat;
  videoDef.eColorFormat = format.eColorFormat;
  videoDef.xFramerate = format.xFramerate;

  auto& def = inPort.getDefinition();
  def.nBufferSize = GetSizeYUV(videoDef.nStride, videoDef.nSliceHeight, videoDef.eColorFormat);

  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessEncode::SetVideoParameterPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE format)
{
  auto& videoDef = outPort.getVideoDefinition();
  videoDef.eCompressionFormat = format.eCompressionFormat;
  videoDef.eColorFormat = format.eColorFormat;
  videoDef.xFramerate = format.xFramerate;

  auto& def = outPort.getDefinition();
  def.nBufferSize = GetMaxNalSize(videoDef.nFrameWidth, videoDef.nFrameHeight, AL_GET_CHROMA_MODE(GetPictureFormat()));
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessEncode::SetVideoParameterRateControl(OMX_VIDEO_PARAM_BITRATETYPE port)
{
  m_VideoParameters.setBitrate(port.nTargetBitrate);
  m_VideoParameters.setRCMode(ConvertRCMode(port.eControlRate));

  auto& videoDef = outPort.getVideoDefinition();
  videoDef.nBitrate = m_VideoParameters.getBitrate();
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessEncode::SetVideoParameterProfileLevel(OMX_VIDEO_PARAM_PROFILELEVELTYPE port)
{
  m_VideoParameters.setProfile(m_pCodec->ConvertProfile(port.eProfile));
  m_VideoParameters.setLevel(m_pCodec->ConvertLevel(port.eLevel));
  return OMX_ErrorNone;
}

OMX_ERRORTYPE ProcessEncode::SetStandardParameterComponentRole(OMX_PARAM_COMPONENTROLETYPE)
{
  auto eRet = OMX_ErrorNone;

  SetStandardPortParameter();
  SetStandardInParameterPortDefinition();
  SetStandardOutParameterPortDefinition();
  SetStandardYUVVideoPortDefinition();
  SetStandardVideoPortDefinition();
  SetStandardInParameterBufferSupplier();
  SetStandardOutParameterBufferSupplier();
  ResetEncodingParameters();

  return eRet;
}

OMX_ERRORTYPE ProcessEncode::SendCommand(OMX_COMMANDTYPE Cmd, OMX_U32 nPortIndex, OMX_PTR pCmdData)
{
  LOGV("_ %s _ ", __func__);
  auto eRet = OMX_ErrorNone;

  // StateSet is a special command

  if((nPortIndex == inPort.getDefinition().nPortIndex) ||
     (nPortIndex == outPort.getDefinition().nPortIndex) ||
     (nPortIndex == OMX_ALL))
  {
    auto ThreadTask = new ComponentThreadTask;

    ThreadTask->data = (uintptr_t*)nPortIndex;
    ThreadTask->opt = pCmdData;
    switch(Cmd)
    {
    case OMX_CommandFlush:
    {
      LOGV("Flush");
      ThreadTask->cmd = ProcessFlush;
      break;
    }
    case OMX_CommandPortDisable:
    {
      LOGV("Port Disable : %lu", nPortIndex);
      ThreadTask->cmd = ProcessStopPort;
      break;
    }
    case OMX_CommandPortEnable:
    {
      LOGV("Port Enable : %lu", nPortIndex);
      ThreadTask->cmd = ProcessRestartPort;
      break;
    }
    case OMX_CommandMarkBuffer:
    {
      LOGV("Mark Buffer");
      ThreadTask->cmd = ProcessMarkBuffer;
      break;
    }
    default:
      eRet = OMX_ErrorBadParameter;
      return eRet;
    }

    SendTask(ThreadTask);
  }
  else
    eRet = OMX_ErrorBadPortIndex;

  return eRet;
}

OMX_ERRORTYPE ProcessEncode::SetState(OMX_U32 newState)
{
  auto eRet = OMX_ErrorNone;
  auto ThreadTask = new ComponentThreadTask;

  if(!ThreadTask)
    return OMX_ErrorInsufficientResources;

  ThreadTask->cmd = ProcessSetComponentState;
  ThreadTask->data = (uintptr_t*)newState;
  ThreadTask->opt = nullptr;

  SendTask(ThreadTask);

  return eRet;
}

void ProcessEncode::SendTask(ComponentThreadTask* ThreadTask)
{
  m_ComponentTaskQueue.push(ThreadTask);
}

void ProcessEncode::BufferHeaderUpdate(OMX_BUFFERHEADERTYPE* pHeader, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, OMX_U32 nSizeBytes)
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
    outPort.putBuffer(pHeader);
  }

  auto const pOMXMeta = (AL_TMetaData*)AL_OMXMetaData_Create(pHeader);
  assert(pOMXMeta);
  AL_Buffer_AddMetaData((AL_TBuffer*)pHeader->pPlatformPrivate, pOMXMeta);
}

OMX_ERRORTYPE ProcessEncode::UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes, OMX_IN OMX_U8* pBuffer)
{
  LOGV("_ %s _,\nnPortIndex = %lu\npAppPrivate = %p\nnSizeBytes = %lu\npBuffer = %p", __func__,
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

OMX_ERRORTYPE ProcessEncode::AllocateBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** ppBufferHdr, OMX_IN OMX_U32 nPortIndex, OMX_IN OMX_PTR pAppPrivate, OMX_IN OMX_U32 nSizeBytes)
{
  LOGV("_ %s _ \nnPortIndex = %lu\npAppPrivate = %p\nnSizeBytes = %lu", __func__,
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

ComponentThreadTask* ProcessEncode::ReceiveTask()
{
  return m_ComponentTaskQueue.pop();
}

void ProcessEncode::ThreadComponent()
{
  while(true)
  {
    // Blocking call
    auto ThreadTask = ReceiveTask();
    switch(ThreadTask->cmd)
    {
    case ProcessSetComponentState:
    {
      LOGI("SetComponentState");
      OMX_STATETYPE newState = (OMX_STATETYPE)((uintptr_t)(ThreadTask->data));
      OMX_EVENTTYPE newEvtCmd = OMX_EventCmdComplete;

      // Delete Encoder
      if((newState == OMX_StateIdle) &&
         (m_state == OMX_StateExecuting) && m_hEncoder)
        DestroyEncoder();

      // Create Encoder
      if((newState == OMX_StateExecuting) &&
         (m_state == OMX_StateIdle))
      {
        auto isCreated = CreateEncoder();

        if(!isCreated)
        {
          newState = OMX_StateInvalid;
          newEvtCmd = OMX_EventError;
        }
      }

      m_MutexState.lock();
      m_state = newState;
      m_MutexState.unlock();

      m_pCallback->EventHandler(m_hComponent, m_pAppData, newEvtCmd, OMX_CommandStateSet, newState, nullptr);

      break;
    }
    case ProcessFlush:
    {
      auto nPortIndex = (OMX_U32)((uintptr_t)(ThreadTask->data));
      auto PortIsIn = (nPortIndex == inPort.getDefinition().nPortIndex);

      LOGI("Flush %lu", (OMX_U32)nPortIndex);

      auto port = PortIsIn ? &inPort : &outPort;

      while(1)
      {
        auto tmp = port->getBuffer();

        if(!tmp)
          break;

        tmp->nFilledLen = 0;

        if(PortIsIn)
          m_pCallback->EmptyBufferDone(m_hComponent, m_pAppData, tmp);
        else
          m_pCallback->FillBufferDone(m_hComponent, m_pAppData, tmp);
      }

      m_pCallback->EventHandler(m_hComponent, m_pAppData, OMX_EventCmdComplete, OMX_CommandFlush, nPortIndex, nullptr);
      break;
    }
    case ProcessStopPort:
    {
      LOGI("StopPort");
      break;
    }
    case ProcessRestartPort:
    {
      LOGI("RestartPort");
      break;
    }
    case ProcessStop:
    {
      LOGI("Stop");
      delete ThreadTask;
      return;
    }
    case ProcessProcessInput:
    {
      LOGI("ProcessInput");
      assert(m_hEncoder);
      auto pInputBuf = inPort.getBuffer();
      SendToEncoder(pInputBuf);
      break;
    }
    case ProcessMarkBuffer:
    {
      LOGI("MarkBuffer");
      break;
    }
    default:
      LOGE("ERROR !! Task Unknown");
      assert(0);
    }

    delete ThreadTask;
  }
}

void ProcessEncode::DestroyEncoder()
{
  LOGV("_ %s _ ", __func__);

  AL_Encoder_Destroy(m_hEncoder);
  m_hEncoder = NULL;
}

bool ProcessEncode::CreateEncoder()
{
  LOGV("_ %s _ ", __func__);
  auto eRet = OMX_ErrorNone;

  // Destroy previous if any
  if(m_hEncoder)
    DestroyEncoder();

  eRet = SetupIpDevice();

  if(eRet == OMX_ErrorNone)
  {
    m_tEncodeParam =
    {
      this
    };

    m_hEncoder = AL_Encoder_Create(m_IpDevice->m_pScheduler, m_IpDevice->m_pAllocator, &m_EncSettings, { RedirectionFrameEncode, &m_tEncodeParam });
    LOGI("Create Encoder");

    if(!m_hEncoder)
    {
      LOGE("Failed to create Encoder");
      return false;
    }
  }
  else
    return false;

  return true;
}

OMX_ERRORTYPE ProcessEncode::FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  auto eRet = OMX_ErrorNone;

  assert(pBufferHdr->nFilledLen == 0);

  auto pStream = static_cast<AL_TBuffer*>(pBufferHdr->pPlatformPrivate);
  assert(pStream);

  auto pMeta = (AL_TMetaData*)(AL_StreamMetaData_Create(AL_MAX_SECTION, pBufferHdr->nAllocLen));

  if(!pStream || !pMeta)
    return OMX_ErrorInsufficientResources;

  ReplaceMetaData(pStream, pMeta, AL_META_TYPE_STREAM);

  {
    AL_Buffer_Ref(pStream);
    map.Add(pBufferHdr, pBufferHdr);
    AL_Encoder_PutStreamBuffer(m_hEncoder, pStream);
    AL_Buffer_Unref(pStream);
  }

  return eRet;
}

OMX_ERRORTYPE ProcessEncode::EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  if(pBufferHdr->nFlags & OMX_BUFFERFLAG_EOS)
    m_EOSFlags = pBufferHdr->nFlags;

  auto ThreadTask = new  ComponentThreadTask;
  ThreadTask->cmd = ProcessProcessInput;
  inPort.putBuffer(pBufferHdr);
  SendTask(ThreadTask);

  return OMX_ErrorNone;
}

AL_EPicFormat ProcessEncode::GetPictureFormat()
{
  auto format = static_cast<OMX_U32>(inPort.getVideoDefinition().eColorFormat);
  switch(format)
  {
  case OMX_COLOR_FormatYUV420SemiPlanar: return AL_420_8BITS;
  case OMX_COLOR_FormatYUV422SemiPlanar: return AL_422_8BITS;
  case OMX_COLOR_FormatYUV420SemiPlanar10bitPacked: return AL_420_10BITS;
  case OMX_COLOR_FormatYUV422SemiPlanar10bitPacked: return AL_422_10BITS;
  default:
    assert(0);
  }
}

void ProcessEncode::SetEncoderSettings()
{
  auto const videoDef = outPort.getVideoDefinition();
  auto const videoQuant = m_VideoParameters.getQuantization();
  auto& chanParam = m_EncSettings.tChParam;

  LOGV("_ %s _ ", __func__);

  // Set Encoder Default Settings
  AL_Settings_SetDefaults(&m_EncSettings);

  chanParam.eProfile = m_VideoParameters.getProfile();

  // Set Encoder Default Parameters
  AL_Settings_SetDefaultParam(&m_EncSettings);

  m_EncSettings.eQpCtrlMode = (AL_EQpCtrlMode)m_VideoParameters.getQpMode();
  m_EncSettings.iCacheLevel2 = m_VideoParameters.getL2CacheSize();
  m_EncSettings.eScalingList = (AL_EScalingList)m_VideoParameters.getScalingListMode();

  chanParam.uWidth = videoDef.nFrameWidth;
  chanParam.uHeight = videoDef.nFrameHeight;
  chanParam.uLevel = m_VideoParameters.getLevel();
  chanParam.ePicFormat = GetPictureFormat();

  auto const framerate = inPort.getVideoDefinition().xFramerate;

  if(framerate == 0)
  {
    chanParam.tRCParam.uFrameRate = FRAMERATE >> 16;
    chanParam.tRCParam.uClkRatio = 1000;
  }
  else
    chanParam.tRCParam.uFrameRate = framerate >> 16;

  chanParam.tRCParam.uClkRatio = ((!(framerate & 0x0000FFFF)) && (!framerate)) ? 1001 : 1000;

  chanParam.tRCParam.eRCMode = m_VideoParameters.getRCMode();
  chanParam.tRCParam.eOptions = m_VideoParameters.getRCOptions();

  if((chanParam.tRCParam.eRCMode == AL_RC_CONST_QP)
     && (videoQuant.initialQP == -1))
    chanParam.tRCParam.iInitialQP = 30;
  else
    chanParam.tRCParam.iInitialQP = videoQuant.initialQP;

  chanParam.tRCParam.uIPDelta = videoQuant.IPDeltaQP;
  chanParam.tRCParam.uPBDelta = videoQuant.PBDeltaQP;

  if(m_VideoParameters.getBitrate() == 0)
  {
    auto const resolutionFramerate = chanParam.uWidth * chanParam.uHeight * chanParam.tRCParam.uFrameRate;
    auto const bitrate_coef = (resolutionFramerate <= 1920 * 1080 * 60) ? 0.2 : 0.125;
    m_VideoParameters.setBitrate(resolutionFramerate * bitrate_coef);
  }

  chanParam.tRCParam.uTargetBitRate = m_VideoParameters.getBitrate();
  chanParam.tRCParam.uMaxBitRate = chanParam.tRCParam.uTargetBitRate;

  if(m_VideoParameters.getCPBSize() != 0)
    chanParam.tRCParam.uCPBSize = m_VideoParameters.getCPBSize();

  if(m_VideoParameters.getInitialRemovalDelay() != 0)
    chanParam.tRCParam.uInitialRemDelay = m_VideoParameters.getInitialRemovalDelay();

  chanParam.tGopParam.eMode = (AL_EGopCtrlMode)m_VideoParameters.getGopMode();
  chanParam.tGopParam.eGdrMode = (AL_EGdrMode)m_VideoParameters.getGdrMode();

  chanParam.tGopParam.uGopLength = (m_pCodec->GetCodecPFrames() + m_pCodec->GetCodecBFrames() + 1);
  chanParam.tGopParam.uNumB = (m_pCodec->GetCodecBFrames() / (m_pCodec->GetCodecPFrames() + 1));

  chanParam.uTier = m_pCodec->GetCodecTier();
  chanParam.eOptions = m_pCodec->GetCodecOptions();
  chanParam.uNumSlices = m_VideoParameters.getNumSlices();

  m_tFourCC = AL_GetSrcFourCC(AL_GET_CHROMA_MODE(chanParam.ePicFormat), AL_GET_BITDEPTH(chanParam.ePicFormat));
  LOGV("\nWidth : %u\nHeight : %u\nLevel : %u\nFramerate : %u\nGopLength : %u\nNumB : %u\nEPicFormat: 0x%.8x",
       chanParam.uWidth,
       chanParam.uHeight,
       chanParam.uLevel,
       chanParam.tRCParam.uFrameRate,
       chanParam.tGopParam.uGopLength,
       chanParam.tGopParam.uNumB,
       chanParam.ePicFormat
       );
}

void ProcessEncode::SendToEncoder(OMX_BUFFERHEADERTYPE* pInputBuf)
{
  AL_TBuffer* const frame = (AL_TBuffer*)pInputBuf->pPlatformPrivate;
  auto const videoDef = inPort.getVideoDefinition();
  auto const chanParam = m_EncSettings.tChParam;

  auto sizePixel = 1;

  AL_TOffsetYC const offsetYC =
  {
    0, (int)(videoDef.nSliceHeight * videoDef.nStride * sizePixel)
  };

  AL_TPitches const pitches =
  {
    (int)videoDef.nStride, (int)videoDef.nStride
  };

  AL_TMetaData* const pMeta = (AL_TMetaData*)AL_SrcMetaData_Create(chanParam.uWidth, chanParam.uHeight, pitches, offsetYC, m_tFourCC);

  if(!frame || !pMeta)
  {
    LOGE("Frame not found !");
    assert(0);
  }

  ReplaceMetaData(frame, pMeta, AL_META_TYPE_SOURCE);

  // UseBuffer when (AL_TBuffer*)pPlatformPrivate->pData != pBuffer;
  if(!inPort.useFileDescriptor())
  {
    if(AL_Buffer_GetBufferData(frame) != pInputBuf->pBuffer)
      memcpy(AL_Buffer_GetBufferData(frame), (pInputBuf->pBuffer + pInputBuf->nOffset), pInputBuf->nFilledLen);
  }

  AL_Buffer_Ref(frame);

  if(pInputBuf->nFilledLen)
  {
    if(!AL_Encoder_Process(m_hEncoder, frame, nullptr))
    {
      LOGE("Failed to process (encode) frame !");
      assert(0);
    }
  }

  if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
  {
    LOGI("Flushing..");
    AL_Encoder_Process(m_hEncoder, nullptr, nullptr);
  }

  AL_Buffer_Unref(frame);
}

void ProcessEncode::FlushEncoder(OMX_U32 nFlags)
{
  LOGI("Complete");
  m_pCallback->EventHandler(m_hComponent, m_pAppData, OMX_EventBufferFlag, outPort.getDefinition().nPortIndex, nFlags, nullptr);
}

AL_ERateCtrlMode ProcessEncode::ConvertRCMode(OMX_VIDEO_CONTROLRATETYPE mode)
{
  switch(static_cast<OMX_U32>(mode))
  {
  case OMX_Video_ControlRateDisable: return AL_RC_CONST_QP;
  case OMX_Video_ControlRateVariable: return AL_RC_VBR;
  case OMX_Video_ControlRateConstant: return AL_RC_CBR;
  case OMX_Video_ControlRateLowLatency: return AL_RC_LOW_LATENCY;
  default:
    assert(0);
  }
}

OMX_VIDEO_CONTROLRATETYPE ProcessEncode::ConvertRCMode(AL_ERateCtrlMode mode)
{
  switch(static_cast<OMX_U32>(mode))
  {
  case AL_RC_CONST_QP: return OMX_Video_ControlRateDisable;
  case AL_RC_VBR: return OMX_Video_ControlRateVariable;
  case AL_RC_CBR: return OMX_Video_ControlRateConstant;
  case AL_RC_LOW_LATENCY: return (OMX_VIDEO_CONTROLRATETYPE)OMX_Video_ControlRateLowLatency;
  default:
    assert(0);
  }
}

OMX_ERRORTYPE ProcessEncode::GetExtensionIndex(OMX_IN OMX_STRING cParameterName, OMX_OUT OMX_INDEXTYPE* pIndexType)
{
  LOGV("_ %s _ ", __func__);
  auto index = 0;

  while(index < (int)ARRAYSIZE(AL_CustomParam))
  {
    if(!strncmp((OMX_STRING)AL_CustomParam[index].cName, cParameterName, OMX_MAX_STRINGNAME_SIZE))
    {
      *pIndexType = static_cast<OMX_INDEXTYPE>(AL_CustomParam[index].nIndexType);
      return OMX_ErrorNone;
    }
    index++;
  }

  return OMX_ErrorNoMore;
}

OMX_ERRORTYPE ProcessEncode::ComponentTunnelRequest(OMX_IN OMX_U32 nPort, OMX_IN OMX_HANDLETYPE hTunneledComp, OMX_IN OMX_U32 nTunneledPort, OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
  auto eRet = OMX_ErrorNone;

  if(nPort == inPort.getDefinition().nPortIndex)
  {
    // Get parameter de l'output port check data compatibility 420/422 input
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

void ProcessEncode::SourceBufferIsDone(OMX_BUFFERHEADERTYPE* pBufHdr)
{
  pBufHdr->nFilledLen = 0;
  m_pCallback->EmptyBufferDone(m_hComponent, m_pAppData, pBufHdr);
}

void ProcessEncode::StreamBufferIsDone(OMX_BUFFERHEADERTYPE* pBufHdr)
{
  m_pCallback->FillBufferDone(m_hComponent, m_pAppData, pBufHdr);
}

