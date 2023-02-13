/// \file pinout.hpp
/// \date 2023-02-12
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief pinouts for this part of the project.

#ifndef PINOUT_HPP
#define PINOUT_HPP

#ifndef __cplusplus
#error "pinout.hpp is a cxx-only header."
#endif // __cplusplus

#include <mbed.h>

// ======================= Public Interface ==========================

#define SD_MOSI p5
#define SD_MISO p6
#define SD_SCK p7
#define SD_CS p8

#define I2C_SDA p9
#define I2C_SCL p10

#define RESERVED_TX p13
#define RESERVED_RX p14

#define MEMS_IN p15

#define MODE_DIP_P1 p16
#define MODE_DIP_P2 p17
#define MODE_DIP_P3 p19
#define MODE_DIP_P4 p20

#define LCD_RST p26
#define LCD_RX p27
#define LCD_TX p28

// ETHERNET goes to ETHERNET

// ===================== Detail Implementation =======================

#endif // PINOUT_HPP
