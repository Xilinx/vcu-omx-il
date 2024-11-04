// SPDX-FileCopyrightText: © 2024 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "settings_codec_itu.h"
#include "settings_checks.h"
#include <cassert>

using namespace std;

vector<Format> CreateFormatsSupported(vector<ColorType> const& colors, vector<int> const& bitdepths, vector<StorageType> const& storages)
{
  vector<Format> formatsSupported;

  for(auto const& color : colors)
  {
    for(auto const& bitdepth : bitdepths)
    {
      for(auto const& storage : storages)
      {
        Format format;
        format.color = color;
        format.bitdepth = bitdepth;
        format.storage = storage;
        formatsSupported.push_back(format);
      }
    }
  }

  return formatsSupported;
}

bool UpdateBufferHandles(BufferHandles& current, BufferHandles const& bufferHandles)
{
  if(!CheckBufferHandles(bufferHandles))
    return false;

  current = bufferHandles;
  return true;
}
