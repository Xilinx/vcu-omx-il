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

#include <OMX_Video.h>
#include <OMX_Allegro.h>
#include "base/omx_checker/omx_checker.h"

extern "C"
{
#include "lib_common/SliceConsts.h"
#include "lib_common_enc/EncChanParam.h"
}
struct Quantization
{
  int initialQP;
  int IPDeltaQP;
  int PBDeltaQP;
};

class EncodingParameters
{
public:
  /* Variables */

  /* Methods */
  int getCPBSize();
  void setCPBSize(int size);
  int getInitialRemovalDelay();
  void setInitialRemovalDelay(int delay);
  int getL2CacheSize();
  void setL2CacheSize(int size);
  int getBitrate();
  void setBitrate(int bitrate);
  int getNumSlices();
  void setNumSlices(int number);
  int getLevel();
  void setLevel(int level);
  AL_EProfile getProfile();
  void setProfile(AL_EProfile);
  AL_ERateCtrlMode getRCMode();
  void setRCMode(AL_ERateCtrlMode mode);
  AL_ERateCtrlOption getRCOptions();
  void setRCOptions(AL_ERateCtrlOption opt);
  Quantization getQuantization();
  void setQuantization(Quantization const quant);
  void setGopMode(OMX_AL_EGopCtrlMode mode);
  OMX_AL_EGopCtrlMode getGopMode();
  void setQpMode(OMX_AL_EQpCtrlMode mode);
  OMX_AL_EQpCtrlMode getQpMode();
  void setGdrMode(OMX_AL_EGdrMode mode);
  OMX_AL_EGdrMode getGdrMode();
  void setScalingListMode(OMX_AL_EScalingList mode);
  OMX_AL_EScalingList getScalingListMode();

protected:
  /* Variables */

  /* Methods */

private:
  /* Variables */
  OMX_AL_EGopCtrlMode gop;
  OMX_AL_EGdrMode gdr;
  OMX_AL_EScalingList sclist;
  Quantization quant;
  OMX_AL_EQpCtrlMode qpMode;
  int bitrate;
  int numSlices;
  int l2CacheSize;
  int cpbSize;
  int initialRemovalDelay;
  AL_ERateCtrlMode rc;
  AL_ERateCtrlOption rcOptions;
  AL_EProfile profile;
  int level;

  /* Methods */
};

