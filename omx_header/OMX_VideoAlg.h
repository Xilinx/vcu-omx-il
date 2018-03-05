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

/** OMX_VideoAlg.h - OpenMax IL version 1.1.2
 * The OMX_VideoAlg header file contains extensions to the
 * definitions used by both the application and the component to
 * access video items.
 */

#ifndef OMX_VideoAlg_h
#define OMX_VideoAlg_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header shall include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */
#include <OMX_Video.h>
#include <OMX_Allegro.h>

/** Enum for vendor video codingtype extensions */
typedef enum OMX_ALG_VIDEO_CODINGTYPE
{
  OMX_ALG_VIDEO_CodingUnused = OMX_VIDEO_CodingVendorStartUnused,
  OMX_ALG_VIDEO_CodingHEVC,        /**< H.265/HEVC */
  OMX_ALG_VIDEO_CodingVP9,        /**< VP9/WebM */
  OMX_ALG_VIDEO_CodingMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_CODINGTYPE;

/** HEVC loop filter modes */
typedef enum OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE
{
  OMX_ALG_VIDEO_HEVCLoopFilterEnable, /**< Loop filter enable */
  OMX_ALG_VIDEO_HEVCLoopFilterDisable,  /**< Loop filter disable */
  OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSlice, /**< Loop filter enable with Cross Slice disable */
  OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossTile, /**< Loop filter enable with Cross Tile disable */
  OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSliceAndTile, /**< Loop filter enable with Cross Slice and Cross Tile disable */
  OMX_ALG_VIDEO_HEVCLoopFilterMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE;

/** HEVC profiles */
typedef enum OMX_ALG_VIDEO_HEVCPROFILETYPE
{
  OMX_ALG_VIDEO_HEVCProfileMain, /**< Main profile */
  OMX_ALG_VIDEO_HEVCProfileMain10, /**< Main 10 profile */
  OMX_ALG_VIDEO_HEVCProfileMainStill, /**< Main still profile */
  OMX_ALG_VIDEO_HEVCProfileMonochrome,
  OMX_ALG_VIDEO_HEVCProfileMonochrome10,
  OMX_ALG_VIDEO_HEVCProfileMonochrome12,
  OMX_ALG_VIDEO_HEVCProfileMonochrome16,
  OMX_ALG_VIDEO_HEVCProfileMain12,
  OMX_ALG_VIDEO_HEVCProfileMain422,
  OMX_ALG_VIDEO_HEVCProfileMain422_10,
  OMX_ALG_VIDEO_HEVCProfileMain422_12,
  OMX_ALG_VIDEO_HEVCProfileMain444,
  OMX_ALG_VIDEO_HEVCProfileMain444_10,
  OMX_ALG_VIDEO_HEVCProfileMain444_12,
  OMX_ALG_VIDEO_HEVCProfileMain_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain10_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain12_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain422_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain422_10_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain422_12_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain444_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain444_10_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain444_12_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain444_16_Intra,
  OMX_ALG_VIDEO_HEVCProfileMain444_Still,
  OMX_ALG_VIDEO_HEVCProfileMain444_16_Still,
  OMX_ALG_VIDEO_HEVCProfileMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_HEVCPROFILETYPE;

/** HEVC levels */
typedef enum OMX_ALG_VIDEO_HEVCLEVELTYPE
{
  OMX_ALG_VIDEO_HEVCLevelUnknown,
  OMX_ALG_VIDEO_HEVCMainTierLevel1,
  OMX_ALG_VIDEO_HEVCMainTierLevel2,
  OMX_ALG_VIDEO_HEVCMainTierLevel21,
  OMX_ALG_VIDEO_HEVCMainTierLevel3,
  OMX_ALG_VIDEO_HEVCMainTierLevel31,
  OMX_ALG_VIDEO_HEVCMainTierLevel4,
  OMX_ALG_VIDEO_HEVCHighTierLevel4,
  OMX_ALG_VIDEO_HEVCMainTierLevel41,
  OMX_ALG_VIDEO_HEVCHighTierLevel41,
  OMX_ALG_VIDEO_HEVCMainTierLevel5,
  OMX_ALG_VIDEO_HEVCHighTierLevel5,
  OMX_ALG_VIDEO_HEVCMainTierLevel51,
  OMX_ALG_VIDEO_HEVCHighTierLevel51,
  OMX_ALG_VIDEO_HEVCMainTierLevel52,
  OMX_ALG_VIDEO_HEVCHighTierLevel52,
  OMX_ALG_VIDEO_HEVCMainTierLevel6,
  OMX_ALG_VIDEO_HEVCHighTierLevel6,
  OMX_ALG_VIDEO_HEVCMainTierLevel61,
  OMX_ALG_VIDEO_HEVCHighTierLevel61,
  OMX_ALG_VIDEO_HEVCMainTierLevel62,
  OMX_ALG_VIDEO_HEVCHighTierLevel62,
  OMX_ALG_VIDEO_HEVCLevelMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_HEVCLEVELTYPE;

/** HEVC Param */
typedef struct OMX_ALG_VIDEO_PARAM_HEVCTYPE
{
  OMX_U32 nSize;  /**< Size of the structure in bytes */
  OMX_VERSIONTYPE nVersion; /**< OMX speicification version information */
  OMX_U32 nPortIndex; /**< Port that this structure applies to */
  OMX_U32 nPFrames; /**< Number of P frames between each I frame */
  OMX_U32 nBFrames; /**< Number of B frames between each I frame */
  OMX_ALG_VIDEO_HEVCPROFILETYPE eProfile; /**< HEVC profile(s) to use */
  OMX_ALG_VIDEO_HEVCLEVELTYPE eLevel; /**< HEVC max level available */
  OMX_BOOL bConstIpred; /**< Enable/disable intra prediction */
  OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE eLoopFilterMode; /**< Enable/disable loop filter */
}OMX_ALG_VIDEO_PARAM_HEVCTYPE;

/** VP9 profiles */
typedef enum OMX_ALG_VIDEO_VP9PROFILETYPE
{
  OMX_ALG_VIDEO_VP9Profile, /**< Profile 0 */
  OMX_ALG_VIDEO_VP9Profile1, /**< Profile 1 */
  OMX_ALG_VIDEO_VP9ProfileMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_VP9PROFILETYPE;

/** VP9 levels */
typedef enum OMX_ALG_VIDEO_VP9LEVELTYPE
{
  OMX_ALG_VIDEO_VP9LevelNo, /**< VP9 Doesn't have any level */
  OMX_ALG_VIDEO_VP9LevelMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_VP9LEVELTYPE;

/** VP9 loop filter modes */
typedef enum OMX_ALG_VIDEO_VP9LOOPFILTERTYPE
{
  OMX_ALG_VIDEO_VP9LoopFilterEnable, /**< Loop filter enable */
  OMX_ALG_VIDEO_VP9LoopFilterDisable, /**< Loop filter disable */
  OMX_ALG_VIDEO_VP9LoopFilterDisableCrossTile, /**< Loop filter enable with Cross Tile disable */
  OMX_ALG_VIDEO_VP9LoopFilterMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_VP9LOOPFILTERTYPE;

/** VP9 Param */
typedef struct OMX_ALG_VIDEO_PARAM_VP9TYPE
{
  OMX_U32 nSize;  /**< Size of the structure in bytes */
  OMX_VERSIONTYPE nVersion; /**< OMX specification version information */
  OMX_U32 nPortIndex; /**< Port that thus structure applies to */
  OMX_U32 nPFrames; /**< Number of P frames between each I frame */
  OMX_U32 nBFrames; /**< Number of B frames between each I frame */
  OMX_ALG_VIDEO_VP9PROFILETYPE eProfile;  /**< VP9 profile(s) to use */
  OMX_ALG_VIDEO_VP9LEVELTYPE eLevel;  /**< VP9 max level available */
  OMX_ALG_VIDEO_VP9LOOPFILTERTYPE eLoopFilterMode;  /**< Enable/disable loop filter */
}OMX_ALG_VIDEO_PARAM_VP9TYPE;

/** AVC Extended level */
typedef enum OMX_ALG_VIDEO_AVCLEVELTYPE
{
  OMX_ALG_VIDEO_AVCLevelUnused = OMX_VIDEO_AVCLevelVendorStartUnused,
  OMX_ALG_VIDEO_AVCLevel52,
  OMX_ALG_VIDEO_AVCLevel60,
  OMX_ALG_VIDEO_AVCLevel61,
  OMX_ALG_VIDEO_AVCLevel62,
  OMX_ALG_VIDEO_AVCLevelMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_AVCLEVELTYPE;

/** Group of pictures Control mode */
typedef struct OMX_ALG_VIDEO_PARAM_GOP_CONTROL
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EGopCtrlMode eGopControlMode;
  OMX_ALG_EGdrMode eGdrMode;
}OMX_ALG_VIDEO_PARAM_GOP_CONTROL;

/** Slices parameters */
typedef struct OMX_ALG_VIDEO_PARAM_SLICES
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_S32 nNumSlices;
  OMX_S32 nSlicesSize;
  OMX_BOOL bDependentSlices;
}OMX_ALG_VIDEO_PARAM_SLICES;

/** Scene Change resilience parameters */
typedef struct OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bDisableSceneChangeResilience;
}OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE;

/** Prefetch Buffer parameters */
typedef struct OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nPrefetchBufferSize; // In units of 1024 bytes
}OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER;

/** CPB parameters */
typedef struct OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nCodedPictureBufferSize; // In milliseconds
  OMX_U32 nInitialRemovalDelay; // In milliseconds
}OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER;

/** QP parameters */
typedef struct OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EQpCtrlMode eQpControlMode;
}OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL;

typedef struct OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_S32 nQpMin;
  OMX_S32 nQpMax;
}OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION;

/** Scaling List parameters */
typedef struct OMX_ALG_VIDEO_PARAM_SCALING_LIST
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EScalingList eScalingListMode;
}OMX_ALG_VIDEO_PARAM_SCALING_LIST;

/** DPB parameters */
typedef struct OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EDpbMode eDecodedPictureBufferMode;
}OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER;

/** ALG Internal Entropy Buffers parameters */
typedef struct OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nNumInternalEntropyBuffers;
}OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS;

/** Enable/Disable Low bandwidth */
typedef struct OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bEnableLowBandwidth;
}OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH;

/** Aspect ratio parameters */
typedef struct OMX_ALG_VIDEO_PARAM_ASPECT_RATIO
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EAspectRatio eAspectRatio;
}OMX_ALG_VIDEO_PARAM_ASPECT_RATIO;

/** Subframe parameters */
typedef struct OMX_ALG_VIDEO_PARAM_SUBFRAME
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bEnableSubframe; /* if enable, data (sent/received) should be slices */
}OMX_ALG_VIDEO_PARAM_SUBFRAME;

/** Instantaneous decoding refresh (IDR) parameters
 * Beside the first frame, IDRs are the next I-frame >= frames_count % nInstantaneousDecodingRefreshFrequency
 */
typedef struct OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nInstantaneousDecodingRefreshFrequency;
}OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH;

/** Max bitrate parameters
 * This parameters is only effective when eControlRate = OMX_Video_ControlRateVariable
 */
typedef struct OMX_ALG_VIDEO_PARAM_MAX_BITRATE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nMaxBitrate; // In kbits per second
}OMX_ALG_VIDEO_PARAM_MAX_BITRATE;

/** Filler Data parameters
 * This parameter permit to disable the filler data
 */
typedef struct OMX_ALG_VIDEO_PARAM_FILLER_DATA
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bDisableFillerData;
}OMX_ALG_VIDEO_PARAM_FILLER_DATA;

typedef enum OMX_ALG_VIDEO_BUFFER_MODE
{
  OMX_ALG_VIDEO_BUFFER_MODE_FRAME, // frame-level buffer, no latency optimization, no custom frame delimiter
  OMX_ALG_VIDEO_BUFFER_MODE_FRAME_NO_REORDERING, //frame-level buffer, reduced latency assuming only I & P frames, custom frame delimiter enabled (SEI)
  OMX_ALG_VIDEO_BUFFER_MODE_SLICE, //slice-level buffer, low latency, custom frame delimiter enabled (SEI)
  OMX_ALG_VIDEO_BUFFER_MODE_MAX = 0x7FFFFFFF,
}OMX_ALG_VIDEO_BUFFER_MODE;

/** Buffer Mode parameters
 * This parameter permit to switch buffer mode
 */
typedef struct OMX_ALG_VIDEO_PARAM_BUFFER_MODE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_VIDEO_BUFFER_MODE eMode;
}OMX_ALG_VIDEO_PARAM_BUFFER_MODE;

/** Extented enumeration of video formats */
typedef enum OMX_ALG_COLOR_FORMATTYPE
{
  OMX_ALG_COLOR_FormatUnused = OMX_COLOR_FormatVendorStartUnused,
  OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked,
  OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked,
  OMX_ALG_COLOR_FormatL10bitPacked,
  OMX_ALG_COLOR_FormatMax = 0x7FFFFFFF,
}OMX_ALG_COLOR_FORMATTYPE;

/** Extended enumeration of bitrate control types */
typedef enum OMX_ALG_VIDEO_CONTROLRATETYPE
{
  OMX_ALG_Video_ControlRateStartUnused = OMX_Video_ControlRateVendorStartUnused,
  OMX_ALG_Video_ControlRateLowLatency,
  OMX_ALG_Video_ControlRateMax = 0x7FFFFFFF,
}OMX_ALG_VIDEO_CONTROLRATETYPE;

/** Insert Instantaneous decoding refresh
 * This configuration permit to insert an Instantaneous Decoding Refresh frame
 */
typedef struct OMX_ALG_VIDEO_CONFIG_INSERT_INSTANTANEOUS_DECODING_REFRESH
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
} OMX_ALG_VIDEO_CONFIG_INSERT_INSTANTANEOUS_DECODING_REFRESH;

typedef struct OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nPFrames;
  OMX_U32 nBFrames;
}OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES;

/**
 * OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nLeft      : X Coordinate of the top left corner of the rectangle
 *  nTop       : Y Coordinate of the top left corner of the rectangle
 *  nWidth     : Width of the rectangle
 *  nHeight    : Height of the rectangle
 *  eQuality   : Quality of the region of interest
 */

typedef struct OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_S32 nLeft;
  OMX_S32 nTop;
  OMX_U32 nWidth;
  OMX_U32 nHeight;
  OMX_ALG_ERoiQuality eQuality;
}OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST;


/**
 * OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nLookAhead : Number of frames before a scene change
 */
typedef struct
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nLookAhead;
}OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_VideoAlg_h */
/* File EOF */

