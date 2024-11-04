// SPDX-FileCopyrightText: © 2024 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <fstream>

#include <omx_header/OMX_IVCommon.h>

int readOneYuvFrame(std::ifstream& ifstream, OMX_COLOR_FORMATTYPE eColor, int width, int height, char* pBuffer, int iBufferPlaneStride, int iBufferPlaneStrideHeight);
int writeOneYuvFrame(std::ofstream& ifstream, OMX_COLOR_FORMATTYPE eColor, int width, int height, char* pBuffer, int iBufferPlaneStride, int iBufferPlaneStrideHeight);
