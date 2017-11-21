/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
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

#include "omx_convert_omx_to_module.h"

#include <OMX_VideoExt.h>
#include <stdexcept>
#include <cmath>

bool ConvertToModuleBool(OMX_BOOL const& boolean)
{
  if(boolean == OMX_FALSE)
    return false;
  return true;
}

CompressionType ConvertToModuleCompression(OMX_VIDEO_CODINGTYPE const& coding)
{
  switch(static_cast<OMX_U32>(coding))
  {
  case OMX_VIDEO_CodingUnused: return COMPRESSION_UNUSED;
  case OMX_VIDEO_CodingAVC: return COMPRESSION_AVC;
  case OMX_ALG_VIDEO_CodingHEVC: return COMPRESSION_HEVC;
  case OMX_ALG_VIDEO_CodingVP9: return COMPRESSION_VP9;
  case OMX_VIDEO_CodingMax: return COMPRESSION_MAX;
  default:
    throw std::invalid_argument("coding");
  }

  throw std::invalid_argument("coding");
}

ColorType ConvertToModuleColor(OMX_COLOR_FORMATTYPE const& format)
{
  switch(static_cast<OMX_U32>(format))
  {
  case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked:
    return COLOR_420;
  case OMX_COLOR_FormatYUV422SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked:
    return COLOR_422;
  case OMX_COLOR_FormatMonochrome: return COLOR_MONO;
  case OMX_COLOR_FormatUnused: return COLOR_UNUSED;
  default:
    throw std::invalid_argument("format");
  }

  throw std::invalid_argument("format");
}

int ConvertToModuleBitdepth(OMX_COLOR_FORMATTYPE const& format)
{
  switch(static_cast<OMX_U32>(format))
  {
  case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_COLOR_FormatYUV422SemiPlanar:
    return 8;
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked:
    return 10;
  case OMX_COLOR_FormatMonochrome: return 0; // XXX
  case OMX_COLOR_FormatUnused: return 0; // XXX
  default:
    throw std::invalid_argument("color");
  }

  throw std::invalid_argument("color");
}

Clock ConvertToModuleClock(OMX_U32 const& framerateInQ16)
{
  Clock clock;
  clock.framerate = std::ceil(framerateInQ16 / 65536.0);
  clock.clockratio = std::rint((clock.framerate * 1000.0 * 65536.0) / framerateInQ16);
  return clock;
}

bool ConvertToModuleFileDescriptor(OMX_ALG_BUFFER_MODE const& bufferMode)
{
  switch(bufferMode)
  {
  case OMX_ALG_BUF_NORMAL: return false;
  case OMX_ALG_BUF_DMA: return true;
  case OMX_ALG_BUF_MAX: // fallthrough
  default:
    throw std::invalid_argument("bufferMode");
  }

  throw std::invalid_argument("bufferMode");
}

