/******************************************************************************
*
* Copyright (C) 2015-2023 Allegro DVT2
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

#pragma once

#include <utility/processor_fifo.h>
#include <memory>
#include <string>
#include <chrono>
#include <sstream>

static inline std::string ToStringAddr(void* addr)
{
  if(addr == nullptr)
    return std::string {
             "nullptr"
    };
  std::stringstream ss {};
  ss << addr;
  return ss.str();
}

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
    static Logger singleton {};
    return singleton;
  }

  static int64_t GetTime()
  {
    auto now = std::chrono::system_clock::now();
    return now.time_since_epoch() / std::chrono::nanoseconds(1);
  }

  Logger(Logger const &) = delete;
  Logger & operator = (Logger const &) = delete;
  Logger(Logger &&) = delete;
  Logger & operator = (Logger &&) = delete;

  void log(TraceType type, int severity, std::string const& msg, int64_t time, std::string const& function, std::string const& file, int line);
  void flush();

private:
  Logger();
  ~Logger();

  char* logFile {
    nullptr
  };
  int logSeverity {
    1
  };
  char* vcdFile {
    nullptr
  };
  int vcdSeverity {
    0
  };
  std::unique_ptr<ProcessorFifo<LogInfo>> processor {};
  int64_t VCDFirstValue {};
  void Sink(LogInfo info);
};

#define LOG_ERROR(msg) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::DEFAULT, 1, std::string { msg }, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_WARNING(msg) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::DEFAULT, 3, std::string { msg }, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_IMPORTANT(msg) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::DEFAULT, 5, std::string { msg }, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VERBOSE(msg) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::DEFAULT, 10, std::string { msg }, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VCD_ERROR(wire) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::VCD_WITHOUT_VALUE, 1, std::string { wire }, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VCD_WARNING(wire) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::VCD_WITHOUT_VALUE, 3, std::string { wire }, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VCD_IMPORTANT(wire) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::VCD_WITHOUT_VALUE, 5, std::string { wire }, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VCD_VERBOSE(wire) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::VCD_WITHOUT_VALUE, 10, std::string { wire }, Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VCD_X_ERROR(wire, value) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::VCD_WITH_VALUE, 1, std::string { wire } +std::string { ' ' } +std::to_string(value), Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VCD_X_WARNING(wire, value) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::VCD_WITH_VALUE, 3, std::string { wire } +std::string { ' ' } +std::to_string(value), Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VCD_X_IMPORTANT(wire, value) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::VCD_WITH_VALUE, 5, std::string { wire } +std::string { ' ' } +std::to_string(value), Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

#define LOG_VCD_X_VERBOSE(wire, value) \
  do { \
    Logger::GetSingleton().log(Logger::TraceType::VCD_WITH_VALUE, 10, std::string { wire } +std::string { ' ' } +std::to_string(value), Logger::GetTime(), __func__, __FILE__, __LINE__); \
  } while(0)

