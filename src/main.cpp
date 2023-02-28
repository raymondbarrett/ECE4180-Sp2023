/// \file main.cpp
/// \date 2023-02-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main file.

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <mbed.h>
#include <rtos.h>

#include <uLCD_4DGL.h>

#include "hardware.hpp"

// ======================= Local Definitions =========================

namespace {

/// \brief Get random number from 0 to 1
inline float
random_number()
{
  return (rand() / (float(RAND_MAX)));
}

/// \brief Create lightning effects
void
lightning(void const* args)
{
  while (1) {
    wait(3 * pow(random_number(), 3));
    RGB[0] = 0.8;
    RGB[1] = 0.8;
    RGB[2] = 0.9;
    wait(0.2 * random_number());
    RGB[0] = 0;
    RGB[1] = 0;
    RGB[2] = 0;
  }
}

template<class T>
struct LockGuard
{
  LockGuard(T& l) _{l} { _.lock(); }

  ~LockGuard() { _.unlock(); }

  T& _;
};

mbed::Mutex lcd_mutex;

#define LCD_MAX_WIDTH (128)
#define LCD_MAX_HEIGHT LCD_MAX_WIDTH
#define LCD_FONT_WIDTH \
  (7) // default and only font that is shipped with LCD, so its a constant.
#define LCD_FONT_HEIGHT (8)
#define LCD_MAX_TEXT_WIDTH (LCD_MAX_WIDTH / LCD_FONT_WIDTH)
#define LCD_MAX_TEXT_HEIGHT (LCD_MAX_HEIGHT / LCD_FONT_HEIGHT)

// Always leaves last character in buffer untounched.
static int
cutBuffer(char* buffer, int buffer_len, const char* str, int start)
{
  int i = 0;
  while (i < buffer_len - 1 && (buffer[i] = str[i + start])) {
    ++i;
  }
  return i;
}

/// \brief The timer LCD.
void
timer(void)
{
  static mbed::Timer time;

  time.reset();
  do {
    if (time.get_us() >= 1000000) {
      time.reset();
      {
        LockGuard<mbed::Mutex> _(lcd_mutex);
        std::time_t            seconds = std::time(NULL);
        char                   seconds_buffer[64];
        LCD.filled_rectangle(
          3,
          LCD_FONT_HEIGHT + 2,
          LCD_MAX_WIDTH - 4,
          LCD_FONT_HEIGHT + 3,
          0xffaaaa);
        std::sprintfv(seconds_buffer, "Time: %s");
        cutBuffer(seconds_buffer, LCD_MAX_TEXT_WIDTH, seconds_buffer, 0);
        LCD.text_string(seconds_buffer, 0, 0, FONT_7X8, 0x000000);
      }
    }
  } while (true)
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
  char bnum = 0;
  int  mode = 0;
  while (1) {
    if (BTInput.getc() == '!') {
      if (BTInput.getc() == 'B') {          // button data
        bnum = BTInput.getc();              // button number
        if ((bnum >= '1') && (bnum <= '4')) // is a number button 1..4
          mode = BTInput.getc() - '0';      // turn on/off that num LED
      }
    }

    if (Switch.up()) {
      mode = 1;
    } else if (Switch.down()) {
      mode = 2;
    } else if (Switch.left()) {
      mode = 3;
    } else if (Switch.right()) {
      mode = 4;
    }

    switch (mode) {
      case 1: {
        Thread lightning_th(lightning);
        Thread timer_th(timer);
        wait(10);
        lightning_th.terminate();
      } break;
      case 2:
        die();
        break;
      case 3:
        die();
        break;
      case 4:
        die();
        break;
    }
  }
}
