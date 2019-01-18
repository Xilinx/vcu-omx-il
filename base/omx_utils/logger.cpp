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

#include "logger.h"
#include "3rd_party/date.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace chrono;
using namespace date;

Logger& logger = Logger::GetSingleton();

Logger::Logger()
{
  flush();
}

Logger::~Logger()
{
  processor.reset();
}

void Logger::log(TraceType type, string const& msg, int64_t time, string const& function, string const& file, int line)
{
  auto logInfo = new LogInfo {};
  logInfo->type = type;
  logInfo->msg = msg;
  logInfo->time = time;
  logInfo->function = function;
  logInfo->file = file;
  logInfo->line = line;
  processor->queue(logInfo);
}

void Logger::flush()
{
  auto log = bind(&Logger::Sink, this, placeholders::_1);
  processor.reset(new ProcessorFifo { log, log });
}

static void dump(char const* env, stringstream& ss)
{
  char* file = getenv(env);

  if(!file)
  {
    cout << ss.str();
    return;
  }
  ofstream logfile {
    file, ofstream::out | ofstream::app
  };
  logfile << ss.str();
}

void Logger::Sink(void* event)
{
  auto logInfo = static_cast<LogInfo*>(event);
  int64_t timeInNano = logInfo->time;

  if(VCDFirstValue == 0)
    VCDFirstValue = timeInNano;
  switch(logInfo->type)
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
    ss << '[' << logInfo->file << ':' << to_string(logInfo->line) << ']';
    ss << '\t';
    ss << '[' << logInfo->function << ']';
    ss << '\t';
    ss << logInfo->msg << endl;

    dump("LOG_FILE", ss);

    break;
  }
  case VCD_WITHOUT_VALUE:
  {
    int64_t t0 = logInfo->time - VCDFirstValue + 1000;
    stringstream ss {};
    ss << logInfo->msg << " 1 " << t0 << endl;
    ss << logInfo->msg << " 0 " << t0 + 1 << endl;

    dump("VCD_FILE", ss);
    break;
  }
  case VCD_WITH_VALUE:
  {
    int64_t t0 = logInfo->time - VCDFirstValue + 1000;
    stringstream ss {};
    ss << logInfo->msg << ' ' << t0 << endl;

    dump("VCD_FILE", ss);
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

  delete logInfo;
}

