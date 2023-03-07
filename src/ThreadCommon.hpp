/// \file ThreadCommon.hpp
/// \date 2023-03-07
/// \author mshakula (matvey@gatech.edu)
///
/// \brief CRTP helper for thread functions + inter-thread communication
/// definition.

#ifndef THREAD_HELPER_HPP
#define THREAD_HELPER_HPP

#ifndef __cplusplus
#error "ThreadCommon.hpp is a cxx-only header."
#endif // __cplusplus

#include <mbed.h>
#include <rtos.h>

// ======================= Public Interface ==========================

/// \brief A CRTP helper to run a thread, minimizing boilerplate.
template<class T>
class ThreadHelper
{
 public:
  /// \brief Interface to the mbed thread mechanic.
  static void main(void* p) { static_cast<T*>(p)->operator()(); }

  /// \brief Start the current thread function in a thread..
  osStatus startIn(rtos::Thread& th)
  {
    return th.start(mbed::callback(ThreadHelper::main, this));
  }
};

// ===================== Detail Implementation =======================

#endif // THREAD_HELPER_HPP
