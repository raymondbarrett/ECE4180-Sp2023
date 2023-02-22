/// \file function_context.cpp
/// \date 2023-01-24
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The support implementation for function contexts.

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include "function_context.hpp"

#include <cassert>
#include <cstddef>
#include <cstdlib>

#include <algorithm>
#include <array>
#include <atomic>
#include <memory>
#include <new>

#include <cmsis.h>
#include <mbed_debug.h>

#include "global_hardware.hpp"

// ======================= Local Definitions =========================

namespace {

/// \brief Pointer block for traversal of the existing FunctionContexts.
///
/// Each Block is associated with the immediately following FunctionContext.
///
/// \note sizeof is guaranteed to be multiple of this alignment. (C++11
/// [[expr.sizeof/2]](https://timsong-cpp.github.io/cppwp/n3337/expr.sizeof#2))
struct alignas(std::max_align_t) Block
{
  Block* const prev; // The previous one will never be modified.
  Block*       next;

  /// \brief Get the associated context.
  FunctionContext* context();
  FunctionContext* context() volatile
  {
    return const_cast<Block*>(this)->context();
  }
};

FunctionContext*
Block::context()
{
  return reinterpret_cast<FunctionContext*>(
    reinterpret_cast<char*>(this) + sizeof(Block));
}

/// \brief Rounds a value away from 0 to the next multiple.
template<class A, class B>
constexpr std::common_type_t<A, B>
roundUp(A val, B multiple) noexcept
{
  assert(multiple);
  return ((val + multiple - 1) / multiple) * multiple;
}

/// \brief The memory bank for contexts.
///
/// MemoryPool is not available in MBedOS Bare-metal, and modifying the linker
/// with MBed to support other memory sections is a pain -- so just resort to
/// a simple large static allocation. Without MBed configuration limitation it
/// would be very nice otherwise.
///
/// LPC1768 has 64kB RAM, use 2kB for the context stack.
alignas(std::max_align_t) std::array<char, 2 << 10> context_stack;

/// \brief Points to most recent globally-active block.
///
/// \note volatile atomic for correctness-shouldn't be an issue w/ IRL
/// compilers.
std::atomic<Block*> volatile active_block{
  reinterpret_cast<Block*>(std::begin(context_stack))};

/// \brief Points to the active_block latched at the beginning of MT context
/// loop. AKA: the currently running context.
///
/// \note ONLY ACCESS FROM MT.
volatile Block* latched_block{[]() {
  auto b = reinterpret_cast<Block*>(std::begin(context_stack));
  ::new (b) Block{nullptr, nullptr}; // always have first target block.
  return b;
}()};

/// \brief The general Context-pipeline error handling function. Kills the
/// process and prints error message.
void
handlePipelineErr(int errc)
{
  printf("Caught pipeline error with code [%i]: TODO\n", errc);
  std::exit(errc);
}

} // namespace

// ====================== Global Definitions =========================

#pragma region FunctionContext

void
FunctionContext::terminate()
{
  __disable_irq();

  if (latched_block->next) // if there is an active context.
    active_block.store(latched_block->prev, std::memory_order_release);
}

void
FunctionContext::start()
{
#define HANDLE_ERROR(expr)    \
  do {                        \
    if ((ret = (expr)))       \
      handlePipelineErr(ret); \
  } while (false)

  __disable_irq();

  printf("Starting up FunctionContext loop.\n");

  int             ret;
  volatile Block* last_block = latched_block =
    active_block.load(std::memory_order_acquire);

  // There are no spawned contexts on startup -- fast exit with no loop.
  if (!latched_block->next)
    goto exit;

  // Enter the initial top block. Skip ones preceding it.
  HANDLE_ERROR(latched_block->context()->enter());

  __enable_irq();

  do {
    // Do all the necessary context switching.
    if (latched_block != last_block) {
      // terminate() and spawn() always disable IRQ.
      if (latched_block > last_block) {
        // Created new context.
        HANDLE_ERROR(last_block->context()->exit());
        HANDLE_ERROR(latched_block->context()->enter());
      } else {
        assert(last_block->prev == latched_block);

        // Deleted context. Can only be one back due to IRQ disabling.
        HANDLE_ERROR(last_block->context()->exit());

        // destroy memory resources.
        last_block->context()->~FunctionContext();
        last_block->next->~Block();
        std::memset(
          static_cast<void*>(last_block->context()),
          0,
          last_block->next->context() - last_block->context());

        last_block->next = nullptr; // this essentially disables this block.
        if (!latched_block)         // last context deleted.
          goto exit;

        HANDLE_ERROR(latched_block->context()->enter());
      }
      __enable_irq();
    }

    // Run background tasks.
    for (Block* block = reinterpret_cast<Block*>(std::begin(context_stack));
         block != latched_block;
         block = block->next) {
      HANDLE_ERROR(block->context()->idle());
    }

    // Run main task.
    HANDLE_ERROR(latched_block->context()->loop());

    // Lock in active_block for next iteration.
    last_block    = latched_block;
    latched_block = active_block.load(std::memory_order_acquire);
  } while (true);

exit:
  // reset global state.
  latched_block = reinterpret_cast<Block*>(std::begin(context_stack)),
  active_block.store(
    const_cast<Block*>(latched_block), std::memory_order_release);

  printf("Terminating FunctionContext loop.\n");
  __enable_irq();
  return;
}

FunctionContext*
FunctionContext::pushOnStack_(FunctionContext* expected, std::size_t size)
{
  auto primask = __get_PRIMASK(); // 0 if enabled.
  __disable_irq();

  auto block = active_block.load(std::memory_order_acquire);
  if (expected && block->context() != expected) {
    if (!primask)
      __enable_irq();
    return nullptr;
  }

  if (block->next) // for first initialization.
    block = block->next;

  block->next = reinterpret_cast<Block*>(
    reinterpret_cast<char*>(block->context()) +
    roundUp(size, alignof(std::max_align_t)));

  // CRITICAL - RAN OUT OF MEMORY...
  if (reinterpret_cast<char*>(block->next) >= std::end(context_stack)) {
    std::exit(1);
  }

  ::new (block->next) Block{block, nullptr};
  active_block.store(block, std::memory_order_release);

  return block->context();
}

#pragma endregion FunctionContext
#pragma region    DefaultContext

DefaultContext::DefaultContext(const char* trace_name, int depth) :
    trace_name_(trace_name ? trace_name : "DefaultContext"), depth_{depth}
{
#ifndef NDEBUG
  for (int i = 0; i < depth - 1; ++i)
    debug("--");
  if (depth)
    debug("- ");
  debug("%s::%s()\n", trace_name_, trace_name_);
#endif // NDEBUG
}

DefaultContext::~DefaultContext()
{
#ifndef NDEBUG
  const int& depth = depth_;
  for (int i = 0; i < depth - 1; ++i)
    debug("--");
  if (depth)
    debug("- ");
  debug("%s::~%s()\n", trace_name_, trace_name_);
#endif // NDEBUG
}

int
DefaultContext::enter()
{
#ifndef NDEBUG
  const int& depth = depth_;
  for (int i = 0; i < depth - 1; ++i)
    debug("--");
  if (depth)
    debug("- ");
  debug("%s::enter()\n", trace_name_);
#endif // NDEBUG
  return 0;
};

int
DefaultContext::loop()
{
  return 0;
}

int
DefaultContext::idle()
{
  return 0;
}

int
DefaultContext::exit()
{
#ifndef NDEBUG
  const int& depth = depth_;
  for (int i = 0; i < depth - 1; ++i)
    debug("--");
  if (depth)
    debug("- ");
  debug("%s::exit()\n", trace_name_);
#endif // NDEBUG
  return 0;
}

#pragma endregion DefaultContext
