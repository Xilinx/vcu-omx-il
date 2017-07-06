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

#include "base/omx_base/omx_base.h"
#include "base/omx_checker/omx_checker.h"
#include "base/omx_processtype/omx_processtype.h"
#include "base/omx_codectype/omx_codectype.h"

class OMXVideoCodec : public OMXBase
{
public:
  /* Variables */

  /* Methods */
  OMXVideoCodec(OMX_HANDLETYPE component, ProcessType* process, OMX_STRING name, OMX_STRING role);
  ~OMXVideoCodec();
  OMX_ERRORTYPE SendCommand(OMX_IN OMX_COMMANDTYPE cmd, OMX_IN OMX_U32 param, OMX_IN OMX_PTR data);
  OMX_ERRORTYPE GetState(OMX_OUT OMX_STATETYPE* state);
  OMX_ERRORTYPE SetCallbacks(OMX_IN OMX_CALLBACKTYPE* callbacks, OMX_IN OMX_PTR app);
  OMX_ERRORTYPE GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param);
  OMX_ERRORTYPE SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param);
  OMX_ERRORTYPE UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size, OMX_IN OMX_U8* buffer);
  OMX_ERRORTYPE AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN OMX_U32 size);
  OMX_ERRORTYPE FreeBuffer(OMX_IN OMX_U32 index, OMX_IN OMX_BUFFERHEADERTYPE* header);
  OMX_ERRORTYPE EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header);
  OMX_ERRORTYPE FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header);
  void ComponentDeInit();
  OMX_ERRORTYPE GetComponentVersion(OMX_OUT OMX_STRING name, OMX_OUT OMX_VERSIONTYPE* version, OMX_OUT OMX_VERSIONTYPE* spec);
  OMX_ERRORTYPE GetConfig(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR config);
  OMX_ERRORTYPE SetConfig(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR config);
  OMX_ERRORTYPE GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index);
  OMX_ERRORTYPE ComponentTunnelRequest(OMX_IN OMX_U32 index, OMX_IN OMX_HANDLETYPE comp, OMX_IN OMX_U32 tunneledIndex, OMX_INOUT OMX_TUNNELSETUPTYPE* setup);
  OMX_ERRORTYPE UseEGLImage(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 index, OMX_IN OMX_PTR app, OMX_IN void* eglImage);
  OMX_ERRORTYPE ComponentRoleEnum(OMX_OUT OMX_U8* role, OMX_IN OMX_U32 index);

protected:
  /* Variables */

  /* Methods */

private:
  /* Variables */
  OMX_HANDLETYPE component;
  ProcessType* process;

  OMX_STRING name;
  OMX_STRING role;
  OMX_VERSIONTYPE version;
  OMX_VERSIONTYPE spec;
  std::mutex mutex;

  OMX_PORT_PARAM_TYPE ports;

  /* Methods */
  void CreateName(OMX_STRING name);
  void CreateRole(OMX_STRING role);

  OMX_STRING GetComponentName();
  OMX_STRING GetComponentRole();
  OMX_VERSIONTYPE GetComponentVersion();
  OMX_VERSIONTYPE GetSpecificationVersion();
};

