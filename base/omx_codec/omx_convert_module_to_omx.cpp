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

#include "omx_convert_module_to_omx.h"
#include <stdexcept>
#include <cmath>

OMX_BOOL ConvertToOMXBool(bool const& boolean)
{
  return (!boolean) ? OMX_FALSE : OMX_TRUE;
}

OMX_COLOR_FORMATTYPE ConvertToOMXColor(ColorType const& color, int const& bitdepth)
{
  switch(color)
  {
  case COLOR_UNUSED: return OMX_COLOR_FormatUnused;
  case COLOR_MAX: return OMX_COLOR_FormatMax;
  case COLOR_MONO: return OMX_COLOR_FormatMonochrome;
  case COLOR_420:
  {
    if(bitdepth == 8)
      return OMX_COLOR_FormatYUV420SemiPlanar;

    if(bitdepth == 10)
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked);

    throw std::invalid_argument("bitdepth");
  }
  case COLOR_422:
  {
    if(bitdepth == 8)
      return OMX_COLOR_FormatYUV422SemiPlanar;

    if(bitdepth == 10)
      return static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked);

    throw std::invalid_argument("bitdepth");
  }
  default:
    throw std::invalid_argument("color");
  }
}

OMX_VIDEO_CODINGTYPE ConvertToOMXCompression(CompressionType const& compression)
{
  switch(compression)
  {
  case COMPRESSION_UNUSED: return OMX_VIDEO_CodingUnused;
  case COMPRESSION_MAX: return OMX_VIDEO_CodingMax;
  case COMPRESSION_AVC: return OMX_VIDEO_CodingAVC;
  case COMPRESSION_VP9: return static_cast<OMX_VIDEO_CODINGTYPE>(OMX_ALG_VIDEO_CodingVP9);
  case COMPRESSION_HEVC: return static_cast<OMX_VIDEO_CODINGTYPE>(OMX_ALG_VIDEO_CodingHEVC);
  default:
    throw std::invalid_argument("compression");
  }
}

OMX_U32 ConvertToOMXFramerate(Clock const& clock)
{
  auto const f = ((clock.framerate * 1000.0) / clock.clockratio) * 65536.0;
  return std::ceil(f);
}

OMX_ALG_BUFFER_MODE ConvertToOMXBufferMode(bool const& useFd)
{
  if(useFd)
    return OMX_ALG_BUF_DMA;
  else
    return OMX_ALG_BUF_NORMAL;
}

