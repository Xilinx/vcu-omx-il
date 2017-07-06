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

#define LOGV(...) ((void)0)
#define LOGI(...) ((void)0)
#define LOGW(...) ((void)0)
#define LOGE(...) ((void)0)

#ifdef DEBUG
#define DEBUG_CORE 1
#else
#define DEBUG_CORE 0
#endif /* DEBUG */

#define debug_print(fmt, ...) \
  do { if(DEBUG_CORE) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                              __LINE__, __FUNCTION__, __VA_ARGS__); } while(0)

#include <OMX_Core.h>
#define OMX_MAX_COMP_ROLES 1

typedef struct
{
  const char name[OMX_MAX_STRINGNAME_SIZE];   // Component name
  void* pLibHandle;                           // Library Handle
  const char* pSoLibName;                     // Dynamic Library Name
  const int nRoles;                  // Number of roles
  const char* roles[OMX_MAX_COMP_ROLES];      // Component roles
}omx_comp_type;

static omx_comp_type AL_COMP_LIST[] =
{
  {
    "OMX.allegro.h265.encoder",
    NULL,
    "libOMX.allegro.video_encoder.so",
    1,
    {
      "video_encoder.hevc",
    }
  },
  {
    "OMX.allegro.h264.encoder",
    NULL,
    "libOMX.allegro.video_encoder.so",
    1,
    {
      "video_encoder.avc",
    }
  },
  {
    "OMX.allegro.h265.decoder",
    NULL,
    "libOMX.allegro.video_decoder.so",
    1,
    {
      "video_decoder.hevc",
    }
  },
  {
    "OMX.allegro.h264.decoder",
    NULL,
    "libOMX.allegro.video_decoder.so",
    1,
    {
      "video_decoder.avc",
    }
  }
};

const int NB_OF_COMP = sizeof(AL_COMP_LIST) / sizeof(omx_comp_type);

