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

