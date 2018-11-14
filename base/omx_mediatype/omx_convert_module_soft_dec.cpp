/******************************************************************************
*
* Copyright (C) 2018 Allegro DVT2.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX OR ALLEGRO DVT2 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of  Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
* Except as contained in this notice, the name of Allegro DVT2 shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Allegro DVT2.
*
******************************************************************************/

#include "omx_convert_module_soft_dec.h"
#include <cassert>

AL_EDpbMode ConvertModuleToSoftDecodedPictureBuffer(DecodedPictureBufferType mode)
{
  switch(mode)
  {
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NORMAL: return AL_DPB_NORMAL;
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NO_REORDERING: return AL_DPB_NO_REORDERING;
  case DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM: return AL_DPB_MAX_ENUM;
  default: return AL_DPB_MAX_ENUM;
  }

  return AL_DPB_MAX_ENUM;
}

AL_EDecUnit ConvertModuleToSoftDecodeUnit(DecodeUnitType unit)
{
  switch(unit)
  {
  case DecodeUnitType::DECODE_UNIT_FRAME: return AL_AU_UNIT;
  case DecodeUnitType::DECODE_UNIT_SLICE: return AL_VCL_NAL_UNIT;
  case DecodeUnitType::DECODE_UNIT_MAX_ENUM: return AL_DEC_UNIT_MAX_ENUM;
  default: return AL_DEC_UNIT_MAX_ENUM;
  }

  return AL_DEC_UNIT_MAX_ENUM;
}

DecodedPictureBufferType ConvertSoftToModuleDecodedPictureBuffer(AL_EDpbMode mode)
{
  switch(mode)
  {
  case AL_DPB_NORMAL: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NORMAL;
  case AL_DPB_NO_REORDERING: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_NO_REORDERING;
  case AL_DPB_MAX_ENUM: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
  default: return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
  }

  return DecodedPictureBufferType::DECODED_PICTURE_BUFFER_MAX_ENUM;
}

DecodeUnitType ConvertSoftToModuleDecodeUnit(AL_EDecUnit unit)
{
  switch(unit)
  {
  case AL_AU_UNIT: return DecodeUnitType::DECODE_UNIT_FRAME;
  case AL_VCL_NAL_UNIT: return DecodeUnitType::DECODE_UNIT_SLICE;
  case AL_DEC_UNIT_MAX_ENUM: return DecodeUnitType::DECODE_UNIT_MAX_ENUM;
  default: return DecodeUnitType::DECODE_UNIT_MAX_ENUM;
  }

  return DecodeUnitType::DECODE_UNIT_MAX_ENUM;
}

