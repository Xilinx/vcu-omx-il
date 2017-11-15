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

#include <stdio.h>
#define ENABLE_LOG 1
#define VERBOSE 0


#define LOG(err, fmt, ...) \
  do { \
    if(ENABLE_LOG) \
    { \
      if(VERBOSE) \
      { \
        fprintf(stderr, "[" err "] %s:%d [%s]: " fmt "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
      } \
      else \
      { \
        fprintf(stderr, "[" err "] [%s]: " fmt "\n", __func__, ## __VA_ARGS__); \
      } \
    } \
  } while(0)

#if VERBOSE
#define LOGV(fmt, ...) LOG("V", fmt, ## __VA_ARGS__)
#else
#define LOGV(fmt, ...) ((void)0)
#endif
#define LOGI(fmt, ...) LOG("I", fmt, ## __VA_ARGS__)
#define LOGW(fmt, ...) LOG("W", fmt, ## __VA_ARGS__)
#define LOGE(fmt, ...) LOG("E", fmt, ## __VA_ARGS__)

