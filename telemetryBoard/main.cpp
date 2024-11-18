#include <cstdint>
#include <cstdio>
#include <stdbool.h>
// #include <ctime>
// #include <string>

#if !DEVICE_CAN
#error [NOT_SUPPORTED] CAN not supported for this target
#endif

// #include <chrono>
#include "mbed.h"

CAN* canBus;

// AnalogIn a1in(A0);
// AnalogIn a2in(A1);
DigitalOut s0(D3);
DigitalOut s1(D4);
DigitalOut s2(D5);
DigitalOut s3(D6);
AnalogIn mp(A2);
// DigitalOut led(LED1);

struct telem_data {
    //int_16 test;
    uint16_t steeringSensor;
    uint16_t brakeSensorR;
    uint16_t brakeSensorF;
    uint16_t shockPositionFR;
    uint16_t strainFR;
    uint16_t wheelSpeedFR;
    uint16_t shockPositionFL;
    uint16_t strainFL;
    uint16_t wheelSpeedFL;
    uint16_t shockPositionBR;
    uint16_t strainBR;
    uint16_t wheelSpeedBR;
    uint16_t shockPositionBL;
    uint16_t strainBL;
    uint16_t wheelSpeedBL;
};

telem_data tData;

/** The constructor takes in RX, and TX pin respectively.
  * These pins, for this example, are defined in mbed_app.json
  */
// CAN can1(PA_11, PA_12);
EventQueue sendQueue(32*EVENTS_EVENT_SIZE);
void initIO();
void send();

void send()
{
    CANMessage steerBrake;
    steerBrake.id = 0x1A4;
    steerBrake.len = 6;
    steerBrake.data[0] = tData.steeringSensor;
    steerBrake.data[1] = tData.steeringSensor >> 8;
    steerBrake.data[2] = tData.brakeSensorR;
    steerBrake.data[3] = tData.brakeSensorR >> 8;
    steerBrake.data[4] = tData.brakeSensorF;
    steerBrake.data[5] = tData.brakeSensorF >> 8;
    canBus->write(steerBrake);

    CANMessage FR;
    FR.id = 0x224;
    FR.len = 6;
    FR.data[0] = tData.shockPositionFR;
    FR.data[1] = tData.shockPositionFR >> 8;
    FR.data[2] = tData.strainFR;
    FR.data[3] = tData.strainFR >> 8;
    FR.data[4] = tData.wheelSpeedFR;
    FR.data[5] = tData.wheelSpeedFR >> 8;
    canBus->write(FR);

    CANMessage FL;
    FL.id = 0x2A4;
    FL.len = 6;
    FL.data[0] = tData.shockPositionFL;
    FL.data[1] = tData.shockPositionFL >> 8;
    FL.data[2] = tData.strainFL;
    FL.data[3] = tData.strainFL >> 8;
    FL.data[4] = tData.wheelSpeedFL;
    FL.data[5] = tData.wheelSpeedFL >> 8;
    canBus->write(FL);

    CANMessage BR;
    BR.id = 0x324;
    BR.len = 6;
    BR.data[0] = tData.shockPositionBR;
    BR.data[1] = tData.shockPositionBR >> 8;
    BR.data[2] = tData.strainBR;
    BR.data[3] = tData.strainBR >> 8;
    BR.data[4] = tData.wheelSpeedBR;
    BR.data[5] = tData.wheelSpeedBR >> 8;
    canBus->write(BR);

    CANMessage BL;
    BL.id = 0x3A4;
    BL.len = 6;
    BL.data[0] = tData.shockPositionBL;
    BL.data[1] = tData.shockPositionBL >> 8;
    BL.data[2] = tData.strainBL;
    BL.data[3] = tData.strainBL >> 8;
    BL.data[4] = tData.wheelSpeedBL;
    BL.data[5] = tData.wheelSpeedBL >> 8;
    canBus->write(BL);
}


CircularBuffer<CANMessage, 32> receiveQueue;

int main(int, char**) {
    printf("main()\n"); 
    initIO();
    //canRX();
    int send_id = sendQueue.call_every(10ms, &send); //Try 2 call_every(s)
    printf("queue prepped\n");
    while(true) {
        for (int i = 0; i < 15; i++) {
            s0 = (0b0001 & i);
            s1 = (0b0010 & i);
            s2 = (0b0100 & i);
            s3 = (0b1000 & i);
            switch (i) {
                case 0:
                    tData.steeringSensor = mp.read_u16();
                    break;
                case 1:
                    tData.brakeSensorR = mp.read_u16();
                    break;
                case 2:
                    tData.brakeSensorF = mp.read_u16();
                    break;
                case 3:
                    tData.shockPositionFR = mp.read_u16();
                    break;
                case 4:
                    tData.strainFR = mp.read_u16();
                    break;
                case 5:
                    tData.wheelSpeedFR = mp.read_u16();
                    break;
                case 6:
                    tData.shockPositionFL = mp.read_u16();
                    break;
                case 7:
                    tData.strainFL = mp.read_u16();
                    break;
                case 8:
                    tData.wheelSpeedBL = mp.read_u16();
                    break;
                case 9:
                    tData.shockPositionBR = mp.read_u16();
                    break;
                case 10:
                    tData.strainBR = mp.read_u16();
                    break;
                case 11:
                    tData.wheelSpeedBR = mp.read_u16();
                    break;
                case 12:
                    tData.shockPositionBL = mp.read_u16();
                    break;
                case 13:
                    tData.strainBL = mp.read_u16();
                    break;
                case 14:
                    tData.wheelSpeedBL = mp.read_u16();
                    break;
            }
        }
        sendQueue.dispatch_once();
        ThisThread::sleep_for(1ms);
    }
    // sendQueue.dispatch_forever();
    printf("Never Reaches Here\n");
    return 0;
}

void initIO() {
    canBus = new CAN(D10, D2, 500000); //Frequency either needs to be set for both sides or neither side
}

// void canRX() {
//     CANMessage msg;

//     if (canBus->read(msg)) {
//         receiveQueue.push(msg);
//     }
// }




// chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
