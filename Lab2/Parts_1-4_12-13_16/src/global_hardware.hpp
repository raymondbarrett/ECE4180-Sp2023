/// \file global_hardware.hpp
/// \date 2023-02-12
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief pinouts for this part of the project.

#ifndef GLOBAL_HARDWARE_HPP
#define GLOBAL_HARDWARE_HPP

#ifndef __cplusplus
#error "global_hardware.hpp is a cxx-only header."
#endif // __cplusplus

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <cstdint>

#include <array>
#include <chrono>

#include <drivers/BusOut.h>

// ======================= Public Interface ==========================

#define SD_MOSI p5
#define SD_MISO p6
#define SD_SCK p7
#define SD_CS p8

#define I2C_SDA p9
#define I2C_SCL p10

#define RESERVED_TX p13
#define RESERVED_RX p14

#define MEMS_IN p16
#define TOF_XSHUT p17

#define MODE_DIP_P1 p30
#define MODE_DIP_P2 p12
#define MODE_DIP_P3 p11

#define LCD_RST p26
#define LCD_TX p28
#define LCD_RX p27 // To LCD RX.

// ETHERNET goes to ETHERNET

/// \brief Enumerate the globally-accessible hardware that requires no
/// configuration.
namespace GlobalHardware {

constexpr std::size_t kOnboardLEDsCount = 4;
extern mbed::BusOut   OnboardLEDs;

} // namespace GlobalHardware

// ===================== Detail Implementation =======================

#endif // GLOBAL_HARDWARE_HPP
