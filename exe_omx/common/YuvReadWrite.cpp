// SPDX-FileCopyrightText: © 2024 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#include "YuvReadWrite.h"

#include <OMX_IVCommonAlg.h>
#include <utility/logger.h>
#include <utility/round.h>
#include <cstring>

#define MAX_PLANES 3

struct plane
{
  int line_count;
  int line_size;
};

static bool calcPlaneSize(struct plane* planes, OMX_COLOR_FORMATTYPE eColor, int width, int height)
{
  memset(planes, 0, sizeof(struct plane) * MAX_PLANES);
// OMX_ALG_COLOR_FORMATTYPE fmt;
// fmt = (OMX_ALG_COLOR_FORMATTYPE) eColor;
  switch((OMX_ALG_COLOR_FORMATTYPE)eColor)
  {
  ///////////////////////////////////////////////
  /// LUMA only
  // Y800
  // case OMX_COLOR_FormatL8:
  case OMX_ALG_COLOR_FormatL8:
    planes[0].line_count = height;
    planes[0].line_size = width;
    break;
  // Y010
  case OMX_ALG_COLOR_FormatL10bit:
    planes[0].line_count = height;
    planes[0].line_size = width * 2;
    break;
  // Y012
  case OMX_ALG_COLOR_FormatL12bit:
    planes[0].line_count = height;
    planes[0].line_size = width * 2;
    break;

  // T5M8
  case OMX_ALG_COLOR_FormatL8bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 4;
    break;
  // T6M8
  case OMX_ALG_COLOR_FormatL8bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 4;
    break;
  // T5MA
  case OMX_ALG_COLOR_FormatL10bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 5;
    break;
  // T5MC
  case OMX_ALG_COLOR_FormatL12bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 6;
    break;
  // T6MA
  case OMX_ALG_COLOR_FormatL10bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 5;
    break;
  // T6MC
  case OMX_ALG_COLOR_FormatL12bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 6;
    break;

  ///////////////////////////////////////////////
  /// 420
  // NV12
  // case OMX_COLOR_FormatYUV420SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar:
    planes[0].line_count = height;
    planes[0].line_size = width;
    planes[1].line_count = height / 2;
    planes[1].line_size = width;
    break;
  // P010
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bit:
    planes[0].line_count = height;
    planes[0].line_size = width * 2;
    planes[1].line_count = height / 2;
    planes[1].line_size = planes[0].line_size;
    break;
  // P012
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar12bit:
    planes[0].line_count = height;
    planes[0].line_size = width * 2;
    planes[1].line_count = height / 2;
    planes[1].line_size = planes[0].line_size;
    break;
  // T508
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar8bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 4;
    planes[1].line_count = RoundUp(height / 2, 4) / 4;
    planes[1].line_size = planes[0].line_size;
    break;
  // T608
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar8bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 4;
    planes[1].line_count = RoundUp(height / 2, 4) / 4;
    planes[1].line_size = planes[0].line_size;
    break;
  // T50A
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 5;
    planes[1].line_count = RoundUp(height / 2, 4) / 4;
    planes[1].line_size = planes[0].line_size;
    break;
  // T60A
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 5;
    planes[1].line_count = RoundUp(height / 2, 4) / 4;
    planes[1].line_size = planes[0].line_size;
    break;
  // T50C
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar12bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 6;
    planes[1].line_count = RoundUp(height / 2, 4) / 4;
    planes[1].line_size = planes[0].line_size;
    break;
  // T60C
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar12bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 6;
    planes[1].line_count = RoundUp(height / 2, 4) / 4;
    planes[1].line_size = planes[0].line_size;
    break;

  ///////////////////////////////////////////////
  /// 422
  // NV16
  // case OMX_COLOR_FormatYUV422SemiPlanar:
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar:
    planes[0].line_count = height;
    planes[0].line_size = width;
    planes[1].line_count = height;
    planes[1].line_size = width;
    break;
  // P210
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bit:
    planes[0].line_count = height;
    planes[0].line_size = width * 2;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;
  // P212
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar12bit:
    planes[0].line_count = height;
    planes[0].line_size = width * 2;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;
  // T528
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar8bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 4;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;
  // T628
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar8bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 4;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;
  // T52A
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 5;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;
  // T62A
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 5;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;
  // T52C
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar12bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 6;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;
  // T62C
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar12bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 6;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;

  ///////////////////////////////////////////////
  /// 444
  // I444
  case OMX_ALG_COLOR_FormatYUV444Planar8bit:
    planes[0].line_count = height;
    planes[0].line_size = width;
    planes[1].line_count = height;
    planes[1].line_size = width;
    planes[2].line_count = height;
    planes[2].line_size = width;
    break;
  // T548
  case OMX_ALG_COLOR_FormatYUV444Planar8bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 4;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    planes[2].line_count = planes[0].line_count;
    planes[2].line_size = planes[0].line_size;
    break;
  // T648
  case OMX_ALG_COLOR_FormatYUV444Planar8bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 4;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    planes[2].line_count = planes[0].line_count;
    planes[2].line_size = planes[0].line_size;
    break;
  // I4AL, I4CL
  case OMX_ALG_COLOR_FormatYUV444Planar10bit:
  case OMX_ALG_COLOR_FormatYUV444Planar12bit:
    planes[0].line_count = height;
    planes[0].line_size = width * 2;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    planes[2].line_count = planes[0].line_count;
    planes[2].line_size = planes[0].line_size;
    break;
  // T54A
  case OMX_ALG_COLOR_FormatYUV444Planar10bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 5;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    planes[2].line_count = planes[0].line_count;
    planes[2].line_size = planes[0].line_size;
    break;
  // T64A
  case OMX_ALG_COLOR_FormatYUV444Planar10bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 5;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    planes[2].line_count = planes[0].line_count;
    planes[2].line_size = planes[0].line_size;
    break;
  // T54C
  case OMX_ALG_COLOR_FormatYUV444Planar12bitTiled32x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 32) * 6;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    planes[2].line_count = planes[0].line_count;
    planes[2].line_size = planes[0].line_size;
    break;
  // T64C
  case OMX_ALG_COLOR_FormatYUV444Planar12bitTiled64x4:
    planes[0].line_count = RoundUp(height, 4) / 4;
    planes[0].line_size = RoundUp(width, 64) * 6;
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    planes[2].line_count = planes[0].line_count;
    planes[2].line_size = planes[0].line_size;
    break;

#define RASTER_3X10B_ON_32B_WIDTH(w) (((w + 2) / 3) * 4)
  // XV10
  case OMX_ALG_COLOR_FormatL10bitPacked:
    planes[0].line_count = height;
    planes[0].line_size = RASTER_3X10B_ON_32B_WIDTH(width);
    break;

  // XV15
  case OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked:
    planes[0].line_count = height;
    planes[0].line_size = RASTER_3X10B_ON_32B_WIDTH(width);
    planes[1].line_count = height / 2;
    planes[1].line_size = planes[0].line_size;
    break;
  // XV20
  case OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked:
    planes[0].line_count = height;
    planes[0].line_size = RASTER_3X10B_ON_32B_WIDTH(width);
    planes[1].line_count = planes[0].line_count;
    planes[1].line_size = planes[0].line_size;
    break;
#undef RASTER_3X10B_ON_32B_WIDTH

  case OMX_ALG_COLOR_FormatUnused:
  case OMX_ALG_COLOR_FormatMaxEnum:
  default:
    return false;
  }

  return true;
}

int writeOneYuvFrame(std::ofstream& ofstream, OMX_COLOR_FORMATTYPE eColor, int iWidth, int iHeight, char* pBuffer, int iBufferPlaneStride, int iBufferPlaneStrideHeight)
{
  struct plane planes[MAX_PLANES];
  int sz;
  char* line;
  bool b;

  b = calcPlaneSize(planes, eColor, iWidth, iHeight);

  if(b == false)
    return 0;

  sz = 0;

  for(int p = 0; p < MAX_PLANES; ++p)
  {
    struct plane* plane = &planes[p];

    if(plane->line_count == 0)
      break;

    line = pBuffer + (iBufferPlaneStrideHeight * iBufferPlaneStride * p);

    for(int l = 0; l < plane->line_count; ++l)
    {
      ofstream.write(line, plane->line_size);
      line += iBufferPlaneStride;
      sz += plane->line_size;
    }
  }

  return sz;
}

int readOneYuvFrame(std::ifstream& ifstream, OMX_COLOR_FORMATTYPE eColor, int iWidth, int iHeight, char* pBuffer, int iBufferPlaneStride, int iBufferPlaneStrideHeight)
{
  struct plane planes[MAX_PLANES];

  if(ifstream.peek() == EOF)
    return 0;

  bool b = calcPlaneSize(planes, eColor, iWidth, iHeight);

  if(b == false)
    return 0;

  int sz = 0;
  char* ptr = pBuffer;

  for(int i = 0; i < MAX_PLANES; ++i)
  {
    struct plane* p;
    p = &planes[i];

    if(p->line_count == 0)
      break;

    ptr = pBuffer + (iBufferPlaneStrideHeight * iBufferPlaneStride * i);

    for(int l = 0; l < p->line_count; ++l)
    {
      ifstream.read(ptr, p->line_size);
      ptr += iBufferPlaneStride;
      sz += p->line_size;
    }
  }

  return sz;
}
