/******************************************************************************
*
* Copyright (C) 2015-2022 Allegro DVT2
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
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
#include <vector>
#include <map>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>
#include <OMX_Video.h>
#include <OMX_VideoExt.h>
#include <OMX_ComponentExt.h>
#include <OMX_IndexAlg.h>
#include <OMX_IVCommonAlg.h>

#include <utility/logger.h>
#include <utility/locked_queue.h>
#include <utility/semaphore.h>
#include <utility/omx_translate.h>

#include "../common/helpers.h"
#include "../common/CommandLineParser.h"
#include "../common/codec.h"

extern "C"
{
#include "lib_fpga/DmaAlloc.h"
#include "lib_fpga/DmaAllocLinux.h"
}

using namespace std;

using EventType = int;

struct EventData
{
};

struct Event
{
  EventType type;
  std::shared_ptr<EventData> data;
};

struct EventBus
{
  void addListener(std::function<void(std::shared_ptr<EventData> data)> delegate, EventType type)
  {
    eventListeners[type].push_back(delegate);
  }

  void queueEvent(Event event)
  {
    lock_guard<mutex> guard(lock);
    eventQueue.push_back(event);
    cv.notify_one();
  }

  void wait(void)
  {
    unique_lock<mutex> guard(lock);
    cv.wait(guard, [&]() {
      return !eventQueue.empty();
    });
  }

  void tick(void)
  {
    lock_guard<mutex> guard(lock);
    auto event = eventQueue.begin();

    while(event != eventQueue.end())
    {
      for(auto delegate : eventListeners[event->type])
        delegate(event->data);

      event = eventQueue.erase(event);
    }
  }

  void waitAndProcess(void)
  {
    wait();
    tick();
  }

private:
  map<EventType, std::vector<std::function<void(std::shared_ptr<EventData> )>>> eventListeners {};
  deque<Event> eventQueue {};
  mutex lock {};
  condition_variable cv {};
};

EventType constexpr eosEvent = 1;
EventType constexpr errorEvent = 2;
EventType constexpr omxEvent = 3;
EventType constexpr pipelineEndEvent = 4;

struct OmxEventData : EventData
{
  OMX_HANDLETYPE hComponent;
  OMX_PTR pAppData;
  OMX_EVENTTYPE eEvent;
  OMX_U32 Data1;
  OMX_U32 Data2;
  OMX_PTR pEventData;
  OmxEventData(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 Data1, OMX_U32 Data2, OMX_PTR pEventData) :
    hComponent{hComponent},
    pAppData{pAppData},
    eEvent{eEvent},
    Data1{Data1},
    Data2{Data2},
    pEventData{pEventData}
  {
  }
};

struct ErrorEventData : EventData
{
  int errorCode;
  ErrorEventData(int errorCode) :
    errorCode{errorCode}
  {
  }
};

static OMX_U32 inportIndex = 0;
static OMX_U32 outportIndex = 1;

struct Settings
{
  Codec codecImplem = Codec::HEVC;
  Codec codec = Codec::HEVC;
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
  OMX_ALG_SEQUENCE_PICTURE_MODE sequencePicture = OMX_ALG_SEQUENCE_PICTURE_FRAME;
  bool hasPrealloc = false;
  bool enableSubframe = false;
};

struct Application
{
  semaphore decoderEventState;
  semaphore flushEvent;
  semaphore disableEvent;

  OMX_HANDLETYPE hDecoder {
    nullptr
  };
  OMX_PTR pAppData {
    nullptr
  };

  Settings settings;
  locked_queue<OMX_BUFFERHEADERTYPE*> inputBuffers;
  list<OMX_BUFFERHEADERTYPE*> outputBuffers;
  AL_TAllocator* pAllocator;
  EventBus eventBus {};
  bool quit = false;
  bool pipelineEnded = false;
};

static string input_file;
static string output_file;
static ifstream infile;
static ofstream outfile;

static OMX_PARAM_PORTDEFINITIONTYPE paramPort;

static void Usage(CommandLineParser& opt, char* ExeName)
{
  cerr << "Usage: " << ExeName << " <InputFile> [options]" << endl;
  cerr << "Options:" << endl;

  for(auto& command: opt.displayOrder)
    cerr << "  " << opt.descs[command] << endl;
}

bool setChroma(string user_chroma, OMX_COLOR_FORMATTYPE* chroma)
{
  if(user_chroma == "y800")
    *chroma = OMX_COLOR_FormatL8;
  else if(user_chroma == "xv10")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatL10bitPacked);
  else if(user_chroma == "nv12")
    *chroma = OMX_COLOR_FormatYUV420SemiPlanar;
  else if(user_chroma == "nv16")
    *chroma = OMX_COLOR_FormatYUV422SemiPlanar;
  else if(user_chroma == "xv15")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV420SemiPlanar10bitPacked);
  else if(user_chroma == "xv20")
    *chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked);
  else
    return false;
  return true;
}

void getExpectedSeparator(stringstream& ss, char expectedSep)
{
  char sep;
  ss >> sep;

  if(sep != expectedSep)
    throw runtime_error("wrong separator in prealloc separator");
}

bool setSequence(string user_seq, OMX_ALG_SEQUENCE_PICTURE_MODE* seqpicture)
{
  if(user_seq == "unkwn")
    *seqpicture = OMX_ALG_SEQUENCE_PICTURE_UNKNOWN;
  else if(user_seq == "progr")
    *seqpicture = OMX_ALG_SEQUENCE_PICTURE_FRAME;
  else if(user_seq == "inter")
    *seqpicture = OMX_ALG_SEQUENCE_PICTURE_FIELD;
  else
    return false;
  return true;
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
  ss >> settings->width;
  getExpectedSeparator(ss, 'x');
  ss >> settings->height;
  getExpectedSeparator(ss, ':');
  char seq[6] {};
  ss >> seq[0];
  ss >> seq[1];
  ss >> seq[2];
  ss >> seq[3];
  ss >> seq[4];
  seq[5] = '\0';
  getExpectedSeparator(ss, ':');
  char chroma[5] {};
  ss >> chroma[0];
  ss >> chroma[1];
  ss >> chroma[2];
  ss >> chroma[3];
  chroma[4] = '\0';
  getExpectedSeparator(ss, ':');
  ss >> settings->profile;
  getExpectedSeparator(ss, ':');
  ss >> settings->level;

  if(!setChroma(chroma, &settings->chroma))
    throw runtime_error("wrong prealloc chroma format");

  if(!setSequence(seq, &settings->sequencePicture))
    throw runtime_error("wrong prealloc sequence picture format");

  if(ss.fail() || ss.tellg() != streampos(-1))
    throw runtime_error("wrong prealloc arguments format");

  if(invalidPreallocSettings(*settings))
    throw runtime_error("wrong prealloc arguments");
}

static bool print_sei = false;
void parseCommandLine(int argc, char** argv, Application& app)
{
  Settings& settings = app.settings;

  auto opt = CommandLineParser();
  bool help = false;
  opt.addString("input_file", &input_file, "Input file");
  opt.addFlag("--help", &help, "Show this help");
  opt.addFlag("--hevc,-hevc", &settings.codecImplem, "load HEVC decoder (default)", Codec::HEVC);
  opt.addFlag("--avc,-avc", &settings.codecImplem, "load AVC decoder", Codec::AVC);

  opt.addFlag("--hevc-hard,-hevc-hard", &settings.codecImplem, "Use hard hevc decoder", Codec::HEVC_HARD);
  opt.addFlag("--avc-hard,-avc-hard", &settings.codecImplem, "Use hard avc decoder", Codec::AVC_HARD);
  opt.addString("--out,-o", &output_file, "Output compressed file name");
  opt.addOption("--dma-in,-dma-in", [&](string) {
    settings.bDMAIn = true;
    settings.eDMAIn = OMX_ALG_BUF_DMA;
  }, "use dmabufs for input port");
  opt.addOption("--dma-out,-dma-out", [&](string) {
    settings.bDMAOut = true;
    settings.eDMAOut = OMX_ALG_BUF_DMA;
  }, "use dmabufs for output port");
  string prealloc_args = "";
  opt.addString("--prealloc-args", &prealloc_args, "Specify the stream dimension: 1920x1080:unkwn:nv12:omx-profile-value:omx-level-value");
  opt.addFlag("--subframe", &settings.enableSubframe, "Use the subframe latency mode");
  opt.addFlag("--print-sei", &print_sei, "Print SEI on stdout");

  if(argc < 2)
  {
    Usage(opt, argv[0]);
    exit(1);
  }

  opt.parse(argc, argv);

  if(help)
  {
    Usage(opt, argv[0]);
    exit(0);
  }

  Codec codec = Codec::HEVC;

  if(settings.codecImplem == Codec::AVC)
    codec = Codec::AVC;

  if(settings.codecImplem == Codec::AVC_HARD)
    codec = Codec::AVC;

  settings.codec = codec;

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
  InitHeader(sSupply);
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
}

static OMX_U32 getSizeBuffer(OMX_U32 nPortIndex, Application& app)
{
  OMX_PARAM_PORTDEFINITIONTYPE sPortParam;
  InitHeader(sPortParam);
  sPortParam.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamPortDefinition, &sPortParam));

  return sPortParam.nBufferSize;
}

static OMX_U32 getMinBufferAlloc(OMX_U32 nPortIndex, Application& app)
{
  OMX_PARAM_PORTDEFINITIONTYPE sPortParam;
  InitHeader(sPortParam);
  sPortParam.nPortIndex = nPortIndex;

  OMX_CALL(OMX_GetParameter(app.hDecoder, OMX_IndexParamPortDefinition, &sPortParam));

  return sPortParam.nBufferCountActual;
}

static auto numberOfAllocatedInputBuffer = 0;
static auto numberOfAllocatedOutputBuffer = 0;

static OMX_ERRORTYPE allocBuffers(OMX_U32 nPortIndex, bool use_dmabuf, Application& app)
{
  auto sizeBuf = getSizeBuffer(nPortIndex, app);
  auto minBuf = getMinBufferAlloc(nPortIndex, app);

  if(isSupplier(nPortIndex, app))
  {
    // Allocate buffer
    LOG_VERBOSE(string { "Component port (" } +to_string(nPortIndex) + string { " is supplier" });

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
    LOG_VERBOSE(string { "Component port (" } +to_string(nPortIndex) + string { " is not supplier" });

    for(decltype(minBuf)nbBuf = 0; nbBuf < minBuf; nbBuf++)
    {
      OMX_BUFFERHEADERTYPE* pBufHeader {
        nullptr
      };
      OMX_U8* pBufData {
        nullptr
      };

      if(use_dmabuf)
      {
        AL_HANDLE hBuf = AL_Allocator_Alloc(app.pAllocator, sizeBuf);

        if(!hBuf)
        {
          LOG_ERROR("Failed to allocate Buffer for dma");
          assert(0);
        }
        auto fd = AL_LinuxDmaAllocator_GetFd((AL_TLinuxDmaAllocator*)(app.pAllocator), hBuf);

        pBufData = (OMX_U8*)(uintptr_t)dup(fd);

        if((int)(uintptr_t)pBufData <= 0)
        {
          LOG_ERROR(string { "Failed to ExportToFd: " } +ToStringAddr(hBuf));
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
}

static OMX_ERRORTYPE freeBuffers(OMX_U32 nPortIndex, Application& app)
{
  LOG_VERBOSE(string { "Port: " } +to_string(nPortIndex));

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
}

OMX_ERRORTYPE handleEvent(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 Data1, OMX_U32 Data2, OMX_PTR pEventData)
{
  (void)pEventData;
  auto app = static_cast<Application*>(pAppData);
  assert(hComponent == app->hDecoder);
  LOG_IMPORTANT(string { "Event from decoder: " } +ToStringOMXEvent(eEvent));

  if(eEvent == OMX_EventCmdComplete)
  {
    LOG_IMPORTANT(string { "Command: " } +ToStringOMXCommand((OMX_COMMANDTYPE)Data1));

    if(Data1 == OMX_CommandStateSet)
    {
      app->decoderEventState.notify();
    }
    else if(Data1 == OMX_CommandPortEnable)
    {
      /* if the output port is enabled */
      if(Data2 == 1)
      {
        for(auto pBuf : app->outputBuffers)
          OMX_CALL(OMX_FillThisBuffer(hComponent, pBuf));
      }
    }
    else if(Data1 == OMX_CommandPortDisable)
      app->disableEvent.notify();
    else if(Data1 == OMX_CommandFlush)
      app->flushEvent.notify();
  }
  else if(eEvent == OMX_EventPortSettingsChanged)
  {
    if(!app->settings.hasPrealloc)
    {
      LOG_IMPORTANT("Port settings change");
      auto increaseActual = [](OMX_PARAM_PORTDEFINITIONTYPE& param)
                            {
                              param.nBufferCountActual++;
                            };
      OMX_CALL(PortSetup<OMX_PARAM_PORTDEFINITIONTYPE>(hComponent, OMX_IndexParamPortDefinition, increaseActual, 1));
      OMX_SendCommand(app->hDecoder, OMX_CommandPortEnable, 1, nullptr);
      allocBuffers(outportIndex, app->settings.bDMAOut, *app);
    }
  }
  else if(eEvent == OMX_EventError)
  {
    auto cmd = static_cast<OMX_ERRORTYPE>(Data1);
    LOG_ERROR(string { "Component (" } +ToStringAddr(hComponent) + string { "): " } +ToStringOMXEvent(eEvent) + string { "(" } +ToStringOMXError(cmd) + string { ")" });
    exit(1);
  }
  return OMX_ErrorNone;
}

static bool isEventPrefixSei(OMX_EVENTTYPE eEvent)
{
  return eEvent == static_cast<OMX_EVENTTYPE>(OMX_ALG_EventSEIPrefixParsed);
}

static bool isEventSuffixSei(OMX_EVENTTYPE eEvent)
{
  return eEvent == static_cast<OMX_EVENTTYPE>(OMX_ALG_EventSEISuffixParsed);
}

OMX_ERRORTYPE onComponentEvent(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 Data1, OMX_U32 Data2, OMX_PTR pEventData)
{
  if(isEventPrefixSei(eEvent) || isEventSuffixSei(eEvent))
  {
    if(!print_sei)
      return OMX_ErrorNone;
    fprintf(stdout, "%s\n", __func__);
    fprintf(stdout, "%s\n", isEventPrefixSei(eEvent) ? "sei-prefix" : "sei-suffix");
    fprintf(stdout, "sei-type:%i\nsei-size:%i\n", Data1, Data2);

    for(auto i = 0; i < static_cast<int>(Data2); i++)
      fprintf(stdout, "0x%.2X ", static_cast<OMX_U8*>(pEventData)[i]);

    fprintf(stdout, "\n");
    return OMX_ErrorNone;
  }
  auto app = static_cast<Application*>(pAppData);
  auto data = make_shared<OmxEventData>(hComponent, pAppData, eEvent, Data1, Data2, pEventData);
  app->eventBus.queueEvent({ omxEvent, data });
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE setPortParameters(Application& app)
{
  // This should always be done at the beginning
  auto updateColorFormat = [&](OMX_VIDEO_PARAM_PORTFORMATTYPE& format)
                           {
                             format.eColorFormat = app.settings.chroma;
                           };
  OMX_CALL(PortSetup<OMX_VIDEO_PARAM_PORTFORMATTYPE>(app.hDecoder, OMX_IndexParamVideoPortFormat, updateColorFormat, 1));

  auto updateBufferMode = [&](OMX_ALG_PORT_PARAM_BUFFER_MODE& mode)
                          {
                            mode.eMode = (mode.nPortIndex == inportIndex) ? app.settings.eDMAIn : app.settings.eDMAOut;
                          };
  OMX_CALL(PortSetup<OMX_ALG_PORT_PARAM_BUFFER_MODE>(app.hDecoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexPortParamBufferMode), updateBufferMode, 0));
  OMX_CALL(PortSetup<OMX_ALG_PORT_PARAM_BUFFER_MODE>(app.hDecoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexPortParamBufferMode), updateBufferMode, 1));
  return OMX_ErrorNone;
}

OMX_ERRORTYPE onInputBufferAvailable(OMX_HANDLETYPE /*hComponent*/, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
  auto app = static_cast<Application*>(pAppData);
  LOG_VERBOSE("One input buffer is available");
  assert(pBuffer->nFilledLen == 0);
  app->inputBuffers.push(pBuffer);
  return OMX_ErrorNone;
}

OMX_ERRORTYPE onOutputBufferAvailable(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
  static bool end = false;
  auto app = static_cast<Application*>(pAppData);

  LOG_VERBOSE("One output buffer is available");

  if(end)
    return OMX_ErrorNone;

  if(!pBuffer)
    return OMX_ErrorBadParameter;

  if(pBuffer->nFilledLen)
  {
    auto data = Buffer_MapData((char*)(pBuffer->pBuffer), pBuffer->nOffset, pBuffer->nAllocLen, app->settings.bDMAOut);

    if(data)
    {
      OMX_PARAM_PORTDEFINITIONTYPE param;

      InitHeader(param);
      param.nPortIndex = 1;
      OMX_CALL(OMX_GetParameter(app->hDecoder, OMX_IndexParamPortDefinition, &param));
      auto videoDef = param.format.video;
      auto stride = videoDef.nStride;
      auto sliceHeight = videoDef.nSliceHeight;
      auto coef = is422(videoDef.eColorFormat) ? 1 : 2;
      auto height = videoDef.nFrameHeight;
      auto row_size = is10bits(videoDef.eColorFormat) ? (((videoDef.nFrameWidth + 2) / 3) * 4) : videoDef.nFrameWidth;

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
    app->eventBus.queueEvent({ eosEvent, nullptr });
  }

  return OMX_ErrorNone;
}

static bool readFrame(OMX_BUFFERHEADERTYPE* pInputBuf, Application& app)
{
  assert(pInputBuf->nAllocLen != 0);
  vector<uint8_t> frame(pInputBuf->nAllocLen);

  infile.read((char*)frame.data(), frame.size());
  size_t zMapSize = pInputBuf->nAllocLen;
  auto data = Buffer_MapData((char*)(pInputBuf->pBuffer), pInputBuf->nOffset, zMapSize, app.settings.bDMAIn);
  memcpy(data, frame.data(), frame.size());
  Buffer_UnmapData(data, pInputBuf->nAllocLen, app.settings.bDMAIn);

  pInputBuf->nFilledLen = infile.gcount();

  if(infile.peek() == EOF)
    return true;

  return false;
}

string chooseComponent(Codec codecImplem)
{
  switch(codecImplem)
  {
  case Codec::AVC:
    return "OMX.allegro.h264.decoder";
  case Codec::AVC_HARD:
    return "OMX.allegro.h264.hardware.decoder";
  case Codec::HEVC:
    return "OMX.allegro.h265.decoder";
  case Codec::HEVC_HARD:
    return "OMX.allegro.h265.hardware.decoder";
  default:
    assert(0);
  }
}

OMX_ERRORTYPE setProfileAndLevel(Application& app)
{
  auto updateProfileLevel = [&](OMX_VIDEO_PARAM_PROFILELEVELTYPE& pf)
                            {
                              pf.eProfile = app.settings.profile;
                              pf.eLevel = app.settings.level;
                            };
  OMX_CALL(PortSetup<OMX_VIDEO_PARAM_PROFILELEVELTYPE>(app.hDecoder, OMX_IndexParamVideoProfileLevelCurrent, updateProfileLevel, 0));
  return OMX_ErrorNone;
}

OMX_ERRORTYPE setDimensions(Application& app)
{
  auto updateDimension = [&](OMX_PARAM_PORTDEFINITIONTYPE& dimension)
                         {
                           dimension.format.video.nFrameWidth = app.settings.width;
                           dimension.format.video.nFrameHeight = app.settings.height;
                           dimension.format.video.nStride = app.settings.width;
                           dimension.format.video.nSliceHeight = app.settings.height;
                         };
  OMX_CALL(PortSetup<OMX_PARAM_PORTDEFINITIONTYPE>(app.hDecoder, OMX_IndexParamPortDefinition, updateDimension, 0));
  return OMX_ErrorNone;
}

OMX_ERRORTYPE setFormat(Application& app)
{
  auto updateFormat = [&](OMX_VIDEO_PARAM_PORTFORMATTYPE& format)
                      {
                        format.eColorFormat = app.settings.chroma;
                        format.xFramerate = app.settings.framerate;
                      };
  OMX_CALL(PortSetup<OMX_VIDEO_PARAM_PORTFORMATTYPE>(app.hDecoder, OMX_IndexParamVideoPortFormat, updateFormat, 0));
  return OMX_ErrorNone;
}

OMX_ERRORTYPE setSequencePicture(Application& app)
{
  auto updateSequencePicture = [&](OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE& spm)
                               {
                                 spm.eMode = app.settings.sequencePicture;
                               };
  OMX_CALL(PortSetup<OMX_ALG_COMMON_PARAM_SEQUENCE_PICTURE_MODE>(app.hDecoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamCommonSequencePictureModeCurrent), updateSequencePicture, 0));
  return OMX_ErrorNone;
}

OMX_ERRORTYPE setPreallocParameters(Application& app)
{
  OMX_ERRORTYPE error = OMX_ErrorNone;

  {
    error = setProfileAndLevel(app);
  }

  if(error != OMX_ErrorNone)
    return error;

  error = setDimensions(app);

  if(error != OMX_ErrorNone)
    return error;

  error = setFormat(app);

  if(error != OMX_ErrorNone)
    return error;

  error = setSequencePicture(app);

  if(error != OMX_ErrorNone)
    return error;

  return OMX_ErrorNone;
}

OMX_ERRORTYPE setWorstCaseParameters(Application& app)
{
  Settings& settings = app.settings;
  settings.width = 3840;
  settings.height = 2160;

  if(settings.codec == Codec::HEVC)
  {
    settings.profile = static_cast<OMX_VIDEO_HEVCPROFILETYPE>(OMX_ALG_VIDEO_HEVCProfileMain422_10);
    settings.level = static_cast<OMX_VIDEO_HEVCLEVELTYPE>(OMX_ALG_VIDEO_HEVCMainTierLevel62);
  }
  else
  {
    settings.profile = OMX_VIDEO_AVCProfileHigh422;
    settings.level = static_cast<OMX_VIDEO_AVCLEVELTYPE>(OMX_ALG_VIDEO_AVCLevel60);
  }

  settings.framerate = 1 << 16;
  settings.chroma = static_cast<OMX_COLOR_FORMATTYPE>(OMX_ALG_COLOR_FormatYUV422SemiPlanar10bitPacked);
  settings.sequencePicture = OMX_ALG_SEQUENCE_PICTURE_UNKNOWN;

  return setPreallocParameters(app);
}

OMX_ERRORTYPE stopPipeline(Application& app)
{
  /** state change of all components from executing to idle */
  OMX_CALL(OMX_SendCommand(app.hDecoder, OMX_CommandFlush, 0, nullptr));
  app.flushEvent.wait();
  OMX_CALL(OMX_SendCommand(app.hDecoder, OMX_CommandFlush, 1, nullptr));
  app.flushEvent.wait();

  OMX_CALL(OMX_SendCommand(app.hDecoder, OMX_CommandStateSet, OMX_StateIdle, nullptr));
  app.decoderEventState.wait();

  return OMX_ErrorNone;
}

static OMX_ERRORTYPE disablePrealloc(Application& app)
{
  OMX_ALG_PARAM_PREALLOCATION param;
  InitHeader(param);
  param.bDisablePreallocation = OMX_TRUE;
  return OMX_SetParameter(app.hDecoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamPreallocation), &param);
}

static OMX_ERRORTYPE disableInputParsed(Application& app)
{
  OMX_ALG_VIDEO_PARAM_INPUT_PARSED param {};
  InitHeader(param);
  param.bDisableInputParsed = OMX_TRUE;
  return OMX_SetParameter(app.hDecoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoInputParsed), &param);
}

static OMX_ERRORTYPE configureComponent(Application& app)
{
  auto outputPortDisabled = false;

  auto disabledError = disableInputParsed(app);

  if(disabledError != OMX_ErrorNone)
    return disabledError;

  if(app.settings.enableSubframe)
  {
    OMX_ALG_VIDEO_PARAM_SUBFRAME param;
    InitHeader(param);
    param.nPortIndex = 1;
    param.bEnableSubframe = OMX_TRUE;
    OMX_SetParameter(app.hDecoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoSubframe), &param);
    OMX_ALG_VIDEO_PARAM_DECODED_PICTURE_BUFFER dpb;
    InitHeader(dpb);
    dpb.nPortIndex = 1;
    dpb.eDecodedPictureBufferMode = OMX_ALG_DPB_NO_REORDERING;
    OMX_SetParameter(app.hDecoder, static_cast<OMX_INDEXTYPE>(OMX_ALG_IndexParamVideoDecodedPictureBuffer), &dpb);
  }

  if(app.settings.hasPrealloc)
  {
    auto error = setPreallocParameters(app);

    if(error != OMX_ErrorNone)
      return error;
  }
  else
  {
    auto error = disablePrealloc(app);

    if(error != OMX_ErrorNone)
      return error;

    error = setWorstCaseParameters(app);

    if(error != OMX_ErrorNone)
      return error;

    OMX_SendCommand(app.hDecoder, OMX_CommandPortDisable, 1, nullptr);
    app.disableEvent.wait();
    outputPortDisabled = true;
  }

  InitHeader(paramPort);
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

  return OMX_ErrorNone;
}

static void omxWorker(Application* app)
{
  auto eof = false;

  auto err = configureComponent(*app);

  if(err != OMX_ErrorNone)
  {
    app->eventBus.queueEvent({ errorEvent, make_shared<ErrorEventData>(err) });
    return;
  }

  /** sending command to video decoder component to go to executing state */
  err = OMX_SendCommand(app->hDecoder, OMX_CommandStateSet, OMX_StateExecuting, nullptr);

  if(err != OMX_ErrorNone)
  {
    app->eventBus.queueEvent({ errorEvent, make_shared<ErrorEventData>(err) });
    return;
  }
  app->decoderEventState.wait();

  if(app->settings.hasPrealloc)
  {
    for(auto pBuf : app->outputBuffers)
    {
      auto err = OMX_FillThisBuffer(app->hDecoder, pBuf);

      if(err != OMX_ErrorNone)
      {
        app->eventBus.queueEvent({ errorEvent, make_shared<ErrorEventData>(err) });
        return;
      }
    }
  }

  while(!eof && !app->quit)
  {
    auto inputBuffer = app->inputBuffers.pop();
    eof = readFrame(inputBuffer, *app);

    auto err = OMX_EmptyThisBuffer(app->hDecoder, inputBuffer);

    if(err != OMX_ErrorNone)
    {
      app->eventBus.queueEvent({ errorEvent, make_shared<ErrorEventData>(err) });
      break;
    }

    if(eof)
    {
      LOG_VERBOSE("End of file");
      auto emptyBuffer = app->inputBuffers.pop();
      emptyBuffer->nFilledLen = 0;
      emptyBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
      auto err = OMX_EmptyThisBuffer(app->hDecoder, emptyBuffer);

      if(err != OMX_ErrorNone)
      {
        app->eventBus.queueEvent({ errorEvent, make_shared<ErrorEventData>(err) });
        break;
      }
    }
  }
}

static void deletePipeline(Application* app)
{
  LOG_VERBOSE("Stopping the pipeline");

  auto err = stopPipeline(*app);

  if(err != OMX_ErrorNone)
    app->eventBus.queueEvent({ errorEvent, make_shared<ErrorEventData>(err) });
  err = OMX_SendCommand(app->hDecoder, OMX_CommandStateSet, OMX_StateLoaded, nullptr);

  if(err != OMX_ErrorNone)
    app->eventBus.queueEvent({ errorEvent, make_shared<ErrorEventData>(err) });

  freeBuffers(inportIndex, *app);
  freeBuffers(outportIndex, *app);

  app->decoderEventState.wait();

  app->eventBus.queueEvent({ pipelineEndEvent, nullptr });
}

static OMX_ERRORTYPE safeMain(int argc, char** argv)
{
  Application app;
  parseCommandLine(argc, argv, app);
  thread deleteThread;

  app.eventBus.addListener([&](shared_ptr<EventData> )
  {
    LOG_IMPORTANT("[EventBus] Received EOS");

    if(app.quit)
      return;
    app.quit = true;
    deleteThread = thread(deletePipeline, &app);
  }, eosEvent);

  app.eventBus.addListener([&](shared_ptr<EventData> )
  {
    app.pipelineEnded = true;
  }, pipelineEndEvent);

  app.eventBus.addListener([&](shared_ptr<EventData> data_)
  {
    ErrorEventData* data = static_cast<ErrorEventData*>(data_.get());
    LOG_ERROR(string { "[EventBus] Got error code: " } +to_string(data->errorCode));

    if(app.quit)
      return;
    app.quit = true;
    deleteThread = thread(deletePipeline, &app);
  }, errorEvent);

  app.eventBus.addListener([&](shared_ptr<EventData> data_)
  {
    OmxEventData* d = static_cast<OmxEventData*>(data_.get());
    handleEvent(d->hComponent, d->pAppData, d->eEvent, d->Data1, d->Data2, d->pEventData);
  }, omxEvent);

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

  OMX_CALL(OMX_Init());
  auto scopeOMX = scopeExit([]() {
    OMX_Deinit();
  });

  auto component = chooseComponent(app.settings.codecImplem);

  OMX_CALLBACKTYPE videoDecoderCallbacks;
  videoDecoderCallbacks.EventHandler = onComponentEvent;
  videoDecoderCallbacks.EmptyBufferDone = onInputBufferAvailable;
  videoDecoderCallbacks.FillBufferDone = onOutputBufferAvailable;

  OMX_CALL(OMX_GetHandle(&app.hDecoder, (OMX_STRING)component.c_str(), &app, const_cast<OMX_CALLBACKTYPE*>(&videoDecoderCallbacks)));
  auto scopeHandle = scopeExit([&]() {
    OMX_FreeHandle(app.hDecoder);
  });

  OMX_CALL(showComponentVersion(&app.hDecoder));
  auto ret = setPortParameters(app);

  if(ret != OMX_ErrorNone)
    return ret;

  app.pAllocator = nullptr;

  if(app.settings.bDMAIn || app.settings.bDMAOut)
  {
    auto constexpr deviceName = "/dev/allegroDecodeIP";
    app.pAllocator = AL_DmaAlloc_Create(deviceName);

    if(!app.pAllocator)
      throw runtime_error(string("Couldn't create dma allocator (using ") + deviceName + string(")"));
  }

  auto scopeAlloc = scopeExit([&]() {
    if(app.pAllocator)
      AL_Allocator_Destroy(app.pAllocator);
  });

  thread omxThread(omxWorker, &app);
  LOG_VERBOSE("Waiting for Events");

  while(!app.pipelineEnded)
  {
    app.eventBus.waitAndProcess();
  }

  omxThread.join();
  deleteThread.join();

  cerr.flush();
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

