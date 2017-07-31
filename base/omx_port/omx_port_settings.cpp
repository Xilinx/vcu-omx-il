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

#include "omx_port_settings.h"
#include "base/omx_checker/omx_checker.h"
#include <assert.h>

OMX_PARAM_BUFFERSUPPLIERTYPE & PortSettings::GetSupplier()
{
  return supplier;
}

OMX_ERRORTYPE PortSettings::SetSupplier(OMX_PARAM_BUFFERSUPPLIERTYPE supplier)
{
  try
  {
    OMXChecker::CheckHeaderVersion(supplier.nVersion);

    this->supplier = supplier;
    return OMX_ErrorNone;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_PARAM_PORTDEFINITIONTYPE & PortSettings::GetDefinition()
{
  return definition;
}

OMX_ERRORTYPE PortSettings::SetDefinition(OMX_PARAM_PORTDEFINITIONTYPE definition)
{
  try
  {
    OMXChecker::CheckHeaderVersion(definition.nVersion);

    if(!isVideoDefinitionCorrect(definition.format.video))
      return OMX_ErrorBadParameter;

    this->definition = definition;

    return OMX_ErrorNone;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

bool PortSettings::isVideoDefinitionCorrect(OMX_VIDEO_PORTDEFINITIONTYPE videoDef)
{
  if((int)videoDef.nStride < (int)videoDef.nFrameWidth)
    return false;

  if(videoDef.nStride % 32)
    return false;

  if((int)videoDef.nSliceHeight < (int)videoDef.nFrameHeight)
    return false;

  return true;
}

OMX_VIDEO_PORTDEFINITIONTYPE & PortSettings::GetVideoDefinition()
{
  return definition.format.video;
}

OMX_ERRORTYPE PortSettings::SetVideoDefinition(OMX_VIDEO_PORTDEFINITIONTYPE videoDefinition)
{
  if(!isVideoDefinitionCorrect(videoDefinition))
    return OMX_ErrorBadParameter;

  this->definition.format.video = videoDefinition;
  return OMX_ErrorNone;
}

OMX_VIDEO_PARAM_PORTFORMATTYPE const PortSettings::GetVideoFormat()
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

