/// \file hardware.cpp
/// \date 2023-01-26
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The static definitions of forward-declared hardware.

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include "hardware.hpp"

#include <mbed.h>

// ======================= Local Definitions =========================

namespace {

} // namespace

// ====================== Global Definitions =========================

namespace GlobalHardware {

std::array<mbed::DigitalOut, 4> OnboardLEDs{
  mbed::DigitalOut{LED1},
  mbed::DigitalOut{LED2},
  mbed::DigitalOut{LED3},
  mbed::DigitalOut{LED4}};

}; // namespace GlobalHardware
