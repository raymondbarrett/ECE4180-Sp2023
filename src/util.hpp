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

/// \brief Cut a string to fit into the buffer.
///
/// \return Amount of characters written.
inline int
cutBuffer(char* buffer, int buffer_len, const char* str, int start)
{
  int i = 0;
  while (i < buffer_len - 1 && (buffer[i] = str[i + start])) {
    ++i;
  }
  return i;
}

/// \brief call only once. not thread-safe.
#define CALL_ONCE(expr)                                        \
  do {                                                         \
    static bool _ = false;                                     \
    if (!_) {                                                  \
      static volatile int __ = (static_cast<void>((expr)), 0); \
      _                      = true;                           \
    }                                                          \
  } while (false)

/// \brief Homebrew nullptr definition to mirror c++11. Stolen from
/// [here](https://stackoverflow.com/questions/44517556/how-to-define-our-own-nullptr-in-c98).
const class nullptr_t
{
 public:
  template<class T>
  operator T*() const
  {
    return 0;
  }

  template<class C, class T>
  operator T C::*() const
  {
    return 0;
  }

 private:
  void operator&() const;

} nullptr = {};

// ===================== Detail Implementation =======================

#endif // UTIL_HPP
