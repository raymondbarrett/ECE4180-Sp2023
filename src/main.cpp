#include "mbed.h"
#include "rtos.h"
#include "SDFileSystem.h"
#include "wave_player.h"


SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card

//LED lighting effects with sound using the RTOS
//Plays the wave file "sample.wav" on the USB flash drive
//using waveplayer code library
//Class D style audio output on P26 or
//use the Audio Out jack for connection to a set of amplified PC speakers (at higher volume)
//Needs a USB flash drive inserted with the *.wav files on it to run with audio effects
//Pins are setup for mbed LPC1768 on mbed application board
//Analog Audio Out Jack for PC Speakers- Sparkfun also has a breakout
//For more info see:
//http://developer.mbed.org/users/4180_1/notebook/led-lighting-effects-for-modelers/
AnalogOut DACout(p18);
//On Board Speaker on Application board but very low volume using PWM
PwmOut PWMout(p26);
//LEDs used in demo
PwmOut myled2(LED2);
PwmOut myled4(LED4);
//Use a PWM output to enable dimming
//1.0 is full on, 0.0 off, 0.5 50% on

inline float random_number()
{
    return (rand()/(float(RAND_MAX)));
}
void beacon(void const *args)
{
    while(1) {
        //LED warm up effect using PWM
        for(int i=0; i<50; i++) {
            myled2 = i/50.0;
            Thread::wait(1000.0*0.02);
        }
        //LED at full brightness level
        myled2 = 1.0;
        Thread::wait(1000.0*0.25);
        //LED cool down effect using PWM
        for(int i=49; i>0; i--) {
            myled2 = i/50.0;
            Thread::wait(1000.0*0.02);
        }
        //LED off
        myled2 = 0.0;
        Thread::wait(1000.0*1.5);
    }
}
void welding(void const *args)
{
    float x = 0.0;
    while(1) {
        //get a new random number for PWM
        x = random_number();
        //add some exponential brightness scaling
        //for more of a fast flash effect
        myled2 = x*x*x;
        //fast update rate for welding flashes
        Thread::wait(1000.0*0.02);
        //add a random pause between welds
        if (random_number()>0.995) {
            myled2 = 0.0;
            Thread::wait(1000.0*random_number());
        }
    }
}
void fire(void const *args)
{
    while(1) {
//get a new random number for PWM
        myled2 = random_number();
//a bit slower time delay can be used for fire
        wait(0.04);
    }
}
void police(void const *args)
{
    while(1) {
        //flash three times on LED1
        for(int i=0; i<3; i++) {
            //ramp up brightness level
            for(double x = 0.0; x <= 1.0; x = x+0.2) {
                myled2 = x*x;
                wait(.02);
            }
        }
        myled2=0.0; //LED1 off
        //flash three times on LED2
        for(int i=0; i<3; i++) {
            //ramp up brightness level
            for(double x = 0.0; x <= 1.0; x = x+0.2) {
                myled4 = x*x;
                wait(.02);
            }
        }
        myled4=0.0; //LED2 off
    }
}
void lighthouse(void const *args)
{
    float y=0.0;

    while(1) {
        for(double x=0.0; x <= 3.14159; x = x + 0.0314159) {
            y = sin(x); //nice periodic function 0..1..0
            myled2 = y*y*y;//exponential effect - needs a sharp peak
            Thread::wait(1000.0*.025);
        }
        myled2 = 0.0;
        //most lighthouses have a 5 second delay - so add another 2.5
        Thread::wait(1000.0*2.5);
    }
}


//function to simulate incandescent bulb warm up
//and cool down cycles
void incandescent_bulb(PwmOut led, float on_time = 0.1,
                       float warm_time = 0.2)
{
    //LED warm up effect using PWM
    for(int i=0; i<10; i++) {
        led = i/10.0;
        Thread::wait(1000.0*warm_time/10.0);
    }
    //LED at full brightness level
    led = 1.0;
    Thread::wait(1000.0*on_time);
    //LED cool down effect using PWM
    for(int i=9; i>0; i--) {
        led = i/10.0;
        Thread::wait(1000.0*warm_time/10.0);
    }
    //LED off
    led = 0.0;
}
//LED Railroad Crossing Lighting Effect
void rrcrossing(void const *args)
{
    while(1) {
        incandescent_bulb(myled2);
        incandescent_bulb(myled4);
    }
}

//wave player plays a *.wav file to D/A
wave_player waver(&DACout);

int main()
{

    FILE *wave_file;
    while(1) {
        {
            //Lighthouse demo with foghorn
            Thread thread(lighthouse); //Start LED effect thread
            for(int i=0; i<4; ++i) {
                //open wav file and play it
                wave_file=fopen("/sd/foghorn.wav","r");
                waver.play(wave_file); //Plays (*.wav file
                fclose(wave_file);
                Thread::wait(1925);
            }
            //end of lighthouse demo;
            thread.terminate(); //kills LED effect thread whan audio stops

        } //code block forces thread out of scope reclaims mem-can recreate thread object
        {
            //Arc Welding with sound
            Thread thread(welding);
            //open wav file and play it
            wave_file=fopen("/sd/welding.wav","r");
            waver.play(wave_file);
            fclose(wave_file);
            //end of welding demo;
            thread.terminate();
        }
        {
            //Police Lights with siren
            //code block forces thread out of scope
            Thread thread(police);
            //open wav file and play it
            wave_file=fopen("/sd/emgsiren.wav","r");
            waver.play(wave_file);
            fclose(wave_file);
            //end of police light demo;
            thread.terminate();
        }
        {
            //Fire with sound
            Thread thread(fire);
            //open wav file and play it
            wave_file=fopen("/sd/fire.wav","r");
            waver.play(wave_file);
            fclose(wave_file);
            //end of fire demo;
            thread.terminate();
        }

        {
            //RR Crossing Lights with Signal Bells
            Thread thread(rrcrossing);
            //open wav file and play it
            wave_file=fopen("/sd/SignalBell.wav","r");
            waver.play(wave_file);
            fclose(wave_file);
            //end of railroad crossing demo;
            thread.terminate();
        }
    }
}
