/// \file main.cpp
/// \date 2023-01-24
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main entry-point.

#if !defined(__cplusplus) || __cplusplus < 201402L
#error "C++14 (max supported by online compiler) required."
#endif

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <mbed.h>

#include "function_context.hpp"
#include "hardware.hpp"
#include "mode_select_context.hpp"

// ======================= Local Definitions =========================

namespace {

class DoNothingContext : public FunctionContext
{
 public:
  DoNothingContext()                   = default;
  virtual ~DoNothingContext() noexcept = default;
  virtual int enter() override { return 0; }
  virtual int loop() override { return 0; }
  virtual int idle() override { return 0; }
  virtual int exit() override { return 0; }
};

class KillContext : public DoNothingContext
{
 public:
  virtual int enter() override { return 1; }
};

int __attribute__((noreturn)) die(int code)
{
  using GlobalHardware::OnboardLEDs;

  printf("PROCESS TERMINATED WITH CODE: %i.\n", code);
  int i = 0;

  // A bouncy animation.
  do {
    for (auto& x : OnboardLEDs) {
      x = false;
    }
    OnboardLEDs
      [i < OnboardLEDs.size()
         ? i
         : OnboardLEDs.size() - i % OnboardLEDs.size() - 2] = true;
    i = (i + 1) % (OnboardLEDs.size() * 2 - 2);
    wait_us(25'000);
  } while (true);
}

std::array<void (*)(FunctionContext*), 1 << 4> function_context_spawners = {
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
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c); }};

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
