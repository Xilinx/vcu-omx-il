/**
 * Copyright (C) 2018 Allegro DVT2.  All rights reserved.
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

/**
 * @file OMX_IVCommon.h - OpenMax IL version 1.1.2
 *  The structures needed by Video and Image components to exchange
 *  parameters and configuration data with the components.
 */
#ifndef OMX_IVCommonAlg_h
#define OMX_IVCommonAlg_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Each OMX header must include all required header files to allow the header
 * to compile without errors.  The includes below are required for this header
 * file to compile successfully
 */

// This value already exist in OpenMax IL version 1.2 (3.7.3.7.1)
#define OMX_InterlaceFrameProgressive                   0x00000001
#define OMX_InterlaceInterleaveFrameTopFieldFirst       0x00000002
#define OMX_InterlaceInterleaveFrameBottomFieldFirst    0x00000004
#define OMX_InterlaceFrameTopFieldFirst                 0x00000008
#define OMX_InterlaceFrameBottomFieldFirst              0x00000010
#define OMX_InterlaceInterleaveFieldTop                 0x00000020
#define OMX_InterlaceInterleaveFieldBottom              0x00000040

// Alternate interlaced format
#define OMX_ALG_InterlaceAlternateTopFieldFirst         0x00000080
#define OMX_ALG_InterlaceAlternateBottomFieldFirst      0x00000100

typedef struct OMX_INTERLACEFORMATTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nFormat;
    OMX_TICKS nTimeStamp;
} OMX_INTERLACEFORMATTYPE;

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
/* File EOF */

