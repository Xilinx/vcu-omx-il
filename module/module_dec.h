// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once
#include "module_interface.h"
#include "device_dec_interface.h"
#include "module_enums.h"
#include "settings_dec_interface.h"

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
  DecModule(std::shared_ptr<DecSettingsInterface> media, std::shared_ptr<DecDeviceInterface> device, std::shared_ptr<AL_TAllocator> allocator);
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
  ErrorType Restart() override;

  ErrorType SetDynamic(std::string index, void const* param) override;
  ErrorType GetDynamic(std::string index, void* param) override;

private:
  std::shared_ptr<DecSettingsInterface> const media;
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
  bool resolutionFoundHasBeenCalled;
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

  static void RedirectionError(AL_ERR error, void* userParam)
  {
    auto pThis = static_cast<DecModule*>(userParam);
    pThis->Error(error);
  };
  void Error(AL_ERR error);

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
