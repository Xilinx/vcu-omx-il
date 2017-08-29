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
#include <queue>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <unistd.h>

using namespace std;

extern "C"
{
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Video.h>
#include <OMX_VideoExt.h>
#include <OMX_ComponentExt.h>
#include <OMX_IndexExt.h>
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

static inline int RoundUp(int iVal, int iRnd)
{
  return (iVal + iRnd - 1) & (~(iRnd - 1));
}

static OMX_U32 inportIndex = 0;
static OMX_U32 outportIndex = 1;

struct Application
{
  semaphore encoderEventSem;
  semaphore eofSem;
  semaphore encoderEventState;

  OMX_HANDLETYPE hEncoder;
  OMX_PTR pAppData;

  locked_queue<OMX_BUFFERHEADERTYPE*> inputBuffers;
  vector<OMX_BUFFERHEADERTYPE*> outputBuffers;
  AL_TAllocator* pAllocator;
};

static Application appPriv;
static string input_file;
static string output_file;


static bool use_avc = false;
static bool bDMAIn = false;
static bool bDMAOut = false;
static OMX_ALG_BUFFER_MODE eDMAIn = OMX_ALG_BUF_NORMAL;
static OMX_ALG_BUFFER_MODE eDMAOut = OMX_ALG_BUF_NORMAL;
static ifstream infile;
static ofstream outfile;
static OMX_U32 in_width = 176;
static OMX_U32 in_height = 144;
static OMX_U32 frame_rate = 1 << 16; // Q16 format
static OMX_COLOR_FORMATTYPE chroma = OMX_COLOR_FormatYUV420SemiPlanar;
static int user_slice = 0;

static OMX_PARAM_PORTDEFINITIONTYPE paramPort;

static void displayUsage()
{
  cout << "Usage: omx_encoder.exe FILE [OPTION]" << endl;
  cout << "Options:" << endl;
  cout << "\t" << "-help help" << endl;
  cout << "\t" << "-o outfile: Output compressed file name" << endl;
  cout << "\t" << "-w <input width>" << endl;
  cout << "\t" << "-h <input height>" << endl;
  cout << "\t" << "-r <input fps>" << endl;
  cout << "\t" << "-chroma <NV12 || RX0A || NV16 || RX2A> ('NV12' default)" << endl;
  cout << "\t" << "-hevc: load HEVC encoder (default)" << endl;
  cout << "\t" << "-avc : load AVC encoder" << endl;
  cout << "\t" << "-dma-in : use dmabufs for input port" << endl;
  cout << "\t" << "-dma-out : use dmabufs for output port" << endl;
  cout << "\t" << "-subframe <4 ||  8 || 16> : activate subframe latency " << endl;
}

static OMX_ERRORTYPE setPortParameters()
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE inParamFormat;
  initHeader(inParamFormat);
  inParamFormat.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(appPriv.hEncoder, OMX_IndexParamVideoPortFormat, &inParamFormat));

  inParamFormat.eColorFormat = chroma;
  inParamFormat.xFramerate = frame_rate;

  OMX_CALL(OMX_SetParameter(appPriv.hEncoder, OMX_IndexParamVideoPortFormat, &inParamFormat));

  initHeader(paramPort);
  paramPort.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(appPriv.hEncoder, OMX_IndexParamPortDefinition, &paramPort));
  paramPort.format.video.nFrameWidth = in_width;
  paramPort.format.video.nFrameHeight = in_height;
  paramPort.format.video.nStride = is10bits(chroma) ? ((in_width + 2) / 3) * 4 : in_width;
  paramPort.format.video.nSliceHeight = RoundUp(in_height, 32);

  OMX_CALL(OMX_SetParameter(appPriv.hEncoder, OMX_IndexParamPortDefinition, &paramPort));
  OMX_CALL(OMX_GetParameter(appPriv.hEncoder, OMX_IndexParamPortDefinition, &paramPort));

  Setters setter(&appPriv.hEncoder);

  auto isBufModeSetted = setter.SetBufferMode(inportIndex, eDMAIn);
  assert(isBufModeSetted);
  isBufModeSetted = setter.SetBufferMode(outportIndex, eDMAOut);
  assert(isBufModeSetted);

  if(user_slice)
  {
    OMX_ALG_VIDEO_PARAM_SLICES slices;
    initHeader(slices);
    slices.nPortIndex = 1;
    slices.nNumSlices = user_slice;
    OMX_SetParameter(appPriv.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoSlices), &slices);

    OMX_ALG_VIDEO_PARAM_SUBFRAME sub;
    initHeader(sub);
    sub.nPortIndex = 1;
    sub.bEnableSubframe = OMX_TRUE;
    OMX_SetParameter(appPriv.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoSubframe), &sub);
  }

  LOGV("Input picture: %ux%u", in_width, in_height);
  return OMX_ErrorNone;
};

static string popArg(queue<string>& args)
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
};

static int popInt(queue<string>& args)
{
  auto const word = popArg(args);
  return atoi(word.c_str());
};

static void parseCommandLine(int argc, char** argv)
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
      else if(word == "-w")
      {
        in_width = popInt(args);
      }
      else if(word == "-h")
      {
        in_height = popInt(args);
      }
      else if(word == "-r")
      {
        frame_rate = popInt(args) << 16;
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
      else if(word == "-hevc")
      {
        use_avc = false;
      }
      else if(word == "-avc")
      {
        use_avc = true;
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
      else if(word == "-subframe")
      {
        user_slice = popInt(args);
        assert(user_slice == 4 || user_slice == 8 || user_slice == 16);
      }
    }
  }

  if(input_file == "")
  {
    cerr << "No input file found" << endl;
    return;
  }

  if(output_file == "")
  {

    if(use_avc)
      output_file = "output.h264";
    else
    output_file = "output.h265";
  }
};

// Callbacks implementation of the video encoder component
static OMX_ERRORTYPE onComponentEvent(OMX_HANDLETYPE /*hComponent*/, OMX_PTR /*pAppData*/, OMX_EVENTTYPE eEvent, OMX_U32 Data1, OMX_U32 Data2, OMX_PTR /*pEventData*/)
{
  LOGI("Event from encoder: 0x%.8X", eEvent);

  if(eEvent == OMX_EventCmdComplete)
  {
    if(Data1 == OMX_CommandStateSet)
    {
      LOGI("State changed to %s", toStringCompState((OMX_STATETYPE)Data2));
      appPriv.encoderEventState.notify();
    }
    else if(Data1 == OMX_CommandPortEnable || Data1 == OMX_CommandPortDisable)
    {
      LOGI("Received Port Enable/Disable Event");
      appPriv.encoderEventSem.notify();
    }
    else if(Data1 == OMX_CommandMarkBuffer)
    {
      LOGI("Mark Buffer Event");
      appPriv.encoderEventSem.notify();
    }
    else if(Data1 == OMX_CommandFlush)
    {
      LOGI("Port %u flushed", Data2);
      appPriv.encoderEventSem.notify();
    }
  }
  else if(eEvent == OMX_EventPortSettingsChanged)
    LOGI("Port settings change");
  else if(eEvent == OMX_EventError)
  {
    LOGI("Event error: 0x%.8X", Data1);
    return OMX_ErrorUndefined;
  }
  else if(eEvent == OMX_EventBufferFlag)
    LOGI("Event BufferFlag (%u) on port : %u", Data2, Data1);
  else
    LOGI("Param1 is %u, Param2 is %u", Data1, Data2);

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE onInputBufferAvailable(OMX_HANDLETYPE /*hComponent*/, OMX_PTR /*pAppData*/, OMX_BUFFERHEADERTYPE* pBuffer)
{
  assert(pBuffer->nFilledLen == 0);
  appPriv.inputBuffers.push(pBuffer);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE onOutputBufferAvailable(OMX_HANDLETYPE hComponent, OMX_PTR /*pAppData*/, OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  if(!pBufferHdr)
    return OMX_ErrorBadParameter;

  auto zMapSize = pBufferHdr->nAllocLen;

  if(zMapSize)
  {
    auto data = Buffer_MapData((char*)(pBufferHdr->pBuffer + pBufferHdr->nOffset), zMapSize, bDMAOut);

    if(data)
    {
      outfile.write((char*)data, pBufferHdr->nFilledLen);
      outfile.flush();
    }

    Buffer_UnmapData(data, zMapSize, bDMAOut);
  }

  pBufferHdr->nFilledLen = 0;

  if(pBufferHdr->nFlags & OMX_BUFFERFLAG_EOS)
    appPriv.eofSem.notify();
  else
  {
    pBufferHdr->nFlags = 0;
    OMX_CALL(OMX_FillThisBuffer(hComponent, pBufferHdr));
  }

  return OMX_ErrorNone;
}

static bool isSupplier(OMX_U32 nPortIndex)
{
  OMX_PARAM_BUFFERSUPPLIERTYPE sSupply;
  initHeader(sSupply);
  sSupply.nPortIndex = nPortIndex;

  OMX_GetParameter(appPriv.hEncoder, OMX_IndexParamCompBufferSupplier, &sSupply);

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

  OMX_CALL(OMX_GetParameter(appPriv.hEncoder, OMX_IndexParamPortDefinition, &sPortParam));

  return sPortParam.nBufferSize;
};

static OMX_U32 getMinBufferAlloc(OMX_U32 nPortIndex)
{
  OMX_PARAM_PORTDEFINITIONTYPE sPortParam;
  initHeader(sPortParam);
  sPortParam.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(appPriv.hEncoder, OMX_IndexParamPortDefinition, &sPortParam));
  assert(sPortParam.nBufferCountMin > 0);

  return sPortParam.nBufferCountMin;
};

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
      OMX_BUFFERHEADERTYPE* pBufHeader;

      if(!nPortIndex)
        OMX_CALL(OMX_AllocateBuffer(appPriv.hEncoder, &pBufHeader, nPortIndex, appPriv.pAppData, sizeBuf));
      else
        OMX_CALL(OMX_AllocateBuffer(appPriv.hEncoder, &pBufHeader, nPortIndex, appPriv.pAppData, sizeBuf));

      if(nPortIndex == inportIndex)
        appPriv.inputBuffers.push(pBufHeader);
      else
        appPriv.outputBuffers.push_back(pBufHeader);
    }
  }
  else
  {
    // Use Buffer
    LOGV("Component port (%u) is not supplier", nPortIndex);

    for(decltype(minBuf)nbBuf = 0; nbBuf < minBuf; nbBuf++)
    {
      OMX_BUFFERHEADERTYPE* pBufHeader;
      OMX_U8* pBufData;

      if(use_dmabuf)
      {
        AL_HANDLE hBuf = AL_Allocator_Alloc(appPriv.pAllocator, sizeBuf);

        if(!hBuf)
        {
          LOGE("Failed to allocate Buffer for dma");
          assert(0);
        }
        auto fd = AL_LinuxDmaAllocator_ExportToFd((AL_TLinuxDmaAllocator*)(appPriv.pAllocator), hBuf);
        pBufData = (OMX_U8*)(uintptr_t)dup(fd);

        if(!pBufData)
        {
          LOGE("Failed to ExportToFd %p", hBuf);
          assert(0);
        }
        AL_Allocator_Free(appPriv.pAllocator, hBuf);
      }
      else
        pBufData = (OMX_U8*)calloc(sizeBuf, sizeof(OMX_U8));

      if(!nPortIndex)
        OMX_CALL(OMX_UseBuffer(appPriv.hEncoder, &pBufHeader, nPortIndex, appPriv.pAppData, sizeBuf, pBufData));
      else
        OMX_CALL(OMX_UseBuffer(appPriv.hEncoder, &pBufHeader, nPortIndex, appPriv.pAppData, sizeBuf, pBufData));

      if(nPortIndex == inportIndex)
        appPriv.inputBuffers.push(pBufHeader);
      else
      {
        appPriv.outputBuffers.push_back(pBufHeader);
      }
    }
  }
  return OMX_ErrorNone;
};

static OMX_ERRORTYPE freeBuffers(OMX_U32 nPortIndex)
{
  LOGV("Port (%u)", nPortIndex);
  // Get MinBuffer
  auto minBuf = getMinBufferAlloc(nPortIndex);

  // Check if port is supplier
  if(isSupplier(nPortIndex))
  {
    // Free Component buffers
    for(decltype(minBuf)nbBuf = 0; nbBuf < minBuf; nbBuf++)
    {
      if(nPortIndex == inportIndex)
      {
        auto pBuf = appPriv.inputBuffers.pop();
        OMX_CALL(OMX_FreeBuffer(appPriv.hEncoder, nPortIndex, pBuf));
      }
      else
      {
        auto pBuf = appPriv.outputBuffers.back();
        appPriv.outputBuffers.pop_back();
        OMX_CALL(OMX_FreeBuffer(appPriv.hEncoder, nPortIndex, pBuf));
      }
    }
  }
  else
  {
    for(decltype(minBuf)nbBuf = 0; nbBuf < minBuf; nbBuf++)
    {
      // Free Client buffers
      if(nPortIndex == inportIndex)
      {
        auto pBufHeader = appPriv.inputBuffers.pop();
        Buffer_FreeData((char*)pBufHeader->pBuffer, bDMAIn);
        OMX_CALL(OMX_FreeBuffer(appPriv.hEncoder, nPortIndex, pBufHeader));
      }
      else
      {
        auto pBufHeader = appPriv.outputBuffers.back();
        appPriv.outputBuffers.pop_back();
        Buffer_FreeData((char*)pBufHeader->pBuffer, bDMAOut);
        OMX_CALL(OMX_FreeBuffer(appPriv.hEncoder, nPortIndex, pBufHeader));
      }
    }
  }
  return OMX_ErrorNone;
};

static bool readOneYuvFrame(OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  auto const width = paramPort.format.video.nFrameWidth;
  auto const row_size = is10bits(chroma) ? (((width + 2) / 3) * 4) : width;
  auto const height = paramPort.format.video.nFrameHeight;
  auto const stride = paramPort.format.video.nStride;
  auto const sliceHeight = paramPort.format.video.nSliceHeight;

  LOGV("w %d, h %d, stride %d, sliceHeight %d", (int)width, (int)height, (int)stride, (int)sliceHeight);

  static int i;
  LOGV("Reading input frame %i %ux%u", i, (unsigned int)width, (unsigned int)height);
  i++;

  auto coef = is422(chroma) ? 1 : 2;
  auto const size = row_size * (height + height / coef);
  vector<uint8_t> frame(size);

  if(infile.peek() == EOF)
    return false;

  infile.read((char*)frame.data(), frame.size());

  auto dst = Buffer_MapData((char*)(pBufferHdr->pBuffer + pBufferHdr->nOffset), pBufferHdr->nAllocLen, bDMAIn);

  /* luma */
  for(auto h = 0; h < (int)height; h++)
    memcpy(&dst[h * stride], &frame.data()[h * row_size], row_size);

  /* chroma */
  for(struct { long unsigned int sh; long unsigned int h; } v { sliceHeight, height }; v.sh < sliceHeight + height / coef; v.sh++, v.h++)
    memcpy(&dst[v.sh * stride], &frame.data()[v.h * row_size], row_size);

  pBufferHdr->nFilledLen = frame.size();
  LOGV("FilledLen: %d, AllocLen: %d", (int)pBufferHdr->nFilledLen, (int)pBufferHdr->nAllocLen);
  assert(pBufferHdr->nFilledLen <= pBufferHdr->nAllocLen);

  Buffer_UnmapData((char*)pBufferHdr->pBuffer, pBufferHdr->nAllocLen, bDMAIn);

  return true;
};

static OMX_ERRORTYPE safeMain(int argc, char** argv)
{
  parseCommandLine(argc, argv);

  infile.open(input_file, ios::binary);

  if(!infile.is_open())
  {
    cerr << "Error in opening input file '" << input_file.c_str() << "'" << endl;
    return OMX_ErrorUndefined;
  }

  outfile.open(output_file, ios::binary);

  if(!outfile.is_open())
  {
    cerr << "Error in opening output file '" << output_file.c_str() << "'" << endl;
    return OMX_ErrorUndefined;
  }

  OMX_CALL(OMX_Init());

  auto componentName = "OMX.allegro.h265.encoder";


  if(use_avc)
    componentName = "OMX.allegro.h264.encoder";

  OMX_CALLBACKTYPE const videoEncoderCallbacks =
  {
    .EventHandler = onComponentEvent,
    .EmptyBufferDone = onInputBufferAvailable,
    .FillBufferDone = onOutputBufferAvailable
  };

  /** getting video encoder handle */
  OMX_CALL(OMX_GetHandle(&appPriv.hEncoder, (OMX_STRING)componentName, nullptr, const_cast<OMX_CALLBACKTYPE*>(&videoEncoderCallbacks)));

  OMX_STRING name = (OMX_STRING)calloc(128, sizeof(char));
  OMX_VERSIONTYPE compType;
  OMX_VERSIONTYPE ilType;

  OMX_CALL(OMX_GetComponentVersion(appPriv.hEncoder, (OMX_STRING)name, &compType, &ilType, nullptr));

  LOGI("Component : %s (v.%u) made for OMX_IL client : %u.%u.%u", name, compType.nVersion, ilType.s.nVersionMajor, ilType.s.nVersionMinor, ilType.s.nRevision);

  free(name);

  /** Set ports **/
  auto const ret = setPortParameters();

  if(ret != OMX_ErrorNone)
    return ret;

  /** Allocate Buffers **/
  appPriv.pAllocator = DmaAlloc_Create("/dev/allegroIP");

  allocBuffers(inportIndex, bDMAIn);
  allocBuffers(outportIndex, bDMAOut);

  /** sending command to video encoder component to go to idle state */
  OMX_SendCommand(appPriv.hEncoder, OMX_CommandStateSet, OMX_StateIdle, nullptr);

  appPriv.encoderEventState.wait();

  /** sending command to video encoder component to go to executing state */
  OMX_CALL(OMX_SendCommand(appPriv.hEncoder, OMX_CommandStateSet, OMX_StateExecuting, nullptr));
  appPriv.encoderEventState.wait();

  unsigned int i = 0;

  for(i = 0; i < getMinBufferAlloc(outportIndex); ++i)
    OMX_CALL(OMX_FillThisBuffer(appPriv.hEncoder, appPriv.outputBuffers.at(i)));

  /** Process **/

  i = 0;

  for(;; ++i)
  {
    auto inputBuffer = appPriv.inputBuffers.pop();
    inputBuffer->nFlags = 0;

    if((paramPort.format.video.eColorFormat != OMX_COLOR_FormatYUV420SemiPlanar) &&
       (paramPort.format.video.eColorFormat != OMX_COLOR_FormatYUV422SemiPlanar) &&
       (paramPort.format.video.eColorFormat != (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked) &&
       (paramPort.format.video.eColorFormat != (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked))
    {
      LOGE("Unsupported color format : 0X%.8X", paramPort.format.video.eColorFormat);
      return OMX_ErrorUnsupportedSetting;
    }
    auto const eof = (readOneYuvFrame(inputBuffer) == 0);

    inputBuffer->nFlags = OMX_BUFFERFLAG_ENDOFFRAME;

    if(eof)
    {
      LOGV("End of file");
      inputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
    }

    OMX_CALL(OMX_EmptyThisBuffer(appPriv.hEncoder, inputBuffer));

    if(eof)
      break;
  }

  LOGV("Waiting for EOS ... ");
  cerr.flush();
  appPriv.eofSem.wait();
  LOGV("EOS received");

  /** send flush in input port */
  OMX_CALL(OMX_SendCommand(appPriv.hEncoder, OMX_CommandFlush, inportIndex, nullptr));
  appPriv.encoderEventSem.wait();

  /** send flush on output port */
  OMX_CALL(OMX_SendCommand(appPriv.hEncoder, OMX_CommandFlush, outportIndex, nullptr));
  appPriv.encoderEventSem.wait();

  /** state change of all components from executing to idle */
  OMX_CALL(OMX_SendCommand(appPriv.hEncoder, OMX_CommandStateSet, OMX_StateIdle, nullptr));
  appPriv.encoderEventState.wait();

  /** sending command to all components to go to loaded state */
  OMX_CALL(OMX_SendCommand(appPriv.hEncoder, OMX_CommandStateSet, OMX_StateLoaded, nullptr));
  appPriv.encoderEventState.wait();

  freeBuffers(inportIndex);
  freeBuffers(outportIndex);

  OMX_FreeHandle(appPriv.hEncoder);
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

