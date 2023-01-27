/// \file led_demos.hpp
/// \date 2023-01-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The Basic I/O (pats 1-3) context declarations.

#ifndef LED_DEMOS_HPP
#define LED_DEMOS_HPP

#ifndef __cplusplus
#error "led_demos.hpp is a cxx-only header."
#endif // __cplusplus

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <mbed.h>

#include "function_context.hpp"

// ======================= Public Interface ==========================

class Checkoff1Context : public DefaultContext
{
 public:
  Checkoff1Context() noexcept;
  virtual int loop() override;

 private:
  mbed::DigitalIn  button_;
  mbed::DigitalOut led_;
};

class Checkoff2Context : public DefaultContext
{
 public:
  Checkoff2Context() noexcept;
  virtual int loop() override;

 private:
  mbed::PwmOut      led_;
  mbed::InterruptIn buttonA_;
  mbed::InterruptIn buttonB_;
  float             dimness_;
};

class Checkoff3Context : public DefaultContext
{
 public:
  Checkoff3Context() noexcept;
  virtual int loop() override;

 private:
  mbed::PwmOut      r_;
  mbed::PwmOut      g_;
  mbed::PwmOut      b_;
  mbed::BusIn       dip_;
  mbed::InterruptIn buttonA_;
  mbed::InterruptIn buttonB_;
  float             dimness_[3];
};

// ===================== Detail Implementation =======================

#endif // LED_DEMOS_HPP
