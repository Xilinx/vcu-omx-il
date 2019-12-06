/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

typedef struct
{
  char const name[OMX_MAX_STRINGNAME_SIZE];
  void* pLibHandle;
  char const* pSoLibName;
  char const role[OMX_MAX_STRINGNAME_SIZE];
}omx_comp_type;

static omx_comp_type AL_COMP_LIST[] =
{
  {
    "OMX.allegro.h265.encoder",
    nullptr,
    "libOMX.allegro.video_encoder.so",
    "video_encoder.hevc",
  },
  {
    "OMX.allegro.h265.hardware.encoder",
    nullptr,
    "libOMX.allegro.video_encoder.so",
    "video_encoder.hevc",
  },
  {
    "OMX.allegro.h264.encoder",
    nullptr,
    "libOMX.allegro.video_encoder.so",
    "video_encoder.avc",
  },
  {
    "OMX.allegro.h264.hardware.encoder",
    nullptr,
    "libOMX.allegro.video_encoder.so",
    "video_encoder.avc",
  },
  {
    "OMX.allegro.h265.decoder",
    nullptr,
    "libOMX.allegro.video_decoder.so",
    "video_decoder.hevc",
  },
  {
    "OMX.allegro.h265.hardware.decoder",
    nullptr,
    "libOMX.allegro.video_decoder.so",
    "video_decoder.hevc",
  },
  {
    "OMX.allegro.h264.decoder",
    nullptr,
    "libOMX.allegro.video_decoder.so",
    "video_decoder.avc",
  },
  {
    "OMX.allegro.h264.hardware.decoder",
    nullptr,
    "libOMX.allegro.video_decoder.so",
    "video_decoder.avc",
  },
};

