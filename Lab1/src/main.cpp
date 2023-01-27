/// \file main.cpp
/// \date 2023-01-24
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main entry-point.

#if !defined(__cplusplus) || __cplusplus < 201402L
#error "C++14 (max supported by online compiler) required."
#endif

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <chrono>

#include <mbed.h>

#include "function_context.hpp"
#include "hardware.hpp"
#include "led_demos.hpp"
#include "mode_select_context.hpp"

// ======================= Local Definitions =========================

namespace {

/// \brief Once increment of a bouncy onboard LED function.
int
bouncy(int i)
{
  using namespace GlobalHardware;

  OnboardLEDs = 0;

  const int index =
    i < kOnboardLEDsCount ? i : kOnboardLEDsCount - i % kOnboardLEDsCount - 2;

  OnboardLEDs[index] = true;
  return (i + 1) % (kOnboardLEDsCount * 2 - 2);
}

/// \brief Context to do nothing.
class DoNothingContext : public DefaultContext
{
 public:
  DoNothingContext() : DefaultContext("DoNothingContext"), c_(), i_{0}
  {
    c_.start();
  }

  virtual int loop() override
  {
    using namespace std::chrono_literals;
    if (c_.read_us() > 100'000) {
      i_ = bouncy(i_);
      c_.reset();
    }
    return 0;
  }
  virtual int exit() override
  {
    DefaultContext::exit();
    return 0;
  }

 private:
  mbed::Timer c_;
  int         i_;
};

/// \brief Abnormally terminate the main context loop upon entry.
class KillContext : public DefaultContext
{
  int code_;

 public:
  KillContext(int code) : DefaultContext("KillContext"), code_{code} {}
  virtual int enter() override
  {
    DefaultContext::enter();
    return code_;
  }
};

int __attribute__((noreturn)) die(int code)
{
  printf("PROCESS TERMINATED WITH CODE: %i.\n", code);
  int i = 0;

  // A bouncy animation.
  do {
    i = bouncy(i);
    wait_us(25'000);
  } while (true);
}

std::array<void (*)(FunctionContext*), 1 << 4> function_context_spawners = {
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<Checkoff1Context>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<Checkoff2Context>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<Checkoff3Context>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 0xff); }};

} // namespace

// ====================== Global Definitions =========================

extern "C" int __attribute__((noreturn)) main()
{
  // Set up mode-switching DIPS.
  FunctionContext::spawn<ModeSelectContext>(nullptr, function_context_spawners);

  // Transfer control to FunctionContext loop.
  FunctionContext::start();

  // Die forever.
  die(0);
}
