/******************************************************************************
*
* Copyright (C) 2016-2020 Allegro DVT2.  All rights reserved.
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

#include "CommandsSender.h"

#include <cassert>
#include <cmath>
#include "../common/helpers.h"

extern "C"
{
#include <OMX_Core.h>
#include <OMX_Video.h>
#include <OMX_VideoAlg.h>
#include <OMX_Component.h>
#include <OMX_Index.h>
#include <OMX_IndexAlg.h>
}

CommandsSender::CommandsSender(OMX_HANDLETYPE hEnc) :
  hEnc(hEnc)
{
}

CommandsSender::~CommandsSender() = default;
void CommandsSender::notifySceneChange(int lookAhead)
{
  OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE notifySceneChange;
  InitHeader(notifySceneChange);
  notifySceneChange.nPortIndex = 1;
  notifySceneChange.nLookAhead = lookAhead;
  auto error = OMX_SetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoNotifySceneChange), &notifySceneChange);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::notifyIsLongTerm()
{
  OMX_ALG_VIDEO_CONFIG_INSERT config;
  InitHeader(config);
  config.nPortIndex = 1;
  auto error = OMX_SetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoInsertLongTerm), &config);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::notifyUseLongTerm()
{
  OMX_ALG_VIDEO_CONFIG_INSERT config;
  InitHeader(config);
  config.nPortIndex = 1;
  auto error = OMX_SetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoUseLongTerm), &config);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::restartGop()
{
  OMX_ALG_VIDEO_CONFIG_INSERT config;
  InitHeader(config);
  config.nPortIndex = 1;
  auto error = OMX_SetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh), &config);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::setGopLength(int gopLength)
{
  OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES gop;
  InitHeader(gop);
  gop.nPortIndex = 1;
  OMX_GetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), &gop);
  int numB = gop.nBFrames / (gop.nPFrames + 1);

  if(gopLength <= numB)
    assert(0);

  gop.nBFrames = (numB * gopLength) / (1 + numB);
  gop.nPFrames = (numB - gopLength + 1) / (-numB - 1);

  auto const error = OMX_SetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), &gop);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::setNumB(int numB)
{
  OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES gop;
  InitHeader(gop);
  gop.nPortIndex = 1;
  OMX_GetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), &gop);
  int omxGopLength = gop.nPFrames + gop.nBFrames + 1;
  gop.nBFrames = (numB * omxGopLength) / (1 + numB);
  gop.nPFrames = (numB - omxGopLength + 1) / (-numB - 1);
  auto const error = OMX_SetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), &gop);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::setFreqIDR(int freqIDR)
{
  (void)freqIDR;
  assert(0 && "setFreqIDR is not supported");
}

void CommandsSender::setFrameRate(int frameRate, int clockRatio)
{
  OMX_CONFIG_FRAMERATETYPE xFramerate;
  InitHeader(xFramerate);
  xFramerate.nPortIndex = 1;
  OMX_GetConfig(hEnc, OMX_IndexConfigVideoFramerate, &xFramerate);
  auto const framerateInQ16 = ((frameRate * 1000.0) / clockRatio) * 65536.0;
  xFramerate.xEncodeFramerate = std::ceil(framerateInQ16);
  auto const error = OMX_SetConfig(hEnc, OMX_IndexConfigVideoFramerate, &xFramerate);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::setBitRate(int bitRate)
{
  OMX_VIDEO_CONFIG_BITRATETYPE bitrate;
  InitHeader(bitrate);
  bitrate.nPortIndex = 1;
  OMX_GetConfig(hEnc, OMX_IndexConfigVideoBitrate, &bitrate);
  bitrate.nEncodeBitrate = bitRate / 1000;
  auto const error = OMX_SetConfig(hEnc, OMX_IndexConfigVideoBitrate, &bitrate);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::setQP(int qp)
{
  (void)qp;
  assert(0 && "setQP is not supported");
}

void CommandsSender::setQPBounds(int iMinQP, int iMaxQP)
{
  (void)iMinQP;
  (void)iMaxQP;
  assert(0 && "setQPBounds is not supported");
}

void CommandsSender::setQPIPDelta(int iQPDelta)
{
  (void)iQPDelta;
  assert(0 && "setQPIPDelta is not supported");
}

void CommandsSender::setQPPBDelta(int iQPDelta)
{
  (void)iQPDelta;
  assert(0 && "setQPPBDelta is not supported");
}

void CommandsSender::setDynamicInput(int iInputIdx)
{
  (void)iInputIdx;
  assert(0 && "setDynamicInput is not supported");
}

void CommandsSender::setLFBetaOffset(int iBetaOffset)
{
  (void)iBetaOffset;
  assert(0 && "setLFBetaOffset is not supported");
}

void CommandsSender::setLFTcOffset(int iTcOffset)
{
  (void)iTcOffset;
  assert(0 && "setLFTcOffset is not supported");
}

void CommandsSender::setHDRIndex(int iHDRIdx)
{
  (void)iHDRIdx;
  assert(0 && "setHDRIndex is not supported");
}
