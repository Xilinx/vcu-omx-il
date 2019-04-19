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
  processor.reset(new ProcessorFifo<void*> { log, log, "logger" });
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
    auto pos = logInfo->file.find_last_of("/\\");
    string file = (pos != string::npos) ? logInfo->file.substr(pos + 1) : logInfo->file;
    ss << '[' << file << ':' << to_string(logInfo->line) << ']';
    ss << '\t';
    ss << '[' << logInfo->function << ']';

    if(!logInfo->msg.empty())
    {
      ss << '\t';
      ss << logInfo->msg;
    }
    ss << endl;

    dump(logFile, ss);

    break;
  }
  case VCD_WITHOUT_VALUE:
  {
    int64_t t0 = logInfo->time - VCDFirstValue + 1000;
    stringstream ss {};
    ss << logInfo->msg << " 1 " << t0 << endl;
    ss << logInfo->msg << " 0 " << t0 + 1 << endl;

    dump(vcdFile, ss);
    break;
  }
  case VCD_WITH_VALUE:
  {
    int64_t t0 = logInfo->time - VCDFirstValue + 1000;
    stringstream ss {};
    ss << logInfo->msg << ' ' << t0 << endl;

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

  delete logInfo;
}

