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

#pragma once

#include <functional>
#include <map>
#include <string>

#include "omx_module_structs.h"
#include "omx_buffer_handle_interface.h"

enum ErrorType
{
  SUCCESS,
  ERROR_BAD_PARAMETER,
  ERROR_NOT_IMPLEMENTED,
  ERROR_BAD_STATE,
  ERROR_UNDEFINED,
  /* channel creation error */
  ERROR_CHAN_CREATION_NO_CHANNEL_AVAILABLE,
  ERROR_CHAN_CREATION_RESOURCE_UNAVAILABLE,
  ERROR_CHAN_CREATION_RESOURCE_FRAGMENTED,
  ERROR_NO_MEMORY,
  ERROR_MAX,
};

static std::map<ErrorType, const char*> ToStringError
{
  {
    SUCCESS, "SUCCESS"
  },
  {
    ERROR_BAD_PARAMETER, "ERROR_BAD_PARAMETER"
  },
  {
    ERROR_NOT_IMPLEMENTED, "ERROR_NOT_IMPLEMENTED"
  },
  {
    ERROR_BAD_STATE, "ERROR_BAD_STATE"
  },
  {
    ERROR_UNDEFINED, "ERROR_UNDEFINED"
  },
  {
    ERROR_CHAN_CREATION_NO_CHANNEL_AVAILABLE, "ERROR_CHAN_CREATION_NO_CHANNEL_AVAILABLE"
  },
  {
    ERROR_CHAN_CREATION_RESOURCE_UNAVAILABLE, "ERROR_CHAN_CREATION_RESOURCE_UNAVAILABLE"
  },
  {
    ERROR_CHAN_CREATION_RESOURCE_FRAGMENTED, "ERROR_CHAN_CREATION_RESOURCE_FRAGMENTED"
  },
  {
    ERROR_NO_MEMORY, "ERROR_NO_MEMORY"
  },
  {
    ERROR_MAX, "ERROR_MAX"
  },
};

#define DYNAMIC_INDEX_GOP "DYNAMIC_INDEX_GOP"
#define DYNAMIC_INDEX_INSERT_IDR "DYNAMIC_INDEX_INSERT_IDR"
#define DYNAMIC_INDEX_CLOCK "DYNAMIC_INDEX_CLOCK"
#define DYNAMIC_INDEX_BITRATE "DYNAMIC_INDEX_BITRATE"
#define DYNAMIC_INDEX_RESOLUTION "DYNAMIC_INDEX_RESOLUTION"
#define DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_SIZE "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_SIZE"
#define DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_FILL "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_FILL"
#define DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_EMPTY "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_EMPTY"
#define DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD"
#define DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_CLEAR "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_CLEAR"
#define DYNAMIC_INDEX_NOTIFY_SCENE_CHANGE "DYNAMIC_INDEX_NOTIFY_SCENE_CHANGE"
#define DYNAMIC_INDEX_IS_LONG_TERM "DYNAMIC_INDEX_IS_LONG_TERM"
#define DYNAMIC_INDEX_USE_LONG_TERM "DYNAMIC_INDEX_USE_LONG_TERM"
#define DYNAMIC_INDEX_INSERT_PREFIX_SEI "DYNAMIC_INDEX_INSERT_PREFIX_SEI"
#define DYNAMIC_INDEX_INSERT_SUFFIX_SEI "DYNAMIC_INDEX_INSERT_SUFFIX_SEI"
#define DYNAMIC_INDEX_CURRENT_DISPLAY_PICTURE_INFO "DYNAMIC_INDEX_CURRENT_DISPLAY_PICTURE_INFO"

struct Callbacks
{
  enum class Event
  {
    ERROR,
    RESOLUTION_CHANGE,
    SEI_PREFIX_PARSED,
    SEI_SUFFIX_PARSED,
    MAX,
  };

  std::function<void(BufferHandleInterface* buffer)> emptied;
  std::function<void(BufferHandleInterface* input, BufferHandleInterface* output)> associate;
  std::function<void(BufferHandleInterface* buffer)> filled;
  std::function<void(bool isInput, BufferHandleInterface* buffer)> release;
  std::function<void(Event event, void* data)> event;
};

static std::map<Callbacks::Event, const char*> ToStringCallbackEvent
{
  {
    Callbacks::Event::ERROR, "ERROR"
  },
  {
    Callbacks::Event::RESOLUTION_CHANGE, "RESOLUTION_CHANGE"
  },
  {
    Callbacks::Event::SEI_PREFIX_PARSED, "SEI_PREFIX_PARSED"
  },
  {
    Callbacks::Event::SEI_SUFFIX_PARSED, "SEI_SUFFIX_PARSED"
  },
  {
    Callbacks::Event::MAX, "MAX"
  },
};

struct ModuleInterface
{
  virtual ~ModuleInterface() = 0;

  virtual void Free(void* buffer) = 0;
  virtual void* Allocate(size_t size) = 0;

  virtual bool SetCallbacks(Callbacks callbacks) = 0;

  virtual bool Empty(BufferHandleInterface* handle) = 0;
  virtual bool Fill(BufferHandleInterface* handle) = 0;

  virtual ErrorType Run(bool shouldPrealloc) = 0;
  virtual bool Flush() = 0;
  virtual void Stop() = 0;

  virtual ErrorType SetDynamic(std::string index, void const* param) = 0;
  virtual ErrorType GetDynamic(std::string index, void* param) = 0;
};

