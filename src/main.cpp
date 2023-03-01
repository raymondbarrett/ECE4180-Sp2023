/// \file main.cpp
/// \date 2023-02-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main file.

#define MBED_NO_GLOBAL_USING_DIRECTIVE
#define MBED_THREAD_STATS_ENABLED
#define DEBUG_MEM_USAGE

#include <cstdlib>
#include <cstring>

#include <mbed.h>
#include <rtos.h>

#include <platform/mbed_stats.h>

#include <uLCD_4DGL.h>

#include "hardware.hpp"
#include "util.hpp"

#define MAX_THREADS (4)

// ======================= Local Definitions =========================

namespace {

rtos::Mutex lcd_mutex;

/// \brief Get the max stack usage of the current thread.
///
/// Could probably be implemented a bit more efficiently, but want to use public
/// api.
void
printMemUsage()
{
  mbed_stats_stack_t stats[MAX_THREADS];
  std::size_t        stats_ct = mbed_stats_stack_get_each(stats, MAX_THREADS);
  std::uint32_t      tid = reinterpret_cast<std::uint32_t>(Thread::gettid());

  PC.printf("\rPRINT MEM USAGE: FOUND %d threads\n", stats_ct);
  for (std::size_t i = 0; i < stats_ct; ++i) {
    if (stats[i].thread_id == tid) {
      PC.printf(
        "\r\nThread [%d] mem used: %d / %d",
        tid,
        stats[i].reserved_size,
        stats[i].max_size);
    }
  }
}

/// \brief Create lightning effects
void
lightning_main()
{
  do {
    rtos::Thread::wait(static_cast<int>(3000 * pow(randf(), 3)));
    RGB = 0xccccdd;
    rtos::Thread::wait(static_cast<int>(200 * randf()));
    RGB = 0;

#ifdef DEBUG_MEM_USAGE
    CALL_ONCE(printMemUsage());
#endif // DEBUG_MEM_USAGE
  } while (true);
}

/// \brief The timer LCD thread function.
void
timer_main()
{
  static const std::time_t kSecondStart = std::time(NULL);

  PC.printf("Crap");
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

#ifdef DEBUG_MEM_USAGE
    CALL_ONCE(printMemUsage());
    PC.printf("SHIT");
#endif // DEBUG_MEM_USAGE
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

#ifdef DEBUG_MEM_USAGE
    CALL_ONCE(printMemUsage());
#endif // DEBUG_MEM_USAGE
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

/// \brief The main function.
int
main()
{
  // Use advanced thread priority/stack size creation for better control.
  //
  // The stack sizes here are just approximate and just eyeballed to be safe.
  // Still massive improvements over the 2048 default size however.
  rtos::Thread th_led(osPriorityNormal, DEFAULT_STACK_SIZE, nullptr);
  rtos::Thread th_lcd_timer(osPriorityNormal, DEFAULT_STACK_SIZE, nullptr);
  rtos::Thread th_lcd_effect(osPriorityNormal, DEFAULT_STACK_SIZE, nullptr);

  // rtos::Thread __(timer_main);
  th_lcd_timer.start(timer_main);

  rtos::Thread::yield();

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
        th_led.start(lightning_main);
        th_lcd_effect.start(LCDLightning_main);
        rtos::Thread::wait(10000);
        th_led.terminate();
        th_lcd_effect.terminate();
      } break;

      default:
        die();
    }
  } while (true);
}
