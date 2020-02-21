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

/** @file OMX_IndexAlg.h - OpenMax IL version 1.1.2
 * The OMX_IndexAlg header file contains extensions to the definitions
 * for both applications and components .
 */

#ifndef OMX_IndexAlg_h
#define OMX_IndexAlg_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header shall include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */
#include <OMX_Index.h>

// This value already exist in OpenMax IL version 1.2 but the index name is unclear
#define OMX_ALG_IndexParamVideoInterlaceFormatSupported 0x07000031 /**< reference: OMX_INTERLACEFORMATTYPE */

/**
 * Vendor standard extension indices.
 * This enum lists the current AllegroDVT2 extension indices to OpenMAX IL.
 */

typedef enum OMX_ALG_INDEXTYPE
{
  /* Component parameters and configurations */
  OMX_ALG_IndexVendorComponentStartUnused = OMX_IndexVendorStartUnused + 0x00100000,
  OMX_ALG_IndexParamReportedLatency,                  /**< reference: OMX_ALG_PARAM_REPORTED_LATENCY */
  OMX_ALG_IndexParamPreallocation,                    /**< reference: OMX_ALG_PARAM_PREALLOCATION */

  /* Port parameters and configurations */
  OMX_ALG_IndexVendorPortStartUnused = OMX_IndexVendorStartUnused + 0x00200000,
  OMX_ALG_IndexPortParamBufferMode,                   /**< reference: OMX_ALG_PORT_PARAM_BUFFER_MODE */
  OMX_ALG_IndexPortParamEarlyCallback,                /**< reference: OMX_ALG_PORT_PARAM_EARLY_CALLBACK */

  /* Vendor Video parameters */
  OMX_ALG_IndexParamVendorVideoStartUnused = OMX_IndexVendorStartUnused + 0x00300000,
  OMX_ALG_IndexParamVideoHevc,                        /**< reference: OMX_ALG_VIDEO_PARAM_HEVCTYPE */
  OMX_ALG_IndexParamVideoVp9,                         /**< reference: OMX_ALG_VIDEO_PARAM_VP9TYPE */
  OMX_ALG_IndexParamVideoGopControl,                  /**< reference: OMX_ALG_VIDEO_PARAM_GOP_CONTROL */
  OMX_ALG_IndexParamVideoSlices,                      /**< reference: OMX_ALG_VIDEO_PARAM_SLICES */
  OMX_ALG_IndexParamVideoSceneChangeResilience,       /**< reference: OMX_ALG_VIDEO_PARAM_SCENE_CHANGE_RESILIENCE */
  OMX_ALG_IndexParamVideoPrefetchBuffer,              /**< reference: OMX_ALG_VIDEO_PARAM_PREFETCH_BUFFER */
  OMX_ALG_IndexParamVideoCodedPictureBuffer,          /**< reference: OMX_ALG_VIDEO_PARAM_CODED_PICTURE_BUFFER */
  OMX_ALG_IndexParamVideoQuantizationControl,         /**< reference: OMX_ALG_VIDEO_PARAM_QUANTIZATION_CONTROL */
  OMX_ALG_IndexParamVideoQuantizationExtension,       /**< refenrece: OMX_ALG_VIDEO_PARAM_QUANTIZATION_EXTENSION */
  OMX_ALG_IndexParamVideoScalingList,                 /**< reference: OMX_ALG_VIDEO_PARAM_SCALING_LIST */
  OMX_ALG_IndexParamVideoDecodedPictureBuffer,        /**< reference: OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER */
  OMX_ALG_IndexParamVideoInternalEntropyBuffers,      /**< reference: OMX_ALG_VIDEO_PARAM_INTERNAL_ENTROPY_BUFFERS */
  OMX_ALG_IndexParamVideoLowBandwidth,                /**< reference: OMX_ALG_VIDEO_PARAM_LOW_BANDWIDTH */
  OMX_ALG_IndexParamVideoAspectRatio,                 /**< reference: OMX_ALG_VIDEO_PARAM_ASPECT_RATIO */
  OMX_ALG_IndexParamVideoSubframe,                    /**< reference: OMX_ALG_VIDEO_PARAM_SUBFRAME */
  OMX_ALG_IndexParamVideoInstantaneousDecodingRefresh,/**< reference: OMX_ALG_VIDEO_PARAM_INSTANTANEOUS_DECODING_REFRESH */
  OMX_ALG_IndexParamVideoMaxBitrate,                  /**< reference: OMX_ALG_VIDEO_PARAM_MAX_BITRATE */
  OMX_ALG_IndexParamVideoFillerData,                  /**< reference: OMX_ALG_VIDEO_PARAM_FILLER_DATA */
  OMX_ALG_IndexParamVideoBufferMode,                  /**< reference: OMX_ALG_VIDEO_PARAM_BUFFER_MODE */
  OMX_ALG_IndexParamVideoInterlaceFormatCurrent,      /**< reference: OMX_INTERLACEFORMATTYPE */
  OMX_ALG_IndexParamVideoLongTerm,                    /**< reference: OMX_ALG_VIDEO_PARAM_LONG_TERM */
  OMX_ALG_IndexParamVideoLookAhead,                   /**< reference: OMX_ALG_VIDEO_PARAM_LOOKAHEAD */
  OMX_ALG_IndexParamVideoTwoPass,                     /**< reference: OMX_ALG_VIDEO_PARAM_TWOPASS */
  OMX_ALG_IndexParamVideoSkipFrame,                   /**< reference: OMX_ALG_VIDEO_PARAM_SKIP_FRAME */
  OMX_ALG_IndexParamVideoColorPrimaries,              /**< reference: OMX_ALG_VIDEO_PARAM_COLOR_PRIMARIES */
  OMX_ALG_IndexParamVideoTransferCharacteristics,     /**< reference: OMX_ALG_VIDEO_PARAM_TRANSFER_CHARACTERISTICS */
  OMX_ALG_IndexParamVideoColorMatrix,                 /**< reference: OMX_ALG_VIDEO_PARAM_COLOR_MATRIX */
  OMX_ALG_IndexParamVideoInputParsed,                 /**< reference: OMX_ALG_VIDEO_PARAM_INPUT_PARSED */
  OMX_ALG_IndexParamVideoMaxPictureSize,              /**< reference: OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZE */
  OMX_ALG_IndexParamVideoMaxPictureSizes,             /**< reference: OMX_ALG_VIDEO_PARAM_MAX_PICTURE_SIZES */
  OMX_ALG_IndexParamVideoLoopFilterBeta,              /**< reference: OMX_ALG_VIDEO_PARAM_LOOP_FILTER_BETA */
  OMX_ALG_IndexParamVideoLoopFilterTc,                /**< reference: OMX_ALG_VIDEO_PARAM_LOOP_FILTER_TC */
  OMX_ALG_IndexParamVideoMaxResolutionChange,         /**< reference: OMX_ALG_VIDEO_PARAM_MAX_RESOLUTION_CHANGE */

  /* Vendor Video configrations */
  OMX_ALG_IndexConfigVendorVideoStartUnused = OMX_IndexVendorStartUnused + 0x00380000,
  OMX_ALG_IndexConfigVideoInsertInstantaneousDecodingRefresh, /**< reference: OMX_ALG_VIDEO_CONFIG_INSERT */
  OMX_ALG_IndexConfigVideoGroupOfPictures,                    /**< reference: OMX_ALG_VIDEO_CONFIG_GROUP_OF_PICTURES */
  OMX_ALG_IndexConfigVideoRegionOfInterest,                   /**< reference: OMX_ALG_VIDEO_CONFIG_REGION_OF_INTEREST */
  OMX_ALG_IndexConfigVideoNotifySceneChange,                  /**< reference: OMX_ALG_VIDEO_CONFIG_NOTIFY_SCENE_CHANGE */
  OMX_ALG_IndexConfigVideoInsertLongTerm,                     /**< reference: OMX_ALG_VIDEO_CONFIG_INSERT */
  OMX_ALG_IndexConfigVideoUseLongTerm,                        /**< reference: OMX_ALG_VIDEO_CONFIG_INSERT */
  OMX_ALG_IndexConfigVideoNotifyResolutionChange,             /**< reference: OMX_ALG_VIDEO_CONFIG_NOTIFY_RESOLUTION_CHANGE */
  OMX_ALG_IndexConfigVideoInsertPrefixSEI,                    /**< reference: OMX_ALG_VIDEO_CONFIG_SEI */
  OMX_ALG_IndexConfigVideoInsertSuffixSEI,                    /**< reference: OMX_ALG_VIDEO_CONFIG_SEI */
  OMX_ALG_IndexConfigVideoQuantizationParameterTable,         /**< reference: OMX_ALG_VIDEO_CONFIG_DATA */
  OMX_ALG_IndexConfigVideoTransferCharacteristics,            /**< reference: OMX_ALG_VIDEO_CONFIG_TRANSFER_CHARACTERISTICS */
  OMX_ALG_IndexConfigVideoColorMatrix,                        /**< reference: OMX_ALG_VIDEO_CONFIG_COLOR_MATRIX */
  OMX_ALG_IndexConfigVideoLoopFilterBeta,                     /**< reference: OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_BETA */
  OMX_ALG_IndexConfigVideoLoopFilterTc,                       /**< reference: OMX_ALG_VIDEO_CONFIG_LOOP_FILTER_TC */
  OMX_ALG_IndexConfigVideoHighDynamicRangeSEIs,               /**< reference: OMX_ALG_VIDEO_HIGH_DYNAMIC_RANGE_SEIs */

  /* Vender Image & Video common configurations */
  OMX_ALG_IndexVendorCommonStartUnused = OMX_IndexVendorStartUnused + 0x00700000,
  OMX_ALG_IndexParamCommonSequencePictureModeCurrent,         /**< reference: OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE */
  OMX_ALG_IndexParamCommonSequencePictureModeQuerySupported,  /**< reference: OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE */

  OMX_ALG_IndexMaxEnum = 0x7FFFFFFF,
}OMX_ALG_INDEXTYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_IndexAlg_h */
/* File EOF */

