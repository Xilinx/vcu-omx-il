/******************************************************************************
*
* Copyright (C) 2015-2022 Allegro DVT2
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

