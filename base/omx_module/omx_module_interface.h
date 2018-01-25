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

#pragma once

#include <functional>
#include <memory>
#include <map>

#include "omx_module_structs.h"

enum ErrorType
{
  SUCCESS,
  ERROR_BAD_PARAMETER,
  ERROR_NOT_IMPLEMENTED,
  ERROR_BAD_STATE,
  ERROR_UNDEFINED,
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
    ERROR_MAX, "ERROR_MAX"
  },
};

enum DynamicIndexType
{
  DYNAMIC_INDEX_GOP,
  DYNAMIC_INDEX_INSERT_IDR,
  DYNAMIC_INDEX_CLOCK,
  DYNAMIC_INDEX_BITRATE,
  DYNAMIC_INDEX_MAX,
};

static std::map<DynamicIndexType, const char*> ToStringDynamicIndex
{
  {
    DYNAMIC_INDEX_GOP, "DYNAMIC_INDEX_GOP"
  },
  {
    DYNAMIC_INDEX_INSERT_IDR, "DYNAMIC_INDEX_INSERT_IDR"
  },
  {
    DYNAMIC_INDEX_CLOCK, "DYNAMIC_INDEX_CLOCK"
  },
  {
    DYNAMIC_INDEX_BITRATE, "DYNAMIC_INDEX_BITRATE"
  },
  {
    DYNAMIC_INDEX_MAX, "DYNAMIC_INDEX_MAX"
  },
};

enum CallbackEventType
{
  CALLBACK_EVENT_ERROR,
  CALLBACK_EVENT_RESOLUTION_CHANGE,
  CALLBACK_EVENT_MAX,
};

static std::map<CallbackEventType, const char*> ToStringCallbackEvent
{
  {
    CALLBACK_EVENT_ERROR, "CALLBACK_EVENT_ERROR"
  },
  {
    CALLBACK_EVENT_RESOLUTION_CHANGE, "CALLBACK_EVENT_RESOLUTION_CHANGE"
  },
  {
    CALLBACK_EVENT_MAX, "CALLBACK_EVENT_MAX"
  },
};

typedef struct
{
  std::function<void (uint8_t* buffer, int offset, int size)> emptied;
  std::function<void (uint8_t* const input, uint8_t* const output)> associate;
  std::function<void (uint8_t* buffer, int offset, int size)> filled;
  std::function<void (bool isInput, uint8_t* buffer)> release;
  std::function<void (CallbackEventType event, void* data)> event;
}Callbacks;

struct ModuleInterface
{
  virtual ~ModuleInterface()
  {
  }

  virtual void ResetRequirements() = 0;
  virtual BufferRequirements GetBufferRequirements() const = 0;
  virtual Resolution GetResolution() const = 0;
  virtual Clock GetClock() const = 0;
  virtual Mimes GetMimes() const = 0;
  virtual Gop GetGop() const = 0;
  virtual Format GetFormat() const = 0;
  virtual bool SetResolution(Resolution const& resolution) = 0;
  virtual bool SetClock(Clock const& clock) = 0;
  virtual bool SetFormat(Format const& format) = 0;
  virtual bool SetGop(Gop const& gop) = 0;

  virtual bool CheckParam() = 0;
  virtual bool Create() = 0;
  virtual void Destroy() = 0;

  virtual void Free(void* buffer) = 0;
  virtual void* Allocate(size_t size) = 0;

  virtual bool SetCallbacks(Callbacks callbacks) = 0;

  virtual bool Empty(uint8_t* buffer, int offset, int size) = 0;
  virtual bool Fill(uint8_t* buffer, int offset, int size) = 0;

  virtual bool Run(bool shouldPrealloc) = 0;
  virtual bool Pause() = 0;
  virtual bool Flush() = 0;
  virtual void Stop() = 0;

  virtual ErrorType SetDynamic(DynamicIndexType index, void const* param) = 0;
  virtual ErrorType GetDynamic(DynamicIndexType index, void* param) = 0;
};

