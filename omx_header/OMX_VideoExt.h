/*
 * Copyright (c) 2010 The Khronos Group Inc.
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

/** OMX_VideoExt.h - OpenMax IL version 1.1.2
 * The OMX_VideoExt header file contains extensions to the
 * definitions used by both the application and the component to
 * access video items.
 */

#ifndef OMX_VideoExt_h
#define OMX_VideoExt_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header shall include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */
#include <OMX_Core.h>
#include <OMX_Video.h>
#include <OMX_Allegro.h>

/** NALU Formats */
typedef enum OMX_NALUFORMATSTYPE
{
  OMX_NaluFormatStartCodes = 1,
  OMX_NaluFormatOneNaluPerBuffer = 2,
  OMX_NaluFormatOneByteInterleaveLength = 4,
  OMX_NaluFormatTwoByteInterleaveLength = 8,
  OMX_NaluFormatFourByteInterleaveLength = 16,
  OMX_NaluFormatCodingMax = 0x7FFFFFFF
}OMX_NALUFORMATSTYPE;

/** NAL Stream Format */
typedef struct OMX_NALSTREAMFORMATTYPE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_NALUFORMATSTYPE eNaluFormat;
}OMX_NALSTREAMFORMATTYPE;

/** Enum for standard video codingtype extensions */
typedef enum OMX_VIDEO_CODINGEXTTYPE
{
  OMX_VIDEO_ExtCodingUnused = OMX_VIDEO_CodingKhronosExtensions,
  OMX_VIDEO_CodingVP8,        /**< VP8/WebM */
  OMX_VIDEO_CodingHEVC = OMX_VIDEO_CodingVendorStartUnused,       /**< H.265/HEVC */
  OMX_VIDEO_CodingVP9         /**< VP9/WebM */
}OMX_VIDEO_CODINGEXTTYPE;

/** VP8 profiles */
typedef enum OMX_VIDEO_VP8PROFILETYPE
{
  OMX_VIDEO_VP8ProfileMain = 0x01,
  OMX_VIDEO_VP8ProfileUnknown = 0x6EFFFFFF,
  OMX_VIDEO_VP8ProfileMax = 0x7FFFFFFF
}OMX_VIDEO_VP8PROFILETYPE;

/** VP8 levels */
typedef enum OMX_VIDEO_VP8LEVELTYPE
{
  OMX_VIDEO_VP8Level_Version0 = 0x01,
  OMX_VIDEO_VP8Level_Version1 = 0x02,
  OMX_VIDEO_VP8Level_Version2 = 0x04,
  OMX_VIDEO_VP8Level_Version3 = 0x08,
  OMX_VIDEO_VP8LevelUnknown = 0x6EFFFFFF,
  OMX_VIDEO_VP8LevelMax = 0x7FFFFFFF
}OMX_VIDEO_VP8LEVELTYPE;

/** VP8 Param */
typedef struct OMX_VIDEO_PARAM_VP8TYPE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_VIDEO_VP8PROFILETYPE eProfile;
  OMX_VIDEO_VP8LEVELTYPE eLevel;
  OMX_U32 nDCTPartitions;
  OMX_BOOL bErrorResilientMode;
}OMX_VIDEO_PARAM_VP8TYPE;

/** Structure for configuring VP8 reference frames */
typedef struct OMX_VIDEO_VP8REFERENCEFRAMETYPE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bPreviousFrameRefresh;
  OMX_BOOL bGoldenFrameRefresh;
  OMX_BOOL bAlternateFrameRefresh;
  OMX_BOOL bUsePreviousFrame;
  OMX_BOOL bUseGoldenFrame;
  OMX_BOOL bUseAlternateFrame;
}OMX_VIDEO_VP8REFERENCEFRAMETYPE;

/** Structure for querying VP8 reference frame type */
typedef struct OMX_VIDEO_VP8REFERENCEFRAMEINFOTYPE
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_BOOL bIsIntraFrame;
  OMX_BOOL bIsGoldenOrAlternateFrame;
}OMX_VIDEO_VP8REFERENCEFRAMEINFOTYPE;

/** HEVC loop filter modes */
typedef enum OMX_VIDEO_HEVCLOOPFILTERTYPE
{
  OMX_VIDEO_HEVCLoopFilterEnable = 0, /**< Loop filter enable */
  OMX_VIDEO_HEVCLoopFilterDisable,  /**< Loop filter disable */
  OMX_VIDEO_HEVCLoopFilterDisableCrossSlice, /**< Loop filter enable with Cross Slice disable */
  OMX_VIDEO_HEVCLoopFilterDisableCrossTile, /**< Loop filter enable with Cross Tile disable */
  OMX_VIDEO_HEVCLoopFilterDisableCrossSliceAndTile, /**< Loop filter enable with Cross Slice and Cross Tile disable */
  OMX_VIDEO_HEVCLoopFilterMax = 0x7FFFFFFF
}OMX_VIDEO_HEVCLOOPFILTERTYPE;

/** HEVC profiles */
typedef enum OMX_VIDEO_HEVCPROFILETYPE
{
  OMX_VIDEO_HEVCProfileMain = 0x01, /**< Main profile */
  OMX_VIDEO_HEVCProfileMain10 = 0x02, /**< Main 10 profile */
  OMX_VIDEO_HEVCProfileMainStill = 0x04, /**< Main still profile */
  OMX_VIDEO_HEVCProfileMain422 = 0x08, /**< Main 4:2:2 profile  (8 bits) */
  OMX_VIDEO_HEVCProfileMain422_10 = 0x10, /**< Main 4:2:2 profile (8/10 bits) */
  OMX_VIDEO_HEVCProfileMax = 0x7FFFFFFF
}OMX_VIDEO_HEVCPROFILETYPE;

/** HEVC levels */
typedef enum OMX_VIDEO_HEVCLEVELTYPE
{
  OMX_VIDEO_HEVCLevelUnknown = 0x0,
  OMX_VIDEO_HEVCMainTierLevel1 = 0x1,
  OMX_VIDEO_HEVCHighTierLevel1 = 0x2,
  OMX_VIDEO_HEVCMainTierLevel2 = 0x4,
  OMX_VIDEO_HEVCHighTierLevel2 = 0x8,
  OMX_VIDEO_HEVCMainTierLevel21 = 0x10,
  OMX_VIDEO_HEVCHighTierLevel21 = 0x20,
  OMX_VIDEO_HEVCMainTierLevel3 = 0x40,
  OMX_VIDEO_HEVCHighTierLevel3 = 0x80,
  OMX_VIDEO_HEVCMainTierLevel31 = 0x100,
  OMX_VIDEO_HEVCHighTierLevel31 = 0x200,
  OMX_VIDEO_HEVCMainTierLevel4 = 0x400,
  OMX_VIDEO_HEVCHighTierLevel4 = 0x800,
  OMX_VIDEO_HEVCMainTierLevel41 = 0x1000,
  OMX_VIDEO_HEVCHighTierLevel41 = 0x2000,
  OMX_VIDEO_HEVCMainTierLevel5 = 0x4000,
  OMX_VIDEO_HEVCHighTierLevel5 = 0x8000,
  OMX_VIDEO_HEVCMainTierLevel51 = 0x10000,
  OMX_VIDEO_HEVCHighTierLevel51 = 0x20000,
  OMX_VIDEO_HEVCMainTierLevel52 = 0x40000,
  OMX_VIDEO_HEVCHighTierLevel52 = 0x80000,
  OMX_VIDEO_HEVCMainTierLevel6 = 0x100000,
  OMX_VIDEO_HEVCHighTierLevel6 = 0x200000,
  OMX_VIDEO_HEVCMainTierLevel61 = 0x400000,
  OMX_VIDEO_HEVCHighTierLevel61 = 0x800000,
  OMX_VIDEO_HEVCMainTierLevel62 = 0x1000000,
  OMX_VIDEO_HEVCHighTierLevel62 = 0x2000000,
  OMX_VIDEO_HEVCHighTierMax = 0x7FFFFFFF
}OMX_VIDEO_HEVCLEVELTYPE;

/** HEVC Param */
typedef struct OMX_VIDEO_PARAM_HEVCTYPE
{
  OMX_U32 nSize;  /**< Size of the structure in bytes */
  OMX_VERSIONTYPE nVersion; /**< OMX speicification version information */
  OMX_U32 nPortIndex; /**< Port that this structure applies to */
  OMX_U32 nPFrames; /**< Number of P frames between each I frame */
  OMX_U32 nBFrames; /**< Number of B frames between each I frame */
  OMX_VIDEO_HEVCPROFILETYPE eProfile; /**< HEVC profile(s) to use */
  OMX_VIDEO_HEVCLEVELTYPE eLevel; /**< HEVC max level available */
  OMX_BOOL bconstIpred; /**< Enable/disable intra prediction */
  OMX_U32 nCabacInitIdc; /**< Index used to init CAVAC contexts */
  OMX_VIDEO_HEVCLOOPFILTERTYPE eLoopFilterMode; /**< Enable/disable loop filter */
}OMX_VIDEO_PARAM_HEVCTYPE;

/** VP9 profiles */
typedef enum OMX_VIDEO_VP9PROFILETYPE
{
  OMX_VIDEO_VP9Profile = 0x01, /**< Profile 0 */
  OMX_VIDEO_VP9Profile1 = 0x02, /**< Profile 1 */
}OMX_VIDEO_VP9PROFILETYPE;

/** VP9 levels */
typedef enum OMX_VIDEO_VP9LEVELTYPE
{
  OMX_VIDEO_VP9LevelNo = 0x01 /**< VP9 Doesn't have any level */
}OMX_VIDEO_VP9LEVELTYPE;

/** VP9 loop filter modes */
typedef enum OMX_VIDEO_VP9LOOPFILTERTYPE
{
  OMX_VIDEO_VP9LoopFilterEnable = 0, /**< Loop filter enable */
  OMX_VIDEO_VP9LoopFilterDisable, /**< Loop filter disable */
  OMX_VIDEO_VP9LoopFilterDisableCrossTile, /**< Loop filter enable with Cross Tile disable */
  OMX_VIDEO_VP9LoopFilterMax = 0x7FFFFFFF
}OMX_VIDEO_VP9LOOPFILTERTYPE;

/** VP9 Param */
typedef struct OMX_VIDEO_PARAM_VP9TYPE
{
  OMX_U32 nSize;  /**< Size of the structure in bytes */
  OMX_VERSIONTYPE nVersion; /**< OMX specification version information */
  OMX_U32 nPortIndex; /**< Port that thus structure applies to */
  OMX_U32 nPFrames; /**< Number of P frames between each I frame */
  OMX_U32 nBFrames; /**< Number of B frames between each I frame */
  OMX_VIDEO_VP9PROFILETYPE eProfile;  /**< VP9 profile(s) to use */
  OMX_VIDEO_VP9LEVELTYPE eLevel;  /**< VP9 max level available */
  OMX_VIDEO_VP9LOOPFILTERTYPE eLoopFilterMode;  /**< Enable/disable loop filter */
}OMX_VIDEO_PARAM_VP9TYPE;

/** AVC Extended level */
typedef enum OMX_VIDEO_AVCLEVELEXTTYPE
{
  OMX_VIDEO_AVCLevel52 = OMX_VIDEO_AVCLevelVendorStartUnused + 0x00000001
}OMX_VIDEO_AVCLEVELEXTTYPE;

/** Group of pictures Control mode */
typedef struct OMX_VIDEO_PARAM_GOPCONTROL
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_AL_EGopCtrlMode eGopControlMode;
  OMX_AL_EGdrMode eGdrMode;
}OMX_VIDEO_PARAM_GOPCONTROL;

/** Encoder Channel parameters */
typedef struct OMX_VIDEO_PARAM_ENCODER_CHANNEL
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_S32 nNumSlices;
  OMX_BOOL bDisableSceneChangeVersatility;
  OMX_U32 nL2CacheSize;
  OMX_AL_EQpCtrlMode eQpControlMode;
  OMX_U32 nCodedPictureBufferSize;
  OMX_U32 nInitialRemovalDelay;
  OMX_AL_EScalingList eScalingListMode;
}OMX_VIDEO_PARAM_ENCODER_CHANNEL;

/** Decoder Channel parameters */
typedef struct OMX_VIDEO_PARAM_DECODER_CHANNEL
{
  OMX_U32 nSize;
  OMX_VERSIONTYPE nVersion;
  OMX_U32 nPortIndex;
  OMX_AL_EDpbMode eDecodedPictureBufferMode;
  OMX_U32 nBufferingCount;
  OMX_U32 nBufferCountHeldByNextComponent;
}OMX_VIDEO_PARAM_DECODER_CHANNEL;

/**
 * Extented enumeration of video formats
 *
 * ENUMS:
 *  ExtUnused                   : Placeholder value when format is N/A
 *  YUV420SemiPlanar10bitPacked : //XXX
 *  YUV422SemiPlanar10bitPacked : //XXX
 */
typedef enum OMX_COLOR_FORMATEXTTYPE
{
  OMX_COLOR_FormatExtUnused = OMX_COLOR_FormatVendorStartUnused,
  OMX_COLOR_FormatYUV420SemiPlanar10bitPacked,
  OMX_COLOR_FormatYUV422SemiPlanar10bitPacked,
}OMX_COLOR_EXTFORMATTYPE;

/**
 * Enumeration of vendor specifics possible bitrate control types
 */
typedef enum OMX_VIDEO_CONTROLRATEEXTTYPE
{
  OMX_Video_ControlRateExtStartUnused = OMX_Video_ControlRateVendorStartUnused,
  OMX_Video_ControlRateLowLatency,
}OMX_VIDEO_CONTROLRATEEXTTYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_VideoExt_h */
/* File EOF */

