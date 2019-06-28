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

