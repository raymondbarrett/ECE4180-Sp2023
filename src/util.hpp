/// \file util.hpp
/// \date 2023-02-28
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief Generic helper functions for use across project.

#ifndef UTIL_HPP
#define UTIL_HPP

#ifndef __cplusplus
#error "util.hpp is a cxx-only header."
#endif // __cplusplus

#include <cstdlib>

// ======================= Public Interface ==========================

template<class T>
struct LockGuard
{
  LockGuard(T& l) : _(l) { _.lock(); }

  ~LockGuard() { _.unlock(); }

  T& _;
};

/// \brief Get random number from 0 to 1
inline float
randf()
{
  return (rand() / (float(RAND_MAX)));
}

/// \brief
inline int
cutBuffer(char* buffer, int buffer_len, const char* str, int start)
{
  int i = 0;
  while (i < buffer_len - 1 && (buffer[i] = str[i + start])) {
    ++i;
  }
  return i;
}

// ===================== Detail Implementation =======================

#endif // UTIL_HPP
