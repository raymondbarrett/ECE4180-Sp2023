/// \file LCDThread.hpp
/// \date 2023-03-07
/// \author mshakula (matvey@gatech.edu)
///
/// \brief The timer thread header and implementatin.

#ifndef LCD_THREAD_HPP
#define LCD_THREAD_HPP

#ifndef __cplusplus
#error "LCDThread.hpp is a cxx-only header."
#endif // __cplusplus

#include <mbed.h>
#include <rtos.h>

#include "ThreadCommon.hpp"

#include "hardware.hpp"
#include "util.hpp"

// ======================= Public Interface ==========================

/// \brief Class encapsulating the timer on the LCD.
class LCDThread : public ThreadHelper<LCDThread>
{
 public:
  void operator()()
  {
    do {
      {
        LockGuard<rtos::Mutex> _(LCD_Mutex);
        LCD.filled_rectangle(
          0,
          LCD_FONT_HEIGHT + 3,
          LCD_MAX_WIDTH - 1,
          LCD_MAX_HEIGHT - 1,
          0xffff00);
      }
      rtos::Thread::wait(static_cast<int>(3000 * pow(randf(), 3)));
      {
        LockGuard<rtos::Mutex> _(LCD_Mutex);
        LCD.filled_rectangle(
          0,
          LCD_FONT_HEIGHT + 3,
          LCD_MAX_WIDTH - 1,
          LCD_MAX_HEIGHT - 1,
          0x000000);
      }
      rtos::Thread::wait(static_cast<int>(200 * randf()));
    } while (true);
  }
};

// ===================== Detail Implementation =======================

#endif // LCD_THREAD_HPP
