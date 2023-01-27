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

/// \brief Context to do nothing.
class DoNothingContext : public DefaultContext
{
 public:
  DoNothingContext() : DefaultContext("DoNothingContext") {}
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
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 1); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 2); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 3); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 4); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 5); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 6); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 7); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 8); },
  [](FunctionContext* c) { FunctionContext::spawn<KillContext>(c, 9); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); },
  [](FunctionContext* c) { FunctionContext::spawn<DoNothingContext>(c); }};

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
