/******************************************************************************
*
* Copyright (C) 2016-2020 Allegro DVT2.  All rights reserved.
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

#include "mediatype_codec_itu.h"
#include "mediatype_checks.h"
#include <cassert>

using namespace std;

vector<Format> CreateFormatsSupported(vector<ColorType> colors, vector<int> bitdepths)
{
  vector<Format> formatsSupported;

  for(auto color : colors)
  {
    for(auto bitdepth : bitdepths)
    {
      Format format;
      format.color = color;
      format.bitdepth = bitdepth;
      formatsSupported.push_back(format);
    }
  }

  return formatsSupported;
}

void CreateFormatsSupportedMap(vector<ColorType> colors, vector<int> bitdepths, map<Format, vector<Format>>& mapFormats)
{
  // by default, all format only support their own format
  for(auto color : colors)
  {
    for(auto bitdepth : bitdepths)
    {
      Format format;
      format.color = color;
      format.bitdepth = bitdepth;
      mapFormats[format] = { format };
    }
  }
}

vector<Format> CreateFormatsSupportedByCurrent(Format current, map<Format, vector<Format>> formats)
{
  vector<Format> supported = formats[current];
  assert(supported.size() > 0);
  return supported;
}

bool UpdateBufferHandles(BufferHandles& current, BufferHandles bufferHandles)
{
  if(!CheckBufferHandles(bufferHandles))
    return false;

  current = bufferHandles;
  return true;
}
