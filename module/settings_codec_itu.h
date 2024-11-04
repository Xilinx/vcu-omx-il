// SPDX-FileCopyrightText: © 2024 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <map>
#include "module_enums.h"
#include "module_structs.h"

std::vector<Format> CreateFormatsSupported(std::vector<ColorType> const& colors, std::vector<int> const& bitdepths, std::vector<StorageType> const& storages);
bool UpdateBufferHandles(BufferHandles& current, BufferHandles const& bufferHandles);
