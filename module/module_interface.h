// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

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
static std::string const DYNAMIC_INDEX_MAX_RESOLUTION_CHANGE_SUPPORTED {
  "DYNAMIC_INDEX_MAX_RESOLUTION_CHANGE_SUPPORTED"
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
static std::string const DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD_BY_PRESET {
  "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD_BY_PRESET"
};
static std::string const DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD_BY_VALUE {
  "DYNAMIC_INDEX_REGION_OF_INTEREST_QUALITY_ADD_BY_VALUE"
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
static std::string const DYNAMIC_INDEX_COLOR_PRIMARIES {
  "DYNAMIC_INDEX_COLOR_PRIMARIES"
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
static std::string const DYNAMIC_INDEX_SKIP_PICTURE {
  "DYNAMIC_INDEX_SKIP_PICTURE"
};

struct Callbacks
{
  enum class Event
  {
    ERROR,
    RESOLUTION_DETECTED,
    RESOLUTION_CHANGED,
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
    Callbacks::Event::RESOLUTION_CHANGED, "RESOLUTION_CHANGED"
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
    CHANNEL_CREATION_LOAD_DISTRIBUTION,
    CHANNEL_CREATION_HARDWARE_CAPACITY_EXCEDEED,
    NO_MEMORY,
    WATCHDOG_TIMEOUT,
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
  virtual ErrorType Restart() = 0;

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
    ModuleInterface::CHANNEL_CREATION_LOAD_DISTRIBUTION, "CHANNEL_CREATION_LOAD_DISTRIBUTION"
  },
  {
    ModuleInterface::CHANNEL_CREATION_HARDWARE_CAPACITY_EXCEDEED, "CHANNEL_CREATION_HARDWARE_CAPACITY_EXCEDEED"
  },
  {
    ModuleInterface::NO_MEMORY, "NO_MEMORY"
  },
  {
    ModuleInterface::WATCHDOG_TIMEOUT, "WATCHDOG_TIMEOUT"
  },
  {
    ModuleInterface::MAX, "MAX"
  },
};
