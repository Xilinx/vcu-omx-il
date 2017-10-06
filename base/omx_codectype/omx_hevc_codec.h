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
#include <OMX_VideoExt.h>
#include "omx_codectype.h"

class HEVCCodec : public CodecType
{
public:
  HEVCCodec();
  ~HEVCCodec();

  OMX_STRING GetType();
  OMX_STRING GetRole();
  OMX_STRING GetMIME();
  OMX_U32 GetCurProfile();
  OMX_U32 GetCurLevel();
  OMX_VIDEO_CODINGTYPE GetCompressionFormat();
  OMX_U32 GeteProfile(OMX_IN OMX_U32 /* nProfileIndex */);
  OMX_U32 GeteLevel(OMX_IN OMX_U32 /* nProfileIndex */);
  OMX_U32 GetSupportedProfileLevelSize();
  int DPBSize(int width, int height, OMX_ALG_EDpbMode mode);
  void SetProfileLevel(VideoProfileLevelType /* proflevel */);
  OMX_BOOL CheckIndexParamVideoCodec(OMX_IN OMX_INDEXTYPE /* nParamIndex */);
  OMX_ERRORTYPE GetIndexParamVideoCodec(OMX_INOUT OMX_PTR /* pParam */, OMX_IN OMX_PARAM_PORTDEFINITIONTYPE /* PortDef */);
  OMX_ERRORTYPE SetIndexParamVideoCodec(OMX_INOUT OMX_PTR /* pParam */, OMX_INOUT OMX_PARAM_PORTDEFINITIONTYPE & /* InPortDef */, OMX_OUT OMX_PARAM_PORTDEFINITIONTYPE & /* OutPortDef */);
  int ConvertLevel(OMX_U32 level);
  OMX_U32 ConvertLevel(int level);
  AL_EProfile ConvertProfile(OMX_U32 profile);
  OMX_U32 ConvertProfile(AL_EProfile profile);
  OMX_U32 GetCodecPFrames();
  OMX_U32 GetCodecBFrames();
  OMX_U32 GetCodecTier();
  AL_EChEncOption GetCodecOptions();
  bool IsCAVLC();
  void EnableLowBandwidth(bool shouldBeEnabled);
  int GetBandwidth();

private:
  OMX_ALG_VIDEO_PARAM_HEVCTYPE m_ParameterVideoCodec;
  OMX_BOOL m_bIsMainTier;
};

