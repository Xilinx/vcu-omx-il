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

#include "omx_port.h"
#include "base/omx_checker/omx_checker.h"
#include <assert.h>

void Port::createAllocator(AL_TAllocator* allocator, bool useFileDescriptor, PFN_RefCount_CallBack userCallback)
{
  this->allocator.reset(new BufferAllocator(allocator, useFileDescriptor, userCallback));
}

OMX_BUFFERHEADERTYPE* Port::getBuffer()
{
  return bufferManager.getProcess();
}

void Port::putBuffer(OMX_BUFFERHEADERTYPE* header)
{
  bufferManager.putProcess(header);
}

void Port::turnOn()
{
  definition.bPopulated = OMX_TRUE;
  definition.bEnabled = OMX_TRUE;
}

OMX_PARAM_BUFFERSUPPLIERTYPE & Port::getSupplier()
{
  return supplier;
}

OMX_ERRORTYPE Port::setSupplier(OMX_PARAM_BUFFERSUPPLIERTYPE supplier)
{
  auto const ret = OMXChecker::CheckHeaderVersion(supplier.nVersion);

  if(ret != OMX_ErrorNone)
    return ret;
  this->supplier = supplier;
  return OMX_ErrorNone;
}

OMX_PARAM_PORTDEFINITIONTYPE & Port::getDefinition()
{
  return definition;
}

OMX_ERRORTYPE Port::setDefinition(OMX_PARAM_PORTDEFINITIONTYPE definition)
{
  auto const ret = OMXChecker::CheckHeaderVersion(definition.nVersion);

  if(ret != OMX_ErrorNone)
    return ret;

  if(!isVideoDefinitionCorrect(definition.format.video))
    return OMX_ErrorBadParameter;

  this->definition = definition;

  return OMX_ErrorNone;
}

bool Port::isVideoDefinitionCorrect(OMX_VIDEO_PORTDEFINITIONTYPE videoDef)
{
  if((int)videoDef.nStride < (int)videoDef.nFrameWidth)
    return false;

  if(videoDef.nStride % 32)
    return false;

  if((int)videoDef.nSliceHeight < (int)videoDef.nFrameHeight)
    return false;

  return true;
}

OMX_VIDEO_PORTDEFINITIONTYPE & Port::getVideoDefinition()
{
  return definition.format.video;
}

OMX_ERRORTYPE Port::setVideoDefinition(OMX_VIDEO_PORTDEFINITIONTYPE videoDefinition)
{
  if(!isVideoDefinitionCorrect(videoDefinition))
    return OMX_ErrorBadParameter;

  this->definition.format.video = videoDefinition;
  return OMX_ErrorNone;
}

OMX_VIDEO_PARAM_PORTFORMATTYPE const Port::getVideoFormat()
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE format;
  OMXChecker::SetHeaderVersion(format);
  auto const videoDef = definition.format.video;
  format.nPortIndex = definition.nPortIndex;
  format.nIndex = 0;
  format.eCompressionFormat = videoDef.eCompressionFormat;
  format.eColorFormat = videoDef.eColorFormat;
  format.xFramerate = videoDef.xFramerate;
  return format;
}

OMX_BUFFERHEADERTYPE* Port::allocateBuffer(int size, void* userData)
{
  if(!allocator)
    return nullptr;

  auto header = new OMX_BUFFERHEADERTYPE;
  OMXChecker::SetHeaderVersion(*header);

  auto codecData = allocator->CreateBuffer(size, userData);
  assert(codecData);
  header->pPlatformPrivate = codecData;
  header->pBuffer = allocator->IsFileDescriptor() ? (uint8_t*)(uintptr_t)allocator->GetBufferFileDescriptor(codecData) : AL_Buffer_GetBufferData(codecData);

  bufferManager.addInAllocated(header);

  if(bufferManager.sizeAllocate() == (int)definition.nBufferCountActual)
    turnOn();

  return header;
}

OMX_BUFFERHEADERTYPE* Port::useBuffer(OMX_U8* data, int size, void* userData)
{
  if(!allocator)
    return nullptr;

  auto header = new OMX_BUFFERHEADERTYPE;
  OMXChecker::SetHeaderVersion(*header);

  auto codecData = allocator->UseBuffer(data, size, userData);
  assert(codecData);
  header->pPlatformPrivate = codecData;
  header->pBuffer = data;

  bufferManager.addInUsed(header);

  if(bufferManager.sizeUse() == (int)definition.nBufferCountActual)
    turnOn();

  return header;
}

void Port::destroyBuffer(OMX_BUFFERHEADERTYPE* header)
{
  if(bufferManager.isInUsed(header))
    bufferManager.removeFromUsed(header);

  if(bufferManager.isInAllocated(header))
    bufferManager.removeFromAllocated(header);

  auto codecData = static_cast<AL_TBuffer*>(header->pPlatformPrivate);
  allocator->DestroyBuffer(codecData);

  delete header;
}

bool Port::useFileDescriptor()
{
  return allocator->IsFileDescriptor();
}

