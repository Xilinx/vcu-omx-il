// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "convert_module_soft_dec.h"
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
