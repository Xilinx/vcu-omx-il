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
}

#include "base/omx_utils/locked_queue.h"
#include "base/omx_utils/semaphore.h"
#include "base/omx_utils/omx_log.h"

#include "../common/helpers.h"
#include "../common/setters.h"
#include "../common/CommandLineParser.h"

extern "C"
{
#include "lib_fpga/DmaAlloc.h"
#include "lib_fpga/DmaAllocLinux.h"
}

enum DecCodec
{
  HEVC,
  HEVC_HARD,
  AVC,
  AVC_HARD,
};

static OMX_U32 inportIndex = 0;
static OMX_U32 outportIndex = 1;

struct Settings
{
  DecCodec codecImplem = HEVC;
  DecCodec codec = HEVC;
  bool bDMAIn = false;
  bool bDMAOut = false;
  OMX_ALG_BUFFER_MODE eDMAIn = OMX_ALG_BUF_NORMAL;
  OMX_ALG_BUFFER_MODE eDMAOut = OMX_ALG_BUF_NORMAL;
  OMX_COLOR_FORMATTYPE chroma = OMX_COLOR_FormatYUV420SemiPlanar;

  int width = 176;
  int height = 144;
  OMX_U32 level = OMX_VIDEO_HEVCLevelUnknown;
  OMX_U32 profile = OMX_VIDEO_HEVCProfileUnknown;
  OMX_U32 framerate = 1 << 16;
  bool hasPrealloc = false;
};

struct Application
{
  semaphore eofSem;
  semaphore decoderEventState;
  semaphore flushEvent;

  OMX_HANDLETYPE hDecoder;
  OMX_PTR pAppData;

  Settings settings;
  locked_queue<OMX_BUFFERHEADERTYPE*> inputBuffers;
  list<OMX_BUFFERHEADERTYPE*> outputBuffers;
  AL_TAllocator* pAllocator;
};

string input_file;
string output_file;
ifstream infile;
ofstream outfile;

static OMX_PARAM_PORTDEFINITIONTYPE paramPort;

static void Usage(CommandLineParser const& opt, char* ExeName)
{
  cerr << "Usage: " << ExeName << " <InputFile> [options]" << endl;
  cerr << "Options:" << endl;

  for(auto& name : opt.displayOrder)
  {
    auto& o = opt.options.at(name);
    cerr << "  " << o.desc << endl;
  }
}

bool setChroma(string user_chroma, OMX_COLOR_FORMATTYPE* chroma)
{
  if(user_chroma == "NV12")
    *chroma = OMX_COLOR_FormatYUV420SemiPlanar;
  else if(user_chroma == "NV16")
    *chroma = OMX_COLOR_FormatYUV422SemiPlanar;
  else if(user_chroma == "RX0A")
    *chroma = (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked;
  else if(user_chroma == "RX2A")
    *chroma = (OMX_COLOR_FORMATTYPE)OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked;
  else
    return false;
  return true;
}

void getExpectedSeparator(stringstream& ss, char expectedSep)
{
  char sep;
  ss >> sep;

  if(sep != expectedSep)
    throw runtime_error("wrong separator in prealloc arguments format");
}

bool invalidPreallocSettings(Settings const& settings)
{
  return settings.profile <= 0 || settings.level <= 0
         || settings.width <= 0 || settings.height <= 0;
}

void parsePreAllocArgs(Settings* settings, string& toParse)
{
  stringstream ss(toParse);
  ss.unsetf(ios::dec);
  ss.unsetf(ios::hex);
  char chroma[4] {};
  ss >> settings->width;
  getExpectedSeparator(ss, 'x');
  ss >> settings->height;
  getExpectedSeparator(ss, ':');
  ss >> chroma[0];
  ss >> chroma[1];
  ss >> chroma[2];
  ss >> chroma[3];
  getExpectedSeparator(ss, ':');
  ss >> settings->profile;
  getExpectedSeparator(ss, ':');
  ss >> settings->level;

  if(!setChroma(chroma, &settings->chroma))
    throw runtime_error("wrong prealloc chroma format");

  if(ss.fail() || ss.tellg() != streampos(-1))
    throw runtime_error("wrong prealloc arguments format");

  if(invalidPreallocSettings(*settings))
    throw runtime_error("wrong prealloc arguments");
}

void parseCommandLine(int argc, char** argv, Application& app)
{
  string user_chroma = "NV12";

  Settings& settings = app.settings;
  auto opt = CommandLineParser();
  bool help = false;
  opt.addFlag("--help,-help,-h", &help, "Show this help");
  opt.addFlag("--hevc,-hevc", &settings.codecImplem, "load HEVC decoder (default)", HEVC);
  opt.addFlag("--avc,-avc", &settings.codecImplem, "load AVC decoder", AVC);
  opt.addFlag("--hevc_hard,-hevc_hard", &settings.codecImplem, "Use hard hevc decoder", HEVC_HARD);
  opt.addFlag("--avc_hard,-hevc_hard", &settings.codecImplem, "Use hard avc decoder", AVC_HARD);
  opt.addString("--out,-o", &output_file, "Output compressed file name");
  opt.addString("--chroma,-chroma", &user_chroma, "<NV12 || RX0A || NV16 || RX2A> ('NV12' default)");
  opt.addOption("--dma-in,-dma-in", [&]() {
    settings.bDMAIn = true;
    settings.eDMAIn = OMX_ALG_BUF_DMA;
  }, "use dmabufs for input port");
  opt.addOption("--dma-out,-dma-out", [&]() {
    settings.bDMAOut = true;
    settings.eDMAOut = OMX_ALG_BUF_DMA;
  }, "use dmabufs for output port");
  string prealloc_args = "";
  opt.addString("--prealloc-args", &prealloc_args, "Specify the stream dimension: 1920x1080:NV12:profile-idc:level");

  if(argc < 2)
  {
    Usage(opt, argv[0]);
    exit(1);
  }

  input_file = string(argv[1]);

  opt.parse(argc - 1, &argv[1]);

  if(help)
  {
    Usage(opt, argv[0]);
    exit(0);
  }

  bool isHevc = settings.codecImplem == HEVC;
  isHevc = isHevc || settings.codecImplem == HEVC_HARD;

  if(isHevc)
    settings.codec = HEVC;
  else
    settings.codec = AVC;

  if(!setChroma(user_chroma, &settings.chroma))
  {
    Usage(opt, argv[0]);
    cerr << "[Error] chroma parameter was incorrectly set" << endl;
    exit(1);
  }

  if(!prealloc_args.empty())
  {
    app.settings.hasPrealloc = true;
    parsePreAllocArgs(&app.settings, prealloc_args);
  }

  if(input_file == "")
  {
    Usage(opt, argv[0]);
    cerr << "[Error] No input file found" << endl;
    exit(1);
  }

  if(output_file == "")
    output_file = "output.nv12";
}

static bool isSupplier(OMX_U32 nPortIndex, Application& app)
{
  OMX_PARAM_BUFFERSUPPLIERTYPE sSupply;
  initHeader(sSupply);
  sSupply.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamCompBufferSupplier, &sSupply));

  if(
    ((sSupply.eBufferSupplier == OMX_BufferSupplyInput) &&
     (sSupply.nPortIndex == outportIndex)) ||
    ((sSupply.eBufferSupplier == OMX_BufferSupplyOutput) &&
     (sSupply.nPortIndex == inportIndex))
    )
    return true;

  return false;
};

static OMX_U32 getSizeBuffer(OMX_U32 nPortIndex, Application& app)
{
  OMX_PARAM_PORTDEFINITIONTYPE sPortParam;
  initHeader(sPortParam);
  sPortParam.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamPortDefinition, &sPortParam));

  return sPortParam.nBufferSize;
};

static OMX_U32 getMinBufferAlloc(OMX_U32 nPortIndex, Application& app)
{
  OMX_PARAM_PORTDEFINITIONTYPE sPortParam;
  initHeader(sPortParam);
  sPortParam.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamPortDefinition, &sPortParam));

  return sPortParam.nBufferCountActual;
};

static auto numberOfAllocatedInputBuffer = 0;
static auto numberOfAllocatedOutputBuffer = 0;

static OMX_ERRORTYPE allocBuffers(OMX_U32 nPortIndex, bool use_dmabuf, Application& app)
{
  auto sizeBuf = getSizeBuffer(nPortIndex, app);
  auto minBuf = getMinBufferAlloc(nPortIndex, app);

  if(isSupplier(nPortIndex, app))
  {
    // Allocate buffer
    LOGV("Component port (%u) is supplier", nPortIndex);

    for(decltype(minBuf)nbBuf = 0; nbBuf < minBuf; nbBuf++)
    {
      OMX_BUFFERHEADERTYPE* pBufHeader = nullptr;
      OMX_AllocateBuffer(app.hDecoder, &pBufHeader, nPortIndex, app.pAppData, sizeBuf);
      assert(pBufHeader);

      if(nPortIndex == inportIndex)
      {
        app.inputBuffers.push(pBufHeader);
        numberOfAllocatedInputBuffer++;
      }
      else
      {
        app.outputBuffers.push_back(pBufHeader);
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
        AL_HANDLE hBuf = AL_Allocator_Alloc(app.pAllocator, sizeBuf);

        if(!hBuf)
        {
          LOGE("Failed to allocate Buffer for dma");
          assert(0);
        }
        auto fd = AL_LinuxDmaAllocator_ExportToFd((AL_TLinuxDmaAllocator*)(app.pAllocator), hBuf);

        pBufData = (OMX_U8*)(uintptr_t)dup(fd);

        if((int)(uintptr_t)pBufData <= 0)
        {
          LOGE("Failed to ExportToFd %p", hBuf);
          assert(0);
        }
        AL_Allocator_Free(app.pAllocator, hBuf);
      }
      else
        pBufData = (OMX_U8*)calloc(sizeBuf, sizeof(OMX_U8));

      OMX_CALL(OMX_UseBuffer(app.hDecoder, &pBufHeader, nPortIndex, app.pAppData, sizeBuf, pBufData));

      if(nPortIndex == inportIndex)
      {
        app.inputBuffers.push(pBufHeader);
        numberOfAllocatedInputBuffer++;
      }
      else
      {
        app.outputBuffers.push_back(pBufHeader);
        numberOfAllocatedOutputBuffer++;
      }
    }
  }
  return OMX_ErrorNone;
};

static OMX_ERRORTYPE freeBuffers(OMX_U32 nPortIndex, Application& app)
{
  LOGV("Port (%u)", nPortIndex);

  if(isSupplier(nPortIndex, app))
  {
    // Free Component buffers
    if(nPortIndex == inportIndex)
    {
      while(numberOfAllocatedInputBuffer > 0)
      {
        auto pBuf = app.inputBuffers.pop();
        OMX_CALL(OMX_FreeBuffer(app.hDecoder, nPortIndex, pBuf));
        numberOfAllocatedInputBuffer--;
      }
    }
    else
    {
      for(auto pBuf : app.outputBuffers)
        OMX_CALL(OMX_FreeBuffer(app.hDecoder, nPortIndex, pBuf));
    }
  }
  else
  {
    if(nPortIndex == inportIndex)
    {
      while(numberOfAllocatedInputBuffer > 0)
      {
        // Free Client buffers
        auto pBufHeader = app.inputBuffers.pop();
        Buffer_FreeData((char*)pBufHeader->pBuffer, app.settings.bDMAIn);
        OMX_CALL(OMX_FreeBuffer(app.hDecoder, nPortIndex, pBufHeader));
        numberOfAllocatedInputBuffer--;
      }
    }
    else
    {
      for(auto pBuf : app.outputBuffers)
      {
        Buffer_FreeData((char*)pBuf->pBuffer, app.settings.bDMAOut);
        OMX_CALL(OMX_FreeBuffer(app.hDecoder, nPortIndex, pBuf));
      }
    }
  }
  return OMX_ErrorNone;
};

OMX_ERRORTYPE onComponentEvent(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 Data1, OMX_U32 Data2, OMX_PTR /*pEventData*/)
{
  auto app = static_cast<Application*>(pAppData);
  assert(hComponent == app->hDecoder);
  LOGI("Event from decoder: 0x%.8X", eEvent);

  if(eEvent == OMX_EventCmdComplete)
  {
    if(Data1 == OMX_CommandStateSet)
    {
      LOGI("State changed to %s", toStringCompState((OMX_STATETYPE)Data2));
      app->decoderEventState.notify();
    }
    else if(Data1 == OMX_CommandPortEnable)
    {
      LOGI("Received Port Enable Event");

      /* if the output port is enabled */
      if(Data2 == 1)
      {
        for(auto pBuf : app->outputBuffers)
          OMX_CALL(OMX_FillThisBuffer(hComponent, pBuf));
      }
    }
    else if(Data1 == OMX_CommandPortDisable)
    {
      LOGI("Received Port Disable Event");
    }
    else if(Data1 == OMX_CommandMarkBuffer)
      LOGI("Mark Buffer Event ");
    else if(Data1 == OMX_CommandFlush)
    {
      LOGI("Flush Port Event");
      app->flushEvent.notify();
    }
  }
  else if(eEvent == OMX_EventPortSettingsChanged)
  {
    LOGI("Port settings change");
    initHeader(paramPort);
    paramPort.nPortIndex = 1;
    OMX_CALL(OMX_GetParameter(hComponent, OMX_IndexParamPortDefinition, &paramPort));
    paramPort.nBufferCountActual++;
    OMX_CALL(OMX_SetParameter(hComponent, OMX_IndexParamPortDefinition, &paramPort));

    OMX_SendCommand(app->hDecoder, OMX_CommandPortEnable, 1, nullptr);
    allocBuffers(outportIndex, app->settings.bDMAOut, *app);
  }
  else if(eEvent == OMX_EventError)
  {
    LOGI("Event error: 0x%.8X", Data1);
    cout << "The component failed with an unknown error" << endl;
    exit(1);
  }
  else if(eEvent == OMX_EventBufferFlag)
    LOGI("Event EOS");
  else
    LOGI("Param1 is %u, Param2 is %u", Data1, Data2);
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE setPortParameters(Application& app)
{
  // This should always be done at the beginning
  OMX_VIDEO_PARAM_PORTFORMATTYPE outParamFormat;
  initHeader(outParamFormat);
  outParamFormat.nPortIndex = 1;
  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamVideoPortFormat, &outParamFormat));

  outParamFormat.eColorFormat = app.settings.chroma;

  OMX_CALL(OMX_SetParameter(app.hDecoder, OMX_IndexParamVideoPortFormat, &outParamFormat));

  Setters setter(&app.hDecoder);
  auto isBufModeSetted = setter.SetBufferMode(inportIndex, app.settings.eDMAIn);
  assert(isBufModeSetted);
  isBufModeSetted = setter.SetBufferMode(outportIndex, app.settings.eDMAOut);
  assert(isBufModeSetted);

  return OMX_ErrorNone;
};

OMX_ERRORTYPE onInputBufferAvailable(OMX_HANDLETYPE /*hComponent*/, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
  auto app = static_cast<Application*>(pAppData);
  LOGV("InputBufferAvailable");
  assert(pBuffer->nFilledLen == 0);
  app->inputBuffers.push(pBuffer);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE onOutputBufferAvailable(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
  static bool end = false;
  auto app = static_cast<Application*>(pAppData);

  if(end)
    return OMX_ErrorNone;

  if(!pBuffer)
    return OMX_ErrorBadParameter;

  if(pBuffer->nFilledLen)
  {
    auto const data = Buffer_MapData((char*)(pBuffer->pBuffer + pBuffer->nOffset), pBuffer->nAllocLen, app->settings.bDMAOut);

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

    Buffer_UnmapData(data, pBuffer->nAllocLen, app->settings.bDMAOut);
  }

  pBuffer->nFilledLen = 0;
  bool wasEos = pBuffer->nFlags == OMX_BUFFERFLAG_EOS;

  OMX_CALL(OMX_FillThisBuffer(hComponent, pBuffer));

  if(wasEos)
  {
    end = true;
    app->eofSem.notify();
  }

  return OMX_ErrorNone;
}

static bool readFrame(OMX_BUFFERHEADERTYPE* pInputBuf, Application& app)
{
  assert(pInputBuf->nAllocLen != 0);
  vector<uint8_t> frame(pInputBuf->nAllocLen);

  infile.read((char*)frame.data(), frame.size());
  size_t zMapSize = pInputBuf->nAllocLen;
  auto data = Buffer_MapData((char*)(pInputBuf->pBuffer + pInputBuf->nOffset), zMapSize, app.settings.bDMAIn);
  memcpy(data, frame.data(), frame.size());
  Buffer_UnmapData(data, pInputBuf->nAllocLen, app.settings.bDMAIn);

  pInputBuf->nFilledLen = infile.gcount();

  if(infile.peek() == EOF)
    return true;

  return false;
};

string chooseComponent(DecCodec codecImplem)
{
  switch(codecImplem)
  {
  case AVC:
    return "OMX.allegro.h264.decoder";
  case AVC_HARD:
    return "OMX.allegro.h264.hardware.decoder";
  case HEVC:
    return "OMX.allegro.h265.decoder";
  case HEVC_HARD:
    return "OMX.allegro.h265.hardware.decoder";
  default:
    assert(0);
  }
}

OMX_ERRORTYPE setProfileAndLevel(Application& app)
{
  OMX_VIDEO_PARAM_PROFILELEVELTYPE param;

  initHeader(param);
  param.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamVideoProfileLevelCurrent, &param));

  param.eProfile = app.settings.profile;
  param.eLevel = app.settings.level;
  // param.eProfile = OMX_VIDEO_HEVCProfileMain;
  // param.eLevel = OMX_VIDEO_HEVCMainTierLevel51;
  OMX_CALL(OMX_SetParameter(app.hDecoder, OMX_IndexParamVideoProfileLevelCurrent, &param));

  return OMX_ErrorNone;
}

static AL_INLINE int RoundUp(int iVal, int iRnd)
{
  return (iVal + iRnd - 1) & (~(iRnd - 1));
}

OMX_ERRORTYPE setDimensions(Application& app)
{
  OMX_PARAM_PORTDEFINITIONTYPE param;

  initHeader(param);
  param.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamPortDefinition, &param));

  param.format.video.nFrameWidth = app.settings.width;
  param.format.video.nFrameHeight = app.settings.height;
  // TODO 10 bits special case
  param.format.video.nStride = RoundUp(app.settings.width, 32);
  param.format.video.nSliceHeight = RoundUp(app.settings.height, 32);
  OMX_CALL(OMX_SetParameter(app.hDecoder, OMX_IndexParamPortDefinition, &param));

  return OMX_ErrorNone;
}

OMX_ERRORTYPE setFormat(Application& app)
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE param;

  initHeader(param);
  param.nPortIndex = 0;
  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamVideoPortFormat, &param));

  param.eColorFormat = app.settings.chroma;
  param.xFramerate = app.settings.framerate;
  OMX_CALL(OMX_SetParameter(app.hDecoder, OMX_IndexParamVideoPortFormat, &param));

  return OMX_ErrorNone;
}

OMX_ERRORTYPE setPreallocParameters(Application& app)
{
  OMX_ERRORTYPE error = setProfileAndLevel(app);

  if(error != OMX_ErrorNone)
    return error;

  error = setDimensions(app);

  if(error != OMX_ErrorNone)
    return error;

  error = setFormat(app);

  if(error != OMX_ErrorNone)
    return error;

  return OMX_ErrorNone;
}

OMX_ERRORTYPE setWorstCaseParameters(Application& app)
{
  Settings& settings = app.settings;
  settings.width = 7680;
  settings.height = 4320;

  if(settings.codec == HEVC)
  {
    settings.profile = OMX_VIDEO_HEVCProfileMain10;
    settings.level = OMX_VIDEO_HEVCMainTierLevel51;
  }
  else
  {
    settings.profile = OMX_VIDEO_AVCProfileHigh422;
    settings.level = OMX_VIDEO_AVCLevel51;
  }

  settings.framerate = 1 << 16;
  settings.chroma = OMX_COLOR_FormatYUV422SemiPlanar;

  return setPreallocParameters(app);
}

OMX_ERRORTYPE safeMain(int argc, char** argv)
{
  Application app;
  parseCommandLine(argc, argv, app);

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

  auto component = chooseComponent(app.settings.codecImplem);

  OMX_CALLBACKTYPE const videoDecoderCallbacks =
  {
    .EventHandler = onComponentEvent,
    .EmptyBufferDone = onInputBufferAvailable,
    .FillBufferDone = onOutputBufferAvailable
  };

  OMX_CALL(OMX_GetHandle(&app.hDecoder, (OMX_STRING)component.c_str(), &app, const_cast<OMX_CALLBACKTYPE*>(&videoDecoderCallbacks)));

  OMX_STRING name = (OMX_STRING)calloc(128, sizeof(char));
  OMX_VERSIONTYPE compType;
  OMX_VERSIONTYPE ilType;

  OMX_CALL(OMX_GetComponentVersion(app.hDecoder, (OMX_STRING)name, &compType, &ilType, nullptr));

  LOGI("Component : %s (v.%u) made for OMX_IL client : %u.%u.%u", name, compType.nVersion, ilType.s.nVersionMajor, ilType.s.nVersionMinor, ilType.s.nRevision);

  free(name);

  auto const ret = setPortParameters(app);

  if(ret != OMX_ErrorNone)
    return ret;

  app.pAllocator = nullptr;

  if(app.settings.bDMAIn || app.settings.bDMAOut)
  {
    auto constexpr deviceName = "/dev/allegroDecodeIP";
    app.pAllocator = DmaAlloc_Create(deviceName);

    if(!app.pAllocator)
      throw runtime_error(string("Couldn't create dma allocator (using ") + deviceName + string(")"));
  }

  auto scopeAlloc = scopeExit([&]() {
    if(app.pAllocator)
      AL_Allocator_Destroy(app.pAllocator);
  });

  auto outputPortDisabled = false;

  if(app.settings.hasPrealloc)
  {
    auto const error = setPreallocParameters(app);

    if(error != OMX_ErrorNone)
      return error;
  }
  else
  {
    auto const error = setWorstCaseParameters(app);

    if(error != OMX_ErrorNone)
      return error;

    OMX_SendCommand(app.hDecoder, OMX_CommandPortDisable, 1, nullptr);
    outputPortDisabled = true;
  }

  initHeader(paramPort);
  paramPort.nPortIndex = 1;
  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamPortDefinition, &paramPort));
  paramPort.nBufferCountActual = paramPort.nBufferCountMin + 1;
  OMX_CALL(OMX_SetParameter(app.hDecoder, OMX_IndexParamPortDefinition, &paramPort));

  /* /!\ Can't set parameters after this line /!\  */

  /** sending command to video decoder component to go to idle state */
  OMX_SendCommand(app.hDecoder, OMX_CommandStateSet, OMX_StateIdle, nullptr);

  allocBuffers(inportIndex, app.settings.bDMAIn, app);

  if(!outputPortDisabled)
    allocBuffers(outportIndex, app.settings.bDMAOut, app);

  app.decoderEventState.wait();

  /** sending command to video decoder component to go to executing state */
  OMX_CALL(OMX_SendCommand(app.hDecoder, OMX_CommandStateSet, OMX_StateExecuting, nullptr));
  app.decoderEventState.wait();

  if(!outputPortDisabled)
  {
    for(auto pBuf : app.outputBuffers)
      OMX_CALL(OMX_FillThisBuffer(app.hDecoder, pBuf));
  }

  /** Process **/
  auto eof = false;

  while(!eof)
  {
    auto inputBuffer = app.inputBuffers.pop();
    eof = readFrame(inputBuffer, app);

    OMX_CALL(OMX_EmptyThisBuffer(app.hDecoder, inputBuffer));

    if(eof)
    {
      LOGV("End of file");
      auto emptyBuffer = app.inputBuffers.pop();
      emptyBuffer->nFilledLen = 0;
      inputBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
      OMX_CALL(OMX_EmptyThisBuffer(app.hDecoder, emptyBuffer));
    }
  }

  LOGV("Waiting for EOS ... ");
  app.eofSem.wait();
  cerr.flush();
  LOGV("EOS received");

  /** state change of all components from executing to idle */
  OMX_CALL(OMX_SendCommand(app.hDecoder, OMX_CommandFlush, 0, nullptr));
  app.flushEvent.wait();

  OMX_CALL(OMX_SendCommand(app.hDecoder, OMX_CommandFlush, 1, nullptr));
  app.flushEvent.wait();

  OMX_CALL(OMX_SendCommand(app.hDecoder, OMX_CommandStateSet, OMX_StateIdle, nullptr));
  app.decoderEventState.wait();

  /** sending command to all components to go to loaded state */
  OMX_CALL(OMX_SendCommand(app.hDecoder, OMX_CommandStateSet, OMX_StateLoaded, nullptr));

  freeBuffers(inportIndex, app);
  freeBuffers(outportIndex, app);

  app.decoderEventState.wait();

  OMX_FreeHandle(app.hDecoder);
  OMX_Deinit();

  infile.close();
  outfile.close();
  return OMX_ErrorNone;
}

int main(int argc, char** argv)
{
  OMX_ERRORTYPE ret;

  try
  {
    ret = safeMain(argc, argv);

    if(ret == OMX_ErrorNone)
      return 0;
    else
    {
      cerr << "Fatal error" << endl;
      return 1;
    }
  }
  catch(runtime_error const& error)
  {
    cerr << endl << "Exception caught: " << error.what() << endl;
    return 1;
  }
}

