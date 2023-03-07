/// \file hardware.cpp
/// \date 2023-02-28
/// \author mshakula (matvey@gatech.edu)
///
/// \brief The definitions for global hardware declarations. This structure is
/// handy, because it provides a spot for well-defined initialization code.

#include "hardware.hpp"

#include <mbed.h>

// ======================= Local Definitions =========================

namespace {

mbed::BusOut&
OnboardLEDs_()
{
  static mbed::BusOut led(LED4, LED3, LED2, LED1);
  return led;
}

struct RGB&
RGB_()
{
  static struct RGB rgb(PIN_R, PIN_G, PIN_B);
  return rgb;
}

struct Switch&
Switch_()
{
  // static struct Switch sw(
  // PIN_SW_UP, PIN_SW_DOWN, PIN_SW_LEFT, PIN_SW_RIGHT, PIN_SW_CENTER);

  // Swapped because of orientation on breadboard.
  static struct Switch sw(
    PIN_SW_DOWN, PIN_SW_UP, PIN_SW_RIGHT, PIN_SW_LEFT, PIN_SW_CENTER);
  return sw;
}

mbed::Serial&
BTInput_()
{
  static mbed::Serial bt(PIN_BLE_TX, PIN_BLE_RX);
  return bt;
}

mbed::Serial&
PC_()
{
  static mbed::Serial pc(USBTX, USBRX);
  pc.baud(115200);
  return pc;
}

uLCD_4DGL&
LCD_()
{
  static uLCD_4DGL lcd(PIN_LCD_TX, PIN_LCD_RX, PIN_LCD_RES);
  lcd.baudrate(600000);
  return lcd;
}

mbed::AnalogOut&
Speaker_()
{
  static mbed::AnalogOut speaker(PIN_SPEAK);
  return speaker;
}

MSCFileSystem&
USB_()
{
  static MSCFileSystem usb("usb");
  LPC_SC->PCLKSEL0 |= 0x1 << 6;  // PCLK_UART0 = CCLK
  LPC_SC->PCLKSEL0 |= 0x1 << 8;  // PCLK_UART1 = CCLK
  LPC_SC->PCLKSEL1 |= 0x1 << 16; // PCLK_UART2 = CCLK
  LPC_SC->PCLKSEL1 |= 0x1 << 18; // PCLK_UART3 = CCLK
  return usb;
}

MODDMA&
DMA_()
{
  static MODDMA dma;
  return dma;
}

rtos::Mutex&
LCD_Mutex_()
{
  static rtos::Mutex mutex;
  return mutex;
}

} // namespace

// ====================== Global Definitions =========================

mbed::BusOut&    OnboardLEDs = OnboardLEDs_();
struct RGB&      RGB         = RGB_();
struct Switch&   Switch      = Switch_();
mbed::Serial&    BTInput     = BTInput_();
mbed::Serial&    PC          = PC_();
uLCD_4DGL&       LCD         = LCD_();
mbed::AnalogOut& Speaker     = Speaker_();
MSCFileSystem&   USB         = USB_();
MODDMA&          DMA         = DMA_();

rtos::Mutex& LCD_Mutex = LCD_Mutex_();
