/// \file assignment.hpp
/// \date 2023-02-21
/// \author mshakula (mshakula3)
///
/// \brief The implementation for all of this parts actual assignment things.

#ifndef ASSIGNMENT_HPP
#define ASSIGNMENT_HPP

#ifndef __cplusplus
#error "assignment.hpp is a cxx-only header."
#endif // __cplusplus

#include <cmath>
#include <cstdlib>

#include "mbed.h"

#include "LSM9DS1.h"
#include "XNucleo53L0A1.h"
#include "uLCD_4DGL.h"

#include "function_context.hpp"
#include "global_hardware.hpp"

// ======================= Public Interface ==========================

class IMUContext : public DefaultContext
{
 protected:
  LSM9DS1     lol_;
  mbed::Timer timer_;

 public:
  IMUContext(int depth) :
      DefaultContext(contextName(), depth),
      lol_(I2C_SDA, I2C_SCL, 0xD6, 0x3C),
      timer_()
  {
  }

  virtual ~IMUContext() = default;

  virtual int enter() override
  {
    DefaultContext::enter();

    timer_.start();
    if (!lol_.begin())
      return 1;
    lol_.calibrate();
    return 0;
  }

  virtual int loop() override
  {
    if (timer_.read_us() > 1'000'000) {
      lol_.readAccel();
      lol_.readMag();
      lol_.readGyro();

      std::printf("gyro: %d %d %d\n\r", lol_.gx, lol_.gy, lol_.gz);
      std::printf("accel: %d %d %d\n\r", lol_.ax, lol_.ay, lol_.az);
      std::printf("mag: %d %d %d\n\n\r", lol_.mx, lol_.my, lol_.mz);

      timer_.reset();
    }
    return 0;
  }

  virtual const char* contextName() const noexcept override
  {
    return "IMUContext";
  }
};

class IMULCDContext : public IMUContext
{

 protected:
  uLCD_4DGL            lcd_; // LCD Screen (tx, rx, reset)
  static constexpr int kBGColor = 0x808080;

 public:
  IMULCDContext(int depth) : IMUContext(depth), lcd_(LCD_TX, LCD_RX, LCD_RST)
  {
    lcd_.baudrate(119600);
  }

  virtual const char* contextName() const noexcept override
  {
    return "IMULCDContext";
  }

  virtual int enter() override
  {
    int ret = 0;
    if (ret = IMUContext::enter())
      return ret;

    lcd_.background_color(kBGColor);
    lcd_.cls();
    return ret;
  }

  virtual int exit() override
  {
    int ret;
    lcd_.background_color(0x000000);
    lcd_.cls();

    if (ret = IMUContext::exit())
      return ret;
    return ret;
  }
};

class IMULCDLevelContext : public IMULCDContext
{
  static constexpr int kLCDSize    = 128;
  static constexpr int kBallRadius = 10;

  int x, y;

  mbed::Timer time_;

 public:
  IMULCDLevelContext(int depth) : IMULCDContext(depth), x{0}, y{0}, time_{} {}

  virtual const char* contextName() const noexcept
  {
    return "IMULCDLevelContext";
  }

  virtual int enter() override
  {
    int ret = 0;
    if (ret = IMULCDContext::enter())
      return ret;

    time_.start();
    return ret;
  }

  virtual int loop() override
  {
    if (timer_.read_us() > 10'000) {
      lol_.readAccel();
      lol_.readMag();
      lol_.readGyro();

      auto ax =
        std::atan(lol_.ax / std::sqrt(lol_.ay * lol_.ay + lol_.az * lol_.az)) /
        (3.14159 / 2);
      auto ay =
        std::atan(lol_.ay / std::sqrt(lol_.ax * lol_.ax + lol_.az * lol_.az)) /
        (3.14159 / 2);

      int xn = ax * (kLCDSize - 2 * kBallRadius) / 2 + kLCDSize / 2;
      int yn = ay * (kLCDSize - 2 * kBallRadius) / 2 + kLCDSize / 2;

      if (xn != x || yn != y) {
        lcd_.filled_circle(x, y, kBallRadius, kBGColor);
        lcd_.filled_circle(xn, yn, kBallRadius, 0xff0020);
      }

      x = xn;
      y = yn;
      timer_.reset();
    }
    return 0;
  }
};

class IMULCDCompassContext :
    // public IMUContext
    public IMULCDLevelContext
{
 public:
  IMULCDCompassContext(int depth) : IMULCDLevelContext(depth) {}

  // TODO.
};

class MemsContext : public DefaultContext
{
  mbed::AnalogIn pin_;
  mbed::Timer    timer_;

 public:
  const char* contextName() const noexcept override { return "MemsContext"; }

  MemsContext(int depth) :
      DefaultContext(contextName(), depth), pin_(MEMS_IN), timer_{}
  {
  }

  virtual int enter() override
  {
    DefaultContext::enter();
    timer_.start();
    GlobalHardware::OnboardLEDs = 0;
    return 0;
  }

  virtual int loop() override
  {
    if (timer_.read_us() > 100'000) {
      GlobalHardware::OnboardLEDs =
        int(abs((pin_.read() - (0.67 / 3.3))) * 500.0);
      timer_.reset();
    }
    return 0;
  }
};

class TOFContext : public DefaultContext
{
  DevI2C*        i2c_;
  XNucleo53L0A1* tof_;
  DigitalOut     rst_;

 public:
  const char* contextName() const noexcept override { return "TOFContext"; }

  TOFContext(int depth) :
      DefaultContext(contextName(), depth),
      i2c_(new DevI2C(I2C_SDA, I2C_SCL)),
      tof_(XNucleo53L0A1::instance(i2c_, A2, D8, D2)),
      rst_(TOF_XSHUT)
  {
    rst_ = 0;
    wait(0.5);
    rst_ = 1;
    wait(0.5);
    int status = tof_->init_board();
    while (status) {
      if (status) {
        printf("Failed to init board\r\n");
        // status = tof_->init_board();
      }
    }
  }

  int loop() override
  {
    std::uint32_t distance;
    int           status = tof_->sensor_centre->get_distance(&distance);
    if (status == VL53L0X_ERROR_NONE) {
      printf("D=%ld mm\r\n", distance);
    }
    return 0;
  }
};

// ===================== Detail Implementation =======================

#endif // ASSIGNMENT_HPP
