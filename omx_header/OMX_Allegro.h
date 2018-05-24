/*
 * Copyright (C) 2017 Allegro DVT2.  All rights reserved.
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

#ifndef OMX_Allegro_h
#define OMX_Allegro_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Each OMX header shall include all required header files to allow the
 * header to compile without errors.  The includes below are required
 * for this header file to compile successfully
 */

/*************************************************************************//*!
   Group of Picture Control Mode
*****************************************************************************/
typedef enum
{
  OMX_ALG_GOP_MODE_DEFAULT,
  OMX_ALG_GOP_MODE_PYRAMIDAL,
  OMX_ALG_GOP_MODE_ADAPTIVE,
  OMX_ALG_GOP_MODE_LOW_DELAY_P,
  OMX_ALG_GOP_MODE_LOW_DELAY_B,
  OMX_ALG_GOP_MODE_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EGopCtrlMode;

/*************************************************************************//*!
   GDR Mode
*****************************************************************************/
typedef enum
{
  OMX_ALG_GDR_OFF,
  OMX_ALG_GDR_VERTICAL,
  OMX_ALG_GDR_HORIZONTAL,
  OMX_ALG_GDR_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EGdrMode;

/*************************************************************************//*!
   Scaling List identifier
*****************************************************************************/
typedef enum
{
  OMX_ALG_SCL_FLAT,
  OMX_ALG_SCL_DEFAULT,
  OMX_ALG_SCL_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EScalingList;

/*************************************************************************//*!
   Quantization Parameter Control Mode
*****************************************************************************/
typedef enum
{
  OMX_ALG_UNIFORM_QP, /*!< default behaviour */
  OMX_ALG_ROI_QP, /*!< must be set for dynamic roi */
  OMX_ALG_AUTO_QP, /*!< compute Qp by MB on the fly */
  OMX_ALG_MAX_ENUM_QP = 0x7FFFFFFF,
}OMX_ALG_EQpCtrlMode;

/*************************************************************************//*!
   Aspect Ratio identifer
*****************************************************************************/
typedef enum
{
  OMX_ALG_ASPECT_RATIO_AUTO,
  OMX_ALG_ASPECT_RATIO_4_3,
  OMX_ALG_ASPECT_RATIO_16_9,
  OMX_ALG_ASPECT_RATIO_NONE,
  OMX_ALG_ASPECT_RATIO_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EAspectRatio;

/*************************************************************************//*!
   Decoder DPB mode enum
*****************************************************************************/
typedef enum
{
  OMX_ALG_DPB_NORMAL,
  OMX_ALG_DPB_LOW_REF,
  OMX_ALG_DPB_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_EDpbMode;

/*************************************************************************//*!
   Region of interest quality
*****************************************************************************/
typedef enum
{
  OMX_ALG_ROI_QUALITY_HIGH,
  OMX_ALG_ROI_QUALITY_MEDIUM,
  OMX_ALG_ROI_QUALITY_LOW,
  OMX_ALG_ROI_QUALITY_DONT_CARE,
  OMX_ALG_ROI_QUALITY_MAX_ENUM = 0x7FFFFFFF,
}OMX_ALG_ERoiQuality;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_Allegro_h */
/* File EOF */

