// SPDX-FileCopyrightText: © 2025 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "EncCmdMngr.h"
#include <stdexcept>

#include "EncTokenizerUtils.h"

using namespace std;

extern "C"
{
#include "lib_common_enc/EncChanParam.h"
}

/****************************************************************************
 * Class CEncCmdMngr                                                         *
 *****************************************************************************/
CEncCmdMngr::CEncCmdMngr(std::istream& CmdInput, int32_t iLookAhead, uint32_t iFreqLT)
  : m_CmdInput(CmdInput),
  m_iLookAhead(iLookAhead),
  m_uFreqLT(iFreqLT),
  m_bHasLT(false)
{
  Refill(0);
}

/****************************************************************************/
void CEncCmdMngr::Refill(int32_t iCurFrame)
{
  while(m_Cmds.size() && m_Cmds.front().iFrame < iCurFrame)
    m_Cmds.pop_front();

  while((int)m_Cmds.size() < m_iLookAhead && (m_Cmds.empty() || m_Cmds.back().iFrame < iCurFrame + m_iLookAhead))
  {
    TFrmCmd Cmd {};

    if(!ReadNextCmd(Cmd))
      break;

    m_Cmds.push_back(Cmd);
  }
}

/****************************************************************************/
bool CEncCmdMngr::ReadNextCmd(TFrmCmd& Cmd)
{
  std::string sLine;
  bool bRet = false;

  for(;;)
  {
    if(!GetNextLine(sLine))
      break;

    if(sLine.empty())
      continue;

    if(ParseCmd(sLine, Cmd, bRet))
    {
      bRet = true;
    }
    else
    {
      m_sBufferedLine = sLine;
      break;
    }
  }

  return bRet;
}

/****************************************************************************/
bool CEncCmdMngr::GetNextLine(string& sNextLine)
{
  bool bSucceed = true;

  if(!m_sBufferedLine.empty())
  {
    sNextLine = m_sBufferedLine;
    m_sBufferedLine.clear();
  }
  else
  {
    getline(m_CmdInput, sNextLine);
    bSucceed = !m_CmdInput.fail();
  }

  return bSucceed;
}

/****************************************************************************/
bool CEncCmdMngr::ParseCmd(std::string sLine, TFrmCmd& Cmd, bool bSameFrame)
{
  CCmdTokenizer Tok(sLine);

  if(!Tok.GetNext())
    return bSameFrame;

  int32_t iFrame = Tok.atoi();

  if(bSameFrame && iFrame != Cmd.iFrame)
    return false;

  Cmd.iFrame = iFrame;

  while(Tok.GetNext())
  {
    if(Tok == "SC")
      Cmd.bSceneChange = true;
    else if(Tok == "LT")
      Cmd.bIsLongTerm = true;
    else if(Tok == "UseLT")
      Cmd.bUseLongTerm = true;

    else if(Tok == "KF")
      Cmd.bKeyFrame = true;
    else if(Tok == "RP")
      Cmd.bRecoveryPoint = true;
    else if(Tok == "GopLen")
    {
      Cmd.bChangeGopLength = true;
      Cmd.iGopLength = int(Tok.GetValue());
    }
    else if(Tok == "NumB")
    {
      Cmd.bChangeGopNumB = true;
      Cmd.iGopNumB = int(Tok.GetValue());
    }
    else if(Tok == "FreqIDR")
    {
      Cmd.bChangeFreqIDR = true;
      Cmd.iFreqIDR = int(Tok.GetValue());
    }
    else if(Tok == "BR")
    {
      Cmd.bChangeBitRate = true;
      Cmd.iBitRate = int(Tok.GetValue()) * 1000;

      if(Cmd.iBitRate == 0)
        throw std::runtime_error("New dynamic bitrate must not be null");
    }
    else if(Tok == "BR.MaxBR")
    {
      Cmd.bChangeMaxBitRate = true;
      TBounds tBounds = Tok.GetValueBounds();
      Cmd.iTargetBitRate = tBounds.min * 1000;
      Cmd.iMaxBitRate = tBounds.max * 1000;

      if(Cmd.iTargetBitRate == 0 || Cmd.iMaxBitRate == 0)
        throw std::runtime_error("New dynamic target bitrate must not be null");
    }
    else if(Tok == "Fps")
    {
      Cmd.bChangeFrameRate = true;
      Cmd.iFrameRate = int(Tok.GetValue());
      Cmd.iClkRatio = 1000;
      int32_t iFrac = int(Tok.GetValue() * 1000) % 1000;

      if(iFrac)
        Cmd.iClkRatio += (1000 - iFrac) / ++Cmd.iFrameRate;
    }
    else if(Tok == "QP")
    {
      Cmd.bChangeQP = true;
      Cmd.iQP = int(Tok.GetValue());
    }
    else if(Tok == "QPOffset")
    {
      Cmd.bChangeQPOffset = true;
      Cmd.iQPOffset = int(Tok.GetValue());
    }
    else if(Tok == "QPBounds")
    {
      Cmd.bChangeQPBounds = true;
      TBounds tBounds = Tok.GetValueBounds();
      Cmd.iMinQP = tBounds.min;
      Cmd.iMaxQP = tBounds.max;
    }
    else if(Tok == "QPBounds.I")
    {
      Cmd.bChangeQPBounds_I = true;
      TBounds tBounds = Tok.GetValueBounds();
      Cmd.iMinQP_I = tBounds.min;
      Cmd.iMaxQP_I = tBounds.max;
    }
    else if(Tok == "QPBounds.P")
    {
      Cmd.bChangeQPBounds_P = true;
      TBounds tBounds = Tok.GetValueBounds();
      Cmd.iMinQP_P = tBounds.min;
      Cmd.iMaxQP_P = tBounds.max;
    }
    else if(Tok == "QPBounds.B")
    {
      Cmd.bChangeQPBounds_B = true;
      TBounds tBounds = Tok.GetValueBounds();
      Cmd.iMinQP_B = tBounds.min;
      Cmd.iMaxQP_B = tBounds.max;
    }
    else if(Tok == "IPDelta")
    {
      Cmd.bChangeIPDelta = true;
      Cmd.iIPDelta = int(Tok.GetValue());
    }
    else if(Tok == "PBDelta")
    {
      Cmd.bChangePBDelta = true;
      Cmd.iPBDelta = int(Tok.GetValue());
    }
    else if(Tok == "Input")
    {
      Cmd.bChangeResolution = true;
      Cmd.iInputIdx = int(Tok.GetValue());
    }
    else if(Tok == "LF.Mode")
    {
      Cmd.bSetLFMode = true;
      Cmd.iLFMode = int(Tok.GetValue());
    }
    else if(Tok == "LF.BetaOffset")
    {
      Cmd.bSetLFBetaOffset = true;
      Cmd.iLFBetaOffset = int(Tok.GetValue());
    }
    else if(Tok == "LF.TcOffset")
    {
      Cmd.bSetLFTcOffset = true;
      Cmd.iLFTcOffset = int(Tok.GetValue());
    }
    else if(Tok == "CostMode")
    {
      Cmd.bSetCostMode = true;
      Cmd.bCostMode = Tok.GetValueList().front().compare("true") == 0;
    }
    else if(Tok == "MaxPictureSize")
    {
      Cmd.bMaxPictureSize = true;
      Cmd.iMaxPictureSize = int(Tok.GetValue()) * 1000;
    }
    else if(Tok == "MaxPictureSize.I")
    {
      Cmd.bMaxPictureSize_I = true;
      Cmd.iMaxPictureSize_I = int(Tok.GetValue()) * 1000;
    }
    else if(Tok == "MaxPictureSize.P")
    {
      Cmd.bMaxPictureSize_P = true;
      Cmd.iMaxPictureSize_P = int(Tok.GetValue()) * 1000;
    }
    else if(Tok == "MaxPictureSize.B")
    {
      Cmd.bMaxPictureSize_B = true;
      Cmd.iMaxPictureSize_B = int(Tok.GetValue()) * 1000;
    }
    else if(Tok == "QPChromaOffsets")
    {
      Cmd.bChangeQPChromaOffsets = true;
      TBounds tBounds = Tok.GetValueBounds();
      Cmd.iQp1Offset = tBounds.min;
      Cmd.iQp2Offset = tBounds.max;
    }
    else if(Tok == "AutoQP")
    {
      Cmd.bSetAutoQP = true;
      Cmd.bUseAutoQP = Tok.GetValueList().front().compare("true") == 0;
    }
    else if(Tok == "HDRIndex")
    {
      Cmd.bChangeHDR = true;
      Cmd.iHDRIdx = int(Tok.GetValue());
    }
  }

  return true;
}

/****************************************************************************/
void CEncCmdMngr::Process(ICommandsSender* sender, int32_t iFrame)
{
  if(!m_Cmds.empty())
  {
    bool bRefill = false;

    // Look ahead command
    if(iFrame + m_iLookAhead == m_Cmds.back().iFrame)
    {
      bRefill = true;

      if(m_Cmds.back().bSceneChange)
        sender->notifySceneChange(m_iLookAhead);
    }

    // On time command
    if(iFrame == m_Cmds.front().iFrame)
    {
      bRefill = true;

      if(m_Cmds.front().bUseLongTerm && (m_uFreqLT || m_bHasLT))
        sender->notifyUseLongTerm();

      if(m_Cmds.front().bIsLongTerm)
      {
        sender->notifyIsLongTerm();
        m_bHasLT = true;
      }

      if(m_Cmds.front().bKeyFrame)
        sender->restartGop();

      if(m_Cmds.front().bRecoveryPoint)
        sender->restartGopRecoveryPoint();

      if(m_Cmds.front().bChangeGopLength)
        sender->setGopLength(m_Cmds.front().iGopLength);

      if(m_Cmds.front().bChangeGopNumB)
        sender->setNumB(m_Cmds.front().iGopNumB);

      if(m_Cmds.front().bChangeFreqIDR)
        sender->setFreqIDR(m_Cmds.front().iFreqIDR);

      if(m_Cmds.front().bChangeFrameRate)
        sender->setFrameRate(m_Cmds.front().iFrameRate, m_Cmds.front().iClkRatio);

      if(m_Cmds.front().bChangeBitRate)
        sender->setBitRate(m_Cmds.front().iBitRate);

      if(m_Cmds.front().bChangeMaxBitRate)
        sender->setMaxBitRate(m_Cmds.front().iTargetBitRate, m_Cmds.front().iMaxBitRate);

      if(m_Cmds.front().bChangeQPBounds)
        sender->setQPBounds(m_Cmds.front().iMinQP, m_Cmds.front().iMaxQP);

      if(m_Cmds.front().bChangeQPBounds_I)
        sender->setQPBounds_I(m_Cmds.front().iMinQP_I, m_Cmds.front().iMaxQP_I);

      if(m_Cmds.front().bChangeQPBounds_P)
        sender->setQPBounds_P(m_Cmds.front().iMinQP_P, m_Cmds.front().iMaxQP_P);

      if(m_Cmds.front().bChangeQPBounds_B)
        sender->setQPBounds_B(m_Cmds.front().iMinQP_B, m_Cmds.front().iMaxQP_B);

      if(m_Cmds.front().bChangeIPDelta)
        sender->setQPIPDelta(m_Cmds.front().iIPDelta);

      if(m_Cmds.front().bChangePBDelta)
        sender->setQPPBDelta(m_Cmds.front().iPBDelta);

      if(m_Cmds.front().bChangeQP)
        sender->setQP(m_Cmds.front().iQP);

      if(m_Cmds.front().bChangeQPOffset)
        sender->setQPOffset(m_Cmds.front().iQPOffset);

      if(m_Cmds.front().bChangeResolution)
        sender->setDynamicInput(m_Cmds.front().iInputIdx);

      if(m_Cmds.front().bSetLFMode)
        sender->setLFMode(m_Cmds.front().iLFMode);

      if(m_Cmds.front().bSetLFBetaOffset)
        sender->setLFBetaOffset(m_Cmds.front().iLFBetaOffset);

      if(m_Cmds.front().bSetLFTcOffset)
        sender->setLFTcOffset(m_Cmds.front().iLFTcOffset);

      if(m_Cmds.front().bSetCostMode)
        sender->setCostMode(m_Cmds.front().bCostMode);

      if(m_Cmds.front().bMaxPictureSize)
        sender->setMaxPictureSize(m_Cmds.front().iMaxPictureSize);

      if(m_Cmds.front().bMaxPictureSize_I)
        sender->setMaxPictureSize_I(m_Cmds.front().iMaxPictureSize_I);

      if(m_Cmds.front().bMaxPictureSize_P)
        sender->setMaxPictureSize_P(m_Cmds.front().iMaxPictureSize_P);

      if(m_Cmds.front().bMaxPictureSize_B)
        sender->setMaxPictureSize_B(m_Cmds.front().iMaxPictureSize_B);

      if(m_Cmds.front().bChangeQPChromaOffsets)
        sender->setQPChromaOffsets(m_Cmds.front().iQp1Offset, m_Cmds.front().iQp2Offset);

      if(m_Cmds.front().bSetAutoQP)
        sender->setAutoQP(m_Cmds.front().bUseAutoQP);

      if(m_Cmds.front().bChangeHDR)
        sender->setHDRIndex(m_Cmds.front().iHDRIdx);
    }

    if(bRefill)
      Refill(iFrame + 1);
  }
}
