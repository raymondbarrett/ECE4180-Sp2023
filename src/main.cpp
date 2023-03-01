/// \file main.cpp
/// \date 2023-02-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main file.

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <cstdlib>
#include <cstring>

#include <mbed.h>
#include <rtos.h>

#include <uLCD_4DGL.h>

#include "hardware.hpp"
#include "util.hpp"

// ======================= Local Definitions =========================

namespace {

/// \brief Create lightning effects
void
lightning_main(void const* args)
{
  while (1) {
    rtos::Thread::wait(3000 * pow(randf(), 3));
    RGB = 0xccccdd;
    rtos::Thread::wait(200 * randf());
    RGB = 0;
  }
}

rtos::Mutex lcd_mutex;

/// \brief The timer LCD thread function.
void
timer_main(void)
{
  static const std::time_t kSecondStart = std::time(NULL);

  do {
    {
      LockGuard<rtos::Mutex> _(lcd_mutex);
      std::time_t            seconds = std::time(NULL) - kSecondStart;
      char                   seconds_buffer[64];

      LCD.filled_rectangle(
        0, 0, LCD_MAX_WIDTH - 1, LCD_FONT_HEIGHT * 1 + 3, 0xffaaaa);
      LCD.textbackground_color(0xffaaaa);

      std::sprintf(seconds_buffer, "Time: %d", seconds);
      int len =
        cutBuffer(seconds_buffer, LCD_MAX_TEXT_WIDTH, seconds_buffer, 0);

      LCD.text_string(seconds_buffer, 0, 0, FONT_7X8, 0xff0000);
    }
    rtos::Thread::wait(1000);
  } while (true);
}

/// \brief The LCD lightning effect function.
void
LCDLightning_main(void)
{
  do {
    {
      LockGuard<rtos::Mutex> _(lcd_mutex);
      LCD.filled_rectangle(
        0,
        LCD_FONT_HEIGHT + 3,
        LCD_MAX_WIDTH - 1,
        LCD_MAX_HEIGHT - 1,
        0xffff00);
    }
    rtos::Thread::wait(static_cast<int>(3000 * pow(randf(), 3)));
    {
      LockGuard<rtos::Mutex> _(lcd_mutex);
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

/// \brief Die in main.
void __attribute__((noreturn)) die()
{
  int i = 0;
  do {
    OnboardLEDs = 0;
    const int index =
      i < ONBOARD_LED_COUNT ? i : ONBOARD_LED_COUNT - i % ONBOARD_LED_COUNT - 2;
    OnboardLEDs[index] = true;
    i                  = (i + 1) % (ONBOARD_LED_COUNT * 2 - 2);
    wait_ms(150);
  } while (true);
}

} // namespace

// ====================== Global Definitions =========================

int
main()
{
  rtos::Thread timer_th(timer_main);

  do {
    int mode = 0;

    if (BTInput.readable()) {
      if (BTInput.getc() == '!') {
        if (BTInput.getc() == 'B') {          // button data
          char bnum = BTInput.getc();         // button number
          if ((bnum >= '1') && (bnum <= '4')) // is a number button 1..4
            mode = BTInput.getc() - '0';      // turn on/off that num LED
        }
      }
    }

    if (Switch.up) {
      mode = 1;
    } else if (Switch.down) {
      mode = 2;
    } else if (Switch.left) {
      mode = 3;
    } else if (Switch.right) {
      mode = 4;
    } else if (Switch.center) {
      mode = 5;
    }

    switch (mode) {
      case 0:
        break;

      case 1: {
        rtos::Thread lightning_th(lightning_main);
        rtos::Thread lightning_lcd_th(LCDLightning_main);
        rtos::Thread::wait(10000);
        lightning_th.terminate();
        lightning_lcd_th.terminate();
      } break;

      default:
        die();
    }
  } while (true);
}
