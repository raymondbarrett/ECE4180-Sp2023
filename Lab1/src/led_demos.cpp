/// \file led_demos.cpp
/// \date 2023-01-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The Basic I/O (parts 1-3) context definitions.

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include "led_demos.hpp"

#include <algorithm>

#include <mbed.h>

#include "hardware.hpp"

// ======================= Local Definitions =========================

// ====================== Global Definitions =========================

#pragma region Checkoff1Context

Checkoff1Context::Checkoff1Context() noexcept :
    DefaultContext("Checkoff1Context"),
    button_(PIN_PUSH_BUTTON_A, PullUp),
    led_(PIN_RGB_R)
{
}

int
Checkoff1Context::loop()
{
  led_ = !button_;
  return 0;
}

#pragma endregion Checkoff1Context
#pragma region    Checkoff2Context

Checkoff2Context::Checkoff2Context() noexcept :
    DefaultContext("Checkoff2Context"),
    led_(PIN_RGB_R),
    buttonA_(PIN_PUSH_BUTTON_A, PullUp),
    buttonB_(PIN_PUSH_BUTTON_B, PullUp),
    dimness_(0)
{
  constexpr auto kDebounceTime = std::chrono::milliseconds(1);

  buttonA_.fall([&]() {
    static mbed::Timer timer;
    if (timer.elapsed_time() > kDebounceTime) {
      timer.reset();
      dimness_ = std::min(dimness_ + .05, 1.);
    }
  });
  buttonB_.fall([&]() {
    static mbed::Timer timer;
    if (timer.elapsed_time() > kDebounceTime) {
      timer.reset();
      dimness_ = std::max(dimness_ - .05, 0.);
    }
  });
}

int
Checkoff2Context::loop()
{
  led_ = dimness_;
  return 0;
}

#pragma endregion Checkoff2Context
#pragma region    Checkoff3Context

Checkoff3Context::Checkoff3Context() noexcept :
    DefaultContext("Checkoff3Context"),
    r_(PIN_RGB_R),
    g_(PIN_RGB_G),
    b_(PIN_RGB_B),
    dip_(PIN_RGB_DIP_1, PIN_RGB_DIP_2, PIN_RGB_DIP_3),
    buttonA_(PIN_PUSH_BUTTON_A, PullUp),
    buttonB_(PIN_PUSH_BUTTON_B, PullUp),
    dimness_{0}
{
  constexpr auto kDebounceTime = std::chrono::milliseconds(1);
  dip_.mode(PullUp);
  buttonA_.fall([&]() {
    static mbed::Timer timer;
    if (timer.elapsed_time() > kDebounceTime) {
      timer.reset();
      for (int i = 0; i < 3; ++i)
        if (!dip_[i])
          dimness_[i] = std::min(dimness_[i] + .05, 1.);
    }
  });
  buttonB_.fall([&]() {
    static mbed::Timer timer;
    if (timer.elapsed_time() > kDebounceTime) {
      timer.reset();
      for (int i = 0; i < 3; ++i)
        if (!dip_[i])
          dimness_[i] = std::max(dimness_[i] - .05, 0.);
    }
  });
}

int
Checkoff3Context::loop()
{
  r_ = dimness_[0];
  g_ = dimness_[1];
  b_ = dimness_[2];
  return 0;
}

#pragma endregion Checkoff3Context
