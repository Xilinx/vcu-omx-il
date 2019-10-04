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
#include <stdexcept>

#include "module_structs.h"
#include "buffer_handle_interface.h"

static std::string const DYNAMIC_INDEX_GOP {
  "DYNAMIC_INDEX_GOP"
};
static std::string const DYNAMIC_INDEX_INSERT_IDR {
  "DYNAMIC_INDEX_INSERT_IDR"
};
static std::string const DYNAMIC_INDEX_CLOCK {
  "DYNAMIC_INDEX_CLOCK"
};
static std::string const DYNAMIC_INDEX_BITRATE {
  "DYNAMIC_INDEX_BITRATE"
};
static std::string const DYNAMIC_INDEX_RESOLUTION {
  "DYNAMIC_INDEX_RESOLUTION"
};
static std::string const DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_SIZE {
  "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_SIZE"
};
static std::string const DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_FILL {
  "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_FILL"
};
static std::string const DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_EMPTY {
  "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_BUFFER_EMPTY"
};
static std::string const DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD {
  "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD"
};
static std::string const DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_CLEAR {
  "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_CLEAR"
};
static std::string const DYNAMIC_INDEX_NOTIFY_SCENE_CHANGE {
  "DYNAMIC_INDEX_NOTIFY_SCENE_CHANGE"
};
static std::string const DYNAMIC_INDEX_IS_LONG_TERM {
  "DYNAMIC_INDEX_IS_LONG_TERM"
};
static std::string const DYNAMIC_INDEX_USE_LONG_TERM {
  "DYNAMIC_INDEX_USE_LONG_TERM"
};
static std::string const DYNAMIC_INDEX_INSERT_PREFIX_SEI {
  "DYNAMIC_INDEX_INSERT_PREFIX_SEI"
};
static std::string const DYNAMIC_INDEX_INSERT_SUFFIX_SEI {
  "DYNAMIC_INDEX_INSERT_SUFFIX_SEI"
};
static std::string const DYNAMIC_INDEX_TRANSFER_CHARACTERISTICS {
  "DYNAMIC_INDEX_TRANSFER_CHARACTERISTICS"
};
static std::string const DYNAMIC_INDEX_COLOUR_MATRIX {
  "DYNAMIC_INDEX_COLOUR_MATRIX"
};
static std::string const DYNAMIC_INDEX_HIGH_DYNAMIC_RANGE_SEIS {
  "DYNAMIC_INDEX_HIGH_DYNAMIC_RANGE_SEIS"
};
static std::string const DYNAMIC_INDEX_CURRENT_DISPLAY_PICTURE_INFO {
  "DYNAMIC_INDEX_CURRENT_DISPLAY_PICTURE_INFO"
};
static std::string const DYNAMIC_INDEX_INSERT_QUANTIZATION_PARAMETER_BUFFER {
  "DYNAMIC_INDEX_INSERT_QUANTIZATION_PARAMETER_BUFFER"
};
static std::string const DYNAMIC_INDEX_STREAM_FLAGS {
  "DYNAMIC_INDEX_STREAM_FLAGS"
};
static std::string const DYNAMIC_INDEX_LOOP_FILTER_BETA {
  "DYNAMIC_INDEX_LOOP_FILTER_BETA"
};
static std::string const DYNAMIC_INDEX_LOOP_FILTER_TC {
  "DYNAMIC_INDEX_LOOP_FILTER_TC"
};
static std::string const DYNAMIC_INDEX_MAX_RESOLUTION_CHANGE_SUPPORTED {
  "DYNAMIC_INDEX_MAX_RESOLUTION_CHANGE_SUPPORTED"
};

struct Callbacks
{
  enum class Event
  {
    ERROR,
    RESOLUTION_DETECTED,
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

static std::map<Callbacks::Event, char const*> ToStringCallbackEvent
{
  {
    Callbacks::Event::ERROR, "ERROR"
  },
  {
    Callbacks::Event::RESOLUTION_DETECTED, "RESOLUTION_DETECTED"
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
  enum ErrorType
  {
    SUCCESS,
    BAD_INDEX,
    BAD_PARAMETER,
    BAD_STATE,
    NOT_IMPLEMENTED,
    UNDEFINED,
    /* channel creation error */
    CHANNEL_CREATION_NO_CHANNEL_AVAILABLE,
    CHANNEL_CREATION_RESOURCE_UNAVAILABLE,
    CHANNEL_CREATION_RESOURCE_FRAGMENTED,
    NO_MEMORY,
    MAX,
  };

  virtual ~ModuleInterface() = 0;

  virtual void Free(void* buffer) = 0;
  virtual void* Allocate(size_t size) = 0;

  virtual bool SetCallbacks(Callbacks callbacks) = 0;

  virtual bool Empty(BufferHandleInterface* handle) = 0;
  virtual bool Fill(BufferHandleInterface* handle) = 0;

  virtual ErrorType Start(bool shouldPrealloc) = 0;
  virtual bool Stop() = 0;

  virtual ErrorType SetDynamic(std::string index, void const* param) = 0;
  virtual ErrorType GetDynamic(std::string index, void* param) = 0;
};

static std::map<ModuleInterface::ErrorType, std::string> ModuleInterfaceErrorInStringMap
{
  {
    ModuleInterface::SUCCESS, "SUCCESS"
  },
  {
    ModuleInterface::BAD_INDEX, "BAD_INDEX"
  },
  {
    ModuleInterface::BAD_PARAMETER, "BAD_PARAMETER"
  },
  {
    ModuleInterface::NOT_IMPLEMENTED, "NOT_IMPLEMENTED"
  },
  {
    ModuleInterface::BAD_STATE, "BAD_STATE"
  },
  {
    ModuleInterface::UNDEFINED, "UNDEFINED"
  },
  {
    ModuleInterface::CHANNEL_CREATION_NO_CHANNEL_AVAILABLE, "CHANNEL_CREATION_NO_CHANNEL_AVAILABLE"
  },
  {
    ModuleInterface::CHANNEL_CREATION_RESOURCE_UNAVAILABLE, "CHANNEL_CREATION_RESOURCE_UNAVAILABLE"
  },
  {
    ModuleInterface::CHANNEL_CREATION_RESOURCE_FRAGMENTED, "CHANNEL_CREATION_RESOURCE_FRAGMENTED"
  },
  {
    ModuleInterface::NO_MEMORY, "NO_MEMORY"
  },
  {
    ModuleInterface::MAX, "MAX"
  },
};

