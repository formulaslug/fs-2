/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

#define MAX_V 3.3

AnalogIn HE1(A0);
AnalogIn HE2(A2);
DigitalOut RTDScontrol(D7);

int main()
{
    //voltage range from 0 to 3.6

    float HE1_V = 0;
    float HE2_V = 0;
    //continuously read analog pins multiply by 3.3V
    while(1) {
        //Throttle angle .5 to 4.5 originally
        //20 degrees
        //.5 to 2.5 V
        //TODO test HE1 and HE2 range so you can calculate it
        HE1_V = HE1.read() * MAX_V;
        HE2_V = HE2.read() * MAX_V;

        //Send CAN 50 hz
        //Calculate percent HE1 and percent HE2
        //Measure percent difference check 
        //implausibility if greater than 10% pedal travel diff for more than 100 ms.
        //Send CAN to stop power
        //How to check for short and open circuit? If 0V or 3.3V do implausibility.

    }
}

void setRTDScontrol(bool setting) {
    const char *s_setting = setting ? "True" : "False";
    printf("RTDS: %s", s_setting);
    RTDScontrol.write(setting);
}
