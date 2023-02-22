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

#include <cstdlib>

#include "mbed.h"

#include "LSM9DS1.h"

#include "function_context.hpp"
#include "global_hardware.hpp"

// ======================= Public Interface ==========================

class IMUContext : public DefaultContext
{
  LSM9DS1     lol_;
  mbed::Timer timer_;

 public:
  IMUContext(int depth) :
      DefaultContext("IMUContext", depth),
      lol_(I2C_SDA, I2C_SCL, 0xD6, 0x3C),
      timer_()
  {
  }

  virtual ~IMUContext() = default;

  virtual int enter() override
  {
    timer_.start();
    if (!lol_.begin()) {
      debug("ERROR INITIALIZING IMU.");
      return 1;
    }
    lol_.calibrate();
    return 0;
  }

  virtual int loop() override
  {
    if (timer_.read_us() > 1'000'000) {
      lol_.readTemp();
      lol_.readMag();
      lol_.readGyro();

      std::printf("gyro: %d %d %d\n\r", lol_.gx, lol_.gy, lol_.gz);
      std::printf("accel: %d %d %d\n\r", lol_.ax, lol_.ay, lol_.az);
      std::printf("mag: %d %d %d\n\n\r", lol_.mx, lol_.my, lol_.mz);

      timer_.reset();
    }
    return 0;
  }
};

// ===================== Detail Implementation =======================

#endif // ASSIGNMENT_HPP
