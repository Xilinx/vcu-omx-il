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
#include <map>
#include <mutex>

template<class K, class V>
struct ThreadSafeMap
{
  void Add(K const& key, V value)
  {
    std::lock_guard<std::mutex> lock(mutex);
    map.insert(std::pair<K, V>(key, value));
  }

  void Remove(K const& key)
  {
    std::lock_guard<std::mutex> lock(mutex);
    _Remove(key);
  }

  V Get(K const& key)
  {
    std::lock_guard<std::mutex> lock(mutex);
    return _Get(key);
  }

  V Pop(K const& key)
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto val = _Get(key);
    _Remove(key);
    return val;
  }

  bool Exist(K const& key)
  {
    std::lock_guard<std::mutex> lock(mutex);

    if(map.find(key) != map.end())
      return true;

    return false;
  }

private:
  V _Get(K const& key)
  {
    return map[key];
  }

  void _Remove(K const& key)
  {
    map.erase(key);
  }

  std::mutex mutex;
  std::map<K, V> map;
};

