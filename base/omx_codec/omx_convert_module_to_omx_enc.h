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
#include "base/omx_mediatype/omx_mediatype_enc_enums.h"
#include "base/omx_module/omx_module_enc_enums.h"
#include "base/omx_module/omx_module_enc_structs.h"

OMX_BOOL ConvertToOMXEntropyCoding(EntropyCodingType const& mode);
OMX_U32 ConvertToOMXBFrames(Gop const& gop);
OMX_U32 ConvertToOMXPFrames(Gop const& gop);
OMX_U32 ConvertToOMXQpI(QPs const& qps);
OMX_U32 ConvertToOMXQpP(QPs const& qps);
OMX_U32 ConvertToOMXQpB(QPs const& qps);
OMX_S32 ConvertToOMXQpMin(QPs const& qps);
OMX_S32 ConvertToOMXQpMax(QPs const& qps);
OMX_ALG_EQpCtrlMode ConvertToOMXQpControl(QPs const& qps);
OMX_VIDEO_CONTROLRATETYPE ConvertToOMXControlRate(RateControlType const& mode);
OMX_ALG_EAspectRatio ConvertToOMXAspectRatio(AspectRatioType const& aspectRatio);
OMX_ALG_EGopCtrlMode ConvertToOMXGopControl(GopControlType const& mode);
OMX_ALG_EGdrMode ConvertToOMXGdr(GdrType const& gdr);
OMX_BOOL ConvertToOMXDisableSceneChangeResilience(RateControlOptionType const& option);
OMX_ALG_EScalingList ConvertToOMXScalingList(ScalingListType const& scalingLisgt);

