/******************************************************************************
*
* Copyright (C) 2016-2020 Allegro DVT2.  All rights reserved.
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
#include "module_interface.h"
#include "device_dec_interface.h"
#include "module_enums.h"
#include "mediatype_dec_interface.h"

#include <vector>
#include <queue>
#include <memory>

#include <utility/threadsafe_map.h>

extern "C"
{
#include <lib_decode/lib_decode.h>
#include <lib_common/SliceConsts.h>
#include <lib_common/StreamBuffer.h>
#include <lib_common/BufferSeiMeta.h>
}

struct DecModule final : ModuleInterface
{
  DecModule(std::shared_ptr<DecMediatypeInterface> media, std::shared_ptr<DecDeviceInterface> device, std::shared_ptr<AL_TAllocator> allocator);
  ~DecModule() override;

  void Free(void* buffer) override;
  void* Allocate(size_t size) override;

  void FreeDMA(int fd);
  int AllocateDMA(int size);

  bool SetCallbacks(Callbacks callbacks) override;

  bool Empty(BufferHandleInterface* handle) override;
  bool Fill(BufferHandleInterface* handle) override;

  ErrorType Start(bool shouldPrealloc) override;
  bool Stop() override;

  ErrorType SetDynamic(std::string index, void const* param) override;
  ErrorType GetDynamic(std::string index, void* param) override;

private:
  std::shared_ptr<DecMediatypeInterface> const media;
  std::shared_ptr<DecDeviceInterface> device;
  std::shared_ptr<AL_TAllocator> allocator;

  DisplayPictureInfo currentDisplayPictureInfo;
  Flags currentFlags;
  TransferCharacteristicsType currentTransferCharacteristics;
  ColourMatrixType currentColourMatrix;
  ColorPrimariesType currentColorPrimaries;
  HighDynamicRangeSeis currentHDRSEIs;

  Callbacks callbacks;
  ThreadSafeMap<AL_TBuffer*, BufferHandleInterface*> handles;
  ThreadSafeMap<char*, AL_TBuffer*> dpb;
  ThreadSafeMap<AL_TBuffer*, char*> shouldBeCopied;

  ThreadSafeMap<void*, AL_HANDLE> allocated;
  ThreadSafeMap<int, AL_HANDLE> allocatedDMA;
  ThreadSafeMap<AL_TBuffer*, std::vector<AL_TSeiMetaData*>> displaySeis;

  AL_HDecoder decoder;
  bool resolutionFoundAsBeenCalled;
  Dimension<int> initialDimension;

  ErrorType CreateDecoder(bool shouldPrealloc);
  bool DestroyDecoder();
  void CopyIfRequired(AL_TBuffer* frameToDisplay, int size);
  bool CreateAndAttachStreamMeta(AL_TBuffer& input);

  AL_TBuffer* CreateInputBuffer(char* buffer, int size);
  AL_TBuffer* CreateOutputBuffer(char* buffer, int size);

  void ReleaseBufs(AL_TBuffer* frame);

  static void RedirectionEndParsing(AL_TBuffer* parsedFrame, void* userParam, int parsingID)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    pThis->EndParsing(parsedFrame, parsingID);
  };
  void EndParsing(AL_TBuffer* parsedFrame, int parsingID);

  static void RedirectionEndDecoding(AL_TBuffer* decodedFrame, void* userParam)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    pThis->EndDecoding(decodedFrame);
  };
  void EndDecoding(AL_TBuffer* decodedFrame);

  static void RedirectionDisplay(AL_TBuffer* frameToDisplay, AL_TInfoDecode* info, void* userParam)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    pThis->Display(frameToDisplay, info);
  };
  void Display(AL_TBuffer* frameToDisplay, AL_TInfoDecode* info);

  static AL_ERR RedirectionResolutionFound(int buffersNumber, int bufferSize, AL_TStreamSettings const* settings, AL_TCropInfo const* crop, void* userParam)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    pThis->ResolutionFound(buffersNumber, bufferSize, *settings, *crop);
    return AL_SUCCESS;
  };
  void ResolutionFound(int bufferNumber, int bufferSize, AL_TStreamSettings const& settings, AL_TCropInfo const& crop);

  static void RedirectionParsedSei(bool isPrefix, int payloadType, uint8_t* payload, int payloadSize, void* userParam)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    isPrefix ? pThis->ParsedPrefixSei(payloadType, payload, payloadSize) : pThis->ParsedSuffixSei(payloadType, payload, payloadSize);
  };
  void ParsedPrefixSei(int type, uint8_t* payload, int size);
  void ParsedSuffixSei(int type, uint8_t* payload, int size);

  static void RedirectionInputBufferDestroy(AL_TBuffer* input)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(input));
    pThis->InputBufferDestroy(input);
  };
  void InputBufferDestroy(AL_TBuffer* input);

  static void RedirectionInputBufferFreeWithoutDestroyingMemory(AL_TBuffer* input)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(input));
    pThis->InputBufferFreeWithoutDestroyingMemory(input);
  };
  void InputBufferFreeWithoutDestroyingMemory(AL_TBuffer* input);

  static void RedirectionOutputBufferDestroy(AL_TBuffer* output)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(output));
    pThis->OutputBufferDestroy(output);
  };
  void OutputBufferDestroy(AL_TBuffer* output);

  static void RedirectionOutputDmaBufferDestroy(AL_TBuffer* output)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(output));
    pThis->OutputDmaBufferDestroy(output);
  };
  void OutputDmaBufferDestroy(AL_TBuffer* output);

  static void RedirectionOutputBufferDestroyAndFree(AL_TBuffer* output)
  {
    auto pThis = static_cast<DecModule*>(AL_Buffer_GetUserData(output));
    pThis->OutputBufferDestroyAndFree(output);
  };
  void OutputBufferDestroyAndFree(AL_TBuffer* output);

  void ResetHDR();
};
