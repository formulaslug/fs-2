/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

#if !DEVICE_CAN
#error [NOT_SUPPORTED] CAN not supported for this target
#endif

#define MAX_V 3.3
#define BRAKE_TOL 1 
#define MAXSPEED 4000
#define RX_pin D10
#define TX_pin D2

AnalogIn HE1(A1);
AnalogIn HE2(A0);
AnalogIn brakes(A2);
InterruptIn Cockpit(D9);
DigitalIn Cockpit_D(D9);
DigitalOut RTDScontrol(D7);
DigitalOut test_led(LED1);
CAN can(RX_pin, TX_pin);

bool TS_Ready = false; //TODO write CAN code to alter 
bool Motor_On = false;
bool Implausability = false;

void runRTDS() {
    // RTDScontrol.write(true);
    // wait_us(3000000);
    // RTDScontrol.write(false);
    printf("RUNNING RTDS");
    test_led.write(true);
    wait_us(3000000);
    test_led.write(false);
    printf("FINISHED RTDS");
}

void cockpit_switch_high() {
    wait_us(10000);
    int signal = Cockpit_D.read();
    if(Cockpit_D.read() == 0) {
        return;
    }
    test_led.write(true);
    if (TS_Ready && brakes.read() >= BRAKE_TOL) {
        Motor_On = true;
    }
    wait_us(10000);
    test_led.write(false);
}

void check_start_conditions() {
    CANMessage msg;
    int counter = 0;
    while (Motor_On == false) {
        if(counter >= 10000) { 
            printf("Still Waiting \n"); 
            counter = 0;
        }

        printf("Cockpit_D: %d\n", Cockpit_D.read());
        if(brakes.read() > BRAKE_TOL) {
            printf("Brakes pressed.\n");
        }

        if(can.read(msg)) {
            printf("Message received: %d\n", msg.data[0]);
            // TODO recognize tractive signal and set TS_rdy if received
        }

    }
}

void cockpit_switch_low() {
    wait_us(10000);
    int signal = Cockpit_D.read();
    if(Cockpit_D.read() == 1) {
        return;
    }
    test_led.write(true);
    Motor_On = false;
    wait_us(10000);
    test_led.write(false);
}

float clamp(float value, float low, float high) {
    if(value < low) { return low;}
    if(value > high) { return high;}
    return value;
}


int main()
{   
    set_time(0);
    // tested range of values 
    const float HE1_LOW = .28;
    const float HE1_HIGH = .85;
    const float HE2_LOW = .21;
    const float HE2_HIGH = .65;

    uint8_t nbbalive = 0;
    printf("Waiting for start conditions!\n");
    Cockpit.rise(&cockpit_switch_high);
    Cockpit.fall(&cockpit_switch_low);
    //check_start_conditions();
    // Cockpit switch needs to be on, Brakes need to be on, tractive system rdy message needs to be sent
    // Two threads check for the start conditions TS_rdy and BSPD and shut down when they are met
    // First we check that if all three thread conditions are met, test_led 3 sec, and then begins to blink
    //voltage range from 0 to 3.3

    runRTDS();
    
    Timer implausability_track;
    //continuously read analog pins multiply by 3.3V
    while(1) {
        //Throttle angle .5 to 4.5 originally
        //20 degrees
        //.5 to 2.5 V

        float HE1_read = HE1.read();
        float HE2_read = HE2.read();
        //printf("HE1: %f  | HE2: %f\n", HE1_read, HE2_read);

        //Calculate pedal travel
        float clamped_HE1 = clamp(HE1_read, HE1_LOW, HE1_HIGH);
        float clamped_HE2 = clamp(HE2_read, HE2_LOW, HE2_HIGH);

        float HE1_travel = (clamped_HE1-HE1_LOW) / (HE1_HIGH - HE1_LOW);
        float HE2_travel = (clamped_HE2-HE2_LOW) / (HE2_HIGH - HE2_LOW);

        //printf("clamped HE1: %f | clamped HE2: %f \n", clamped_HE1, clamped_HE2);
        printf("HE1_travel: %f  | HE2_travel: %f\n", HE1_travel, HE2_travel);

        //implausibility if greater than 10% pedal travel diff for more than 100 ms.
        //How to check for short and open circuit? If 0V or 3.3V do implausibility.
        float pedal_travel = 0.5*(HE1_travel + HE2_travel);
        float travel_diff = std::abs(HE1_travel - HE2_travel);
        if(travel_diff > .1f) {
            printf("implausability");
            implausability_track.stop();
            unsigned long long current_time = duration_cast<std::chrono::milliseconds>(implausability_track.elapsed_time()).count();
            if(current_time > 100) {
                Implausability = true;
                Motor_On = false;
                implausability_track.reset();
            } else {
                implausability_track.start();
            }
            
        } else {
            implausability_track.reset();
        }
        // THROTTLE (uint16 torquedemand (%of0xffff), uint16 maxspeed (rpm), 
        // bool powerrdy, bool reverse = False, bool forward = True, 
        // uint8 nbbalive (increment and reset when max)) 
        // send 20 ms freq

        //Sync message  (ID is 0x80) (Msg is 0x00 100 ms freq)

        //if(Motor_On == false) break;


        // bool powerrdy = Motor_On;
        // bool reverse = false;
        // bool forward = true;
        // if(nbbalive == 255) { nbbalive = 0; } else {nbbalive++;}
        // uint16_t torquedemand = 100 * pedal_travel;
        // uint16_t maxspeed = MAXSPEED;
    }

    main();
}



