/* mbed Microcontroller Library
 * Copyright (c) 2023 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"

#define SPEAKER_PIN D6

class Speaker {

    private:
        DigitalOut outputSignal;
        char state;
    
    public:
        Speaker(PinName pin) : outputSignal(pin){off();}; // Constructor Creates Digital Out on Speaker Pin
        void on(void){
            outputSignal = 1; // 
            state =  1;
        } 
        void off(void){
            outputSignal = 0;
            state = 0;
        }
        void toggle(void){
            state = !state;
            outputSignal = !state; // flip
        }
};


void hz2halfT(float freq){
    
    uint32_t period = (1/freq) * pow(10,6); // convert from us
    wait_us(period/2);
}


int main(void){         
             

    Speaker speaker(SPEAKER_PIN);
    speaker.off();
    for(;;){
        speaker.toggle();
        //speaker.on();
        hz2halfT(1200);
        //speaker.off();
        //hz2halfT(1200);
    }                                     
}