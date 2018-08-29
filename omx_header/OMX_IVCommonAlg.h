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
 * @file OMX_IVCommonAlg.h
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

/**
 * Interlaced format parameters
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nFormat    : Specifies a bitmapped value identifying the interlace formats supported by the component port. This format information identifies the temporal relationship between the two fields
 *  nTimeStamp : Specifies the temporal timestamp information for the second field. If the temporal timestamp information can not be determined for the second field, the nTimeStamp parameter for OMX_INTERLACEFORMATTYPE structure shall be set the same as the nTimeStamp parameter via the OMX_BUFFERHEADERTYPE structure
 */
typedef struct OMX_INTERLACEFORMATTYPE {
    OMX_U32 nSize;
    OMX_VERSIONTYPE nVersion;
    OMX_U32 nPortIndex;
    OMX_U32 nFormat;
    OMX_TICKS nTimeStamp;
} OMX_INTERLACEFORMATTYPE;

typedef enum OMX_ALG_SEQUENCE_PICTURE_MODE
{
  OMX_ALG_SEQUENCE_PICTURE_UNKNOWN,
  OMX_ALG_SEQUENCE_PICTURE_FRAME,
  OMX_ALG_SEQUENCE_PICTURE_FIELD,
  OMX_ALG_SEQUENCE_PICTURE_FRAME_AND_FIELD,
  OMX_ALG_SEQUENCE_PICTURE_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_SEQUENCE_PICTURE_MODE;

/**
 * Sequence picture mode parameters
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  eMode      : Sequence picture mode type enum
 *  eModeIndex : Used to query for individual profile support information. This parameter is valid only for OMX_ALG_IndexParamCommonSequencePictureModeQuerySupported index. For all other indices this parameter is to be ignored
 */
typedef struct OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_SEQUENCE_PICTURE_MODE eMode;
  OMX_U32 nModeIndex;
} OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE;

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_IVCommonAlg_h */
/* File EOF */

