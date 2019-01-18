/*
 * Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

/** OMX_VideoHEVCAndroid.h - OpenMax IL version 1.1.2
 * This header contains an API compatiblity layer defining a subset of the
 * OMX HEVC video API defined by Android:
 * https://android.googlesource.com/platform/frameworks/native/+/master/include/media/openmax/
 *
 * It's aimed to ease porting or code sharing of applications wishing to support
 * the Allegro and Android OMX stack.
 * Note that ABI compatiblity is NOT guaranteed. The actual values of the enums
 * may differ from Android.
 */

#ifndef OMX_VideoHEVCAndroid_h
#define OMX_VideoHEVCAndroid_h

#include <OMX_VideoAlg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define OMX_VIDEO_CodingHEVC OMX_ALG_VIDEO_CodingHEVC

/** HEVC Profile enum type */
typedef enum OMX_VIDEO_HEVCPROFILETYPE
{
  OMX_VIDEO_HEVCProfileUnknown      = 0x0,
  OMX_VIDEO_HEVCProfileMain         = OMX_ALG_VIDEO_HEVCProfileMain,
  OMX_VIDEO_HEVCProfileMain10       = OMX_ALG_VIDEO_HEVCProfileMain10,
  OMX_VIDEO_HEVCProfileMax          = OMX_ALG_VIDEO_HEVCProfileMaxEnum,
  /* Not implemented */
  OMX_VIDEO_HEVCProfileMain10HDR10  = OMX_ALG_VIDEO_HEVCProfileMaxEnum,
} OMX_VIDEO_HEVCPROFILETYPE;

/** HEVC Level enum type */
typedef enum OMX_VIDEO_HEVCLEVELTYPE
{
  OMX_VIDEO_HEVCLevelUnknown    = 0x0,
  OMX_VIDEO_HEVCMainTierLevel1  = OMX_ALG_VIDEO_HEVCMainTierLevel1,
  OMX_VIDEO_HEVCMainTierLevel2  = OMX_ALG_VIDEO_HEVCMainTierLevel2,
  OMX_VIDEO_HEVCMainTierLevel21 = OMX_ALG_VIDEO_HEVCMainTierLevel21,
  OMX_VIDEO_HEVCMainTierLevel3  = OMX_ALG_VIDEO_HEVCMainTierLevel3,
  OMX_VIDEO_HEVCMainTierLevel31 = OMX_ALG_VIDEO_HEVCMainTierLevel31,
  OMX_VIDEO_HEVCMainTierLevel4  = OMX_ALG_VIDEO_HEVCMainTierLevel4,
  OMX_VIDEO_HEVCHighTierLevel4  = OMX_ALG_VIDEO_HEVCHighTierLevel4,
  OMX_VIDEO_HEVCMainTierLevel41 = OMX_ALG_VIDEO_HEVCMainTierLevel41,
  OMX_VIDEO_HEVCHighTierLevel41 = OMX_ALG_VIDEO_HEVCHighTierLevel41,
  OMX_VIDEO_HEVCMainTierLevel5  = OMX_ALG_VIDEO_HEVCMainTierLevel5,
  OMX_VIDEO_HEVCHighTierLevel5  = OMX_ALG_VIDEO_HEVCHighTierLevel5,
  OMX_VIDEO_HEVCMainTierLevel51 = OMX_ALG_VIDEO_HEVCMainTierLevel51,
  OMX_VIDEO_HEVCHighTierLevel51 = OMX_ALG_VIDEO_HEVCHighTierLevel51,
  OMX_VIDEO_HEVCMainTierLevel52 = OMX_ALG_VIDEO_HEVCMainTierLevel52,
  OMX_VIDEO_HEVCHighTierLevel52 = OMX_ALG_VIDEO_HEVCHighTierLevel52,
  OMX_VIDEO_HEVCMainTierLevel6  = OMX_ALG_VIDEO_HEVCMainTierLevel6,
  OMX_VIDEO_HEVCHighTierLevel6  = OMX_ALG_VIDEO_HEVCHighTierLevel6,
  OMX_VIDEO_HEVCMainTierLevel61 = OMX_ALG_VIDEO_HEVCMainTierLevel61,
  OMX_VIDEO_HEVCHighTierLevel61 = OMX_ALG_VIDEO_HEVCHighTierLevel61,
  OMX_VIDEO_HEVCMainTierLevel62 = OMX_ALG_VIDEO_HEVCMainTierLevel62,
  OMX_VIDEO_HEVCHighTierLevel62 = OMX_ALG_VIDEO_HEVCHighTierLevel62,
  OMX_VIDEO_HEVCHighTiermax     = OMX_ALG_VIDEO_HEVCLevelMaxEnum,
  /* Not implemented */
  OMX_VIDEO_HEVCHighTierLevel1  = OMX_ALG_VIDEO_HEVCLevelMaxEnum,
  OMX_VIDEO_HEVCHighTierLevel2  = OMX_ALG_VIDEO_HEVCLevelMaxEnum,
  OMX_VIDEO_HEVCHighTierLevel21 = OMX_ALG_VIDEO_HEVCLevelMaxEnum,
  OMX_VIDEO_HEVCHighTierLevel3  = OMX_ALG_VIDEO_HEVCLevelMaxEnum,
  OMX_VIDEO_HEVCHighTierLevel31 = OMX_ALG_VIDEO_HEVCLevelMaxEnum,
} OMX_VIDEO_HEVCLEVELTYPE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_VideoHEVCAndroid_h */
