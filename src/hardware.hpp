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
#include <mpr121.h>
#include <uLCD_4DGL.h>

// ======================= Public Interface ==========================

#define PIN_R p24
#define PIN_G p23
#define PIN_B p22
#define PIN_SPEAK p18
#define PIN_SW_RIGHT p11 // SWAP THESE FOR DIRECTINOS.
#define PIN_SW_DOWN p8
#define PIN_SW_LEFT p7
#define PIN_SW_CENTER p6
#define PIN_SW_UP p5
#define PIN_LCD_RES p30
#define PIN_LCD_RX p14
#define PIN_LCD_TX p13
#define PIN_BLE_RX p10
#define PIN_BLE_TX p9
#define PIN_SDA p28
#define PIN_SCL p27
#define PIN_TOUCH_IRQ p29

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

/// \brief Keypad wrapper.
struct Touchpad
{
  static void irq(Touchpad* tp)
  {
    int key_code = 0;
    int i        = 0;
    int value    = tp->mpr.read(0x00);
    value += tp->mpr.read(0x01) << 8;
    // LED demo mod by J. Hamblen
    // pc.printf("MPR value: %x \r\n", value);
    i = 0;
    // puts key number out to LEDs for demo
    for (i = 0; i < 12; i++) {
      if (((value >> i) & 0x01) == 1)
        key_code = i + 1;
    }
  }

  Touchpad(mbed::I2C* i2c, Mpr121::Address addr, PinName interrupt_) :
      mpr(i2c, addr), interrupt(interrupt_)
  {
    interrupt.mode(PullUp);
    interrupt.attach(mbed::callback(irq, this));
  }

  void test()
  {
    printf("\nHello from the mbed & mpr\n\r");

    unsigned char dataArray[2];
    int           key;
    int           count = 0;

    printf("Test 1: read a value: \r\n");
    dataArray[0] = mpr.read(AFE_CFG);
    printf("Read value=%x\r\n\n", dataArray[0]);

    printf("Test 2: read a value: \r\n");
    dataArray[0] = mpr.read(0x5d);
    printf("Read value=%x\r\n\n", dataArray[0]);

    printf("Test 3: write & read a value: \r\n");
    mpr.read(ELE0_T);
    mpr.write(ELE0_T, 0x22);
    dataArray[0] = mpr.read(ELE0_T);
    printf("Read value=%x\r\n\n", dataArray[0]);

    printf("Test 4: Write many values: \r\n");
    unsigned char data[] = {0x1, 0x3, 0x5, 0x9, 0x15, 0x25, 0x41};
    mpr.writeMany(0x42, data, 7);

    // Now read them back ..
    key   = 0x42;
    count = 0;
    while (count < 7) {
      char result = mpr.read(key);
      key++;
      count++;
      printf("Read value: '%x'=%x\n\r", key, result);
    }

    printf("Test 5: Read Electrodes:\r\n");
    key   = ELE0_T;
    count = 0;
    while (count < 24) {
      char result = mpr.read(key);
      printf("Read key:%x value:%x\n\r", key, result);
      key++;
      count++;
    }
    printf("--------- \r\n\n");

    // mpr.setProximityMode(true);

    printf("ELE_CFG=%x", mpr.read(ELE_CFG));

    interrupt.fall(&fallInterrupt);
    interrupt.mode(PullUp);
  }

  /// \return number of read.
  int readAll(bool* loc)
  {
    mpr.readTouchData();
    int ct = 0;
    for (int i = 0; i < 12; ++i) {
      bool a = read_(i);
      loc[i] = a;
      ct += a;
    }
    return ct;
  }

  /// \return if read,
  /// \param
  char read(int key)
  {
    mpr.readTouchData();
    return read_(key);
  }

  Mpr121            mpr;
  mbed::InterruptIn interrupt;

 private:
  char read_(int key) { return mpr.read(ELE0_T + 2 * key); }
};

extern mbed::BusOut&    OnboardLEDs;
extern struct RGB&      RGB;
extern struct Switch&   Switch;
extern mbed::Serial&    BTInput;
extern mbed::Serial&    PC;
extern uLCD_4DGL&       LCD;
extern mbed::AnalogOut& Speaker; // neccessary for dma to work.
extern mbed::AnalogIn&  Mic;
extern MSCFileSystem&   USB;
extern MODDMA&          DMA;
extern struct Touchpad& Touchpad;

extern rtos::Mutex& LCD_Mutex;

// ===================== Detail Implementation =======================

#endif // HARDWARE_HPP
