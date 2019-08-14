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

#pragma once

#include <queue>
#include <utility/semaphore.h>
#include <mutex>

/**
 * @brief A generic thread-safe FIFO queue. Does not copy its elements (they
 * must be moveable).
 *
 * Don't add a "size" function to it : its result would be obsolete as soon
 * as the call would have returned.
 */
template<typename T>
struct locked_queue
{
  locked_queue() = default;
  locked_queue(locked_queue const &&) = delete;
  locked_queue(locked_queue &&) = delete;
  locked_queue & operator = (locked_queue const &) = delete;
  locked_queue & operator = (locked_queue &&) = delete;

  ~locked_queue() = default;

  /**
   * @brief Adds a new element at the end of the queue
   *
   * @param val the element to add (will be moved);
   */
  void push(T val)
  {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Queue.push(val);
    lock.unlock();
    m_Semaphore.notify();
  }

  /**
   * @brief Gets the next element from the head of the queue. Waits infinitely
   * until one element is available
   *
   * @param val a reference to a variable which will be filled with the element
   *
   * @return The next element
   */
  T pop()
  {
    m_Semaphore.wait();
    std::unique_lock<std::mutex> lock(m_Mutex);
    auto val = std::move(m_Queue.front());
    m_Queue.pop();
    return val;
  }

private:
  semaphore m_Semaphore;
  std::queue<T> m_Queue;
  std::mutex m_Mutex;
};

