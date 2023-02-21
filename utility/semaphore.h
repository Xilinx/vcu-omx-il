// SPDX-FileCopyrightText: © 2023 Allegro DVT <github-ip@allegrodvt.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <condition_variable>
#include <mutex>

struct semaphore
{
  semaphore() :
    m_Mutex{},
    m_Condition{},
    m_Count{0}
  {
  }

  semaphore(unsigned long long count) :
    m_Mutex{},
    m_Condition{},
    m_Count{count}
  {
  }

  semaphore(semaphore const &&) = delete;
  semaphore(semaphore &&) = delete;
  semaphore & operator = (semaphore const &) = delete;
  semaphore & operator = (semaphore &&) = delete;

  ~semaphore() = default;

  void notify()
  {
    std::unique_lock<std::mutex> lock(m_Mutex);
    ++m_Count;
    m_Condition.notify_one();
  }

  void wait()
  {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Condition.wait(lock, [&] { return m_Count > 0;
                     });
    --m_Count;
  }

  void reset()
  {
    m_Count = 0;
  }

private:
  std::mutex m_Mutex;
  std::condition_variable m_Condition;
  unsigned long long m_Count;
};

