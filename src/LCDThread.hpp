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
    {
      SemGuard<rtos::Semaphore> _(LCD_Semaphore);
      LCD.background_color(0x000000);
      LCD.cls();
      LCD.media_init();
      LCD.set_sector_address(0x00, 0x00);
      LCD.display_image(0, 16);
    }
    while (true)
      osThreadYield();
  }
};

// ===================== Detail Implementation =======================

#endif // LCD_THREAD_HPP
