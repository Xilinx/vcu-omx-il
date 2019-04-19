#pragma once

template<typename T>
static inline T RoundDown(T value, T multiple)
{
  return value >= 0 ? (value / multiple) * multiple : ((value - multiple + 1) / multiple) * multiple;
}

template<typename T>
static inline T RoundUp(T value, T multiple)
{
  return value >= 0 ? ((value + multiple - 1) / multiple) * multiple : (value / multiple) * multiple;
}

