/// \file src/main.cpp
///
/// \brief Main entry point.

#if !defined(__cplusplus) || __cplusplus < 201402L
#error "C++14 (max supported by online compiler) required."
#endif

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <array>

#include <mbed.h>

#include "hardware.hpp"

/// \brief Main entrypoint.
[[noreturn]] int
main()
{
  // Set watchdog for function selection.

  // Transfer control to function loop.
  do {
    DemoFunction* current_function = requested_function;

    core_util_critical_section_enter();
    current_function->setup();
    core_util_critical_section_exit();

    while (current_function == requested_function)
      current_function->loop();

    core_util_critical_section_enter();
    current_function->end();
    core_util_critical_section_exit();
  } while (true);
}
