// Hello World to sweep a servo through its full range

#include "mbed.h"

DigitalOut motorControl(p22);

int main() {
	while(1){
      motorControl = 1;
	  wait(0.1);
	  motorControl = 0;
	  wait(5);
	}
}

