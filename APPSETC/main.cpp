/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <chrono>
#include <cstdint>
#include <cstdio>

#if !DEVICE_CAN
#error [NOT_SUPPORTED] CAN not supported for this target
#endif

#define MAX_V 3.3
#define BRAKE_TOL .1
#define MAXSPEED 7500
#define CAN_RX_PIN D10
#define CAN_TX_PIN D2
#define CAN_FREQ 500000
#define MAX_TORQUE 30000

// Constant tested range of values for pedal travel calculation
const float HE1_LOW = .15;
const float HE1_HIGH = .73;
const float HE2_LOW = .14;
const float HE2_HIGH = .57;

AnalogIn HE1(A1);
AnalogIn HE2(A0);
AnalogIn brakes(A2);
InterruptIn Cockpit(D9);
DigitalOut RTDScontrol(D6);
DigitalOut test_led(LED1);
CAN* canBus;

bool TS_Ready = false; 
bool Motor_On = false;
bool CANFlag = false;
bool RTDSqueued = false;

bool cockpitSwitchHighQueued = false;
bool cockpitSwitchLowQueued = false;

void initIO();
void canRX();
void sendSync();
void sendThrottle();
void sendState();

EventQueue queue(64*EVENTS_EVENT_SIZE);

uint8_t mbbAlive = 0;
bool powerRdy = Motor_On;
bool motorReverse = false;
bool motorForward = true;
int16_t torqueDemand;
int16_t maxSpeed = MAXSPEED;

void runRTDS();
void stopRTDS();
void check_start_conditions();
void cockpit_switch_high();
void cockpit_switch_low();
void check_switch_low();
void implausability();
void initIO();
void canRX();
void sendSync();
void sendThrottle();
void sendState();
void printStatusMessage();
float getPedalTravel(Timer* implausability_track);


void runRTDS() {
    // printf("RUNNING RTDS\n");
    // test_led.write(true);
    RTDScontrol.write(true);
    queue.call_in(1000ms, &stopRTDS);
}

void stopRTDS() {
    // test_led.write(false);
    RTDScontrol.write(false);
    // printf("FINISHED RTDS\n");
    // RTDSqueued = false;
}

void cockpit_switch_high() {
    if (!cockpitSwitchHighQueued) {
        cockpitSwitchHighQueued = true;
        queue.call_in(10ms, &check_start_conditions);
    }
}

void check_start_conditions() {
    cockpitSwitchHighQueued = false;
    if(Cockpit.read() == 0) {
        return;
    }
    if (TS_Ready/* && brakes.read() >= BRAKE_TOL*/) {
        Motor_On = true;
        if(RTDSqueued){return;}
        RTDSqueued = true;
        queue.call(&runRTDS);
    }
}

void cockpit_switch_low() {
    if (!cockpitSwitchLowQueued) {
        cockpitSwitchLowQueued = true;
        queue.call_in(10ms, &check_switch_low);
    }
}

void check_switch_low() {
    cockpitSwitchLowQueued = false;
    if(Cockpit.read() == 1) {return;}
    Motor_On = false;
}

void implausability() {
    // printf("implausability occured");
    Motor_On = false;
}

float clamp(float value, float low, float high) {
    if(value < low) { return low;}
    if(value > high) { return high;}
    return value;
}


void canRX() {
    CANFlag = true;
}

void sendSync() {
    unsigned char data[0];
    CANMessage syncMessage(0x80, data, 0);
    canBus->write(syncMessage);
}

void sendThrottle() {
    mbbAlive++;
    mbbAlive %= 16;

    CANMessage throttleMessage;
    throttleMessage.id = 0x186;
    
    throttleMessage.data[0] = torqueDemand;
    throttleMessage.data[1] = torqueDemand >> 8;
    throttleMessage.data[2] = maxSpeed;
    throttleMessage.data[3] = maxSpeed >> 8;
    throttleMessage.data[4] = 0x00 | (0x01 & motorForward) | ((0x01 & motorReverse) << 1) | ((0x01 & powerRdy) << 3);
    throttleMessage.data[5] = 0x00 | (0x0f & mbbAlive);
    throttleMessage.data[6] = 0x00;
    throttleMessage.data[7] = 0x00;

    canBus->write(throttleMessage);
}

void sendState() {
    // We calculate in a long-winded fashion for debug purposes
    float HE1_read = HE1.read();
    float HE2_read = HE2.read();
    //printf("HE1: %f  | HE2: %f\n", HE1_read, HE2_read);

    float clamped_HE1 = clamp(HE1_read, HE1_LOW, HE1_HIGH);
    float clamped_HE2 = clamp(HE2_read, HE2_LOW, HE2_HIGH);
    float HE1_travel = (clamped_HE1-HE1_LOW) / (HE1_HIGH - HE1_LOW);
    float HE2_travel = (clamped_HE2-HE2_LOW) / (HE2_HIGH - HE2_LOW);

    float pedal_travel = 0.5*(HE1_travel + HE2_travel); // take the avg of the two pedal travels

    CANMessage stateMessage;
    stateMessage.id = 0x1A1;

    stateMessage.data[0] = 0x00 | (0x01 & TS_Ready) | ((0x01 & Motor_On) << 1) | ((0x01 & CANFlag) << 2) | ((0x01 & RTDSqueued) << 3) | ((0x01 & Cockpit.read() << 4));
    stateMessage.data[1] = (int8_t)(brakes.read()*100);
    stateMessage.data[2] = (int8_t)(HE1_read*100);
    stateMessage.data[3] = (int8_t)(HE2_read*100);
    stateMessage.data[4] = (int8_t)(HE1_travel*100);
    stateMessage.data[5] = (int8_t)(HE2_travel*100);
    stateMessage.data[6] = (int8_t)(pedal_travel*100);
    stateMessage.data[7] = 0x00;
    
    canBus->write(stateMessage);
}

void printStatusMessage() {
    int ts_rdy = TS_Ready ? 1 : 0;
    int m_on = Motor_On ? 1 : 0;
    int cockpit = Cockpit.read();
    float b = brakes.read();
    float HE1_read = HE1.read();
    float HE2_read = HE2.read();

    // printf("Cockpit Switch: %i | TS_RDY: %i | Brakes: %f | Motor_On: %i | HE1: %f | HE2: %f\n", cockpit, ts_rdy, b, m_on, HE1_read, HE2_read);
}

float getPedalTravel(Timer* implausability_track) {
    // We calculate in a long-winded fashion for debug purposes
    float HE1_read = HE1.read();
    float HE2_read = HE2.read();
    //printf("HE1: %f  | HE2: %f\n", HE1_read, HE2_read);

    float clamped_HE1 = clamp(HE1_read, HE1_LOW, HE1_HIGH);
    float clamped_HE2 = clamp(HE2_read, HE2_LOW, HE2_HIGH);
    float HE1_travel = (clamped_HE1-HE1_LOW) / (HE1_HIGH - HE1_LOW);
    float HE2_travel = (clamped_HE2-HE2_LOW) / (HE2_HIGH - HE2_LOW);
    //printf("HE1_travel: %f  | HE2_travel: %f\n", HE1_travel, HE2_travel);

    //implausibility if greater than 10% pedal travel diff for more than 100 ms.
    float pedal_travel = 0.5*(HE1_travel + HE2_travel); // take the avg of the two pedal travels
    // printf("Pedal Travel: %f\n", pedal_travel);
    float travel_diff = std::abs(HE1_travel - HE2_travel);
    
    if(travel_diff > .1f) {
        if (implausability_track->elapsed_time() > 100ms) {
            implausability();
        } else {
            implausability_track->start();
        }
        
    } else {
        implausability_track->stop();
        implausability_track->reset();
    }

    if(HE1_read == 0 || HE2_read == 0 || HE1_read >= 0.9 || HE2_read >= 0.9) {
        // printf("implausability\n");
        implausability();
    }

    return pedal_travel;

}

int main()
{
    // printf("main\n");
    // Initiate CAN Bus 
    initIO();
    // Begin waiting for start conditions

    // forward decl. for timer tracking 100ms implausabilities for rules
    Timer implausability_timer;

    while(1) {
        // printf("loop\n");
        if(CANFlag) {
            CANMessage msg;
            CANFlag = false;
            
            if(canBus->read(msg)) {
                uint32_t id = msg.id;
                unsigned char* data = msg.data;

                switch (id) {
                    case 0x183:
                        if (data[3] & 0x4) {
                            // printf("TS RDY rx");
                            TS_Ready = true;
                        } else {
                            TS_Ready = false;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        if (Motor_On) {
            // testing implausability
            // for some reason it is not dispatching send events
            float pedalTravel = getPedalTravel(&implausability_timer);
            // THROTTLE (uint16 torquedemand (%of0xffff), uint16 maxspeed (rpm), 
            // bool powerrdy, bool reverse = False, bool forward = True, 
            // uint8 nbbalive (increment and reset when max)) 
            // send 20 ms freq

            //Sync message  (ID is 0x80) (Msg is 0x00 100 ms freq)

            powerRdy = true;
            motorReverse = false;
            motorForward = true;
            torqueDemand = int16_t(MAX_TORQUE * pedalTravel); // Dunno if it should be between 0 and 1 or 0 and 100
            maxSpeed = MAXSPEED;
        } else {
            powerRdy = false;
            motorReverse = false;
            motorForward = true;
            torqueDemand = 0; 
            maxSpeed = MAXSPEED;
            // printStatusMessage();
        }

        queue.dispatch_once();
        ThisThread::sleep_for(1ms);
    }

}

void initIO() {
    // printf("initIO\n");
    canBus = new CAN(CAN_RX_PIN, CAN_TX_PIN, CAN_FREQ);

    queue.call_every(20ms, &sendThrottle);
    queue.call_every(100ms, &sendSync);
    queue.call_every(100ms, &sendState);

    canBus->attach(canRX);
    
    // printf("Waiting for start conditions!\n");

    Cockpit.rise(&cockpit_switch_high);

    // Event for closing motor
    Cockpit.fall(&cockpit_switch_low);
}