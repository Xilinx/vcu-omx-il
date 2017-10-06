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

#include "base/omx_videocodec/omx_videocodec.h"

#include <assert.h>
#include <string.h>

#define ALLEGRODVT_OMX_VERSION 3

static void AssociateSpecVersion(OMX_VERSIONTYPE& spec)
{
  spec.s.nVersionMajor = OMX_VERSION_MAJOR;
  spec.s.nVersionMinor = OMX_VERSION_MINOR;
  spec.s.nRevision = OMX_VERSION_REVISION;
  spec.s.nStep = OMX_VERSION_STEP;
}

static void InitPorts(OMX_PORT_PARAM_TYPE& ports)
{
  OMXChecker::SetHeaderVersion(ports);
  ports.nPorts = 2;
  ports.nStartPortNumber = 0;
}

OMXVideoCodec::OMXVideoCodec(OMX_HANDLETYPE component, ProcessType* process, OMX_STRING name, OMX_STRING role) : component(component), process(process)
{
  assert(component);
  assert(process);
  assert(name);
  assert(role);

  CreateName(name);
  CreateRole(role);
  version.nVersion = ALLEGRODVT_OMX_VERSION;
  AssociateSpecVersion(spec);

  InitPorts(ports);
}

OMXVideoCodec::~OMXVideoCodec()
{
  std::lock_guard<std::mutex> lock(mutex);

  if(name)
    free(name);

  if(role)
    free(role);
}

void OMXVideoCodec::CreateName(OMX_STRING name)
{
  this->name = (OMX_STRING)malloc(OMX_MAX_STRINGNAME_SIZE * sizeof(char));
  strncpy(this->name, name, OMX_MAX_STRINGNAME_SIZE);
}

void OMXVideoCodec::CreateRole(OMX_STRING role)
{
  this->role = (OMX_STRING)malloc(OMX_MAX_STRINGNAME_SIZE * sizeof(char));
  strncpy(this->role, role, OMX_MAX_STRINGNAME_SIZE);
}

OMX_ERRORTYPE OMXVideoCodec::SendCommand(OMX_IN OMX_COMMANDTYPE cmd, OMX_IN OMX_U32 param, OMX_IN OMX_PTR data)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckStateOperation(AL_SendCommand, process->GetState());

    if(cmd == OMX_CommandStateSet)
    {
      OMXChecker::CheckStateTransition(process->GetState(), (OMX_STATETYPE)param);
      return process->SetState(param);
    }

    return process->SendCommand(cmd, param, data);
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::GetState(OMX_OUT OMX_STATETYPE* state)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull<OMX_STATETYPE*>(state);

    *state = process->GetState();

    return OMX_ErrorNone;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::SetCallbacks(OMX_IN OMX_CALLBACKTYPE* callbacks, OMX_IN OMX_PTR app)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull<OMX_CALLBACKTYPE*>(callbacks);
    OMXChecker::CheckStateOperation(AL_SetCallbacks, process->GetState());

    process->SetCallBack(callbacks);
    process->SetAppData(app);

    return OMX_ErrorNone;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::GetParameter(OMX_IN OMX_INDEXTYPE index, OMX_INOUT OMX_PTR param)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull(param);

    auto ret = process->GetParameter(index, param);
    return ret;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::SetParameter(OMX_IN OMX_INDEXTYPE index, OMX_IN OMX_PTR param)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull(param);

    auto ret = process->SetParameter(index, param);
    return ret;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::UseBuffer(OMX_OUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 indexIndex, OMX_IN OMX_PTR app, OMX_IN OMX_U32 nSizeBytes, OMX_IN OMX_U8* pBuffer)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull(header);
    OMXChecker::CheckStateOperation(AL_UseBuffer, process->GetState());
    return process->UseBuffer(header, indexIndex, app, nSizeBytes, pBuffer);
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::AllocateBuffer(OMX_INOUT OMX_BUFFERHEADERTYPE** header, OMX_IN OMX_U32 indexIndex, OMX_IN OMX_PTR app, OMX_IN OMX_U32 nSizeBytes)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull(header);
    OMXChecker::CheckStateOperation(AL_AllocateBuffer, process->GetState());
    return process->AllocateBuffer(header, indexIndex, app, nSizeBytes);
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::FreeBuffer(OMX_IN OMX_U32 indexIndex, OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);
    OMXChecker::CheckNotNull(header);
    OMXChecker::CheckStateOperation(AL_FreeBuffer, process->GetState());
    return process->FreeBuffer(indexIndex, header);
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::EmptyThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* pInputBuf)
{
  try
  {
    OMXChecker::CheckNotNull(pInputBuf);
    OMXChecker::CheckStateOperation(AL_EmptyThisBuffer, process->GetState());
    return process->EmptyThisBuffer(pInputBuf);
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::FillThisBuffer(OMX_IN OMX_BUFFERHEADERTYPE* header)
{
  try
  {
    OMXChecker::CheckNotNull(header);
    OMXChecker::CheckStateOperation(AL_FillThisBuffer, process->GetState());
    return process->FillThisBuffer(header);
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

void OMXVideoCodec::ComponentDeInit()
{
  std::lock_guard<std::mutex> lock(mutex);
  process->ComponentDeInit();
}

OMX_ERRORTYPE OMXVideoCodec::GetComponentVersion(OMX_OUT OMX_STRING name, OMX_OUT OMX_VERSIONTYPE* version, OMX_OUT OMX_VERSIONTYPE* spec)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull(name);
    OMXChecker::CheckNotNull(version);
    OMXChecker::CheckNotNull(spec);
    OMXChecker::CheckStateOperation(AL_GetComponentVersion, process->GetState());

    strncpy(name, GetComponentName(), OMX_MAX_STRINGNAME_SIZE);
    *version = GetComponentVersion();
    *spec = GetSpecificationVersion();

    return OMX_ErrorNone;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::GetConfig(OMX_IN OMX_INDEXTYPE /* index */, OMX_INOUT OMX_PTR /* config*/)
{
  std::lock_guard<std::mutex> lock(mutex);
  return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE OMXVideoCodec::SetConfig(OMX_IN OMX_INDEXTYPE /* index */, OMX_IN OMX_PTR /* config */)
{
  std::lock_guard<std::mutex> lock(mutex);
  return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE OMXVideoCodec::GetExtensionIndex(OMX_IN OMX_STRING name, OMX_OUT OMX_INDEXTYPE* index)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull(name);
    OMXChecker::CheckNotNull(index);
    OMXChecker::CheckStateOperation(AL_GetExtensionIndex, process->GetState());
    return process->GetExtensionIndex(name, index);
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::ComponentTunnelRequest(OMX_IN OMX_U32 index, OMX_IN OMX_HANDLETYPE comp, OMX_IN OMX_U32 tunneledIndex, OMX_INOUT OMX_TUNNELSETUPTYPE* setup)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull(comp);
    OMXChecker::CheckNotNull(setup);

    auto ret = process->ComponentTunnelRequest(index, comp, tunneledIndex, setup);
    return ret;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_ERRORTYPE OMXVideoCodec::UseEGLImage(OMX_INOUT OMX_BUFFERHEADERTYPE** /* header */, OMX_IN OMX_U32 /* index */, OMX_IN OMX_PTR /* app */, OMX_IN void* /* eglImage*/)
{
  std::lock_guard<std::mutex> lock(mutex);
  return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE OMXVideoCodec::ComponentRoleEnum(OMX_OUT OMX_U8* role, OMX_IN OMX_U32 index)
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex);

    OMXChecker::CheckNotNull(role);
    OMXChecker::CheckStateOperation(AL_ComponentRoleEnum, process->GetState());

    if(index == 0)
      strncpy((OMX_STRING)role, GetComponentRole(), OMX_MAX_STRINGNAME_SIZE);
    else
      return OMX_ErrorNoMore;

    return OMX_ErrorNone;
  }
  catch(OMX_ERRORTYPE e)
  {
    return e;
  }
}

OMX_STRING OMXVideoCodec::GetComponentName()
{
  return name;
}

OMX_STRING OMXVideoCodec::GetComponentRole()
{
  return role;
}

OMX_VERSIONTYPE OMXVideoCodec::GetComponentVersion()
{
  return version;
}

OMX_VERSIONTYPE OMXVideoCodec::GetSpecificationVersion()
{
  return spec;
}

