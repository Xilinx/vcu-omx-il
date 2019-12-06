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
