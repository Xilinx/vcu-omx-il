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

#include "logger.h"
#include <3rd_party/date.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <sstream>

using namespace std;
using namespace chrono;
using namespace date;

static char const* logFileEnvName = "AL_LOG_FILE";
static char const* logSeverityEnvName = "AL_LOG_LEVEL";
static char const* vcdFileEnvName = "AL_VCD_FILE";
static char const* vcdSeverityEnvName = "AL_VCD_LEVEL";

Logger::Logger()
{
  char* logSeverityEnvValue = getenv(logSeverityEnvName);

  if(logSeverityEnvValue != nullptr)
  {
    stringstream ssLog {
      string {
        logSeverityEnvValue
      }
    };
    ssLog >> logSeverity;
  }
  char* vcdSeverityEnvValue = getenv(vcdSeverityEnvName);

  if(vcdSeverityEnvValue != nullptr)
  {
    stringstream ssVcd {
      string {
        vcdSeverityEnvValue
      }
    };
    ssVcd >> vcdSeverity;
  }

  logFile = getenv(logFileEnvName);
  vcdFile = getenv(vcdFileEnvName);
  flush();
}

Logger::~Logger()
{
  processor.reset();
}

static bool shouldBeLogged(int actualSeverity, int maxSeverityRequired)
{
  return actualSeverity <= maxSeverityRequired;
}

static bool isVcdTraceType(Logger::TraceType type)
{
  return (type == Logger::TraceType::VCD_WITH_VALUE) || (type == Logger::TraceType::VCD_WITHOUT_VALUE);
}

void Logger::log(TraceType type, int severity, string const& msg, int64_t time, string const& function, string const& file, int line)
{
  if(type == TraceType::MAX_ENUM)
    return;

  int maxSeverityRequired = 0;

  if(type == TraceType::DEFAULT)
    maxSeverityRequired = logSeverity;
  else if(isVcdTraceType(type))
    maxSeverityRequired = vcdSeverity;

  if(!shouldBeLogged(severity, maxSeverityRequired))
    return;
  LogInfo info;
  info.type = type;
  info.msg = msg;
  info.time = time;
  info.function = function;
  info.file = file;
  info.line = line;
  processor->queue(info);
}

void Logger::flush()
{
  auto log = bind(&Logger::Sink, this, placeholders::_1);
  processor.reset(new ProcessorFifo<LogInfo> { log, log, "logger" });
}

static void dump(char const* file, stringstream& msg)
{
  if(!file)
  {
    cout << msg.str();
    return;
  }
  ofstream logfile {
    file, ofstream::out | ofstream::app
  };
  logfile << msg.str();
}

void Logger::Sink(LogInfo info)
{
  int64_t timeInNano = info.time;

  if(VCDFirstValue == 0)
    VCDFirstValue = timeInNano;
  switch(info.type)
  {
  case DEFAULT:
  {
    auto const tp_ns = floor<nanoseconds>(system_clock::time_point(nanoseconds(timeInNano)));
    auto const tp_days = floor<days>(tp_ns);
    auto const ymd = year_month_day {
      tp_days
    };
    auto const time = make_time(tp_ns - tp_days);

    stringstream ss {};
    ss << '[' << ymd << ' ' << time << ']';
    ss << '\t';
    auto pos = info.file.find_last_of("/\\");
    string file = (pos != string::npos) ? info.file.substr(pos + 1) : info.file;
    ss << '[' << file << ':' << to_string(info.line) << ']';
    ss << '\t';
    ss << '[' << info.function << ']';

    if(!info.msg.empty())
    {
      ss << '\t';
      ss << info.msg;
    }
    ss << endl;

    dump(logFile, ss);

    break;
  }
  case VCD_WITHOUT_VALUE:
  {
    int64_t t0 = info.time - VCDFirstValue + 1000;
    stringstream ss {};
    ss << info.msg << " 1 " << t0 << endl;
    ss << info.msg << " 0 " << t0 + 1 << endl;

    dump(vcdFile, ss);
    break;
  }
  case VCD_WITH_VALUE:
  {
    int64_t t0 = info.time - VCDFirstValue + 1000;
    stringstream ss {};
    ss << info.msg << ' ' << t0 << endl;

    dump(vcdFile, ss);
    break;
  }
  case MAX_ENUM:
  {
    break;
  }
  default:
  {
    break;
  }
  }
}

