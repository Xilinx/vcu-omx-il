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
#include <atomic>
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
#include "base/omx_utils/omx_translate.h"

#include "../common/helpers.h"
#include "../common/setters.h"
#include "../common/getters.h"

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
#include <lib_fpga/DmaAllocLinux.h>
}

enum EncCodec
{
  HEVC,
  HEVC_HARD,
  AVC,
  AVC_HARD,
};

struct Ports
{
  int index;
  bool isDMA;
  atomic<bool> isEOS;
  atomic<bool> isFlushing;
  vector<OMX_BUFFERHEADERTYPE*> buffers;
};

static inline OMX_ALG_BUFFER_MODE GetBufferMode(bool isDMA)
{
  return isDMA ? OMX_ALG_BUF_DMA : OMX_ALG_BUF_NORMAL;
}

struct Settings
{
  int width;
  int height;
  int framerate;
  EncCodec codec;
  OMX_COLOR_FORMATTYPE format;
};

struct Application
{
  semaphore encoderEventSem;
  semaphore eof;
  semaphore encoderEventState;

  OMX_HANDLETYPE hEncoder;

  Settings settings;
  std::mutex mutex;
  condition_variable cv;
  bool read;
  AL_TAllocator* pAllocator;

  Ports input;
  Ports output;
};

static inline void SetDefaultSettings(Settings& settings)
{
  settings.width = 176;
  settings.height = 144;
  settings.framerate = 1;
  settings.codec = HEVC;
  settings.format = OMX_COLOR_FormatYUV420SemiPlanar;
}

static inline void SetDefaultApplication(Application& app)
{
  SetDefaultSettings(app.settings);

  app.input.index = 0;
  app.input.isDMA = false;
  app.input.isFlushing = false;
  app.input.isEOS = false;
  app.output.index = 1;
  app.output.isDMA = false;
  app.output.isFlushing = false;
  app.output.isEOS = false;
  app.read = false;
}

static inline int RoundUp(int iVal, int iRnd)
{
  return (iVal + iRnd - 1) & (~(iRnd - 1));
}

static string input_file;
static string output_file;

static ifstream infile;
static ofstream outfile;
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
}

static OMX_ERRORTYPE setPortParameters(Application& app)
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE inParamFormat;
  initHeader(inParamFormat);
  inParamFormat.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(app.hEncoder, OMX_IndexParamVideoPortFormat, &inParamFormat));

  inParamFormat.eColorFormat = app.settings.format;
  inParamFormat.xFramerate = app.settings.framerate << 16;

  OMX_CALL(OMX_SetParameter(app.hEncoder, OMX_IndexParamVideoPortFormat, &inParamFormat));

  initHeader(paramPort);
  paramPort.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(app.hEncoder, OMX_IndexParamPortDefinition, &paramPort));
  paramPort.format.video.nFrameWidth = app.settings.width;
  paramPort.format.video.nFrameHeight = app.settings.height;
  paramPort.format.video.nStride = is10bits(app.settings.format) ? ((app.settings.width + 2) / 3) * 4 : app.settings.width;
  paramPort.format.video.nSliceHeight = RoundUp(app.settings.height, 32);

  OMX_CALL(OMX_SetParameter(app.hEncoder, OMX_IndexParamPortDefinition, &paramPort));
  OMX_CALL(OMX_GetParameter(app.hEncoder, OMX_IndexParamPortDefinition, &paramPort));

  Setters setter(&app.hEncoder);

  auto isBufModeSetted = setter.SetBufferMode(app.input.index, GetBufferMode(app.input.isDMA));
  assert(isBufModeSetted);
  isBufModeSetted = setter.SetBufferMode(app.output.index, GetBufferMode(app.output.isDMA));
  assert(isBufModeSetted);

  if(user_slice)
  {
    OMX_ALG_VIDEO_PARAM_SLICES slices;
    initHeader(slices);
    slices.nPortIndex = 1;
    slices.nNumSlices = user_slice;
    OMX_SetParameter(app.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoSlices), &slices);

    OMX_ALG_VIDEO_PARAM_SUBFRAME sub;
    initHeader(sub);
    sub.nPortIndex = 1;
    sub.bEnableSubframe = OMX_TRUE;
    OMX_SetParameter(app.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoSubframe), &sub);
  }

  LOGV("Input picture: %ux%u", app.settings.width, app.settings.height);
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

static void parseCommandLine(int argc, char** argv, Application& app)
{
  queue<string> args;
  Settings& settings = app.settings;

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
        settings.width = popInt(args);
      else if(word == "-h")
        settings.height = popInt(args);
      else if(word == "-r")
        settings.framerate = popInt(args);
      else if(word == "-o")
        output_file = popArg(args);
      else if(word == "-chroma")
      {
        const string user_chroma = popArg(args);

        if(!strncmp(user_chroma.c_str(), "NV12", strlen(user_chroma.c_str())))
          settings.format = OMX_COLOR_FormatYUV420SemiPlanar;
        else if(!strncmp(user_chroma.c_str(), "NV16", strlen(user_chroma.c_str())))
          settings.format = OMX_COLOR_FormatYUV422SemiPlanar;
        else if(!strncmp(user_chroma.c_str(), "RX0A", strlen(user_chroma.c_str())))
          settings.format = (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked;
        else if(!strncmp(user_chroma.c_str(), "RX2A", strlen(user_chroma.c_str())))
          settings.format = (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked;
        else
          assert(0);
      }
      else if(word == "-hevc_hard")
        settings.codec = HEVC_HARD;
      else if(word == "-avc_hard")
        settings.codec = AVC_HARD;

      else if(word == "-hevc")
        settings.codec = HEVC;
      else if(word == "-avc")
        settings.codec = AVC;
      else if(word == "-dma-in")
        app.input.isDMA = true;
      else if(word == "-dma-out")
        app.output.isDMA = true;
    }
  }

  if(input_file == "")
  {
    cerr << "No input file found" << endl;
    exit(0);
  }

  if(output_file == "")
  {
    switch(settings.codec)
    {
    case AVC:
      output_file = "output.hardware.h264";
      break;
    case AVC_HARD:
      output_file = "output.hardware.h264";
      break;
    case HEVC:
      output_file = "output.hardware.h265";
      break;

    case HEVC_HARD:
      output_file = "output.hardware.h265";
      break;
    default:
      assert(0);
      break;
    }
  }
};

static bool readOneYuvFrame(OMX_BUFFERHEADERTYPE* pBufferHdr, Application const& app)
{
  auto const width = paramPort.format.video.nFrameWidth;
  auto const row_size = is10bits(app.settings.format) ? (((width + 2) / 3) * 4) : width;
  auto const height = paramPort.format.video.nFrameHeight;
  auto const stride = paramPort.format.video.nStride;
  auto const sliceHeight = paramPort.format.video.nSliceHeight;

  LOGV("w %d, h %d, stride %d, sliceHeight %d", (int)width, (int)height, (int)stride, (int)sliceHeight);

  static int i;
  LOGV("Reading input frame %i %ux%u", i, (unsigned int)width, (unsigned int)height);
  i++;

  auto coef = is422(app.settings.format) ? 1 : 2;
  auto const size = row_size * (height + height / coef);
  vector<uint8_t> frame(size);

  if(infile.peek() == EOF)
    return false;

  infile.read((char*)frame.data(), frame.size());

  auto dst = Buffer_MapData((char*)(pBufferHdr->pBuffer + pBufferHdr->nOffset), pBufferHdr->nAllocLen, app.input.isDMA);

  /* luma */
  for(auto h = 0; h < (int)height; h++)
    memcpy(&dst[h * stride], &frame.data()[h * row_size], row_size);

  /* chroma */
  for(struct { long unsigned int sh; long unsigned int h; } v { sliceHeight, height }; v.sh < sliceHeight + height / coef; v.sh++, v.h++)
    memcpy(&dst[v.sh * stride], &frame.data()[v.h * row_size], row_size);

  pBufferHdr->nFilledLen = pBufferHdr->nAllocLen;
  assert(pBufferHdr->nFilledLen <= pBufferHdr->nAllocLen);

  Buffer_UnmapData((char*)pBufferHdr->pBuffer, pBufferHdr->nAllocLen, app.input.isDMA);

  return true;
};

// Callbacks implementation of the video encoder component
static OMX_ERRORTYPE onComponentEvent(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 Data1, OMX_U32 Data2, OMX_PTR /*pEventData*/)
{
  auto app = static_cast<Application*>(pAppData);
  switch(eEvent)
  {
  case OMX_EventCmdComplete:
  {
    auto cmd = static_cast<OMX_COMMANDTYPE>(Data1);
    switch(cmd)
    {
    case OMX_CommandStateSet:
    {
      printf("Comp %p : %s : %s : %s\n", hComponent, ToStringEvent.at(eEvent), ToStringCommand.at(cmd), ToStringState.at(static_cast<OMX_STATETYPE>(Data2)));
      app->encoderEventState.notify();
      break;
    }
    case OMX_CommandPortEnable:
    case OMX_CommandPortDisable:
    {
      printf("Comp %p : %s : %s : %i\n", hComponent, ToStringEvent.at(eEvent), ToStringCommand.at(cmd), (int)Data2);
      app->encoderEventSem.notify();
      break;
    }
    case OMX_CommandMarkBuffer:
    {
      printf("Comp %p : %s : %s : %i\n", hComponent, ToStringEvent.at(eEvent), ToStringCommand.at(cmd), (int)Data2);
      app->encoderEventSem.notify();
      break;
    }
    case OMX_CommandFlush:
    {
      printf("Comp %p : %s : %s : %i\n", hComponent, ToStringEvent.at(eEvent), ToStringCommand.at(cmd), (int)Data2);
      app->encoderEventSem.notify();
      break;
    }
    default:
      break;
    }

    break;
  }

  case OMX_EventError:
  {
    printf("Comp %p : %s\n", hComponent, ToStringEvent.at(eEvent));
    return OMX_ErrorUndefined;
  }
  default:
  {
    printf("Comp %p : Unspported %s\n", hComponent, ToStringEvent.at(eEvent));
    return OMX_ErrorNotImplemented;
  }
  }

  return OMX_ErrorNone;
}

static void Read(OMX_BUFFERHEADERTYPE* pBuffer, Application& app)
{
  pBuffer->nFlags = 0; // clear flags;
  auto eos = (readOneYuvFrame(pBuffer, app) == 0);
  pBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

  if(eos)
  {
    pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
    app.input.isEOS = true;
    printf("Waiting for EOS...  \n");
  }
}

static OMX_ERRORTYPE onInputBufferAvailable(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
  auto app = static_cast<Application*>(pAppData);
  assert(pBuffer->nFilledLen == 0);
  assert(hComponent == app->hEncoder);

  if(app->input.isEOS)
    return OMX_ErrorNone;

  if(app->input.isFlushing)
    return OMX_ErrorNone;

  unique_lock<mutex> lock(app->mutex);
  app->cv.wait(lock, [&] { return app->read = true;
               });

  Read(pBuffer, *app);
  OMX_EmptyThisBuffer(hComponent, pBuffer);

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE onOutputBufferAvailable(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBufferHdr)
{
  auto app = static_cast<Application*>(pAppData);
  assert(hComponent == app->hEncoder);

  if(app->output.isEOS)
    return OMX_ErrorNone;

  if(app->output.isFlushing)
    return OMX_ErrorNone;

  if(!pBufferHdr)
    assert(0);

  auto zMapSize = pBufferHdr->nAllocLen;

  if(zMapSize)
  {
    auto data = Buffer_MapData((char*)(pBufferHdr->pBuffer + pBufferHdr->nOffset), zMapSize, app->output.isDMA);

    if(data)
    {
      outfile.write((char*)data, pBufferHdr->nFilledLen);
      outfile.flush();
    }

    Buffer_UnmapData(data, zMapSize, app->output.isDMA);
  }

  if(pBufferHdr->nFlags & OMX_BUFFERFLAG_EOS)
  {
    app->eof.notify();
    app->output.isEOS = true;
  }
  pBufferHdr->nFilledLen = 0;
  pBufferHdr->nFlags = 0;
  OMX_CALL(OMX_FillThisBuffer(hComponent, pBufferHdr));

  return OMX_ErrorNone;
}

static void useBuffers(OMX_U32 nPortIndex, bool use_dmabuf, Application& app)
{
  Getters get(&app.hEncoder);
  auto const size = get.GetBuffersSize(nPortIndex);
  auto const minBuf = get.GetBuffersCount(nPortIndex);
  auto const isInput = ((int)nPortIndex == app.input.index);

  for(auto nbBuf = 0; nbBuf < minBuf; nbBuf++)
  {
    OMX_U8* pBufData;

    if(use_dmabuf)
    {
      AL_HANDLE hBuf = AL_Allocator_Alloc(app.pAllocator, size);

      if(!hBuf)
      {
        fprintf(stderr, "Failed to allocate Buffer for dma");
        assert(0);
      }
      auto fd = AL_LinuxDmaAllocator_ExportToFd((AL_TLinuxDmaAllocator*)(app.pAllocator), hBuf);
      pBufData = (OMX_U8*)(uintptr_t)dup(fd);

      if(!pBufData)
      {
        fprintf(stderr, "Failed to ExportToFd %p", hBuf);
        assert(0);
      }

      AL_Allocator_Free(app.pAllocator, hBuf);
    }
    else
      pBufData = (OMX_U8*)calloc(size, sizeof(OMX_U8));

    OMX_BUFFERHEADERTYPE* pBufHeader;
    OMX_UseBuffer(app.hEncoder, &pBufHeader, nPortIndex, &app, size, pBufData);
    isInput ? app.input.buffers.push_back(pBufHeader) : app.output.buffers.push_back(pBufHeader);
  }
}

static void allocBuffers(OMX_U32 nPortIndex, Application& app)
{
  Getters get(&app.hEncoder);
  auto const size = get.GetBuffersSize(nPortIndex);
  auto const minBuf = get.GetBuffersCount(nPortIndex);
  auto const isInput = ((int)nPortIndex == app.input.index);

  for(auto nbBuf = 0; nbBuf < minBuf; nbBuf++)
  {
    OMX_BUFFERHEADERTYPE* pBufHeader;
    OMX_AllocateBuffer(app.hEncoder, &pBufHeader, nPortIndex, &app, size);
    isInput ? app.input.buffers.push_back(pBufHeader) : app.output.buffers.push_back(pBufHeader);
  }
}

static void freeUseBuffers(OMX_U32 nPortIndex, Application& app)
{
  Getters get(&app.hEncoder);
  auto const minBuf = get.GetBuffersCount(nPortIndex);
  auto buffers = ((int)nPortIndex == app.input.index) ? app.input.buffers : app.output.buffers;
  auto const isDMA = ((int)nPortIndex == app.input.index) ? app.input.isDMA : app.output.isDMA;

  for(auto nbBuf = 0; nbBuf < minBuf; nbBuf++)
  {
    auto pBufHeader = buffers.back();
    buffers.pop_back();
    Buffer_FreeData((char*)pBufHeader->pBuffer, isDMA);
    OMX_FreeBuffer(app.hEncoder, nPortIndex, pBufHeader);
  }
}

static void freeAllocBuffers(OMX_U32 nPortIndex, Application& app)
{
  Getters get(&app.hEncoder);
  auto const minBuf = get.GetBuffersCount(nPortIndex);
  auto buffers = ((int)nPortIndex == app.input.index) ? app.input.buffers : app.output.buffers;

  for(auto nbBuf = 0; nbBuf < minBuf; nbBuf++)
  {
    auto pBuf = buffers.back();
    buffers.pop_back();
    OMX_FreeBuffer(app.hEncoder, nPortIndex, pBuf);
  }
}

static OMX_ERRORTYPE safeMain(int argc, char** argv)
{
  Application app;
  SetDefaultApplication(app);
  parseCommandLine(argc, argv, app);

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

  string component;
  switch(app.settings.codec)
  {
  case AVC:
    component = "OMX.allegro.h264.encoder";
    break;
  case AVC_HARD:
    component = "OMX.allegro.h264.hardware.encoder";
    break;
  case HEVC:
    component = "OMX.allegro.h265.encoder";
    break;
  case HEVC_HARD:
    component = "OMX.allegro.h265.hardware.encoder";
    break;

  default:
    assert(0);
  }

  OMX_CALLBACKTYPE const videoEncoderCallbacks =
  {
    .EventHandler = onComponentEvent,
    .EmptyBufferDone = onInputBufferAvailable,
    .FillBufferDone = onOutputBufferAvailable
  };

  /** getting video encoder handle */
  OMX_CALL(OMX_GetHandle(&app.hEncoder, (OMX_STRING)component.c_str(), &app, const_cast<OMX_CALLBACKTYPE*>(&videoEncoderCallbacks)));

  auto name = (OMX_STRING)calloc(OMX_MAX_STRINGNAME_SIZE, sizeof(char));
  OMX_VERSIONTYPE compType;
  OMX_VERSIONTYPE ilType;

  OMX_CALL(OMX_GetComponentVersion(app.hEncoder, (OMX_STRING)name, &compType, &ilType, nullptr));

  LOGI("Component : %s (v.%u) made for OMX_IL client : %u.%u.%u", name, compType.nVersion, ilType.s.nVersionMajor, ilType.s.nVersionMinor, ilType.s.nRevision);

  free(name);

  auto const ret = setPortParameters(app);

  if(ret != OMX_ErrorNone)
    return ret;

  /** Allocate Buffers **/
  app.pAllocator = DmaAlloc_Create("/dev/allegroIP");

  Getters get(&app.hEncoder);

  /** sending command to video encoder component to go to idle state */
  OMX_SendCommand(app.hEncoder, OMX_CommandStateSet, OMX_StateIdle, nullptr);

  get.IsComponentSupplier(app.input.index) ?
  allocBuffers(app.input.index, app) : useBuffers(app.input.index, app.input.isDMA, app);

  get.IsComponentSupplier(app.output.index) ?
  allocBuffers(app.output.index, app) : useBuffers(app.output.index, app.output.isDMA, app);

  app.encoderEventState.wait();

  /** sending command to video encoder component to go to executing state */
  OMX_CALL(OMX_SendCommand(app.hEncoder, OMX_CommandStateSet, OMX_StateExecuting, nullptr));
  app.encoderEventState.wait();

  if((app.settings.format != OMX_COLOR_FormatYUV420SemiPlanar) &&
     (app.settings.format != OMX_COLOR_FormatYUV422SemiPlanar) &&
     (app.settings.format != (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked) &&
     (app.settings.format != (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked))
  {
    fprintf(stderr, "Unsupported color format : 0X%.8X", app.settings.format);
    return OMX_ErrorUnsupportedSetting;
  }

  for(auto i = 0; i < get.GetBuffersCount(app.output.index); ++i)
    OMX_CALL(OMX_FillThisBuffer(app.hEncoder, app.output.buffers.at(i)));

  unique_lock<mutex> lock(app.mutex);
  app.read = false;
  lock.unlock();

  for(auto i = 0; i < get.GetBuffersCount(app.input.index); i++)
  {
    auto buf = app.input.buffers.at(i);
    Read(buf, app);
    OMX_EmptyThisBuffer(app.hEncoder, buf);
  }

  lock.lock();
  app.read = true;
  app.cv.notify_one();
  lock.unlock();

  app.eof.wait();
  LOGV("EOS received");

  /** send flush in input port */
  app.input.isFlushing = true;
  OMX_CALL(OMX_SendCommand(app.hEncoder, OMX_CommandFlush, app.input.index, nullptr));
  app.encoderEventSem.wait();
  app.input.isFlushing = false;

  /** send flush on output port */
  app.output.isFlushing = true;
  OMX_CALL(OMX_SendCommand(app.hEncoder, OMX_CommandFlush, app.output.index, nullptr));
  app.encoderEventSem.wait();
  app.output.isFlushing = false;

  /** state change of all components from executing to idle */
  OMX_CALL(OMX_SendCommand(app.hEncoder, OMX_CommandStateSet, OMX_StateIdle, nullptr));
  app.encoderEventState.wait();

  /** free buffers */
  get.IsComponentSupplier(app.input.index) ? freeAllocBuffers(app.input.index, app) : freeUseBuffers(app.input.index, app);
  get.IsComponentSupplier(app.output.index) ? freeAllocBuffers(app.input.index, app) : freeUseBuffers(app.output.index, app);

  /** sending command to all components to go to loaded state */
  OMX_CALL(OMX_SendCommand(app.hEncoder, OMX_CommandStateSet, OMX_StateLoaded, nullptr));
  app.encoderEventState.wait();

  OMX_FreeHandle(app.hEncoder);
  OMX_Deinit();

  if(app.pAllocator)
    AL_Allocator_Destroy(app.pAllocator);

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

