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
#include <OMX_IVCommonAlg.h>

/** Enum for vendor video codingtype extensions */
typedef enum OMX_ALG_VIDEO_CODINGTYPE
{
  OMX_ALG_VIDEO_CodingUnused = OMX_VIDEO_CodingVendorStartUnused,
  OMX_ALG_VIDEO_CodingHEVC,        /**< H.265/HEVC */
  OMX_ALG_VIDEO_CodingVP9,        /**< VP9/WebM */
  OMX_ALG_VIDEO_CodingMaxEnum = 0x7FFFFFFF,
}OMX_ALG_VIDEO_CODINGTYPE;

/** HEVC loop filter modes */
typedef enum OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE
{
  OMX_ALG_VIDEO_HEVCLoopFilterEnable, /**< Loop filter enable */
  OMX_ALG_VIDEO_HEVCLoopFilterDisable,  /**< Loop filter disable */
  OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSlice, /**< Loop filter enable with Cross Slice disable */
  OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossTile, /**< Loop filter enable with Cross Tile disable */
  OMX_ALG_VIDEO_HEVCLoopFilterDisableCrossSliceAndTile, /**< Loop filter enable with Cross Slice and Cross Tile disable */
  OMX_ALG_VIDEO_HEVCLoopFilterMaxEnum = 0x7FFFFFFF,
}OMX_ALG_VIDEO_HEVCLOOPFILTERTYPE;

/** HEVC profiles */
typedef enum OMX_ALG_VIDEO_HEVCPROFILETYPE
{
  OMX_ALG_VIDEO_HEVCProfileMain = 0x00000001, /**< Main profile */
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
  OMX_ALG_VIDEO_HEVCProfileHighThroughtPut444_16_Intra,
  OMX_ALG_VIDEO_HEVCProfileMaxEnum = 0x7FFFFFFF,
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
  OMX_ALG_VIDEO_HEVCLevelMaxEnum = 0x7FFFFFFF,
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
  OMX_ALG_VIDEO_VP9ProfileMaxEnum = 0x7FFFFFFF,
}OMX_ALG_VIDEO_VP9PROFILETYPE;

/** VP9 levels */
typedef enum OMX_ALG_VIDEO_VP9LEVELTYPE
{
  OMX_ALG_VIDEO_VP9LevelNo, /**< VP9 Doesn't have any level */
  OMX_ALG_VIDEO_VP9LevelMaxEnum = 0x7FFFFFFF,
}OMX_ALG_VIDEO_VP9LEVELTYPE;

/** VP9 loop filter modes */
typedef enum OMX_ALG_VIDEO_VP9LOOPFILTERTYPE
{
  OMX_ALG_VIDEO_VP9LoopFilterEnable, /**< Loop filter enable */
  OMX_ALG_VIDEO_VP9LoopFilterDisable, /**< Loop filter disable */
  OMX_ALG_VIDEO_VP9LoopFilterDisableCrossTile, /**< Loop filter enable with Cross Tile disable */
  OMX_ALG_VIDEO_VP9LoopFilterMaxEnum = 0x7FFFFFFF,
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

/** AVC Extended profiles */
typedef enum OMX_ALG_VIDEO_AVCPROFILETYPE
{
  OMX_ALG_VIDEO_AVCProfileUnused = OMX_VIDEO_AVCProfileVendorStartUnused,
  OMX_ALG_VIDEO_AVCProfileConstrainedBaseline,
  OMX_ALG_VIDEO_AVCProfileProgressiveHigh,
  OMX_ALG_VIDEO_AVCProfileConstrainedHigh,
  OMX_ALG_VIDEO_AVCProfileHigh10_Intra,
  OMX_ALG_VIDEO_AVCProfileHigh422_Intra,
  OMX_ALG_VIDEO_AVCProfileHigh444_Intra,
  OMX_ALG_VIDEO_AVCProfileCAVLC444_Intra,
  OMX_ALG_VIDEO_XAVCProfileHigh10_Intra_CBG,
  OMX_ALG_VIDEO_XAVCProfileHigh10_Intra_VBR,
  OMX_ALG_VIDEO_XAVCProfileHigh422_Intra_CBG,
  OMX_ALG_VIDEO_XAVCProfileHigh422_Intra_VBR,
  OMX_ALG_VIDEO_XAVCProfileLongGopMain_MP4,
  OMX_ALG_VIDEO_XAVCProfileLongGopHigh_MP4,
  OMX_ALG_VIDEO_XAVCProfileLongGopHigh_MXF,
  OMX_ALG_VIDEO_XAVCProfileLongGopHigh422_MXF,
  OMX_ALG_VIDEO_AVCProfileMaxEnum = 0x7FFFFFFF,
}OMX_ALG_VIDEO_AVCPROFILETYPE;

/** AVC Extended levels */
typedef enum OMX_ALG_VIDEO_AVCLEVELTYPE
{
  OMX_ALG_VIDEO_AVCLevelUnused = OMX_VIDEO_AVCLevelVendorStartUnused,
  OMX_ALG_VIDEO_AVCLevel52,
  OMX_ALG_VIDEO_AVCLevel60,
  OMX_ALG_VIDEO_AVCLevel61,
  OMX_ALG_VIDEO_AVCLevel62,
  OMX_ALG_VIDEO_AVCLevelMaxEnum = 0x7FFFFFFF,
}OMX_ALG_VIDEO_AVCLEVELTYPE;

/**
 * Enumeration of possible group of picture (GOP) types
 */
typedef enum OMX_ALG_EGopCtrlMode
{
  OMX_ALG_GOP_MODE_DEFAULT,
  OMX_ALG_GOP_MODE_DEFAULT_B,
  OMX_ALG_GOP_MODE_PYRAMIDAL,
  OMX_ALG_GOP_MODE_PYRAMIDAL_B,
  OMX_ALG_GOP_MODE_ADAPTIVE,
  OMX_ALG_GOP_MODE_LOW_DELAY_P,
  OMX_ALG_GOP_MODE_LOW_DELAY_B,
  OMX_ALG_GOP_MODE_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EGopCtrlMode;

/**
 * Enumeration of possible gradual decoder refresh (GDR) types
 */
typedef enum OMX_ALG_EGdrMode
{
  OMX_ALG_GDR_OFF,
  OMX_ALG_GDR_VERTICAL,
  OMX_ALG_GDR_HORIZONTAL,
  OMX_ALG_GDR_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EGdrMode;

/**
 * Group of pictures control parameters
 *
 * STRUCT MEMBERS:
 *  nSize           : Size of the structure in bytes
 *  nVersion        : OMX specification version information
 *  nPortIndex      : Port that this structure applies to
 *  eGopControlMode : Group of pictures type enum
 *  eGdrMode        : Gradual decoder refresh type enum
 */
typedef struct OMX_ALG_VIDEO_PARAM_GOP_CONTROL
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EGopCtrlMode eGopControlMode;
  OMX_ALG_EGdrMode eGdrMode;
}OMX_ALG_VIDEO_PARAM_GOP_CONTROL;

/**
 * Slices parameters
 *
 * STRUCT MEMBERS:
 *  nSize            : Size of the structure in bytes
 *  nVersion         : OMX specification version information
 *  nPortIndex       : Port that this structure applies to
 *  nSlicesSize      : Size of the slices
 *  bDependentSlices : Indicate if slices are dependent
 */
typedef struct OMX_ALG_VIDEO_PARAM_SLICES
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_S32 nNumSlices;
  OMX_S32 nSlicesSize;
  OMX_BOOL bDependentSlices;
}OMX_ALG_VIDEO_PARAM_SLICES;

/**
 * LookAhead parameters
 *
 * STRUCT MEMBERS:
 *  nSize                : Size of the structure in bytes
 *  nVersion             : OMX specification version information
 *  nPortIndex           : Port that this structure applies to
 *  nLookAhead           : Indicate the Lookahead size, disabled if 0
 *  bEnableFirstPassSceneChangeDetection : Speed up the first pass, by encoding 5 zones of the frame
 */
typedef struct OMX_ALG_VIDEO_PARAM_LOOKAHEAD
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nLookAhead;
  OMX_BOOL bEnableFirstPassSceneChangeDetection;
}OMX_ALG_VIDEO_PARAM_LOOKAHEAD;

/**
 * TwoPass parameters
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nPass      : Indicate which pass we encode (1 or 2), disabled if 0
 *  cLogFile   : Path of the pass 1 statistics logfile
 */
typedef struct OMX_ALG_VIDEO_PARAM_TWOPASS
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nPass;
  OMX_U8 cLogFile[OMX_MAX_STRINGNAME_SIZE];
}OMX_ALG_VIDEO_PARAM_TWOPASS;

/**
 * Scene change resilience parameters
 *
 * STRUCT MEMBERS:
 *  nSize                         : Size of the structure in bytes
 *  nVersion                      : OMX specification version information
 *  nPortIndex                    : Port that this structure applies to
 *  bDisableSceneChangeResilience : Indicate if scene change resilience should be disabled
 */
typedef struct OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bDisableSceneChangeResilience;
}OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE;

/**
 * Prefetch buffer parameters
 *
 * STRUCT MEMBERS:
 *  nSize                 : Size of the structure in bytes
 *  nVersion              : OMX specification version information
 *  nPortIndex            : Port that this structure applies to
 *  bEnablePrefetchBuffer : Indicate if prefetch buffer should be enabled
 */
typedef struct OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bEnablePrefetchBuffer;
}OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER;

/**
 * Coded picture buffer parameters
 *
 * STRUCT MEMBERS:
 *  nSize                   : Size of the structure in bytes
 *  nVersion                : OMX specification version information
 *  nPortIndex              : Port that this structure applies to
 *  nCodedPictureBufferSize : Size of the coded picture buffer in milliseconds
 *  nInitialRemovalDelay    : Time of the initial removal delay in milliseconds
 */
typedef struct OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nCodedPictureBufferSize;
  OMX_U32 nInitialRemovalDelay;
}OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER;

/**
 * Enumeration of possible quantization parameter (QP) types
 */
typedef enum
{
  OMX_ALG_UNIFORM_QP, /*!< default behaviour */
  OMX_ALG_ROI_QP, /*!< must be set for dynamic roi */
  OMX_ALG_AUTO_QP, /*!< compute Qp by MB on the fly */
  OMX_ALG_ROI_AUTO_QP, /*!< ROI and on the fly QP offsets summed */
  OMX_ALG_MAX_ENUM_QP = 0x7FFFFFFF,
}OMX_ALG_EQpCtrlMode;

/**
 * Quantization parameter control parameters
 *
 * STRUCT MEMBERS:
 *  nSize          : Size of the structure in bytes
 *  nVersion       : OMX specification version information
 *  nPortIndex     : Port that this structure applies to
 *  eQpControlMode : Quantization parameter type enum
 */
typedef struct OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EQpCtrlMode eQpControlMode;
}OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL;

/**
 * Quantization extension parameters
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nQpMin     : Quantization parameter minimum value
 *  nQpMax     : Quantization parameter maximum value
 */
typedef struct OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_S32 nQpMin;
  OMX_S32 nQpMax;
}OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION;

/**
 *  Enumeration of possible scaling list (SCL) types
 */
typedef enum OMX_ALG_EScalingList
{
  OMX_ALG_SCL_FLAT,
  OMX_ALG_SCL_DEFAULT,
  OMX_ALG_SCL_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EScalingList;

/**
 * Scaling list parameters
 *
 * STRUCT MEMBERS:
 *  nSize            : Size of the structure in bytes
 *  nVersion         : OMX specification version information
 *  nPortIndex       : Port that this structure applies to
 *  eScalingListMode : Scaling list type enum
 */
typedef struct OMX_ALG_VIDEO_PARAM_SCALING_LIST
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EScalingList eScalingListMode;
}OMX_ALG_VIDEO_PARAM_SCALING_LIST;

/**
 * Enumeration of possible decoded picture buffer (DPB) types
 */
typedef enum OMX_ALG_EDpbMode
{
  OMX_ALG_DPB_NORMAL,
  OMX_ALG_DPB_NO_REORDERING,
  OMX_ALG_DPB_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EDpbMode;

/**
 * Decoded picture buffer parameters
 *
 * STRUCT MEMBERS:
 *  nSize                     : Size of the structure in bytes
 *  nVersion                  : OMX specification version information
 *  nPortIndex                : Port that this structure applies to
 *  eDecodedPictureBufferMode : Decoded picture buffer type enum
 */
typedef struct OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EDpbMode eDecodedPictureBufferMode;
}OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER;

/**
 * Internal entropy buffers parameters
 *
 * STRUCT MEMBERS:
 *  nSize                      : Size of the structure in bytes
 *  nVersion                   : OMX specification version information
 *  nPortIndex                 : Port that this structure applies to
 *  nNumInternalEntropyBuffers : Number of internal entropy buffers
 */
typedef struct OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nNumInternalEntropyBuffers;
}OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS;

/**
 * Low bandwidth parameters
 *
 * STRUCT MEMBERS:
 *  nSize               : Size of the structure in bytes
 *  nVersion            : OMX specification version information
 *  nPortIndex          : Port that this structure applies to
 *  bEnableLowBandwidth : Indicate if low bandwidth should be enabled
 */
typedef struct OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bEnableLowBandwidth;
}OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH;

/**
 * Enumration of possible aspect ratio types
 */
typedef enum OMX_ALG_EAspectRatio
{
  OMX_ALG_ASPECT_RATIO_AUTO,
  OMX_ALG_ASPECT_RATIO_4_3,
  OMX_ALG_ASPECT_RATIO_16_9,
  OMX_ALG_ASPECT_RATIO_NONE,
  OMX_ALG_ASPECT_RATIO_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EAspectRatio;

/**
 * Aspect ratio parameters
 *
 * STRUCT MEMBERS:
 *  nSize        : Size of the structure in bytes
 *  nVersion     : OMX specification version information
 *  nPortIndex   : Port that this structure applies to
 *  eAspectRatio : Aspect ratio type enum
 */
typedef struct OMX_ALG_VIDEO_PARAM_ASPECT_RATIO
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_EAspectRatio eAspectRatio;
}OMX_ALG_VIDEO_PARAM_ASPECT_RATIO;

/**
 * Subframe parameters
 *
 * STRUCT MEMBERS:
 *  nSize           : Size of the structure in bytes
 *  nVersion        : OMX specification version information
 *  nPortIndex      : Port that this structure applies to
 *  bEnableSubframe : Indicate if subframe should be enabled
 */
typedef struct OMX_ALG_VIDEO_PARAM_SUBFRAME
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bEnableSubframe; /* if enable, data (sent/received) should be slices */
}OMX_ALG_VIDEO_PARAM_SUBFRAME;

/**
 * Instantaneous decoding refresh parameters
 *
 * Beside the first frame, IDRs are the next I-frame >= frames_count % nInstantaneousDecodingRefreshFrequency
 *
 * STRUCT MEMBERS:
 *  nSize                                  : Size of the structure in bytes
 *  nVersion                               : OMX specification version information
 *  nPortIndex                             : Port that this structure applies to
 *  nInstantaneousDecodingRefreshFrequency : Number of frame between 2 IDR
 */
typedef struct OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nInstantaneousDecodingRefreshFrequency;
}OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH;

/**
 * Max bitrate parameters
 *
 * This parameters is only effective when eControlRate = OMX_Video_ControlRateVariable or OMX_ALG_Video_ControlRateVariableCapped
 *
 * STRUCT MEMBERS:
 *  nSize       : Size of the structure in bytes
 *  nVersion    : OMX specification version information
 *  nPortIndex  : Port that this structure applies to
 *  nMaxBitrate : Maximum bitrate threshold in kbits per second
 *  nMaxQuality : Maximum quality target. This parameter should only be used when eControlRate = OMX_ALG_Video_ControlRateVariableCapped
 */
typedef struct OMX_ALG_VIDEO_PARAM_MAX_BITRATE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nMaxBitrate;
  OMX_U32 nMaxQuality;
}OMX_ALG_VIDEO_PARAM_MAX_BITRATE;

/**
 * Filler data parameters
 *
 * STRUCT MEMBERS:
 *  nSize              : Size of the structure in bytes
 *  nVersion           : OMX specification version information
 *  nPortIndex         : Port that this structure applies to
 *  bDisableFillerData : Indicate if filler data should be disabled
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
  OMX_ALG_VIDEO_BUFFER_MODE_FRAME_NO_REORDERING, // frame-level buffer, reduced latency assuming only I & P frames, custom frame delimiter enabled (SEI)
  OMX_ALG_VIDEO_BUFFER_MODE_SLICE, // slice-level buffer, low latency, custom frame delimiter enabled (SEI)
  OMX_ALG_VIDEO_BUFFER_MODE_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_VIDEO_BUFFER_MODE;

/**
 * Buffer mode parameters
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  eMode      : Buffer mode type enum
 */
typedef struct OMX_ALG_VIDEO_PARAM_BUFFER_MODE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_VIDEO_BUFFER_MODE eMode;
}OMX_ALG_VIDEO_PARAM_BUFFER_MODE;

/**
 * Long term parameters
 *
 * STRUCT MEMBERS:
 *  nSize              : Size of the structure in bytes
 *  nVersion           : OMX specification version information
 *  nPortIndex         : Port that this structure applies to
 *  bEnableLongTerm    : Indicate if long term should be enabled
 *  nLongTermFrequency : Frequency of long term reference picture
 */
typedef struct OMX_ALG_VIDEO_PARAM_LONG_TERM
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bEnableLongTerm;
  OMX_S32 nLongTermFrequency;
}OMX_ALG_VIDEO_PARAM_LONG_TERM;

/**
 * Skip frame parameters
 *
 * STRUCT MEMBERS:
 *  nSize            : Size of the structure in bytes
 *  nVersion         : OMX specification version information
 *  nPortIndex       : Port that this structure applies to
 *  bEnableSkipFrame : Indicate if skip frame should be enabled
 */
typedef struct OMX_ALG_VIDEO_PARAM_SKIP_FRAME
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bEnableSkipFrame;
}OMX_ALG_VIDEO_PARAM_SKIP_FRAME;

/** ColorPrimaries enumeration */
typedef enum OMX_ALG_VIDEO_COLOR_PRIMARIESTYPE
{
  OMX_ALG_VIDEO_COLOR_PRIMARIES_UNSPECIFIED,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_RESERVED,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_470_NTSC,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_601_PAL,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_601_NTSC,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_709,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_BT_2020,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_240M,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_ST_428,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_RP_431,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_SMPTE_EG_432,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_GENERIC_FILM,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_EBU_3213,
  OMX_ALG_VIDEO_COLOR_PRIMARIES_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_VIDEO_COLOR_PRIMARIESTYPE;

/**
 * ColorPrimaries parameters
 *
 * STRUCT MEMBERS:
 *  nSize           : Size of the structure in bytes
 *  nVersion        : OMX specification version information
 *  nPortIndex      : Port that this structure applies to
 *  eColorPrimaries : Color primaries
 */
typedef struct OMX_ALG_VIDEO_PARAM_COLOR_PRIMARIES
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_ALG_VIDEO_COLOR_PRIMARIESTYPE eColorPrimaries;
}OMX_ALG_VIDEO_PARAM_COLOR_PRIMARIES;

/**
 * Input parsed parameters
 *
 * STRUCT MEMBERS:
 *  nSize               : Size of the structure in bytes
 *  nVersion            : OMX specification version information
 *  nPortIndex          : Port that this structure applies to
 *  bDisableInputParsed : Indicate if input parsed should be disabled
 */
typedef struct OMX_ALG_VIDEO_PARAM_INPUT_PARSED
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bDisableInputParsed;
}OMX_ALG_VIDEO_PARAM_INPUT_PARSED;


/**
 * Max picture size parameters
 *
 * STRUCT MEMBERS:
 *  nSize            : Size of the structure in bytes
 *  nVersion         : OMX specification version information
 *  nPortIndex       : Port that this structure applies to
 *  nMaxPictureSizeI : Max picture size for I frames in kbits
 *  nMaxPictureSizeP : Max picture size for P frames in kbits
 *  nMaxPictureSizeB : Max picture size for B frames in kbits
 */
typedef struct OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_S32 nMaxPictureSizeI;
  OMX_S32 nMaxPictureSizeP;
  OMX_S32 nMaxPictureSizeB;
}OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE;

/** Extented enumeration of video formats */
typedef enum OMX_ALG_COLOR_FORMATTYPE
{
  OMX_ALG_COLOR_FormatUnused = OMX_COLOR_FormatVendorStartUnused,
  OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked,
  OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked,
  OMX_ALG_COLOR_FormatL10bitPacked,
  OMX_ALG_COLOR_FormatMaxEnum = 0x7FFFFFFF,
}OMX_ALG_COLOR_FORMATTYPE;

/** Extended enumeration of bitrate control types */
typedef enum OMX_ALG_VIDEO_CONTROLRATETYPE
{
  OMX_ALG_Video_ControlRateStartUnused = OMX_Video_ControlRateVendorStartUnused,
  OMX_ALG_Video_ControlRateLowLatency,
  OMX_ALG_Video_ControlRateVariableCapped,
  OMX_ALG_Video_ControlRateMaxEnum = 0x7FFFFFFF,
}OMX_ALG_VIDEO_CONTROLRATETYPE;

/**
 * Structure for dynamically signalling component
 *
 * STRUCT MEMBERS:
 *  nSize            : Size of the structure in bytes
 *  nVersion         : OMX specification version information
 *  nPortIndex       : Port that this structure applies to
 */
typedef struct OMX_ALG_VIDEO_CONFIG_INSERT
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
}OMX_ALG_VIDEO_CONFIG_INSERT;

/**
 * Structure for dynamically changing group of picture
 *
 * STRUCT MEMBERS:
 *  nSize            : Size of the structure in bytes
 *  nVersion         : OMX specification version information
 *  nPortIndex       : Port that this structure applies to
 *  nPFrames         : Number of P frames between each I frame
 *  nBFrames         : Number of B frames between each I frame
 */
typedef struct OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nPFrames;
  OMX_U32 nBFrames;
}OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES;

/**
 * Enumeration of possible region of interest (ROI) quality types
 */
typedef enum
{
  OMX_ALG_ROI_QUALITY_HIGH,
  OMX_ALG_ROI_QUALITY_MEDIUM,
  OMX_ALG_ROI_QUALITY_LOW,
  OMX_ALG_ROI_QUALITY_DONT_CARE,
  OMX_ALG_ROI_QUALITY_INTRA,
  OMX_ALG_ROI_QUALITY_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_ERoiQuality;

/**
 * Structure for dynamically adding a region of interest
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nLeft      : X Coordinate of the top left corner of the rectangle
 *  nTop       : Y Coordinate of the top left corner of the rectangle
 *  nWidth     : Width of the rectangle
 *  nHeight    : Height of the rectangle
 *  eQuality   : Quality of the region of interest type enum
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
 * Structure for dynamically notifying a scene change
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nPortIndex : Port that this structure applies to
 *  nLookAhead : Number of frames before a scene change
 */
typedef struct OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_U32 nLookAhead;
}OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE;

/**
 * Structure for dynamically notifying a resolution change
 *
 * STRUCT MEMBERS:
 *  nSize    : Size of the structure in bytes
 *  nVersion : OMX specification version information
 *  nWidth   : Width of the resolution
 *  nHeight  : Height of the resoluton
 */
typedef struct OMX_ALG_VIDEO_CONFIG_NOTIFY_RESOLUTION_CHANGE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nWidth;
  OMX_U32 nHeight;
}OMX_ALG_VIDEO_CONFIG_NOTIFY_RESOLUTION_CHANGE;

/**
 * Struct for dynamically send sei
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  nType      : Sei payload type
 *  pBuffer    : Pointer to actual block of memory that is acting as the buffer
 *  nAllocLen  : Size of the buffer allocated, in bytes
 *  nFilledLen : Number of bytes currently in the buffer
 *  nOffset    : Start offset of valid data in bytes from the start of the buffer
 */
typedef struct OMX_ALG_VIDEO_CONFIG_SEI
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_S32 nType;
  OMX_U8* pBuffer;
  OMX_U32 nAllocLen;
  OMX_U32 nFilledLen;
  OMX_U32 nOffset;
}OMX_ALG_VIDEO_CONFIG_SEI;

/**
 * Struct for dynamically send data
 *
 * STRUCT MEMBERS:
 *  nSize      : Size of the structure in bytes
 *  nVersion   : OMX specification version information
 *  pBuffer    : Pointer to actual block of memory that is acting as the buffer
 *  nAllocLen  : Size of the buffer allocated, in bytes
 *  nFilledLen : Number of bytes currently in the buffer
 *  nOffset    : Start offset of valid data in bytes from the start of the buffer
 */
typedef struct OMX_ALG_VIDEO_CONFIG_DATA
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U8* pBuffer;
  OMX_U32 nAllocLen;
  OMX_U32 nFilledLen;
  OMX_U32 nOffset;
}OMX_ALG_VIDEO_CONFIG_DATA;

/**
 * Struct for dynamically change loop filter beta
 *
 * STRUCT MEMBERS:
 *  nSize           : Size of the structure in bytes
 *  nVersion        : OMX specification version information
 *  nLoopFilterBeta : Loop filter beta offset value
 */
typedef struct OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_BETA
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U8 nLoopFilterBeta;
}OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_BETA;

/**
 * Struct for dynamically change loop filter tc
 *
 * STRUCT MEMBERS:
 *  nSize         : Size of the structure in bytes
 *  nVersion      : OMX specification version information
 *  nLoopFilterTc : Loop filter tc offset value
 */
typedef struct OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_TC
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U8 nLoopFilterTc;
}OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_TC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_VideoAlg_h */
/* File EOF */

