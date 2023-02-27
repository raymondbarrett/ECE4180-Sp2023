/// \file mode_select_context.hpp
/// \date 2023-02-20
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

#include "function_context.hpp"
#include "global_hardware.hpp"

// ======================= Public Interface ==========================

/// \brief The top-level context to select function for demo.
class ModeSelectContext : public DefaultContext
{
  static constexpr int kDipCount = 3;

  using SpawnFunctions = std::array<void (*)(FunctionContext*), 1 << kDipCount>;

 public:
  ModeSelectContext(int depth, const SpawnFunctions& spawn_functions);

  virtual ~ModeSelectContext() noexcept;
  virtual int enter() override;
  virtual int loop() override;
  virtual int exit() override;

 protected:
  virtual const char* contextName() const noexcept
  {
    return "ModeSelectContext";
  }

 private:
  int readDips_()
  {
    int v = 0;
    for (int i = 0; i < kDipCount; ++i)
      v |= !reinterpret_cast<mbed::InterruptIn*>(&dips_[i])->read()
        << (kDipCount - 1 - i);
    return v;
  }

  int                   currently_selected_;
  const SpawnFunctions& spawn_funcs_;
  std::aligned_storage_t<sizeof(mbed::InterruptIn), alignof(mbed::InterruptIn)>
    dips_[kDipCount];
};

// ===================== Detail Implementation =======================

ModeSelectContext::ModeSelectContext(
  int                   depth,
  const SpawnFunctions& spawn_functions) noexcept :
    DefaultContext(contextName(), depth),
    currently_selected_{0},
    spawn_funcs_{spawn_functions}
{
  ::new (&dips_[0]) mbed::InterruptIn(MODE_DIP_P1, PullUp);
  ::new (&dips_[1]) mbed::InterruptIn(MODE_DIP_P2, PullUp);
  ::new (&dips_[2]) mbed::InterruptIn(MODE_DIP_P3, PullUp);
  // ::new (dips_ + 3) mbed::InterruptIn(MODE_DIP_P4, PullUp);

  currently_selected_ = readDips_();
}

inline ModeSelectContext::~ModeSelectContext() noexcept
{
  for (int i = 0; i < kDipCount; ++i) {
    auto dip = reinterpret_cast<mbed::InterruptIn*>(dips_) + i;
    dip->~InterruptIn();
  }
}

inline int
ModeSelectContext::enter()
{
  DefaultContext::enter();

  for (int i = 0; i < kDipCount; ++i) {
    auto dip = reinterpret_cast<mbed::InterruptIn*>(dips_) + i;
    dip->fall(nullptr);
    dip->rise(nullptr);
  }

  return 0;
}

inline int
ModeSelectContext::loop()
{
  currently_selected_         = readDips_();
  GlobalHardware::OnboardLEDs = currently_selected_;
  spawn_funcs_[currently_selected_](this);
  return 0;
}

inline int
ModeSelectContext::exit()
{
  // IRQ for all changes in the switches.
  auto switch_callback = [&]() {
    int val = readDips_();

    if (val != currently_selected_)
      FunctionContext::terminate();
  };

  for (int i = 0; i < kDipCount; ++i) {
    auto dip = reinterpret_cast<mbed::InterruptIn*>(dips_) + i;
    dip->fall(switch_callback);
    dip->rise(switch_callback);
  }

  DefaultContext::exit();

  return 0;
}

#endif // MODE_SELECT_CONTEXT_HPP
