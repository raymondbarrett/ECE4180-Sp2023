/// \file LEDThread.hpp
/// \date 2023-03-07
/// \author mshakula (matvey@gatech.edu)
///
/// \brief The led thread header and implementatin.

#ifndef LED_THREAD_HPP
#define LED_THREAD_HPP

#ifndef __cplusplus
#error "LEDThread.hpp is a cxx-only header."
#endif // __cplusplus

#include <mbed.h>
#include <rtos.h>

#include "ThreadCommon.hpp"

#include "hardware.hpp"

// ======================= Public Interface ==========================

/// \brief Class encapsulating the flashing LED.
class LEDThread : public ThreadHelper<LEDThread>
{
 public:
  /// \brief Create a lightning effect.
  void operator()()
  {
    do {
      rtos::Thread::wait(static_cast<int>(3000 * pow(randf(), 3)));
      RGB = 0xccccdd;
      rtos::Thread::wait(static_cast<int>(200 * randf()));
      RGB = 0;
    } while (true);
  }
};

// ===================== Detail Implementation =======================

#endif // LED_THREAD_HPP
