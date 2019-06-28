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

