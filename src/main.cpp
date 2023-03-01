/// \file main.cpp
/// \date 2023-02-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main file.

#define MBED_NO_GLOBAL_USING_DIRECTIVE
#define MBED_THREAD_STATS_ENABLED
#define DEBUG_MEM_USAGE

#include <stdint.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <mbed.h>
#include <rtos.h>

#include <MSCFileSystem.h>
#include <uLCD_4DGL.h>

#include "hardware.hpp"
#include "util.hpp"

// Fine-grained control of stack sizes, to get more from the system.
//
// The stack sizes here are just approximate and just eyeballed /
// trial-and-error to work. Still massive improvements over the 2048 default
// size however.
//
// Intuition suggests that each stack frame is at least roughly 64 bytes.
#define MAX_THREADS (4)
#define TH_LED_SSIZE (128)
#define TH_LCD_TIMER_SSIZE (1536) // 1.5 << 10
#define TH_LCD_EFFECT_SSIZE (DEFAULT_STACK_SIZE)

// ======================= Local Definitions =========================

namespace {

rtos::Mutex lcd_mutex;

/// \brief Create lightning effects
void
lightning_main()
{
  do {
    rtos::Thread::wait(static_cast<int>(3000 * pow(randf(), 3)));
    RGB = 0xccccdd;
    rtos::Thread::wait(static_cast<int>(200 * randf()));
    RGB = 0;
  } while (true);
}

/// \brief The timer LCD thread function.
void
timer_main()
{
  static const std::time_t kSecondStart = std::time(nullptr);

  do {
    {
      LockGuard<rtos::Mutex> _(lcd_mutex);
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
      LCD.text_string(fmt_buf, LCD_MAX_TEXT_WIDTH - len, 0, FONT_7X8, 0xff0000);
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

// Huge Audio buffer.
#define AUDIO_BUF_SIZE (16 << 10)

unsigned char audio_buf[AUDIO_BUF_SIZE];

} // namespace

// ====================== Global Definitions =========================

/// \brief The main function.
int
main()
{
  // Use advanced thread priority/stack size creation for better control.

  // Very low prio since its very low resrouce + lots of waiting.
  rtos::Thread th_led(osPriorityLow, TH_LED_SSIZE, nullptr);

  // has to be normal prio to always continue while main thread waits.
  rtos::Thread th_lcd_timer(osPriorityNormal, TH_LCD_TIMER_SSIZE, nullptr);

  //
  rtos::Thread th_lcd_effect(
    osPriorityBelowNormal, TH_LCD_EFFECT_SSIZE, nullptr);

  th_lcd_timer.start(timer_main);
  int last_mode = 0;
  /*
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

    if (Switch.get_up()) {
      mode = 1;
    } else if (Switch.get_down()) {
      mode = 2;
    } else if (Switch.get_left()) {
      mode = 3;
    } else if (Switch.get_right()) {
      mode = 4;
    } else if (Switch.get_center()) {
      mode = 5;
    }

    PC.printf("Mode: %d\r\n", mode);

    switch (mode) {
      case 0:
        break;

      case 1: {
        th_led.start(lightning_main);
        th_lcd_effect.start(LCDLightning_main);

        th_led.terminate();
        th_lcd_effect.terminate();
        last_mode = mode;
      } break;

      default:
        break;
        die();
    }
  } while (true);
  */

  // /*
  MSCFileSystem usb("usb");
  {
    mbed::Timer t;
    t.start();
    FILE* file = std::fopen("/usb/sample.wav", "r");
    int   init = t.read_us();
    int   c;
    // while (c = std::fgetc(file) != EOF) {
    // }
    while (std::fread(audio_buf, 1, AUDIO_BUF_SIZE, file)) {
    }
    int read = t.read_us();
    std::fclose(file);
    int close = t.read_us();
    PC.printf(
      "[init, read, close]: %d, %d, %d\r\n", init, read - init, close - read);
  }
}
