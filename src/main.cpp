/// \file main.cpp
/// \date 2023-02-27
/// \author mshakula (matvey@gatech.edu)
///
/// \brief The main file.

#include <errno.h>

#include <cstdint>
#include <cstdio>
#include <ctime>

#include <mbed.h>
#include <rtos.h>

#include <uLCD_4DGL.h>

#include "LCDThread.hpp"
#include "LEDThread.hpp"
#include "MusicThread.hpp"
#include "TimerThread.hpp"

#include "hardware.hpp"
#include "util.hpp"

// The location under which audio files are hosted.
#define FILE_DIR "/usb/__LOAN_WAVES__"

// Prefer to keep threads separate, and also statically allocate their frames,
// so that the compiler can know ahead of time if we use too much memory.
#define STATIC_THREAD_STACKS 1

// Fine-grained control of stack sizes, to get more from the system.
//
// The stack sizes here are just approximate and just eyeballed /
// trial-and-error to work. Still massive improvements over the 2048 default
// size however.
#define kiB (1 << 10)
#define TH_LED_SSIZE (kiB * 1 / 4)    // 0.250kiB = 256bytes
#define TH_TIMER_SSIZE (kiB * 11 / 8) // 1.375kiB = 1408bytes
#define TH_LCD_SSIZE (kiB * 11 / 8)
#define TH_MUSIC_SSIZE (kiB * 2)

#define TH_LED_PRIO (osPriorityNormal)
#define TH_TIMER_PRIO (osPriorityNormal)
#define TH_LCD_PRIO (osPriorityNormal)
#define TH_MUSIC_PRIO (osPriorityRealtime)

// ======================= Local Definitions =========================

namespace {

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

int
printdir()
{
  DIR* d = opendir(FILE_DIR);
  if (!d)
    return 1;
  printf("[main] Dumping %s: {\r\n", FILE_DIR);
  for (struct dirent* e = readdir(d); e; e = readdir(d))
    printf("  %s\r\n", e->d_name);
  printf("}\r\n");
  closedir(d);
  return 0;
}

#if defined(STATIC_THREAD_STACKS) && STATIC_THREAD_STACKS
unsigned char TH_MUSIC_STACK[TH_MUSIC_SSIZE];
unsigned char TH_LED_STACK[TH_LED_SSIZE];
unsigned char TH_LCD_STACK[TH_LCD_SSIZE];
unsigned char TH_TIMER_STACK[TH_TIMER_SSIZE];
#else
#define TH_MUSIC_STACK nullptr
#define TH_LED_STACK nullptr
#define TH_LCD_STACK nullptr
#define TH_TIMER_STACK nullptr
#endif

} // namespace

// ====================== Global Definitions =========================

/// \brief The main function.
int
main()
{
  printf("\n\r\n[main] Starting program...\r\n");

  rtos::Thread th_timer(TH_TIMER_PRIO, TH_TIMER_SSIZE, TH_TIMER_STACK);
  TimerThread  timer;
  timer.startIn(th_timer);

  if (printdir()) {
    error(
      "[main] Could not open root file directory %s. [code %d: %s]\r\n",
      FILE_DIR,
      -errno,
      strerror(errno));
    goto end0;
  }
  printf("[main] Select mode.\r\n");

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

    switch (mode) {
      case 0: {
        osThreadYield();
      } break;

      case 1: {
        rtos::Thread th_led(TH_LED_PRIO, TH_LED_SSIZE, TH_LED_STACK);
        rtos::Thread th_lcd(TH_LCD_PRIO, TH_LCD_SSIZE, TH_LCD_STACK);

        MusicThread music(FILE_DIR "/thunder.pcm", 0.8);
        LEDThread   led;
        LCDThread   lcd;

        led.startIn(th_led);
        lcd.startIn(th_lcd);

        for (int i = 0; i < 3; ++i) {
          rtos::Thread th_music(TH_MUSIC_PRIO, TH_MUSIC_SSIZE, TH_MUSIC_STACK);
          music.startIn(th_music);
          th_music.join();
          rtos::Thread::wait(5000 * pow(randf(), 5));
        }

        th_led.terminate();
        th_lcd.terminate();

        {
          SemGuard<rtos::Semaphore> _(LCD_Semaphore);
          LCD.cls();
        }

      } break;

      case 2: {
        rtos::Thread th_music(TH_MUSIC_PRIO, TH_MUSIC_SSIZE, TH_MUSIC_STACK);

        MusicThread music(FILE_DIR "/jpn-amend.pcm", 1.0);

        music.startIn(th_music);

        th_music.join();
      } break;

      case 3: {
        rtos::Thread th_music(TH_MUSIC_PRIO, TH_MUSIC_SSIZE, TH_MUSIC_STACK);

        MusicThread music(FILE_DIR "/all-the-things-she-said.pcm", 1.0);

        music.startIn(th_music);

        th_music.join();
      } break;

      case 4: {
        rtos::Thread th_music(TH_MUSIC_PRIO, TH_MUSIC_SSIZE, TH_MUSIC_STACK);

        MusicThread music(FILE_DIR "/tetris.pcm", 1.0);

        music.startIn(th_music);

        th_music.join();
      } break;

      default:
        goto end0;
    }
  } while (true);

  // End program.
end0:
  th_timer.terminate();
  printf("[main] Death...\r\n");
  die();
}
