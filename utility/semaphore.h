#pragma once

#include <condition_variable>
#include <mutex>

template<typename L>
std::unique_lock<L> Lock(L& lockMe)
{
  return std::unique_lock<L>(lockMe);
}

struct semaphore
{
  semaphore() :
    m_Count{0},
    m_cancelWait{false}
  {
  }

  ~semaphore()
  {
    auto lock = Lock(m_Mutex);

    if(m_cancelWait)
      m_Condition.notify_all();
  }

  void notify()
  {
    auto lock = Lock(m_Mutex);
    ++m_Count;
    m_Condition.notify_one();
  }

  bool wait()
  {
    auto lock = Lock(m_Mutex);

    while(!m_Count && !m_cancelWait)
      m_Condition.wait(lock);

    if(m_cancelWait)
    {
      m_cancelWait = false;
      return false;
    }
    --m_Count;
    return true;
  }

  void reset()
  {
    m_Count = 0;
  }

private:
  std::mutex m_Mutex;
  std::condition_variable m_Condition;
  unsigned long m_Count;
  bool m_cancelWait;
};

