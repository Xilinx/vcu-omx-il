/******************************************************************************
*
* Copyright (C) 2018 Allegro DVT2.  All rights reserved.
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

#include "processor_fifo.h"
#include <memory>
#include <string>
#include <chrono>

struct Logger
{
  enum TraceType
  {
    DEFAULT,
    VCD_WITH_VALUE,
    VCD_WITHOUT_VALUE,
    MAX_ENUM,
  };

  struct LogInfo
  {
    TraceType type {};
    std::string msg {};
    std::string function {};
    std::string file {};
    int line {};
    int64_t time {};
  };

  static Logger& GetSingleton()
  {
    static Logger singleton;
    return singleton;
  }

  static int64_t GetTime()
  {
    auto now = std::chrono::system_clock::now();
    return now.time_since_epoch() / std::chrono::nanoseconds(1);
  }

  Logger(Logger const &) = delete;
  void operator = (Logger const &) = delete;
  ~Logger();

  void log(TraceType type, std::string const& msg, int64_t time, std::string const& function, std::string const& file, int line);
  void flush();

private:
  Logger();

  std::unique_ptr<ProcessorFifo> processor {};
  int64_t VCDFirstValue {};
  void Sink(void* event);
};

extern Logger& logger;

#define LOG(msg) \
  { \
    logger.log(Logger::TraceType::DEFAULT, msg, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } \
  void FORCE_SEMICOLON()

#define LOG_VCD(wire) \
  { \
    logger.log(Logger::TraceType::VCD_WITHOUT_VALUE, wire, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } \
  void FORCE_SEMICOLON()

#define LOG_VCD_X(wire, value) \
  { \
    logger.log(Logger::TraceType::VCD_WITH_VALUE, wire + std::string { ' ' } +std::to_string(value), Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } \
  void FORCE_SEMICOLON()

