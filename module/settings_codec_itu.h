// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <map>
#include "module_enums.h"
#include "module_structs.h"

std::vector<Format> CreateFormatsSupported(std::vector<ColorType> colors, std::vector<int> bitdepths);
void CreateFormatsSupportedMap(std::vector<ColorType> colors, std::vector<int> bitdepths, std::map<Format, std::vector<Format>>& mapFormats);
std::vector<Format> CreateFormatsSupportedByCurrent(Format current, std::map<Format, std::vector<Format>> formats);
bool UpdateBufferHandles(BufferHandles& current, BufferHandles bufferHandles);
