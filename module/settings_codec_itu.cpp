/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
******************************************************************************/

#include "settings_codec_itu.h"
#include "settings_checks.h"
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
