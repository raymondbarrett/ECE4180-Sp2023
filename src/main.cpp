/// \file main.cpp
/// \date 2023-02-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main file.

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <mbed.h>
#include <rtos.h>

#include "hardware.hpp"

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

} // namespace

// ====================== Global Definitions =========================

int
main()
{
  mbed::PwmOut r(PIN_R);
  mbed::PwmOut g(PIN_G);
  mbed::PwmOut b(PIN_B);

  r = 1.0;
  g = 0.1;
  b = 0.5;

  die();
}
