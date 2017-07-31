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

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <sstream>
#include <list>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

extern "C"
{
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Video.h>
#include <OMX_VideoExt.h>
#include <OMX_ComponentExt.h>
}

#include "base/omx_utils/locked_queue.h"
#include "base/omx_utils/semaphore.h"
#include "base/omx_utils/omx_log.h"

#include "../common/helpers.h"
#include "../common/setters.h"

extern "C"
{
#include "lib_fpga/DmaAlloc.h"
#include "lib_fpga/DmaAllocLinux.h"
}

static OMX_U32 inportIndex = 0;
static OMX_U32 outportIndex = 1;

struct Application
{
  semaphore decoderEventSem;
  semaphore eofSem;
  semaphore decoderEventState;

  OMX_HANDLETYPE hDecoder;
  OMX_PTR pAppData;

  locked_queue<OMX_BUFFERHEADERTYPE*> inputBuffers;
  list<OMX_BUFFERHEADERTYPE*> outputBuffers;
  AL_TAllocator* pAllocator;
};

Application appPriv;
string input_file;
string output_file;
ifstream infile;
ofstream outfile;

static OMX_PARAM_PORTDEFINITIONTYPE paramPort;
static OMX_COLOR_FORMATTYPE chroma = OMX_COLOR_FormatYUV420SemiPlanar;
static bool use_avc = false;
static bool bDMAIn = false;
static bool bDMAOut = false;
static OMX_ALG_BUFFER_MODE eDMAIn = OMX_ALG_BUF_NORMAL;
static OMX_ALG_BUFFER_MODE eDMAOut = OMX_ALG_BUF_NORMAL;


void displayUsage()
{
  cout << "Usage: omx_decoder.exe FILE [OPTION]" << endl;
  cout << "Options:" << endl;

  cout << "\t" << "-help help" << endl;
  cout << "\t" << "-o outfile: Output compressed file name" << endl;
  cout << "\t" << "-hevc: load HEVC decoder (default)" << endl;
  cout << "\t" << "-avc : load AVC decoder" << endl;
  cout << "\t" << "-chroma <NV12 || RX0A || NV16 || RX2A> ('NV12' default)" << endl;
  cout << "\t" << "-dma-in : use dmabufs for input port" << endl;
  cout << "\t" << "-dma-out : use dmabufs for output port" << endl;
}

string popArg(queue<string>& args)
{
  if(args.empty())
  {
    cerr << "Invalid command line" << endl;
    displayUsage();
    return "";
  }
  auto const word = args.front();
  args.pop();
  return word;
}

int popInt(queue<string>& args)
{
  auto const word = popArg(args);
  return atoi(word.c_str());
}

void parseCommandLine(int argc, char** argv)
{
  queue<string> args;

  for(int i = 1; i < argc; ++i)
    args.push(argv[i]);

  if(args.empty())
  {
    displayUsage();
    exit(0);
  }

  auto const word = popArg(args);
  input_file = word;

  while(!args.empty())
  {
    auto const word = popArg(args);

    if(!word.empty() && word[0] == '-')
    {
      if(word == "-help")
      {
        displayUsage();
        exit(0);
      }
      else if(word == "-hevc")
      {
        use_avc = false;
      }
      else if(word == "-avc")
      {
        use_avc = true;
      }
      else if(word == "-o")
      {
        output_file = popArg(args);
      }
      else if(word == "-chroma")
      {
        const string user_chroma = popArg(args);

        if(!strncmp(user_chroma.c_str(), "NV12", strlen(user_chroma.c_str())))
          chroma = OMX_COLOR_FormatYUV420SemiPlanar;
        else if(!strncmp(user_chroma.c_str(), "NV16", strlen(user_chroma.c_str())))
          chroma = OMX_COLOR_FormatYUV422SemiPlanar;
        else if(!strncmp(user_chroma.c_str(), "RX0A", strlen(user_chroma.c_str())))
          chroma = (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked;
        else if(!strncmp(user_chroma.c_str(), "RX2A", strlen(user_chroma.c_str())))
          chroma = (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked;
        else
          assert(0);
      }
      else if(word == "-dma-in")
      {
        bDMAIn = true;
        eDMAIn = OMX_ALG_BUF_DMA;
      }
      else if(word == "-dma-out")
      {
        bDMAOut = true;
        eDMAOut = OMX_ALG_BUF_DMA;
      }
    }
  }

  if(input_file == "")
  {
    cerr << "No input file found" << endl;
    return;
  }

  if(output_file == "")
    output_file = "output.nv12";
}

static bool isSupplier(OMX_U32 nPortIndex)
{
  OMX_PARAM_BUFFERSUPPLIERTYPE sSupply;
  initHeader(sSupply);
  sSupply.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(appPriv.hDecoder, OMX_IndexParamCompBufferSupplier, &sSupply));

  if(
    ((sSupply.eBufferSupplier == OMX_BufferSupplyInput) &&
     (sSupply.nPortIndex == outportIndex)) ||
    ((sSupply.eBufferSupplier == OMX_BufferSupplyOutput) &&
     (sSupply.nPortIndex == inportIndex))
    )
    return true;

  return false;
};

static OMX_U32 getSizeBuffer(OMX_U32 nPortIndex)
{
  OMX_PARAM_PORTDEFINITIONTYPE sPortParam;
  initHeader(sPortParam);
  sPortParam.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(appPriv.hDecoder, OMX_IndexParamPortDefinition, &sPortParam));

  return sPortParam.nBufferSize;
};

static OMX_U32 getMinBufferAlloc(OMX_U32 nPortIndex)
{
  OMX_PARAM_PORTDEFINITIONTYPE sPortParam;
  initHeader(sPortParam);
  sPortParam.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(appPriv.hDecoder, OMX_IndexParamPortDefinition, &sPortParam));

  return sPortParam.nBufferCountActual;
};
static auto numberOfAllocatedInputBuffer = 0;
static auto numberOfAllocatedOutputBuffer = 0;

static OMX_ERRORTYPE allocBuffers(OMX_U32 nPortIndex, bool use_dmabuf)
{
  // Get Buffer size
  auto sizeBuf = getSizeBuffer(nPortIndex);

  // Get MinBuffer
  auto minBuf = getMinBufferAlloc(nPortIndex);

  // Check if port is supplier
  if(isSupplier(nPortIndex))
  {
    // Allocate buffer
    LOGV("Component port (%u) is supplier", nPortIndex);

    for(decltype(minBuf)nbBuf = 0; nbBuf < minBuf; nbBuf++)
    {
      OMX_BUFFERHEADERTYPE* pBufHeader = nullptr;
      OMX_AllocateBuffer(appPriv.hDecoder, &pBufHeader, nPortIndex, appPriv.pAppData, sizeBuf);
      assert(pBufHeader);

      if(nPortIndex == inportIndex)
      {
        appPriv.inputBuffers.push(pBufHeader);
        numberOfAllocatedInputBuffer++;
      }
      else
      {
        appPriv.outputBuffers.push_back(pBufHeader);
        numberOfAllocatedOutputBuffer++;
      }
    }
  }
  else
  {
    // Use Buffer
    LOGV("Component port (%u) is not supplier", nPortIndex);

    for(decltype(minBuf)nbBuf = 0; nbBuf < minBuf; nbBuf++)
    {
      OMX_BUFFERHEADERTYPE* pBufHeader = nullptr;
      OMX_U8* pBufData;

      if(use_dmabuf)
      {
        AL_HANDLE hBuf = AL_Allocator_Alloc(appPriv.pAllocator, sizeBuf);

        if(!hBuf)
        {
          LOGE("Failed to allocate Buffer for dma");
          assert(0);
        }
        pBufData = (OMX_U8*)(uintptr_t)AL_LinuxDmaAllocator_ExportToFd((AL_TLinuxDmaAllocator*)(appPriv.pAllocator), hBuf);

        if((int)(uintptr_t)pBufData <= 0)
        {
          LOGE("Failed to ExportToFd %p", hBuf);
          assert(0);
        }
        AL_Allocator_Free(appPriv.pAllocator, hBuf);
      }
      else
        pBufData = (OMX_U8*)calloc(sizeBuf, sizeof(OMX_U8));

      OMX_CALL(OMX_UseBuffer(appPriv.hDecoder, &pBufHeader, nPortIndex, appPriv.pAppData, sizeBuf, pBufData));

      if(nPortIndex == inportIndex)
      {
        appPriv.inputBuffers.push(pBufHeader);
        numberOfAllocatedInputBuffer++;
      }
      else
      {
        appPriv.outputBuffers.push_back(pBufHeader);
        numberOfAllocatedOutputBuffer++;
      }
    }
  }
  return OMX_ErrorNone;
};

static OMX_ERRORTYPE freeBuffers(OMX_U32 nPortIndex)
{
  LOGV("Port (%u)", nPortIndex);

  // Check if port is supplier
  if(isSupplier(nPortIndex))
  {
    // Free Component buffers
    if(nPortIndex == inportIndex)
    {
      while(numberOfAllocatedInputBuffer > 0)
      {
        auto pBuf = appPriv.inputBuffers.pop();
        OMX_CALL(OMX_FreeBuffer(appPriv.hDecoder, nPortIndex, pBuf));
        numberOfAllocatedInputBuffer--;
      }
    }
    else
    {
      for(auto pBuf : appPriv.outputBuffers)
        OMX_CALL(OMX_FreeBuffer(appPriv.hDecoder, nPortIndex, pBuf));
    }
  }
  else
  {
    if(nPortIndex == inportIndex)
    {
      while(numberOfAllocatedInputBuffer > 0)
      {
        // Free Client buffers
        auto pBufHeader = appPriv.inputBuffers.pop();
        Buffer_FreeData((char*)pBufHeader->pBuffer, bDMAIn);
        OMX_CALL(OMX_FreeBuffer(appPriv.hDecoder, nPortIndex, pBufHeader));
        numberOfAllocatedInputBuffer--;
      }
    }
    else
    {
      for(auto pBuf : appPriv.outputBuffers)
      {
        Buffer_FreeData((char*)pBuf->pBuffer, bDMAOut);
        OMX_CALL(OMX_FreeBuffer(appPriv.hDecoder, nPortIndex, pBuf));
      }
    }
  }
  return OMX_ErrorNone;
};

// Callbacks implementation of the video decoder component
OMX_ERRORTYPE onComponentEvent(OMX_HANDLETYPE /*hComponent*/, OMX_PTR /*pAppData*/, OMX_EVENTTYPE eEvent, OMX_U32 Data1, OMX_U32 Data2, OMX_PTR /*pEventData*/)
{
  LOGI("Event from decoder: 0x%.8X", eEvent);

  if(eEvent == OMX_EventCmdComplete)
  {
    if(Data1 == OMX_CommandStateSet)
    {
      LOGI("State changed to %s", toStringCompState((OMX_STATETYPE)Data2));
      appPriv.decoderEventState.notify();
    }
    else if(Data1 == OMX_CommandPortEnable || Data1 == OMX_CommandPortDisable)
    {
      LOGI("Received Port Enable/Disable Event");
      appPriv.decoderEventSem.notify();
    }
    else if(Data1 == OMX_CommandMarkBuffer)
    {
      LOGI("Mark Buffer Event ");
      appPriv.decoderEventSem.notify();
    }
  }
  else if(eEvent == OMX_EventPortSettingsChanged)
  {
    LOGI("Port settings change");
    initHeader(paramPort);
    paramPort.nPortIndex = 1;
    OMX_CALL(OMX_GetParameter(appPriv.hDecoder, OMX_IndexParamPortDefinition, &paramPort));
    paramPort.nBufferCountActual++;
    OMX_CALL(OMX_SetParameter(appPriv.hDecoder, OMX_IndexParamPortDefinition, &paramPort));
    freeBuffers(outportIndex);
    allocBuffers(outportIndex, bDMAOut);

    for(auto pBuf : appPriv.outputBuffers)
      OMX_CALL(OMX_FillThisBuffer(appPriv.hDecoder, pBuf));
  }
  else if(eEvent == OMX_EventError)
    LOGI("Event error: 0x%.8X", Data1);
  else
  {
    LOGI("Param1 is %u, Param2 is %u", Data1, Data2);
  }
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE setPortParameters()
{
  // This should always be done at the beginning
  OMX_VIDEO_PARAM_PORTFORMATTYPE outParamFormat;
  initHeader(outParamFormat);
  outParamFormat.nPortIndex = 1;
  OMX_CALL(OMX_GetParameter(appPriv.hDecoder, OMX_IndexParamVideoPortFormat, &outParamFormat));

  outParamFormat.eColorFormat = chroma;

  OMX_CALL(OMX_SetParameter(appPriv.hDecoder, OMX_IndexParamVideoPortFormat, &outParamFormat));

  Setters setter(&appPriv.hDecoder);

  auto isBufModeSetted = setter.SetBufferMode(inportIndex, eDMAIn);
  assert(isBufModeSetted);
  isBufModeSetted = setter.SetBufferMode(outportIndex, eDMAOut);
  assert(isBufModeSetted);

  return OMX_ErrorNone;
};

OMX_ERRORTYPE onInputBufferAvailable(OMX_HANDLETYPE /*hComponent*/, OMX_PTR /*pAppData*/, OMX_BUFFERHEADERTYPE* pBuffer)
{
  LOGV("InputBufferAvailable");
  assert(pBuffer->nFilledLen == 0);
  appPriv.inputBuffers.push(pBuffer);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE onOutputBufferAvailable(OMX_HANDLETYPE hComponent, OMX_PTR /*pAppData*/, OMX_BUFFERHEADERTYPE* pBuffer)
{
  static bool end = false;

  if(!pBuffer)
    return OMX_ErrorBadParameter;

  if(pBuffer->nFilledLen)
  {
    auto const data = Buffer_MapData((char*)(pBuffer->pBuffer + pBuffer->nOffset), pBuffer->nAllocLen, bDMAOut);

    if(data)
    {
      auto const videoDef = paramPort.format.video;
      auto const stride = videoDef.nStride;
      auto const sliceHeight = videoDef.nSliceHeight;
      auto const coef = is422(videoDef.eColorFormat) ? 1 : 2;
      auto const height = videoDef.nFrameHeight;
      auto const row_size = is10bits(videoDef.eColorFormat) ? (((videoDef.nFrameWidth + 2) / 3) * 4) : videoDef.nFrameWidth;

      /* luma */
      for(auto h = 0; h < (int)height; h++)
        outfile.write(&data[h * stride], row_size);

      /* chroma */
      for(auto h = sliceHeight; h < sliceHeight + height / coef; h++)
        outfile.write(&data[h * stride], row_size);

      outfile.flush();
    }
    else
      assert(0);

    Buffer_UnmapData(data, pBuffer->nAllocLen, bDMAOut);
  }

  if(end)
    return OMX_ErrorNone;

  pBuffer->nFilledLen = 0;
  pBuffer->nFilledLen = 0;

  OMX_CALL(OMX_FillThisBuffer(hComponent, pBuffer));

  if(pBuffer->nFlags == OMX_BUFFERFLAG_EOS)
  {
    appPriv.eofSem.notify();
    end = true;
  }

  return OMX_ErrorNone;
}

static bool readFrame(OMX_BUFFERHEADERTYPE* pInputBuf)
{
  assert(pInputBuf->nAllocLen != 0);
  vector<uint8_t> frame(pInputBuf->nAllocLen);

  infile.read((char*)frame.data(), frame.size());
  size_t zMapSize = pInputBuf->nAllocLen;
  auto data = Buffer_MapData((char*)(pInputBuf->pBuffer + pInputBuf->nOffset), zMapSize, bDMAIn);
  memcpy(data, frame.data(), frame.size());
  Buffer_UnmapData(data, pInputBuf->nAllocLen, bDMAIn);

  pInputBuf->nFilledLen = infile.gcount();

  if(infile.peek() == EOF)
    return true;

  return false;
};

OMX_ERRORTYPE safeMain(int argc, char** argv)
{
  parseCommandLine(argc, argv);

  infile.open(input_file, ios::binary);

  if(!infile.is_open())
  {
    cerr << "Error in opening input file '" << input_file << "'" << endl;
    return OMX_ErrorUndefined;
  }

  outfile.open(output_file, ios::binary);

  if(!outfile.is_open())
  {
    cerr << "Error in opening output file '" << output_file << "'" << endl;
    return OMX_ErrorUndefined;
  }

  OMX_CALL(OMX_Init());

  auto componentName = "OMX.allegro.h265.decoder";


  if(use_avc)
    componentName = "OMX.allegro.h264.decoder";

  OMX_CALLBACKTYPE const videoDecoderCallbacks =
  {
    .EventHandler = onComponentEvent,
    .EmptyBufferDone = onInputBufferAvailable,
    .FillBufferDone = onOutputBufferAvailable
  };

  /** getting video decoder handle */
  OMX_CALL(OMX_GetHandle(&appPriv.hDecoder, (OMX_STRING)componentName, nullptr, const_cast<OMX_CALLBACKTYPE*>(&videoDecoderCallbacks)));

  OMX_STRING name = (OMX_STRING)calloc(128, sizeof(char));
  OMX_VERSIONTYPE compType;
  OMX_VERSIONTYPE ilType;

  OMX_CALL(OMX_GetComponentVersion(appPriv.hDecoder, (OMX_STRING)name, &compType, &ilType, nullptr));

  LOGI("Component : %s (v.%u) made for OMX_IL client : %u.%u.%u", name, compType.nVersion, ilType.s.nVersionMajor, ilType.s.nVersionMinor, ilType.s.nRevision);

  free(name);

  /** Set ports **/
  auto const ret = setPortParameters();

  if(ret != OMX_ErrorNone)
    return ret;

  appPriv.pAllocator = DmaAlloc_Create("/dev/allegroDecodeIP");

  allocBuffers(inportIndex, bDMAIn);
  allocBuffers(outportIndex, bDMAOut);

  /** sending command to video decoder component to go to idle state */
  OMX_SendCommand(appPriv.hDecoder, OMX_CommandStateSet, OMX_StateIdle, nullptr);

  appPriv.decoderEventState.wait();

  /** sending command to video decoder component to go to executing state */
  OMX_CALL(OMX_SendCommand(appPriv.hDecoder, OMX_CommandStateSet, OMX_StateExecuting, nullptr));
  appPriv.decoderEventState.wait();

  /** Process **/
  for(;;)
  {
    auto inputBuffer = appPriv.inputBuffers.pop();
    auto n = readFrame(inputBuffer);

    auto const eof = n;

    if(eof)
    {
      LOGV("End of file");
      inputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
    }

    OMX_CALL(OMX_EmptyThisBuffer(appPriv.hDecoder, inputBuffer));

    if(eof)
      break;
  }

  LOGV("Waiting for EOS ... ");
  appPriv.eofSem.wait();
  cerr.flush();
  LOGV("EOS received");

  /** state change of all components from executing to idle */
  OMX_CALL(OMX_SendCommand(appPriv.hDecoder, OMX_CommandStateSet, OMX_StateIdle, nullptr));
  appPriv.decoderEventState.wait();

  /** sending command to all components to go to loaded state */
  OMX_CALL(OMX_SendCommand(appPriv.hDecoder, OMX_CommandStateSet, OMX_StateLoaded, nullptr));

  appPriv.decoderEventState.wait();

  freeBuffers(inportIndex);
  freeBuffers(outportIndex);

  OMX_FreeHandle(appPriv.hDecoder);
  OMX_Deinit();

  if(appPriv.pAllocator)
    AL_Allocator_Destroy(appPriv.pAllocator);

  infile.close();
  outfile.close();
  return OMX_ErrorNone;
}

int main(int argc, char** argv)
{
  OMX_ERRORTYPE ret;

  ret = safeMain(argc, argv);

  if(ret == OMX_ErrorNone)
    return 0;
  else
  {
    cerr << "Fatal error" << endl;
    return -1;
  }
}

