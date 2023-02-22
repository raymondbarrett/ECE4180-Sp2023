# CMakeLists.txt

cmake_minimum_required(VERSION 3.19.0 FATAL_ERROR)

project(
  Lab2-MS
  DESCRIPTION "Advanced I/O for Lab2 Parts 1-4, 12-13, 13-16."
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
add_executable(Lab2-MS)
mbed_configure_app_target(Lab2-MS)

set_target_properties(Lab2-MS PROPERTY CXX_STANDARD 14)
target_sources(Lab2-MS PRIVATE src/main.cpp src/function_context.cpp
                               src/global_hardware.cpp)
target_link_libraries(Lab2-MS PRIVATE mbed-baremetal)

mbed_set_post_build(Lab2-MS)

option(VERBOSE_BUILD "Have a verbose build process")
if(VERBOSE_BUILD)
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
