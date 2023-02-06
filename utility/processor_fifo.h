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

#include <utility/locked_queue.h>
#include <thread>
#include <functional>

#if defined __linux__
#include <sys/prctl.h>
static inline void SetCurrentThreadName(char const* name)
{
  prctl(PR_SET_NAME, (unsigned long)name, 0, 0, 0);
}

#else
static inline void SetCurrentThreadName(char const*)
{
}

#endif

template<typename T>
struct ProcessorFifo
{
  ProcessorFifo(std::function<void(T)> process_, std::function<void(T)> delete_, std::string name_) :
    process_{process_}, delete_{delete_}, name_{name_}, thread{ & ProcessorFifo::Worker, this}
  {
  }

  ~ProcessorFifo()
  {
    {
      std::unique_lock<std::mutex> sync(mutex);
      process_ = delete_;
    }
    tasks.push(Task { true, T {}
               });
    thread.join();
  }

  void queue(T process)
  {
    tasks.push(Task { false, process });
  }

private:
  std::mutex mutex;
  struct Task
  {
    bool quit;
    T data;
  };
  locked_queue<Task> tasks;

  std::function<void(T)> process_;
  std::function<void(T)> delete_;
  std::string name_;

  void Worker(void)
  {
    if(!name_.empty())
      SetCurrentThreadName(name_.c_str());

    while(true)
    {
      auto task = tasks.pop();

      if(task.quit)
        break;

      std::function<void(T)> p {};
      {
        std::unique_lock<std::mutex> sync(mutex);
        p = process_;
      }

      if(p)
        p(task.data);
    }
  }

  std::thread thread;
};

