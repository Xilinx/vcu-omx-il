// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

struct ICommandsSender
{
  virtual ~ICommandsSender() = default;
  virtual void notifySceneChange(int32_t lookAhead) = 0;
  virtual void notifyIsLongTerm() = 0;
  virtual void notifyUseLongTerm() = 0;
  virtual void restartGop() = 0;
  virtual void restartGopRecoveryPoint() = 0;
  virtual void setGopLength(int32_t gopLength) = 0;
  virtual void setNumB(int32_t numB) = 0;
  virtual void setFreqIDR(int32_t freqIDR) = 0;
  virtual void setFrameRate(int32_t frameRate, int32_t clockRatio) = 0;
  virtual void setBitRate(int32_t bitRate) = 0;
  virtual void setMaxBitRate(int32_t iTargetBitRate, int32_t iMaxBitRate) = 0;
  virtual void setQP(int32_t qp) = 0;
  virtual void setQPOffset(int32_t iQpOffset) = 0;
  virtual void setQPBounds(int32_t iMinQP, int32_t iMaxQP) = 0;
  virtual void setQPBounds_I(int32_t iMinQP_I, int32_t iMaxQP_I) = 0;
  virtual void setQPBounds_P(int32_t iMinQP_P, int32_t iMaxQP_P) = 0;
  virtual void setQPBounds_B(int32_t iMinQP_B, int32_t iMaxQP_B) = 0;
  virtual void setQPIPDelta(int32_t iQPDelta) = 0;
  virtual void setQPPBDelta(int32_t iQPDelta) = 0;
  virtual void setDynamicInput(int32_t iInputIdx) = 0;
  virtual void setLFMode(int32_t iMode) = 0;
  virtual void setLFBetaOffset(int32_t iBetaOffset) = 0;
  virtual void setLFTcOffset(int32_t iTcOffset) = 0;
  virtual void setCostMode(bool bCostMode) = 0;
  virtual void setMaxPictureSize(int32_t iMaxPictureSize) = 0;
  virtual void setMaxPictureSize_I(int32_t iMaxPictureSize_I) = 0;
  virtual void setMaxPictureSize_P(int32_t iMaxPictureSize_P) = 0;
  virtual void setMaxPictureSize_B(int32_t iMaxPictureSize_B) = 0;

  virtual void setQPChromaOffsets(int32_t iQp1Offset, int32_t iQp2Offset) = 0;
  virtual void setAutoQP(bool bUseAutoQP) = 0;
  virtual void setHDRIndex(int32_t iHDRIdx) = 0;
};
