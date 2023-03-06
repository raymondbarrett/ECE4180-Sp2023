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
#include "cmsis_os.h"
#include "hardware.hpp"
#include "util.hpp"

// Prefer to keep threads separate, and also statically allocate their frames,
// so that the compiler can know ahead of time if we use too much memory.
#define MUSIC_THREAD_STANDALONE 1
#define STATIC_THREAD_STACKS 0

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

      debug("TIMER\r\n");

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

namespace VideoThread {

enum Type
{
  Face,
  LightningVideo,
};

struct Params
{
  Type mode;
};

void
main(const void* p)
{
  const Params* params = static_cast<const Params*>(p);

  switch (params->mode) {
    case LightningVideo: {
      {
        LockGuard<rtos::Mutex> _(LCD_Mutex);
        LCD.background_color(0x000000);
        LCD.cls();
        LCD.media_init();
        LCD.set_sector_address(0x00, 0x00);
        LCD.display_video(0, 0);
      }
      while (true)
        osThreadYield();
    } break;

    case Face: {
      {
        LockGuard<rtos::Mutex> _(LCD_Mutex);
        LCD.background_color(0xffaaaa);
        LCD.cls();
        LCD.media_init();
        LCD.set_sector_address(0x10, 0x00);
        LCD.display_image(0, 16);
      }
      while (true)
        osThreadYield();
    } break;
  }
}

} // namespace VideoThread

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

#if defined(STATIC_THREAD_STACKS) && STATIC_THREAD_STACKS
#if defined(MUSIC_THREAD_STANDALONE) && MUSIC_THREAD_STANDALONE
unsigned char TH_MUSIC_STACK[TH_MUSIC_SSIZE];
#endif
unsigned char TH_LED_STACK[TH_LED_SSIZE];
unsigned char TH_LCD_STACK[TH_LCD_SSIZE];
unsigned char TH_TIMER_STACK[TH_TIMER_SSIZE];
#else
#if defined(MUSIC_THREAD_STANDALONE) && MUSIC_THREAD_STANDALONE
#define TH_MUSIC_STACK nullptr
#endif
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
  // Use advanced thread priority/stack size creation for better control.

  // Very low prio since its very low resrouce + lots of waiting.
  rtos::Thread th_led(osPriorityNormal, TH_LED_SSIZE, TH_LED_STACK);

  // Has to be normal prio to always continue while main thread waits.
  rtos::Thread th_timer(osPriorityHigh, TH_TIMER_SSIZE, TH_TIMER_STACK);

  // make video thread
  rtos::Thread th_video(osPriorityNormal, TH_LCD_SSIZE, TH_LCD_STACK);

  // Not sure about priority effect on this one currently.
  // rtos::Thread th_lcd(osPriorityNormal, TH_LCD_SSIZE, TH_LCD_STACK);

  // Realtime priority to make sure the audio is buffered properly.
#if defined(MUSIC_THREAD_STANDALONE) && MUSIC_THREAD_STANDALONE
  rtos::Thread th_musicPlayer(
    osPriorityRealtime, TH_MUSIC_SSIZE, TH_MUSIC_STACK);
#else
  osThreadSetPriority(osThreadGetId(), osPriorityRealtime);
#endif

  debug("\n\r\nStarting program...\r\n");

  VideoThread::Params v_params = {VideoThread::Face};

  th_led.start(LEDThread::main);
  th_timer.start(TimerThread::main);
  th_video.start(mbed::callback(VideoThread::main, &v_params));
  /*
    // MusicThread::Params params = {"/usb/wavves/tetris-48k.pcm", 4.0};
    MusicThread::Params params = {"/usb/wavves/jpn-amend.mp3", 1};
  #if defined(MUSIC_THREAD_STANDALONE) && MUSIC_THREAD_STANDALONE
    th_musicPlayer.start(mbed::callback(MusicThread::main, &params));
    th_musicPlayer.join();
  #else
    MusicThread::main(&params);
  #endif
  */
  wait(20);
  // End program.
  debug("Finished...\r\n");
  th_timer.terminate();
  th_led.terminate();
  th_video.terminate();
  die();
}
