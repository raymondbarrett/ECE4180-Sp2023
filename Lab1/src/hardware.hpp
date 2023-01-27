/// \file hardware.hpp
/// \date 2023-01-24
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief Specify the hardware setup + hardware that always exists.

#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#ifndef __cplusplus
#error "hardware.hpp is a cxx-only header."
#endif // __cplusplus

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <array>
#include <chrono>

#include <drivers/BusOut.h>
#include <drivers/Timer.h>
#include <platform.h>

// ======================= Public Interface ==========================

#define PIN_MODE_DIP_4 p6
#define PIN_MODE_DIP_3 p8
#define PIN_MODE_DIP_2 p9
#define PIN_MODE_DIP_1 p10

#define PIN_PUSH_BUTTON_A p7
#define PIN_PUSH_BUTTON_B p5

#define PIN_RGB_B p21
#define PIN_RGB_G p22
#define PIN_RGB_R p23

#define PIN_RGB_DIP_1 p17
#define PIN_RGB_DIP_2 p19
#define PIN_RGB_DIP_3 p20

#define PIN_POT p16
#define PIN_ANALOG p18

/// \brief Enumerate the globally-accessible hardware that requires no
/// configuration.
namespace GlobalHardware {

constexpr std::size_t kOnboardLEDsCount = 4;
extern mbed::BusOut   OnboardLEDs;

} // namespace GlobalHardware

/*
/// \brief Enumerate the possible used pins.
enum class Pins : int
{
  // Mode-switching DIP switches for function selection.
  kModeDip4 = p6,  // SW4 - LSB
  kModeDip3 = p8,  // SW3 - LSB + 1
  kModeDip2 = p9,  // SW2 - MSB - 1
  kModeDip1 = p10, // SW1 - MSB

  // Generic push-button switches.
  kPushButtonA = p7,
  kPushButtonB = p5,

  // RGB LED pins.
  kRGB_B = p21,
  kRGB_G = p22,
  kRGB_R = p23,

  // RGB DIP.
  kRGBDip1 = p17, // SW1
  kRGBDip2 = p19, // Sw2
  kRGBDip3 = p20, // SW3

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
  kPot    = p16,
  kAnalog = p18
};
*/

/// \brief A debounce-handling function wrapper for button inputs.
template<class Callback>
struct DebounceWrapper
{
  static constexpr auto kDebounceTime = std::chrono::milliseconds(5);

  static mbed::Timer timer;

  Callback t_;

  DebounceWrapper(const Callback& t) : t_(t) {}

  void operator()()
  {
    if (timer.elapsed_time() > kDebounceTime) {
      timer.reset();
      t_();
    }
  }
};

// ===================== Detail Implementation =======================

#endif // HARDWARE_HPP
