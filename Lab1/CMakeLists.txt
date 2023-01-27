# CMakeLists.txt

cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

project(
  Lab1
  DESCRIPTION "Basic MBed exploration."
  LANGUAGES C CXX)

# MBed stuff.
set(MBED_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/mbed-os
    CACHE INTERNAL "")
set(MBED_CONFIG_PATH
    ${CMAKE_CURRENT_BINARY_DIR}
    CACHE INTERNAL "")
include(${MBED_PATH}/tools/cmake/app.cmake)
add_subdirectory(${MBED_PATH})

# Main target.
add_executable(Lab1)
mbed_configure_app_target(Lab1)

target_sources(Lab1 PRIVATE src/main.cpp src/function_context.cpp
                            src/hardware.cpp)
target_link_libraries(Lab1 PRIVATE mbed-baremetal)

mbed_set_post_build(Lab1)

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
