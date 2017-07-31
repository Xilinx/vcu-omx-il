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

#include <OMX_Core.h>
#include <OMX_Video.h>
#include <OMX_Component.h>

extern "C"
{
#include "lib_common_enc/EncChanParam.h"
}

#define ARRAYSIZE(a) \
  ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

struct VideoProfileLevelType
{
  OMX_U32 eProfile;
  OMX_U32 eLevel;
};

class CodecType
{
public:
  virtual ~CodecType()
  {
  };

  virtual OMX_STRING GetType() = 0;
  virtual OMX_STRING GetRole() = 0;
  virtual OMX_STRING GetMIME() = 0;
  virtual OMX_U32 GetCurProfile() = 0;
  virtual OMX_U32 GetCurLevel() = 0;
  virtual OMX_VIDEO_CODINGTYPE GetCompressionFormat() = 0;
  virtual OMX_U32 GeteProfile(OMX_U32 /* nProfileIndex */) = 0;
  virtual OMX_U32 GeteLevel(OMX_U32 /* nProfileIndex */) = 0;
  virtual void SetProfileLevel(VideoProfileLevelType /* proflevel */) = 0;
  virtual int DPBSize(int width, int height) = 0;
  virtual OMX_U32 GetSupportedProfileLevelSize() = 0;

  virtual OMX_BOOL CheckIndexParamVideoCodec(OMX_INDEXTYPE /* nParamIndex */) = 0;
  virtual OMX_ERRORTYPE GetIndexParamVideoCodec(OMX_INOUT OMX_PTR /* pParam */, OMX_PARAM_PORTDEFINITIONTYPE /* PortDef */) = 0;
  virtual OMX_ERRORTYPE SetIndexParamVideoCodec(OMX_INOUT OMX_PTR /* pParam */, OMX_PARAM_PORTDEFINITIONTYPE & /* InPortDef */, OMX_OUT OMX_PARAM_PORTDEFINITIONTYPE & /* OutPortDef */) = 0;
  virtual OMX_U32 GetCodecPFrames() = 0;
  virtual int ConvertLevel(OMX_U32 level) = 0;
  virtual OMX_U32 ConvertLevel(int level) = 0;
  virtual AL_EProfile ConvertProfile(OMX_U32 profile) = 0;
  virtual OMX_U32 ConvertProfile(AL_EProfile profile) = 0;
  virtual OMX_U32 GetCodecBFrames() = 0;
  virtual OMX_U32 GetCodecTier() = 0;
  virtual bool IsCAVLC() = 0;
  virtual AL_EChEncOption GetCodecOptions() = 0;
  virtual void EnableLowBandwidth(bool shouldBeEnabled) = 0;
  virtual int GetBandwidth() = 0;

protected:
  bool m_bLowBW;
};

