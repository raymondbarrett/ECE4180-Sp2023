/// \file src/hardware.hpp
///
/// \brief Basic hardware support code.

#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#ifndef __cplusplus
#error "hardware.hpp is a c++-only header."
#endif // __cplusplus

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <chrono>

#include <drivers/Timer.h>

/// \brief Enumerate the possible used pins.
enum class Pins : int
{
  // Mode-switching DIP switches for function selection.
  kModeDip4 = 6,  // SW4 - LSB
  kModeDip3 = 8,  // SW3 - LSB + 1
  kModeDip2 = 9,  // SW2 - MSB - 1
  kModeDip1 = 10, // SW1 - MSB

  // Generic push-button switches.
  kPushButtonA = 7,
  kPushButtonB = 5,

  // RGB LED pins.
  kRGB_B = 21,
  kRGB_G = 22,
  kRGB_R = 23,

  // RGB DIP.
  kRGBDip1 = 17, // SW1
  kRGBDip2 = 19, // Sw2
  kRGBDip3 = 20, // SW3

  // Navigation switch pins (TODO).
  // kNavSwitchX = 24,
  // kNavSwitchX = 25,
  // kNavSwitchX = 26,
  // kNavSwitchX = 29,
  // kNavSwitchX = 30,

  // I2C pins (TOOD)
  // kI2C_SDA = 27,
  // kI2C_SCL = 28,
  // kI2C_IRQ = 29,

  // SPI pins (TODO),
  // SPI 11-13
  // RESET 14
  // CS 15

  // Potentiometer and analog.
  kPot    = 16,
  kAnalog = 18
};

/// \brief A debounce-handling function wrapper for button inputs.
template<void (*Callback)()>
struct DebounceWrapper
{
  static constexpr auto kDebounceTime = std::chrono::milliseconds(5);

  static mbed::Timer timer;

  void operator()()
  {
    if (timer.elapsed_time() > kDebounceTime) {
      timer.reset();
      Callback();
    }
  }
};

#endif // HARDWARE_HPP
