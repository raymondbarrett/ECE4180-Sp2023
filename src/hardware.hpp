/// \file hardware.hpp
/// \date 2023-02-28
/// \author mshakula (matvey@gatech.edu)
///
/// \brief The hardware definitions

#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#ifndef __cplusplus
#error "hardware.hpp is a cxx-only header."
#endif // __cplusplus

#include <mbed.h>
#include <rtos.h>

#include <MODDMA.h>
#include <MSCFileSystem.h>
#include <uLCD_4DGL.h>

// ======================= Public Interface ==========================

#define PIN_R p24
#define PIN_G p23
#define PIN_B p22
#define PIN_MIC p20
#define PIN_SPEAK p18
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

#define LCD_MAX_WIDTH (128)
#define LCD_MAX_HEIGHT LCD_MAX_WIDTH
#define LCD_FONT_WIDTH (7)
#define LCD_FONT_HEIGHT (8)
#define LCD_MAX_TEXT_WIDTH (LCD_MAX_WIDTH / LCD_FONT_WIDTH)
#define LCD_MAX_TEXT_HEIGHT (LCD_MAX_HEIGHT / LCD_FONT_HEIGHT)

// Experimentally found. from nominal 96Mhz.
#define CCK_SPEED (95300000)

/// \brief Nice wrapper for RGB LED.
struct RGB
{
  RGB(PinName r_, PinName g_, PinName b_) : r(r_), g(g_), b(b_) {}

  RGB& operator=(int x)
  {
    r = static_cast<float>((x >> 16) & 0xff);
    g = static_cast<float>((x >> 8) & 0xff);
    b = static_cast<float>((x >> 8) & 0xff);
    return *this;
  }

  mbed::PwmOut r, g, b;
};

/// \brief Nice wrapper for 5-mode switch.
struct Switch
{
  Switch(
    PinName up_,
    PinName down_,
    PinName left_,
    PinName right_,
    PinName center_) :
      up(up_), down(down_), left(left_), right(right_), center(center_)
  {
  }

  bool get_up() { return !up; }
  bool get_down() { return !down; }
  bool get_left() { return !left; }
  bool get_right() { return !right; }
  bool get_center() { return !center; }

  mbed::InterruptIn up, down, left, right, center;
};

extern mbed::BusOut&    OnboardLEDs;
extern struct RGB&      RGB;
extern struct Switch&   Switch;
extern mbed::Serial&    BTInput;
extern mbed::Serial&    PC;
extern uLCD_4DGL&       LCD;
extern mbed::AnalogOut& Speaker; // neccessary for dma to work.
extern MSCFileSystem&   USB;
extern MODDMA&          DMA;

extern rtos::Mutex& LCD_Mutex;

// ===================== Detail Implementation =======================

#endif // HARDWARE_HPP
