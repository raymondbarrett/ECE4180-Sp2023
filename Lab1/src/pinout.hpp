/// \file src/pinout.hpp
/// \brief Define the I/O pins for project.

#ifndef PINOUT_HPP
#define PINOUT_HPP

/// \brief Enumerate the possible used pins.
enum class Pins : int
{
  // Mode-switching DIP switches for function selection.
  kModeDip1 = 7,
  kModeDip1 = 8,
  kModeDip1 = 9,
  kModeDip1 = 10,

  // Generic push-button switches.
  kPushButtonA = 5,
  kPushButtonB = 6,

  // RGB LED pins.
  kRGB_R = 21,
  kRGB_G = 22,
  kRGB_B = 23,

  // RGB DIP.
  kRGBDip1 = 17,
  kRGBDip2 = 19,
  kRGBDip3 = 20,

  // Navigation switch pins (TODO).
  kNavSwitch1 = 24,
  kNavSwitch1 = 25,
  kNavSwitch1 = 26,
  kNavSwitch1 = 29,
  kNavSwitch1 = 30,

  // I2C pins (TOOD)
  // kI2C_SDA = 27,
  // kI2C_SCL = 28,
  kI2C_IRQ = 29,

  // SPI pins (TODO),
  // SPI 11-13
  // RESET 14
  // CS 15

  // Potentiometer and analog.
  kPot    = 16,
  kAnalog = 18
};

#endif // PINOUT_HPP
