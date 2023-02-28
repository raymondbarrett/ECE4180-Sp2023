/// \file hardware.cpp
/// \date 2023-02-28
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The definitions for global hardware dclaratsions.

#include "hardware.hpp"

// ======================= Local Definitions =========================

namespace {

} // namespace

// ====================== Global Definitions =========================

mbed::BusOut OnboardLEDs(LED4, LED3, LED2, LED1);

// MAYBE SWITCH AROUND THE ORDER TO MAKE IT ALIGN PROPERLY... -MS
DirectionSwitch
  Switch(PIN_SW_UP, PIN_SW_DOWN, PIN_SW_LEFT, PIN_SW_RIGHT, PIN_SW_CENTER);
