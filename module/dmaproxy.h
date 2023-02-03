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

#ifndef DMAPROXY_IOCTL_H
#define DMAPROXY_IOCTL_H
#include <linux/ioctl.h>

typedef struct dmaproxy_arg
{
  int src_fd;
  int dst_fd;
  int src_offset;
  int dst_offset;
  size_t size;
}dmaproxy_arg_t;

#define DMAPROXY_IOCTL_MAGIC 0x32
#define DMAPROXY_COPY _IOWR(DMAPROXY_IOCTL_MAGIC, 1, dmaproxy_arg_t*)

#endif
