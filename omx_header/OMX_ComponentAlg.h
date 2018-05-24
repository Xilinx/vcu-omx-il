/*
 * Copyright (C) 2017 Allegro DVT2.  All rights reserved.
 * Copyright (c) 2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/** OMX_ComponentAlg.h - OpenMax IL version 1.1.2
 * The OMX_ComponentAlg header file contains extensions to the definitions used
 * by both the application and the component to access common items.
 */

#ifndef OMX_ComponentAlg_h
#define OMX_ComponentAlg_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header must include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */
#include <OMX_Component.h>

/** Enum buffers modes */
typedef enum OMX_ALG_BUFFER_MODE
{
  OMX_ALG_BUF_NORMAL = 0x0,
  OMX_ALG_BUF_DMA = 0x1,
  OMX_ALG_BUF_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_BUFFER_MODE;

/** Port buffers configuration */
typedef struct OMX_ALG_PORT_PARAM_BUFFER_MODE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_BUFFER_MODE eMode;
}OMX_ALG_PORT_PARAM_BUFFER_MODE;

/** Component Latency */
typedef struct OMX_ALG_PARAM_REPORTED_LATENCY
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nLatency; // Computed in milliseconds
}OMX_ALG_PARAM_REPORTED_LATENCY;

/** Component preallocation */
typedef struct OMX_ALG_PARAM_PREALLOCATION
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_BOOL bDisablePreallocation;
}OMX_ALG_PARAM_PREALLOCATION;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_ComponentAlg_h */

