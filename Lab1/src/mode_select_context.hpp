/// \file mode_select_context.hpp
/// \date 2023-01-26
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The top-level DIP mode-switching context manager.

#ifndef MODE_SELECT_CONTEXT_HPP
#define MODE_SELECT_CONTEXT_HPP

#ifndef __cplusplus
#error "mode_select_context.hpp is a cxx-only header."
#endif // __cplusplus

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <array>
#include <new>
#include <utility>

#include <mbed_debug.h>

#include "function_context.hpp"
#include "hardware.hpp"

// ======================= Public Interface ==========================

/// \brief The top-level context to select function for demo.
class ModeSelectContext : public FunctionContext
{
  static constexpr int kDipCount = 4;

  using SpawnFunctions = std::array<void (*)(FunctionContext*), 1 << kDipCount>;

 public:
  ModeSelectContext(const SpawnFunctions& spawn_functions);

  virtual ~ModeSelectContext() noexcept;
  virtual int enter() override;
  virtual int loop() override;
  virtual int idle() override;
  virtual int exit() override;

 private:
  int readDips_()
  {
    int v = 0;
    for (int i = 0; i < kDipCount; ++i) {
      auto dip = reinterpret_cast<mbed::InterruptIn*>(dips_) + i;
      v |= !dip->read() << (kDipCount - 1 - i);
    }
    return v;
  }

  int                   currently_selected_;
  const SpawnFunctions& spawn_funcs_;
  alignas(mbed::InterruptIn) char dips_[sizeof(mbed::InterruptIn) * kDipCount];
};

// ===================== Detail Implementation =======================

ModeSelectContext::ModeSelectContext(
  const SpawnFunctions& spawn_functions) noexcept :
    FunctionContext(),
    currently_selected_{0},
    spawn_funcs_{spawn_functions},
    dips_{0}
{
  ::new (reinterpret_cast<mbed::InterruptIn*>(dips_) + 0)
    mbed::InterruptIn(PIN_MODE_DIP_1, PullUp);
  ::new (reinterpret_cast<mbed::InterruptIn*>(dips_) + 1)
    mbed::InterruptIn(PIN_MODE_DIP_2, PullUp);
  ::new (reinterpret_cast<mbed::InterruptIn*>(dips_) + 2)
    mbed::InterruptIn(PIN_MODE_DIP_3, PullUp);
  ::new (reinterpret_cast<mbed::InterruptIn*>(dips_) + 3)
    mbed::InterruptIn(PIN_MODE_DIP_4, PullUp);

  currently_selected_ = readDips_();

  debug("ModeSelectContext::ModeSelectContext()\n");
}

inline ModeSelectContext::~ModeSelectContext() noexcept
{
  for (int i = 0; i < kDipCount; ++i) {
    auto dip = reinterpret_cast<mbed::InterruptIn*>(dips_) + i;
    dip->~InterruptIn();
  }

  debug("ModeSelectContext::~ModeSelectContext()\n");
}

inline int
ModeSelectContext::enter()
{
  for (int i = 0; i < kDipCount; ++i) {
    auto dip = reinterpret_cast<mbed::InterruptIn*>(dips_) + i;
    dip->fall(nullptr);
    dip->rise(nullptr);
  }

  debug("ModeSelectContext::enter()\n");

  return 0;
}

inline int
ModeSelectContext::loop()
{
  currently_selected_ = readDips_();
  spawn_funcs_[currently_selected_](this);
  idle();
  return 0;
}

inline int
ModeSelectContext::idle()
{
  for (int i = 0; i < kDipCount; ++i) {
    GlobalHardware::OnboardLEDs[i] =
      currently_selected_ & (1 << (kDipCount - 1 - i));
  }
  return 0;
}

inline int
ModeSelectContext::exit()
{
  // IRQ for all changes in the switches.
  auto switch_callback = [&]() {
    // debug("IRQ");
    int val = readDips_();
    if (val != currently_selected_) {
      // debug(" terminate.\n");
      FunctionContext::terminate();
    } else {
      // debug("\n");
    }
  };

  for (int i = 0; i < kDipCount; ++i) {
    auto dip = reinterpret_cast<mbed::InterruptIn*>(dips_) + i;
    dip->fall(switch_callback);
    dip->rise(switch_callback);
  }

  debug("ModeSelectContext::exit()\n");
  return 0;
}

#endif // MODE_SELECT_CONTEXT_HPP
