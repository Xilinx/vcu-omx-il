/*
 * Copyright (c) 2017 Allegro DVT 2
 * Copyright (c) 2008 The Khronos Group Inc.
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
  OMX_AL_GOP_MODE_DEFAULT = 0x00,
  OMX_AL_GOP_MODE_PYRAMIDAL = 0x01,

  OMX_AL_GOP_MODE_BYPASS = 0x7F,

  OMX_AL_GOP_FLAG_LOW_DELAY = 0x80,
  OMX_AL_GOP_MODE_LOW_DELAY_P = OMX_AL_GOP_FLAG_LOW_DELAY | 0x00,
  OMX_AL_GOP_MODE_LOW_DELAY_B = OMX_AL_GOP_FLAG_LOW_DELAY | 0x01,
}OMX_AL_EGopCtrlMode;

/*************************************************************************//*!
   GDR Mode
*****************************************************************************/
typedef enum
{
  OMX_AL_GDR_OFF = 0x00,
  OMX_AL_GDR_VERTICAL = 0x02,
  OMX_AL_GDR_HORIZONTAL = 0x03,
}OMX_AL_EGdrMode;

/*************************************************************************//*!
   Scaling List identifier
*****************************************************************************/
typedef enum
{
  OMX_AL_FLAT = 0,
  OMX_AL_DEFAULT = 1,
  OMX_AL_CUSTOM = 2,
  OMX_AL_RANDOM_SCL = 3
}OMX_AL_EScalingList;

/*************************************************************************//*!
   Quantization Parameter Control Mode
*****************************************************************************/
typedef enum
{
  // exclusive modes
  OMX_UNIFORM_QP = 0x00, /*!< default behaviour */
  OMX_CHOOSE_QP = 0x01, /*!< used for test purpose, need preprocessing */
  OMX_RAMP_QP = 0x02, /*!< used for test purpose */
  OMX_RANDOM_QP = 0x03, /*!< used for test purpose */
  OMX_LOAD_QP = 0x04, /*!< used for test purpose */
  OMX_BORDER_QP = 0x05, /*!< used for test purpose */

  OMX_MASK_QP_TABLE = 0x07,

  // additional modes
  OMX_RANDOM_SKIP = 0x20, /*!< used for test purpose */
  OMX_RANDOM_I_ONLY = 0x40, /*!< used for test purpose */

  OMX_BORDER_SKIP = 0x100,
  OMX_FULL_SKIP = 0x200,

  OMX_MASK_QP_TABLE_EXT = 0x367,

  // Auto QP
  OMX_AUTO_QP = 0x400, /*!< compute Qp by MB on the fly */
  OMX_ADAPTIVE_AUTO_QP = 0x800, /*!< Dynamically compute Qp by MB on the fly */
  OMX_MASK_AUTO_QP = 0xC00,

  // QP table mode
  OMX_RELATIVE_QP = 0x8000,
}OMX_AL_EQpCtrlMode;

/*************************************************************************//*!
   Decoder DPB mode enum
*****************************************************************************/
typedef enum
{
  OMX_AL_DPB_NORMAL,
  OMX_AL_DPB_LOW_REF
}OMX_AL_EDpbMode;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_Allegro_h */
/* File EOF */

