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

#pragma once

struct ICommandsSender
{
  virtual ~ICommandsSender() = default;
  virtual void notifySceneChange(int lookAhead) = 0;
  virtual void notifyIsLongTerm() = 0;
  virtual void notifyUseLongTerm() = 0;
  virtual void restartGop() = 0;
  virtual void restartGopRecoveryPoint() = 0;
  virtual void setGopLength(int gopLength) = 0;
  virtual void setNumB(int numB) = 0;
  virtual void setFreqIDR(int freqIDR) = 0;
  virtual void setFrameRate(int frameRate, int clockRatio) = 0;
  virtual void setBitRate(int bitRate) = 0;
  virtual void setMaxBitRate(int iTargetBitRate, int iMaxBitRate) = 0;
  virtual void setQP(int qp) = 0;
  virtual void setQPBounds(int iMinQP, int iMaxQP) = 0;
  virtual void setQPBounds_I(int iMinQP_I, int iMaxQP_I) = 0;
  virtual void setQPBounds_P(int iMinQP_P, int iMaxQP_P) = 0;
  virtual void setQPBounds_B(int iMinQP_B, int iMaxQP_B) = 0;
  virtual void setQPIPDelta(int iQPDelta) = 0;
  virtual void setQPPBDelta(int iQPDelta) = 0;
  virtual void setDynamicInput(int iInputIdx) = 0;
  virtual void setLFBetaOffset(int iBetaOffset) = 0;
  virtual void setLFTcOffset(int iTcOffset) = 0;
  virtual void setCostMode(bool bCostMode) = 0;
  virtual void setQPChromaOffsets(int iQp1Offset, int iQp2Offset) = 0;
  virtual void setAutoQP(bool bUseAutoQP) = 0;
  virtual void setHDRIndex(int iHDRIdx) = 0;
};

