/// \file main.cpp
/// \date 2023-02-27
/// \author mshakula (mshakula3@gatech.edu)
///
/// \brief The main file.

#define MBED_NO_GLOBAL_USING_DIRECTIVE

#include <mbed.h>
#include <rtos.h>

#include "hardware.hpp"

// ======================= Local Definitions =========================

namespace {

  mbed::PwmOut r(PIN_R);
  mbed::PwmOut g(PIN_G);
  mbed::PwmOut b(PIN_B);
  Serial btInput(PIN_BLE_TX, PIN_BLE_RX);

/// Get random number from 0 to 1
inline float random_number()
{
    return (rand()/(float(RAND_MAX)));
}

/// Create lightning effects
void lightning(void const *args)
{
    while(1){
        wait(3*pow(random_number(),3));
        r= 0.8;
        g= 0.8;
        b= 0.9;
        wait(0.2 * random_number());
        r= 0;
        g= 0;
        b= 0;
    }
    

}

/// \brief Die in main.
void __attribute__((noreturn)) die()
{
  int i = 0;
  do {
    OnboardLEDs = 0;
    const int index =
      i < ONBOARD_LED_COUNT ? i : ONBOARD_LED_COUNT - i % ONBOARD_LED_COUNT - 2;
    OnboardLEDs[index] = true;
    i                  = (i + 1) % (ONBOARD_LED_COUNT * 2 - 2);
    wait_ms(150);
  } while (true);
}



} // namespace

// ====================== Global Definitions =========================

int
main()
{
  char bnum = 0;
  int mode = 0;
  while(1){
    if (btInput.getc() == '!'){
        if (btInput.getc()=='B') { //button data
        bnum = btInput.getc(); //button number
        if ((bnum>='1')&&(bnum<='4')) //is a number button 1..4
            mode=btInput.getc()-'0'; //turn on/off that num LED
        }
    }
  
    switch(mode){
        case 1:
        {
            Thread thread(lightning);
            wait(10);
            thread.terminate();
        }
            break;
        case 2:
            die();
            break;
        case 3:
            die();
            break;
        case 4:
            die();
            break;
    }
  }
}
