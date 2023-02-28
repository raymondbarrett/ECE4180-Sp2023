/// \file hardware.cpp
/// \date 2023-02-28
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The definitions for global hardware dclaratsions.

#include "hardware.hpp"

// ======================= Local Definitions =========================

namespace {

uLCD_4DGL&
getLCD()
{
  static uLCD_4DGL lcd(PIN_LCD_TX, PIN_LCD_RX, PIN_LCD_RES);
  lcd.baudrate(3000000);
  return lcd;
}

} // namespace

// ====================== Global Definitions =========================

mbed::BusOut OnboardLEDs(LED4, LED3, LED2, LED1);

// MAYBE SWITCH AROUND THE ORDER TO MAKE IT ALIGN PROPERLY... -MS
DirectionSwitch
  Switch(PIN_SW_UP, PIN_SW_DOWN, PIN_SW_LEFT, PIN_SW_RIGHT, PIN_SW_CENTER);

mbed::PwmOut
  RGB[](mbed::PwmOut(PIN_R), mbed::PwmOut(PIN_G), mbed::PwmOut(PIN_B));

mbed::Serial BTInput(PIN_BLE_TX, PIN_BLE_RX);

uLCD_4DGL& LCD = getLCD();
