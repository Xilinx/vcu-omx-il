/******************************************************************************
*
* Copyright (C) 2019 Allegro DVT2.  All rights reserved.
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

#if __ANDROID_API__
#define LOG_NDEBUG 0
#define LOG_TAG "AL_OMX_MAIN_EXE"
#include <android/log.h>
#include "HardwareAPI.h"
#endif

using namespace std;

extern "C"
{
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Video.h>
#include <OMX_VideoAlg.h>
#include <OMX_VideoExt.h>
#include <OMX_ComponentExt.h>
#include <OMX_IndexExt.h>
#include <OMX_IndexAlg.h>
#include <OMX_IVCommonAlg.h>
}

#include "CommandsSender.h"
#include "EncCmdMngr.h"

#include "base/omx_utils/locked_queue.h"
#include "base/omx_utils/semaphore.h"
#include "base/omx_utils/omx_log.h"
#include "base/omx_utils/omx_translate.h"
#include "base/omx_utils/round.h"

#include "../common/helpers.h"
#include "../common/setters.h"
#include "../common/getters.h"
#include "../common/CommandLineParser.h"
#include "../common/codec.h"

extern "C"
{
#include <lib_fpga/DmaAlloc.h>
#include <lib_fpga/DmaAllocLinux.h>
}

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
  Codec codec;
  OMX_COLOR_FORMATTYPE format;
  int lookahead;
  int pass;
  string twoPassLogFile;
  bool isDummySeiEnabled;
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

  CEncCmdMngr* encCmd;
  CommandsSender* cmdSender;
};

static inline void SetDefaultSettings(Settings& settings)
{
  settings.width = 176;
  settings.height = 144;
  settings.framerate = 1;
  settings.codec = Codec::HEVC;
  settings.format = OMX_COLOR_FormatYUV420SemiPlanar;
  settings.lookahead = 0;
  settings.pass = 0;
  settings.twoPassLogFile = "";
  settings.isDummySeiEnabled = false;
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

static string input_file;
static string output_file;
static string cmd_file;

static ifstream infile;
static ofstream outfile;
static int user_slice = 0;

static OMX_PARAM_PORTDEFINITIONTYPE paramPort;

static OMX_ERRORTYPE setEnableLongTerm(Application& app)
{
  OMX_ALG_VIDEO_PARAM_LONG_TERM lt;
  initHeader(lt);
  lt.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(app.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoLongTerm), &lt));
  lt.bEnableLongTerm = OMX_TRUE;
  OMX_CALL(OMX_SetParameter(app.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoLongTerm), &lt));
  return OMX_ErrorNone;
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
  paramPort.format.video.nSliceHeight = RoundUp(app.settings.height, 8);

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


  if(app.settings.lookahead)
  {
    OMX_ALG_VIDEO_PARAM_LOOKAHEAD la;
    initHeader(la);
    la.nPortIndex = 1;
    la.nLookAhead = app.settings.lookahead;
    la.bEnableFirstPassCrop = OMX_FALSE;
    OMX_SetParameter(app.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoLookAhead), &la);
  }

  if(app.settings.pass)
  {
    OMX_ALG_VIDEO_PARAM_TWOPASS tp;
    initHeader(tp);
    tp.nPortIndex = 1;
    tp.nPass = app.settings.pass;
    tp.sLogFile = const_cast<char*>(app.settings.twoPassLogFile.c_str());
    OMX_SetParameter(app.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoTwoPass), &tp);
  }

  OMX_PARAM_PORTDEFINITIONTYPE paramPortForActual;
  initHeader(paramPortForActual);
  paramPortForActual.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(app.hEncoder, OMX_IndexParamPortDefinition, &paramPortForActual));
  paramPortForActual.nBufferCountActual = paramPortForActual.nBufferCountMin + 4; // alloc max for b frames
  OMX_CALL(OMX_SetParameter(app.hEncoder, OMX_IndexParamPortDefinition, &paramPortForActual));
  paramPortForActual.nPortIndex = 1;
  OMX_CALL(OMX_GetParameter(app.hEncoder, OMX_IndexParamPortDefinition, &paramPortForActual));
  paramPortForActual.nBufferCountActual = paramPortForActual.nBufferCountMin + 4; // alloc max for b frames
  OMX_CALL(OMX_SetParameter(app.hEncoder, OMX_IndexParamPortDefinition, &paramPortForActual));
  OMX_CALL(OMX_GetParameter(app.hEncoder, OMX_IndexParamPortDefinition, &paramPort));

  setEnableLongTerm(app);

  LOGV("Input picture: %ux%u\n", app.settings.width, app.settings.height);
  return OMX_ErrorNone;
}

static void Usage(CommandLineParser& opt, char* ExeName)
{
  cerr << "Usage: " << ExeName << " <InputFile> [options]" << endl;
  cerr << "Options:" << endl;

  for(auto& command: opt.displayOrder)
    cerr << "  " << opt.descs[command] << endl;
}

static bool isFourCCExist(string const& fourcc)
{
  return fourcc == "y800" || fourcc == "xv10" || fourcc == "nv12" || fourcc == "xv15" || fourcc == "nv16" || fourcc == "xv20";
}

static void parseCommandLine(int argc, char** argv, Application& app)
{
  Settings& settings = app.settings;
  bool help = false;
  string fourcc = "nv12";

  auto opt = CommandLineParser();
  opt.addString("input_file", &input_file, "Input file");
  opt.addFlag("--help", &help, "Show this help");
  opt.addInt("--width", &settings.width, "Input width ('176')");
  opt.addInt("--height", &settings.height, "Input height ('144')");
  opt.addInt("--framerate", &settings.framerate, "Input fps ('1')");
  opt.addString("--out", &output_file, "Output compressed file name");
  opt.addString("--fourcc", &fourcc, "Input file fourcc <y800 || xv10 || nv12 || xv15 || nv16 || xv20> ('nv12')");
  opt.addFlag("--hevc", &settings.codec, "", Codec::HEVC);
  opt.addFlag("--avc", &settings.codec, "", Codec::AVC);
  opt.addFlag("--hevc-hard", &settings.codec, "Use hard hevc encoder", Codec::HEVC_HARD);
  opt.addFlag("--avc-hard", &settings.codec, "Use hard avc encoder", Codec::AVC_HARD);
  opt.addFlag("--dma-in", &app.input.isDMA, "Use dmabufs on input port");
  opt.addFlag("--dma-out", &app.output.isDMA, "Use dmabufs on output port");
  opt.addInt("--subframe", &user_slice, "<4 || 8 || 16>: activate subframe latency '(0)'");
  opt.addString("--cmd-file", &cmd_file, "File to precise for dynamic cmd");
  opt.addInt("--lookahead", &settings.lookahead, "<0 || above 2>: activate lookahead mode '(0)'");
  opt.addInt("--pass", &settings.pass, "<0 || 1 || 2>: specify which pass we encode'(0)'");
  opt.addString("--pass-logfile", &settings.twoPassLogFile, "LogFile to transmit dualpass statistics");
  opt.addFlag("--dummy-sey", &settings.isDummySeiEnabled, "Enable dummy seis on firsts frames");

  opt.parse(argc, argv);

  if(help)
  {
    Usage(opt, argv[0]);
    exit(0);
  }

  if(!isFourCCExist(fourcc))
  {
    cerr << "[Error] fourcc doesn't exist" << endl;
    Usage(opt, argv[0]);
    exit(1);
  }

  if(fourcc == "y800")
    settings.format = OMX_COLOR_FormatL8;

  if(fourcc == "xv10")
    settings.format = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL10bitPacked);

  if(fourcc == "nv12")
    settings.format = OMX_COLOR_FormatYUV420SemiPlanar;

  if(fourcc == "xv15")
    settings.format = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked);

  if(fourcc == "nv16")
    settings.format = OMX_COLOR_FormatYUV422SemiPlanar;

  if(fourcc == "xv20")
    settings.format = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked);

  if(!(user_slice == 0 || user_slice == 4 || user_slice == 8 || user_slice == 16))
  {
    Usage(opt, argv[0]);
    cerr << "[Error] subframe parameter was incorrectly set" << endl;
    exit(1);
  }

  if(input_file == "")
  {
    Usage(opt, argv[0]);
    cerr << "[Error] No input file found" << endl;
    exit(1);
  }

  if(output_file == "")
  {
    switch(settings.codec)
    {
    case Codec::AVC:
      output_file = "output.hardware.h264";
      break;
    case Codec::AVC_HARD:
      output_file = "output.hardware.h264";
      break;
    case Codec::HEVC:
      output_file = "output.hardware.h265";
      break;

    case Codec::HEVC_HARD:
      output_file = "output.hardware.h265";
      break;
    default:
      assert(0);
      break;
    }
  }
}

static bool readOneYuvFrame(OMX_BUFFERHEADERTYPE* pBufferHdr, Application const& app)
{
  if(infile.peek() == EOF)
    return false;

  auto width = paramPort.format.video.nFrameWidth;
  auto height = paramPort.format.video.nFrameHeight;

  static int input_frame_count;
  LOGV("Reading input frame %i\n", input_frame_count);
  auto stride = paramPort.format.video.nStride;
  auto sliceHeight = paramPort.format.video.nSliceHeight;
  LOGV("%dx%d, stride %d, sliceHeight %d\n", (int)width, (int)height, (int)stride, (int)sliceHeight);
  input_frame_count++;

  auto color = app.settings.format;
  auto row_size = is10bits(color) ? (((width + 2) / 3) * 4) : width;
  auto coef = is422(color) ? 1 : 2;
  auto column_size = is400(color) ? height : height + height / coef;
  auto size = row_size * column_size;
  vector<uint8_t> frame(size);

  infile.read((char*)frame.data(), frame.size());

  auto dst = Buffer_MapData((char*)(pBufferHdr->pBuffer + pBufferHdr->nOffset), pBufferHdr->nAllocLen, app.input.isDMA);

  /* luma */
  for(auto h = 0; h < (int)height; h++)
    memcpy(&dst[h * stride], &frame.data()[h * row_size], row_size);

  /* chroma */
  if(!is400(color))
  {
    for(struct { long unsigned int sh; long unsigned int h; } v { sliceHeight, height }; v.sh < sliceHeight + height / coef; v.sh++, v.h++)
      memcpy(&dst[v.sh * stride], &frame.data()[v.h * row_size], row_size);
  }

  pBufferHdr->nFilledLen = pBufferHdr->nAllocLen;
  assert(pBufferHdr->nFilledLen <= pBufferHdr->nAllocLen);

  Buffer_UnmapData((char*)pBufferHdr->pBuffer, pBufferHdr->nAllocLen, app.input.isDMA);

  return true;
}

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
      LOGI("Comp %p: %s: %s: %s\n", hComponent, ToStringOMXEvent.at(eEvent), ToStringOMXCommand.at(cmd), ToStringOMXState.at(static_cast<OMX_STATETYPE>(Data2)));
      app->encoderEventState.notify();
      break;
    }
    case OMX_CommandPortEnable:
    case OMX_CommandPortDisable:
    {
      LOGI("Comp %p: %s: %s: %i\n", hComponent, ToStringOMXEvent.at(eEvent), ToStringOMXCommand.at(cmd), (int)Data2);
      app->encoderEventSem.notify();
      break;
    }
    case OMX_CommandMarkBuffer:
    {
      LOGI("Comp %p: %s: %s: %i\n", hComponent, ToStringOMXEvent.at(eEvent), ToStringOMXCommand.at(cmd), (int)Data2);
      app->encoderEventSem.notify();
      break;
    }
    case OMX_CommandFlush:
    {
      LOGI("Comp %p: %s: %s: %i\n", hComponent, ToStringOMXEvent.at(eEvent), ToStringOMXCommand.at(cmd), (int)Data2);
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
    auto cmd = static_cast<OMX_ERRORTYPE>(Data1);
    LOGE("Comp %p: %s (%s)\n", hComponent, ToStringOMXEvent.at(eEvent), ToStringOMXError.at(cmd));
    exit(1);
  }
  /* this event will be fired by the component but we have nothing special to do with them */
  case OMX_EventBufferFlag: // fallthrough
  case OMX_EventPortSettingsChanged:
  {
    LOGI("Comp %p: Got %s\n", hComponent, ToStringOMXEvent.at(eEvent));
    break;
  }
  default:
  {
    LOGI("Comp %p: Unsupported %s\n", hComponent, ToStringOMXEvent.at(eEvent));
    return OMX_ErrorNotImplemented;
  }
  }

  return OMX_ErrorNone;
}

static void Read(OMX_BUFFERHEADERTYPE* pBuffer, Application& app)
{
  static int frame = 0;
  pBuffer->nFlags = 0; // clear flags;
  auto eos = (readOneYuvFrame(pBuffer, app) == false);
  pBuffer->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

  if(eos)
  {
    pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
    app.input.isEOS = true;
    LOGI("Waiting for EOS...  \n");
    return;
  }

  app.encCmd->Process(app.cmdSender, frame);
  frame++;
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
  auto size = get.GetBuffersSize(nPortIndex);
  auto minBuf = get.GetBuffersCount(nPortIndex);
  auto isInput = ((int)nPortIndex == app.input.index);

  for(auto nbBuf = 0; nbBuf < minBuf; nbBuf++)
  {
    OMX_U8* pBufData;

    if(use_dmabuf)
    {
      AL_HANDLE hBuf = AL_Allocator_Alloc(app.pAllocator, size);

      if(!hBuf)
      {
        LOGE("Failed to allocate Buffer for dma\n");
        assert(0);
      }
      auto fd = AL_LinuxDmaAllocator_GetFd((AL_TLinuxDmaAllocator*)(app.pAllocator), hBuf);
      pBufData = (OMX_U8*)(uintptr_t)dup(fd);

      if(!pBufData)
      {
        LOGE("Failed to ExportToFd %p\n", hBuf);
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
  auto size = get.GetBuffersSize(nPortIndex);
  auto minBuf = get.GetBuffersCount(nPortIndex);
  auto isInput = ((int)nPortIndex == app.input.index);

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
  auto minBuf = get.GetBuffersCount(nPortIndex);
  auto buffers = ((int)nPortIndex == app.input.index) ? app.input.buffers : app.output.buffers;
  auto isDMA = ((int)nPortIndex == app.input.index) ? app.input.isDMA : app.output.isDMA;

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
  auto minBuf = get.GetBuffersCount(nPortIndex);
  auto buffers = ((int)nPortIndex == app.input.index) ? app.input.buffers : app.output.buffers;

  for(auto nbBuf = 0; nbBuf < minBuf; nbBuf++)
  {
    auto pBuf = buffers.back();
    buffers.pop_back();
    OMX_FreeBuffer(app.hEncoder, nPortIndex, pBuf);
  }
}

static string chooseComponent(Codec codec)
{
  switch(codec)
  {
  case Codec::AVC:
    return "OMX.allegro.h264.encoder";
  case Codec::AVC_HARD:
    return "OMX.allegro.h264.hardware.encoder";
  case Codec::HEVC:
    return "OMX.allegro.h265.encoder";
  case Codec::HEVC_HARD:
    return "OMX.allegro.h265.hardware.encoder";
  default:
    assert(0);
  }
}

static bool isFormatSupported(OMX_COLOR_FORMATTYPE format)
{
  return (format == OMX_COLOR_FormatL8) ||
         (format == OMX_COLOR_FormatYUV420SemiPlanar) ||
         (format == OMX_COLOR_FormatYUV422SemiPlanar) ||
         (format == static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL10bitPacked)) ||
         (format == static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked)) ||
         (format == static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked))
  ;
}

static OMX_ERRORTYPE showComponentVersion(Application& app)
{
  char name[OMX_MAX_STRINGNAME_SIZE];
  OMX_VERSIONTYPE compType;
  OMX_VERSIONTYPE ilType;

  OMX_CALL(OMX_GetComponentVersion(app.hEncoder, (OMX_STRING)name, &compType, &ilType, nullptr));

  LOGI("Component: %s (v.%u) made for OMX_IL client: %u.%u.%u\n", name, compType.nVersion, ilType.s.nVersionMajor, ilType.s.nVersionMinor, ilType.s.nRevision);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE safeMain(int argc, char** argv)
{
  Application app;
  SetDefaultApplication(app);
  parseCommandLine(argc, argv, app);

  infile.open(input_file, ios::binary);

  if(!infile.is_open())
  {
    cerr << "Error in opening input file '" << input_file << "'" << endl;
    return OMX_ErrorUndefined;
  }

  auto scopeInfile = scopeExit([]() {
    infile.close();
  });

  outfile.open(output_file, ios::binary);

  if(!outfile.is_open())
  {
    cerr << "Error in opening output file '" << output_file << "'" << endl;
    return OMX_ErrorUndefined;
  }

  auto scopeOutfile = scopeExit([]() {
    outfile.close();
  });

  LOGI("cmd file = %s\n", cmd_file.c_str());
  ifstream cmdfile(cmd_file != "" ? cmd_file.c_str() : "/dev/null");
  auto scopeCmdfile = scopeExit([&]() {
    cmdfile.close();
  });
  auto encCmd = CEncCmdMngr(cmdfile, 3, -1);
  app.encCmd = &encCmd;

  OMX_CALL(OMX_Init());
  auto scopeOMX = scopeExit([]() {
    OMX_Deinit();
  });
  auto component = chooseComponent(app.settings.codec);

  OMX_CALLBACKTYPE videoEncoderCallbacks;
  videoEncoderCallbacks.EventHandler = onComponentEvent;
  videoEncoderCallbacks.EmptyBufferDone = onInputBufferAvailable;
  videoEncoderCallbacks.FillBufferDone = onOutputBufferAvailable;

  OMX_CALL(OMX_GetHandle(&app.hEncoder, (OMX_STRING)component.c_str(), &app, const_cast<OMX_CALLBACKTYPE*>(&videoEncoderCallbacks)));
  auto scopeHandle = scopeExit([&]() {
    OMX_FreeHandle(app.hEncoder);
  });

  OMX_CALL(showComponentVersion(app));
  auto ret = setPortParameters(app);

  if(ret != OMX_ErrorNone)
    return ret;

  app.pAllocator = nullptr;

  if(app.input.isDMA || app.output.isDMA)
  {
    auto constexpr deviceName = "/dev/allegroIP";
    app.pAllocator = AL_DmaAlloc_Create(deviceName);

    if(!app.pAllocator)
      throw runtime_error(string("Couldn't create dma allocator (using ") + deviceName + string(")"));
  }

  auto scopeAlloc = scopeExit([&]() {
    if(app.pAllocator)
      AL_Allocator_Destroy(app.pAllocator);
  });

  Getters get(&app.hEncoder);

  if(!isFormatSupported(app.settings.format))
  {
    LOGE("Unsupported color format : 0X%.8X\n", app.settings.format);
    return OMX_ErrorUnsupportedSetting;
  }

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

  for(auto i = 0; i < get.GetBuffersCount(app.output.index); ++i)
    OMX_CALL(OMX_FillThisBuffer(app.hEncoder, app.output.buffers.at(i)));

  unique_lock<mutex> lock(app.mutex);
  app.read = false;
  lock.unlock();

  auto cmdSender = CommandsSender(app.hEncoder);
  app.cmdSender = &cmdSender;

  OMX_ALG_VIDEO_CONFIG_SEI seiPrefix;
  OMX_ALG_VIDEO_CONFIG_SEI seiSuffix;
  initHeader(seiPrefix);
  initHeader(seiSuffix);
  seiPrefix.nType = 15;
  seiPrefix.pBuffer = new OMX_U8[128];
  seiPrefix.nOffset = 0;
  seiPrefix.nFilledLen = 128;
  seiPrefix.nAllocLen = 128;
  seiSuffix.nType = 18;
  seiSuffix.pBuffer = new OMX_U8[128];
  seiSuffix.nOffset = 0;
  seiSuffix.nFilledLen = 128;
  seiSuffix.nAllocLen = 128;

  for(int i = 0; i < static_cast<int>(seiPrefix.nFilledLen); i++)
  {
    seiPrefix.pBuffer[i] = i;
    seiSuffix.pBuffer[i] = seiSuffix.nFilledLen - 1 - i;
  }

  for(auto i = 0; i < get.GetBuffersCount(app.input.index); i++)
  {
    auto buf = app.input.buffers.at(i);
    Read(buf, app);

    if(app.settings.isDummySeiEnabled)
    {
      OMX_SetConfig(app.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoInsertPrefixSEI), &seiPrefix);
      OMX_SetConfig(app.hEncoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexConfigVideoInsertSuffixSEI), &seiSuffix);
    }
    OMX_EmptyThisBuffer(app.hEncoder, buf);

    if(app.input.isEOS)
      break;
  }

  lock.lock();
  app.read = true;
  app.cv.notify_one();
  lock.unlock();

  app.eof.wait();
  LOGV("EOS received\n");
  delete[] seiPrefix.pBuffer;
  delete[] seiSuffix.pBuffer;

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

  /** sending command to all components to go to loaded state */
  OMX_CALL(OMX_SendCommand(app.hEncoder, OMX_CommandStateSet, OMX_StateLoaded, nullptr));

  /** free buffers */
  get.IsComponentSupplier(app.input.index) ? freeAllocBuffers(app.input.index, app) : freeUseBuffers(app.input.index, app);
  get.IsComponentSupplier(app.output.index) ? freeAllocBuffers(app.input.index, app) : freeUseBuffers(app.output.index, app);

  app.encoderEventState.wait();

  return OMX_ErrorNone;
}

int main(int argc, char** argv)
{
  try
  {
    if(safeMain(argc, argv) != OMX_ErrorNone)
    {
      cerr << "Fatal error" << endl;
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
  catch(runtime_error const& error)
  {
    cerr << endl << "Exception caught: " << error.what() << endl;
    return EXIT_FAILURE;
  }
}

