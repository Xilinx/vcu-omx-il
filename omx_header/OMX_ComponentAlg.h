/*
 * Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

/**
 * Enumuration of possible buffer mode types
 */
typedef enum OMX_ALG_BUFFER_MODE
{
  OMX_ALG_BUF_NORMAL,
  OMX_ALG_BUF_DMA,
  OMX_ALG_BUF_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_BUFFER_MODE;

/**
 * Port buffer mode parameters
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  eMode      : Buffer mode type enum
 */
typedef struct OMX_ALG_PORT_PARAM_BUFFER_MODE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_BUFFER_MODE eMode;
}OMX_ALG_PORT_PARAM_BUFFER_MODE;

/**
 * Early Callback parameters
 *
 * STRUCT MEMBERS:
 *  nSize                : Size of the structure in bytes
 *  nVersion             : OMX specification version information
 *  nPortIndex           : Port that this structure applies to
 *  bEnableEarlyCallback : Indicate if sync IP should be enabled
 */
typedef struct OMX_ALG_PORT_PARAM_EARLY_CALLBACK
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bEnableEarlyCallback;
}OMX_ALG_PORT_PARAM_EARLY_CALLBACK;

/**
 * Component reported latency parameters
 *
 * STRUCT MEMBERS:
 *  nSize    : Size of the structure in bytes
 *  nVersion : OMX specification version information
 *  nLatency : Reported latency in milliseconds
 */
typedef struct OMX_ALG_PARAM_REPORTED_LATENCY
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nLatency;
}OMX_ALG_PARAM_REPORTED_LATENCY;

/**
 * Preallocation parameters
 *
 * STRUCT MEMBERS:
 *  nSize                 : Size of the structure in bytes
 *  nVersion              : OMX specification version information
 *  bDisablePreallocation : Indicate if preallocation should be disabled
 */
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

