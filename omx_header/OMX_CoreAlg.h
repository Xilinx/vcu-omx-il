/*
 * Copyright (c) 2019 Allegro DVT2. All rights reserved.
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

/** OMX_CoreAlg.h - OpenMax IL version 1.1.2
 * The OMX_CoreAlg header file contains extensions to the definitions used
 * by both the application and the component to access common items.
 */

#ifndef OMX_CoreAlg_h
#define OMX_CoreAlg_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header shall include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */
#include <OMX_Core.h>

// This buffer already exist in OpenMax IL version 1.2 (3.7.3.7.1)
// Keep there names and values
#define OMX_BUFFERFLAG_TIMESTAMPINVALID         0x00000100
#define OMX_BUFFERFLAG_READONLY                 0x00000200
#define OMX_BUFFERFLAG_ENDOFSUBFRAME            0x00000400
#define OMX_BUFFERFLAG_SKIPFRAME                0x00000800
// INTERLACED FLAG
#define OMX_ALG_BUFFERFLAG_TOP_FIELD            0x00001000
#define OMX_ALG_BUFFERFLAG_BOT_FIELD            0x00002000

/**
 * Vendor standard extension indices.
 * This enum lists the current AllegroDVT2 extension indices to OpenMAX IL.
 */

typedef enum OMX_ALG_ERRORTYPE
{
  OMX_ALG_ErrorVendorStartUnused = (OMX_S32) OMX_ErrorVendorStartUnused,

  OMX_ALG_ErrorNoChannelLeft = (OMX_S32) OMX_ErrorVendorStartUnused + 0x1000,
  OMX_ALG_ErrorChannelResourceUnavailable = (OMX_S32) OMX_ErrorVendorStartUnused + 0x1001,
  OMX_ALG_ErrorChannelResourceFragmented = (OMX_S32) OMX_ErrorVendorStartUnused + 0x1002,

  OMX_ALG_ErrorMax = (OMX_S32) 0x9000FFFF,
}OMX_ALG_ERRORTYPE;

typedef enum OMX_ALG_EVENTTYPE
{
  OMX_ALG_EventVendorStartUnused = OMX_EventVendorStartUnused,

  /** OMX_ALG_EventSEIPrefixParsed should be called when a decoder parsed a prefix SEI
   * nData1: payload type
   * nData2: payload size
   * pEventData: payload buffer (OMX_U8*)
   */
  OMX_ALG_EventSEIPrefixParsed,

  /** OMX_ALG_EventSEISuffixParsed should be called when a decoder parsed a suffix SEI
   * nData1: payload type
   * nData2: payload size
   * pEventData: payload buffer (OMX_U8*)
   */
  OMX_ALG_EventSEISuffixParsed,

  /** OMX_ALG_EventResolutionChanged should be fired when a video codec detect a resolution changes
   * Stride and nSliceHeight shall not be modified and reallocation shall not be done. Component keeps the same buffers
   * nData1: width
   * nData2: height
   * pEventData: NULL
   */
  OMX_ALG_EventResolutionChanged,

  OMX_ALG_EventMax = OMX_EventMax,
}OMX_ALG_EVENTTYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_CoreAlg_h */
/* File EOF */

