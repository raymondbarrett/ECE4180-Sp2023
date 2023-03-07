/// \file TimerThread.hpp
/// \date 2023-03-07
/// \author mshakula (matvey@gatech.edu)
///
/// \brief The timer thread header and implementatin.

#ifndef TIMER_THREAD_HPP
#define TIMER_THREAD_HPP

#ifndef __cplusplus
#error "TimerThread.hpp is a cxx-only header."
#endif // __cplusplus

#include <mbed.h>
#include <rtos.h>

#include "ThreadCommon.hpp"

#include "hardware.hpp"
#include "util.hpp"

// ======================= Public Interface ==========================

/// \brief Class encapsulating the timer on the LCD.
class TimerThread : public ThreadHelper<TimerThread>
{
 public:
  void operator()()
  {
    static const std::time_t kSecondStart = std::time(nullptr);

    do {
      {
        LockGuard<rtos::Mutex> _(LCD_Mutex);
        std::time_t            seconds = std::time(nullptr) - kSecondStart;
        char                   fmt_buf[LCD_MAX_TEXT_WIDTH * 2];

        LCD.filled_rectangle(
          0, 0, LCD_MAX_WIDTH - 1, LCD_FONT_HEIGHT * 1 + 3, 0xffaaaa);
        LCD.textbackground_color(0xffaaaa);
        std::sprintf(
          fmt_buf,
          "Time: %02d:%02d:%02d",
          seconds / 3600,
          (seconds / 60) % 60,
          seconds % 60);
        int len = cutBuffer(fmt_buf, LCD_MAX_TEXT_WIDTH, fmt_buf, 0);
        LCD.text_string(
          fmt_buf, LCD_MAX_TEXT_WIDTH - len, 0, FONT_7X8, 0xff0000);
      }
      rtos::Thread::wait(1000);
    } while (true);
  }
};

// ===================== Detail Implementation =======================

#endif // TIMER_THREAD_HPP
