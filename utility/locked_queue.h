#pragma once

#include <queue>
#include <utility/semaphore.h>

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
  /**
   * @brief Adds a new element at the end of the queue
   *
   * @param val the element to add (will be moved);
   */
  void push(T val)
  {
    auto lock = Lock(m_Mutex);
    m_Queue.push(val);
    m_Semaphore.notify();
  }

  /**
   * @brief Gets the next element from the head of the queue. Waits infinitely
   * until one element is available, or "cancel" is called.
   *
   * @param val a reference to a variable which will be filled with the element
   *
   * @return true if a new element has been retrieved, false if "cancel" was
   * called.
   */
  T pop()
  {
    m_Semaphore.wait();
    auto lock = Lock(m_Mutex);
    auto val = std::move(m_Queue.front());
    m_Queue.pop();
    return val;
  }

private:
  semaphore m_Semaphore;
  std::queue<T> m_Queue;
  std::mutex m_Mutex;
};

