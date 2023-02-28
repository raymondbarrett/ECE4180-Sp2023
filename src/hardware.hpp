/// \file hardware.hpp
/// \date 2023-02-28
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The hardware definitions

#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#ifndef __cplusplus
#error "hardware.hpp is a cxx-only header."
#endif // __cplusplus

#include <mbed.h>

// ======================= Public Interface ==========================

#define PIN_R p24
#define PIN_G p23
#define PIN_B p22
#define PIN_MIC p20
#define PIN_SPEAK p21
#define PIN_SW_RIGHT p29 // SWAP THESE FOR DIRECTINOS.
#define PIN_SW_DOWN p28
#define PIN_SW_LEFT p27
#define PIN_SW_CENTER p26
#define PIN_SW_UP p25
#define PIN_LCD_RES p30
#define PIN_LCD_RX p14
#define PIN_LCD_TX p13
#define PIN_BLE_RX p10
#define PIN_BLE_TX p9

#define ONBOARD_LED_COUNT (4)

class DirectionSwitch
{
 public:
  /// \brief Construct the switch class with these pins.
  DirectionSwitch(
    mbed::PinName u,
    mbed::PinName d,
    mbed::PinName l,
    mbed::PinName r,
    mbed::PinName c) :
      switches_(u, d, l, r, c)
  {
    switches_.pinMode(PullUp)
  }

  bool up() { return !switches_[0]; }
  bool down() { return !switches_[1]; }
  bool left() { return !switches_[2]; }
  bool right() { return !switches_[3]; }
  bool center() { return !switches_[4]; }

 private:
  mbed::BusIn switches_;
};

extern mbed::BusOut    OnboardLEDs;
extern DirectionSwitch Switch;

// ===================== Detail Implementation =======================

#endif // HARDWARE_HPP
