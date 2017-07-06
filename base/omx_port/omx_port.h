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

#pragma once

#include "base/omx_buffer/omx_buffer.h"
#include "base/omx_port/omx_buffer_manager.h"
#include <OMX_Video.h>
#include <OMX_Component.h>
#include <list>
#include <queue>
#include <mutex>
#include <memory>

class Port
{
public:
  OMX_PARAM_BUFFERSUPPLIERTYPE& getSupplier();
  OMX_ERRORTYPE setSupplier(OMX_PARAM_BUFFERSUPPLIERTYPE bufferSupplier);
  OMX_PARAM_PORTDEFINITIONTYPE& getDefinition();
  OMX_ERRORTYPE setDefinition(OMX_PARAM_PORTDEFINITIONTYPE definition);
  OMX_VIDEO_PORTDEFINITIONTYPE& getVideoDefinition();
  OMX_ERRORTYPE setVideoDefinition(OMX_VIDEO_PORTDEFINITIONTYPE videoDefinition);

  OMX_VIDEO_PARAM_PORTFORMATTYPE const getVideoFormat();

  void createAllocator(AL_TAllocator* allocator, bool useFileDescriptor, PFN_RefCount_CallBack userCallback);
  OMX_BUFFERHEADERTYPE* allocateBuffer(int size, void* userData);
  OMX_BUFFERHEADERTYPE* useBuffer(OMX_U8* data, int size, void* userData);
  void destroyBuffer(OMX_BUFFERHEADERTYPE* header);
  bool useFileDescriptor();

  OMX_BUFFERHEADERTYPE* getBuffer();
  void putBuffer(OMX_BUFFERHEADERTYPE* header);

protected:
  /* Variables */

  /* Methods */

private:
  /* Variables */
  std::unique_ptr<BufferAllocator> allocator;
  BufferManager bufferManager;

  OMX_PARAM_PORTDEFINITIONTYPE definition;
  OMX_PARAM_BUFFERSUPPLIERTYPE supplier;

  /* Methods */
  void turnOn();
  bool isVideoDefinitionCorrect(OMX_VIDEO_PORTDEFINITIONTYPE videoDef);
};

