/// \file main.cpp
/// \date 2023-02-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main file.

#include <cstdint>
#include <cstdio>
#include <ctime>

#include <mbed.h>
#include <rtos.h>

#include <uLCD_4DGL.h>

#include "MusicThread.hpp"
#include "hardware.hpp"
#include "util.hpp"

#define MUSIC_THREAD_STANDALONE 0

// Fine-grained control of stack sizes, to get more from the system.
//
// The stack sizes here are just approximate and just eyeballed /
// trial-and-error to work. Still massive improvements over the 2048 default
// size however.
#define kiB (1 << 10)
#define TH_LED_SSIZE (kiB * 3 / 32)   // 0.09375kiB = 96bytes
#define TH_TIMER_SSIZE (kiB * 11 / 8) // 1.375kiB = 1408bytes
#define TH_LCD_SSIZE (kiB * 11 / 8)

// ======================= Local Definitions =========================

namespace {

namespace LEDThread {

/// \brief Create lightning effects
void
main()
{
  do {
    rtos::Thread::wait(static_cast<int>(3000 * pow(randf(), 3)));
    RGB = 0xccccdd;
    rtos::Thread::wait(static_cast<int>(200 * randf()));
    RGB = 0;
  } while (true);
}

} // namespace LEDThread

namespace TimerThread {

/// \brief The timer LCD thread function.
void
main()
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
      LCD.text_string(fmt_buf, LCD_MAX_TEXT_WIDTH - len, 0, FONT_7X8, 0xff0000);
    }
    rtos::Thread::wait(1000);
  } while (true);
}

} // namespace TimerThread

namespace LCDThread {

/// \brief The LCD lightning effect function.
void
main()
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

} // namespace LCDThread

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

  // Very low prio since its very low resrouce + lots of waiting.
  rtos::Thread th_led(osPriorityNormal, TH_LED_SSIZE, nullptr);

  // Has to be normal prio to always continue while main thread waits.
  rtos::Thread th_timer(osPriorityNormal, TH_TIMER_SSIZE, nullptr);

  // Not sure about priority effect on this one currently.
  rtos::Thread th_lcd(osPriorityNormal, TH_LCD_SSIZE, nullptr);

  // Realtime priority to make sure the audio is buffered properly.
#if defined(MUSIC_THREAD_STANDALONE) && MUSIC_THREAD_STANDALONE
  rtos::Thread th_musicPlayer(
    osPriorityRealtime, TH_MUSIC_PLAYER_SSIZE, nullptr);
#else
  osThreadSetPriority(osThreadGetId(), osPriorityRealtime);
#endif

  debug("\n\n\rStarting program...\n\r");

  th_led.start(LEDThread::main);
  th_timer.start(TimerThread::main);

  MusicThread::Params params = {"/usb/sample4.pcm"};
#if defined(MUSIC_THREAD_STANDALONE) && MUSIC_THREAD_STANDALONE
  th_musicPlayer.start(mbed::callback(MusicThread::main, &params));
  th_musicPlayer.join();
#else
  MusicThread::main(&params);
#endif

  // End program.
  debug("Finished...\n\r");
  th_timer.terminate();
  // th_lcd.terminate();
  th_led.terminate();
  die();
}
