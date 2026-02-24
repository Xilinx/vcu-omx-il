// SPDX-FileCopyrightText: © 2026 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <list>
#include <iostream>
#include <string>
#include <vector>

#include "lib_rtos/types.h"
#include "ICommandsSender.hpp"

struct CEncCmdMngr
{
  CEncCmdMngr(std::istream& CmdInput, int32_t iLookAhead, uint32_t uFreqLT);

  void Process(ICommandsSender* sender, AL_64S iFrame);

private:
  std::istream& m_CmdInput;
  int32_t const m_iLookAhead;
  uint32_t const m_uFreqLT;
  bool m_bHasLT;
  std::string m_sBufferedLine;

  struct TFrmCmd
  {
    AL_64S iFrame = 0;
    bool bSceneChange = false;
    bool bIsLongTerm = false;
    bool bUseLongTerm = false;
    bool bKeyFrame = false;
    bool bRecoveryPoint = false;
    bool bChangeGopLength = false;
    int32_t iGopLength = 0;
    bool bChangeGopNumB = false;
    int32_t iGopNumB = 0;
    bool bChangeFreqIDR = false;
    int32_t iFreqIDR = 0;
    bool bChangeBitRate = false;
    int32_t iBitRate = 0;
    bool bChangeMaxBitRate = false;
    int32_t iTargetBitRate = 0;
    int32_t iMaxBitRate = 0;
    bool bChangeFrameRate = false;
    int32_t iFrameRate = 0;
    int32_t iClkRatio = 0;
    bool bChangeQP = false;
    int32_t iQP = 0;
    bool bChangeQPOffset = false;
    int32_t iQPOffset = 0;
    bool bChangeQPBounds = false;
    int32_t iMinQP = 0;
    int32_t iMaxQP = 0;
    bool bChangeQPBounds_I = false;
    int32_t iMinQP_I = 0;
    int32_t iMaxQP_I = 0;
    bool bChangeQPBounds_P = false;
    int32_t iMinQP_P = 0;
    int32_t iMaxQP_P = 0;
    bool bChangeQPBounds_B = false;
    int32_t iMinQP_B = 0;
    int32_t iMaxQP_B = 0;
    bool bChangeIPDelta = false;
    int32_t iIPDelta = 0;
    bool bChangePBDelta = false;
    int32_t iPBDelta = 0;
    bool bChangeResolution = false;
    int32_t iInputIdx;
    bool bSetLFMode = false;
    int32_t iLFMode;
    bool bSetLFBetaOffset = false;
    int32_t iLFBetaOffset;
    bool bSetLFTcOffset = false;
    int32_t iLFTcOffset;
    bool bSetCostMode = false;
    bool bCostMode;
    bool bMaxPictureSize = false;
    int32_t iMaxPictureSize;
    bool bMaxPictureSize_I = false;
    int32_t iMaxPictureSize_I;
    bool bMaxPictureSize_P = false;
    int32_t iMaxPictureSize_P;
    bool bMaxPictureSize_B = false;
    int32_t iMaxPictureSize_B;
    bool bChangeQPChromaOffsets = false;
    int32_t iQp1Offset = 0;
    int32_t iQp2Offset = 0;
    bool bSetAutoQP = false;
    bool bUseAutoQP = false;
    bool bChangeHDR = false;
    int32_t iHDRIdx = 0;
  };

  std::list<TFrmCmd> m_Cmds;

  void Refill(AL_64S iCurFrame);
  bool ReadNextCmd(TFrmCmd& Cmd);
  bool ParseCmd(std::string sLine, TFrmCmd& Cmd, bool bSameFrame);
  bool GetNextLine(std::string& sNextLine);
};
