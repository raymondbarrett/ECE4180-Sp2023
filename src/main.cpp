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
#define TH_MUSIC_PLAYER_SSIZE (DEFAULT_STACK_SIZE)

#define DAC_POWER_MODE (1 << 16)

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
#define AUDIO_BUF_BANK_SIZE (2 << 10)

// switching audio buffer.
std::uint32_t audio_buf[2][AUDIO_BUF_BANK_SIZE];

void
musicPlayer_err()
{
  error("Error in DMA Callback\r\n");
}

struct musicPlayer_param_t
{
  const char*   name;
  rtos::Thread* thread;
};

struct musicPlayer_callback_t
{
  // osThreadId tid;
  volatile bool& flag;

  // musicPlayer_callback_t(osThreadId tid_) : tid(tid_) {}
  musicPlayer_callback_t(volatile bool& flag_) : flag(flag_) {}

  void operator()()
  {
    OnboardLEDs[0] = !OnboardLEDs[0];
    // osSignalSet(tid, 0x1);
    flag = true;
  }
};

void
musicPlayer_main(const void* p)
{
  const char*   name      = ((musicPlayer_param_t*)p)->name;
  osThreadId    tid       = rtos::Thread::gettid();
  std::size_t   read_ct   = 0;
  int           curr_bank = 0;
  bool          more      = true;
  FILE*         fp;
  MODDMA_Config bank_conf[2];
  volatile bool need_more = false;
  // musicPlayer_callback_t callback(tid);
  musicPlayer_callback_t callback(need_more);

  fp = std::fopen(name, "r");
  if (!fp) {
    error("Cannot open file %s!\n\r", name);
    return;
  }

  // fill initial buffer.
  for (int i = 0; i < 2; ++i) {
    read_ct =
      std::fread(audio_buf[i], sizeof(std::uint32_t), AUDIO_BUF_BANK_SIZE, fp);
    if (std::feof(fp)) {
      more = false;
    } else if (std::ferror(fp)) {
      error("Error reading file %s!\n\r", name);
      goto end;
    } else if (read_ct < AUDIO_BUF_BANK_SIZE) {
      std::memset(audio_buf[i], 0, AUDIO_BUF_BANK_SIZE - read_ct);
      more = false;
    }
    for (int j = 0; j < read_ct; ++j) {
      audio_buf[i][j] = DAC_POWER_MODE | ((audio_buf[i][j] << 6) & 0xFFC0);
    }
  }

  PC.printf("WOAH\r\n");

  (&bank_conf[0])
    ->channelNum(MODDMA::Channel_0)
    ->srcMemAddr((uint32_t)&audio_buf[0])
    ->dstMemAddr(MODDMA::DAC)
    ->transferSize(AUDIO_BUF_BANK_SIZE)
    ->transferType(MODDMA::m2p)
    ->dstConn(MODDMA::DAC)
    ->attach_tc(&callback, &musicPlayer_callback_t::operator())
    ->attach_err(musicPlayer_err);

  (&bank_conf[1])
    ->channelNum(MODDMA::Channel_1)
    ->srcMemAddr((uint32_t)&audio_buf[1])
    ->dstMemAddr(MODDMA::DAC)
    ->transferSize(AUDIO_BUF_BANK_SIZE)
    ->transferType(MODDMA::m2p)
    ->dstConn(MODDMA::DAC)
    ->attach_tc(&callback, &musicPlayer_callback_t::operator())
    ->attach_err(musicPlayer_err);

  // Calculating the transfer frequency:
  // By default, the Mbed library sets the PCLK_DAC clock value
  // to 24MHz. Divide this by amount of samples * rate. Sample rate = 384kb/s,
  // so thats 0x60000 samples.
  LPC_DAC->DACCNTVAL = 24000000 / AUDIO_BUF_BANK_SIZE;

  if (!DMA.Prepare(&bank_conf[0])) {
    error("Error preparing DMA!\n\r");
    goto end;
  }

  LPC_DAC->DACCTRL |= (3UL << 2);

  PC.printf("WOAH2\r\n");

  // osSignalWait(0x1, osWaitForever);
  while (!need_more)
    rtos::Thread::wait(2);

  while (more) {
    PC.printf("WOAH3\r\n");

    // swap to next bank...
    curr_bank = (curr_bank + 1) % 2;
    DMA.Disable((MODDMA::CHANNELS)DMA.getConfig()->channelNum());
    DMA.Prepare(&bank_conf[curr_bank]);
    if (DMA.irqType() == MODDMA::TcIrq)
      DMA.clearTcIrq();

    // read in new bank...
    int nb = (curr_bank + 1) % 2;
    read_ct =
      std::fread(audio_buf[nb], sizeof(std::uint32_t), AUDIO_BUF_BANK_SIZE, fp);
    if (std::feof(fp)) {
      more = false;
    } else if (std::ferror(fp)) {
      error("Error reading file %s!\n\r", name);
      goto end2;
    } else if (read_ct < AUDIO_BUF_BANK_SIZE) {
      std::memset(audio_buf[nb], 0, AUDIO_BUF_BANK_SIZE - read_ct);
      more = false;
    }
    // Thread::signal_wait(0x1);
    while (!need_more)
      rtos::Thread::wait(2);
  }

  PC.printf("WOAH4\r\n");

end2:
  DMA.Disable((MODDMA::CHANNELS)DMA.getConfig()->channelNum());
end:
  std::fclose(fp);
}

} // namespace

// ====================== Global Definitions =========================

/// \brief The main function.
int
main()
{
  // Use advanced thread priority/stack size creation for better control.

  // Very low prio since its very low resrouce + lots of waiting.
  rtos::Thread th_led(osPriorityLow, TH_LED_SSIZE, nullptr);

  // Has to be normal prio to always continue while main thread waits.
  rtos::Thread th_lcd_timer(osPriorityNormal, TH_LCD_TIMER_SSIZE, nullptr);

  rtos::Thread th_lcd_effect(
    osPriorityBelowNormal, TH_LCD_EFFECT_SSIZE, nullptr);

  rtos::Thread th_musicPlayer(
    osPriorityRealtime, TH_MUSIC_PLAYER_SSIZE, nullptr);

  th_lcd_timer.start(timer_main);
  int last_mode = 0;

  musicPlayer_param_t params = {"/usb/sample.wav", &th_musicPlayer};
  th_musicPlayer.start(mbed::callback(musicPlayer_main, &params));
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
}
