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

mbed::BusOut OnboardLEDs(LED4, LED3, LED2, LED1);

}; // namespace GlobalHardware
