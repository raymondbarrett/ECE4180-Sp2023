/// \file function_context.hpp
/// \date 2023-01-24
/// \author mshakula (mshakula3@gatech.edu)
/// \version 0.2.0
///
/// \brief Bare-level task primitive for configurable layered
/// architecture support.

#ifndef FUNCTION_CONTEXT_HPP
#define FUNCTION_CONTEXT_HPP

#ifndef __cplusplus
#error "function_context.hpp is a cxx-only header."
#endif // __cplusplus

#include <cstddef>

#include <new>
#include <type_traits>
#include <utility>

// ======================= Public Interface ==========================

/// \brief A function context for the demo. Handles the setup, destruction, and
/// everything in between of a context.
///
/// A bare-level task primitive, since this project targets the MbedOS
/// Bare-metal profile. In future, this could be probably adapted to an even
/// lower level, but that has its complications.
///
/// All contexts should inherit from this.
class FunctionContext
{
 public:
  /// \brief Create the context-running loop. Should only be called from
  /// main thread. Non-reentrant. Runs until top-most context terminates.
  ///
  /// Once this takes control, it takes absolute control over IRQ state, and
  /// will disable/reenable them as it sees fit.
  static void start();

  /// \brief Spawn a new context and schedule for future execution. Interrupt
  /// safe, and can be called within interrupt. May fail due to prior context
  /// spawns.
  ///
  /// \note Disables IRQ until processed.
  ///
  /// \tparam Context the context type which needs to be instantiated.
  /// \tparam Ts context construction parameters.
  ///
  /// \param expected_current_context the expected context that is spawning the
  /// new one. Ignores parameter if it is nullptr.
  /// \param params the parameters to pass to Context construction.
  ///
  /// \return The spawned FunctionContext*, null if was not able to.
  template<class Context, class... Ts>
  static auto spawn(FunctionContext* expected_current_context, Ts&&... params)
    -> std::enable_if_t<
      std::is_base_of<FunctionContext, Context>::value,
      FunctionContext*>;

  /// \brief Terminates the currently-running (latched) context. Does not fail.
  ///
  /// \note Disables IRQ until processed.
  static void terminate();

 protected:
  /// \brief Should construct the invariants that are required for the
  /// context to exist. In most cases, this should only be the memory
  /// allocation.
  ///
  /// May run in signal handler, but runs in safe block. May not be called in
  /// FIQ.
  ///
  /// On construction, the context is considered ready in queue.
  FunctionContext() noexcept = default;

  /// \brief Runs on final destruction of object. Context ceases to exist.
  ///
  /// Runs in safe block.
  virtual ~FunctionContext() noexcept = default;

  /// \brief Enter the current context, acquiring any necessary for normal
  /// operation resources.
  ///
  /// Runs in a safe block. May not #spawn.
  ///
  /// \return non-zero return code on success.
  virtual int enter() = 0;

  /// \brief Run continuously while the context is active.
  ///
  /// Runs in unsafe block, must be interrupt-safe.
  ///
  /// \return non-zero return code on success.
  virtual int loop() = 0;

  /// \brief Run continuously while context is not active but exists.
  ///
  /// \return non-zero return code on success.
  virtual int idle() = 0;

  /// \brief Exits from the current context, releasing any non-invariant
  /// resources.
  ///
  /// Runs in safe block. May not #spawn.
  ///
  /// \return non-zero return code on success.
  virtual int exit() = 0;

 private:
  /// \brief Allocate a context's memory onto the context stack.
  /// \return FunctionContext*
  static FunctionContext* pushOnStack_(
    FunctionContext* expected,
    std::size_t      size);
};

/// \brief Helper implementation of a FunctionContext with debugging helpers.
class DefaultContext : public FunctionContext
{
 public:
  DefaultContext(const char* trace_name, int depth);
  virtual ~DefaultContext() noexcept;
  virtual int enter() override;
  virtual int loop() override;
  virtual int idle() override;
  virtual int exit() override;

 protected:
  int depth() const noexcept { return depth_; }

 private:
  const char* trace_name_;
  int         depth_;
};

// ===================== Detail Implementation =======================

template<class Context, class... Ts>
auto
FunctionContext::spawn(
  FunctionContext* expected_current_context,
  Ts&&... params)
  -> std::enable_if_t<
    std::is_base_of<FunctionContext, Context>::value,
    FunctionContext*>
{
  static_assert(
    alignof(Context) <= alignof(std::max_align_t),
    "Dont support over-aligned types.");

  void* c_ptr = pushOnStack_(expected_current_context, sizeof(Context));
  ::new (c_ptr) Context(std::forward<Ts>(params)...);
  return static_cast<FunctionContext*>(c_ptr);
}

#endif // FUNCTION_CONTEXT_HPP
