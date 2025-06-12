// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

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
void CommandsSender::notifySceneChange(int32_t lookAhead)
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

void CommandsSender::restartGopRecoveryPoint()
{
  assert(0 && "restartGopRecoveryPoint32_t is not supported");
}

void CommandsSender::setGopLength(int32_t gopLength)
{
  OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES gop;
  InitHeader(gop);
  gop.nPortIndex = 1;
  OMX_GetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), &gop);
  int32_t numB = gop.nBFrames / (gop.nPFrames + 1);

  if(gopLength <= numB)
    assert(0);

  gop.nBFrames = (numB * gopLength) / (1 + numB);
  gop.nPFrames = (numB - gopLength + 1) / (-numB - 1);

  auto const error = OMX_SetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), &gop);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::setNumB(int32_t numB)
{
  OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES gop;
  InitHeader(gop);
  gop.nPortIndex = 1;
  OMX_GetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), &gop);
  int32_t omxGopLength = gop.nPFrames + gop.nBFrames + 1;
  gop.nBFrames = (numB * omxGopLength) / (1 + numB);
  gop.nPFrames = (numB - omxGopLength + 1) / (-numB - 1);
  auto const error = OMX_SetConfig(hEnc, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoGroupOfPictures), &gop);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::setFreqIDR(int32_t freqIDR)
{
  (void)freqIDR;
  assert(0 && "setFreqIDR is not supported");
}

void CommandsSender::setFrameRate(int32_t frameRate, int32_t clockRatio)
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

void CommandsSender::setBitRate(int32_t bitRate)
{
  OMX_VIDEO_CONFIG_BITRATETYPE bitrate;
  InitHeader(bitrate);
  bitrate.nPortIndex = 1;
  OMX_GetConfig(hEnc, OMX_IndexConfigVideoBitrate, &bitrate);
  bitrate.nEncodeBitrate = bitRate / 1000;
  auto const error = OMX_SetConfig(hEnc, OMX_IndexConfigVideoBitrate, &bitrate);
  assert(error == OMX_ErrorNone);
}

void CommandsSender::setMaxBitRate(int32_t iTargetBitRate, int32_t iMaxBitRate)
{
  (void)iTargetBitRate;
  (void)iMaxBitRate;
  assert(0 && "setMaxBitRate is not supported");
}

void CommandsSender::setQP(int32_t qp)
{
  (void)qp;
  assert(0 && "setQP is not supported");
}

void CommandsSender::setQPOffset(int32_t iQpOffset)
{
  (void)iQpOffset;
  assert(0 && "setQPOffset is not supported");
}

void CommandsSender::setQPBounds(int32_t iMinQP, int32_t iMaxQP)
{
  (void)iMinQP;
  (void)iMaxQP;
  assert(0 && "setQPBounds is not supported");
}

void CommandsSender::setQPBounds_I(int32_t iMinQP_I, int32_t iMaxQP_I)
{
  (void)iMinQP_I;
  (void)iMaxQP_I;
  assert(0 && "setQPBounds_I is not supported");
}

void CommandsSender::setQPBounds_P(int32_t iMinQP_P, int32_t iMaxQP_P)
{
  (void)iMinQP_P;
  (void)iMaxQP_P;
  assert(0 && "setQPBounds_P is not supported");
}

void CommandsSender::setQPBounds_B(int32_t iMinQP_B, int32_t iMaxQP_B)
{
  (void)iMinQP_B;
  (void)iMaxQP_B;
  assert(0 && "setQPBounds_B is not supported");
}

void CommandsSender::setQPIPDelta(int32_t iQPDelta)
{
  (void)iQPDelta;
  assert(0 && "setQPIPDelta is not supported");
}

void CommandsSender::setQPPBDelta(int32_t iQPDelta)
{
  (void)iQPDelta;
  assert(0 && "setQPPBDelta is not supported");
}

void CommandsSender::setDynamicInput(int32_t iInputIdx)
{
  (void)iInputIdx;
  assert(0 && "setDynamicInput is not supported");
}

void CommandsSender::setLFMode(int32_t iMode)
{
  (void)iMode;
  assert(0 && "setLFMode is not supported");
}

void CommandsSender::setLFBetaOffset(int32_t iBetaOffset)
{
  (void)iBetaOffset;
  assert(0 && "setLFBetaOffset is not supported");
}

void CommandsSender::setLFTcOffset(int32_t iTcOffset)
{
  (void)iTcOffset;
  assert(0 && "setLFTcOffset is not supported");
}

void CommandsSender::setCostMode(bool bCostMode)
{
  (void)bCostMode;
  assert(0 && "setCostMode is not supported");
}

void CommandsSender::setMaxPictureSize(int32_t iMaxPictureSize)
{
  (void)iMaxPictureSize;
  assert(0 && "setMaxPictureSize is not supported");
}

void CommandsSender::setMaxPictureSize_I(int32_t iMaxPictureSize_I)
{
  (void)iMaxPictureSize_I;
  assert(0 && "setMaxPictureSize_I is not supported");
}

void CommandsSender::setMaxPictureSize_P(int32_t iMaxPictureSize_P)
{
  (void)iMaxPictureSize_P;
  assert(0 && "setMaxPictureSize_P is not supported");
}

void CommandsSender::setMaxPictureSize_B(int32_t iMaxPictureSize_B)
{
  (void)iMaxPictureSize_B;
  assert(0 && "setMaxPictureSize_B is not supported");
}

void CommandsSender::setQPChromaOffsets(int32_t iQp1Offset, int32_t iQp2Offset)
{
  (void)iQp1Offset;
  (void)iQp2Offset;
  assert(0 && "setQPChromaOffsets is not supported");
}

void CommandsSender::setAutoQP(bool bUseAutoQP)
{
  (void)bUseAutoQP;
  assert(0 && "setAutoQP is not supported");
}

void CommandsSender::setHDRIndex(int32_t iHDRIdx)
{
  (void)iHDRIdx;
  assert(0 && "setHDRIndex is not supported");
}
