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
DigitalOut s0(PB0);
DigitalOut s1(PB7);
DigitalOut s2(PB6);
DigitalOut s3(PB1);
AnalogIn mp(PB3);
DigitalOut led(LED1);

struct telem_data {
    int_16 test;
    float steeringSensor;
    float brakeSensorF;
    float brakeSensorR;
    float shockPositionFR;
    float strainFR;
    float wheelSpeedFR;
    float shockPositionFL;
    float strainFL;
    float wheelSpeedFL;
    float shockPositionBR;
    float strainBR;
    float wheelSpeedBR;
    float shockPositionBL;
    float strainBL;
    float wheelSpeedBL;
}

/** The constructor takes in RX, and TX pin respectively.
  * These pins, for this example, are defined in mbed_app.json
  */
// CAN can1(PA_11, PA_12);
EventQueue sendQueue(4*EVENTS_EVENT_SIZE);
void initIO();
// void canRX();
void sendArrayTest()
{
    char shutdownOutput [3];//3 is the number of positions in the array
    float shutdownVoltage = shutdownIn.read() * 3.72;// arrived at experimentally. Voltage reading on MCU tends to drop when measured with multimeter
    shutdownOutput[0] = '0';
    shutdownOutput[1] = '0';
    shutdownOutput[2] = '0';
    if (shutdownVoltage < threshhold1) {
        shutdownOutput[0] = '1';
    }
    if (shutdownVoltage >= threshhold1 && shutdownVoltage < threshhold2){
        shutdownOutput[1] = '1';
    }
    if (shutdownVoltage >= threshhold2 && shutdownVoltage < threshhold3) {
        shutdownOutput[2] = '1';
    }
    // printf("voltage = %s\n", to_string(shutdownVoltage).c_str()); <- does not work
    printf("SH Voltage = %f\n", shutdownVoltage);
    // if (can1.write(CANMessage(1400, shutdownOutput, 3))) {
    //     printf("Sending Shutdown Voltage: %d and %d and %d\n", (int) shutdownOutput[0], (int) shutdownOutput[1], (int) shutdownOutput[2]); //%d is integer %c is char %f is float
    // }
    int sent = canBus->write(CANMessage(1400, shutdownOutput, 3));
    if (sent) {
        printf("CAN Sent\n");
    }
    else if (sent == 0){
        printf("CAN Failed to Send\n");
    }
}

void testSend()
{
    printf("test send\n");
    char test3Char [3] = {0,1,2};//3 is the number of positions in the array
    printf("char* created\n");
    int sent = canBus->write(CANMessage(1400, test3Char, 3));
    if (sent) {
        printf("CAN Sent\n");
    }
    else if (sent == 0){
        printf("CAN Failed to Send\n");
    } 
}

void send()
{
    printf("trying message send\n");
    led = !led;
    char shutdownOutput [3];//3 is the number of positions in the array
    float shutdownVoltage = shutdownIn.read() * 3.72;// arrived at experimentally. Voltage reading on MCU tends to drop when measured with multimeter
    shutdownOutput[0] = '0';
    shutdownOutput[1] = '0';
    shutdownOutput[2] = '0';
    if (shutdownVoltage < threshhold1) {
        shutdownOutput[0] = '1';
    }
    if (shutdownVoltage >= threshhold1 && shutdownVoltage < threshhold2){
        shutdownOutput[1] = '1';
    }
    if (shutdownVoltage >= threshhold2 && shutdownVoltage < threshhold3) {
        shutdownOutput[2] = '1';
    }
    // printf("voltage = %s\n", to_string(shutdownVoltage).c_str()); <- does not work
    printf("SH Voltage = %f\n", shutdownVoltage);
    // if (can1.write(CANMessage(1400, shutdownOutput, 3))) {
    //     printf("Sending Shutdown Voltage: %d and %d and %d\n", (int) shutdownOutput[0], (int) shutdownOutput[1], (int) shutdownOutput[2]); //%d is integer %c is char %f is float
    // }
    int sent = canBus->write(CANMessage(1400, shutdownOutput, 3));
    if (sent) {
        printf("CAN Sent\n");
    }
    else if (sent == 0){
        printf("CAN Failed to Send\n");
    }
}


CircularBuffer<CANMessage, 32> receiveQueue;

int main(int, char**) {
    printf("main()\n"); 
    initIO();
    //canRX();
    int send_id = sendQueue.call_every(500ms, &send); //Try 2 call_every(s)
    printf("queue prepped\n");
    sendQueue.dispatch_forever();
    printf("Never Reaches Here\n");
    return 0;
}

void initIO() {
    canBus = new CAN(PA_11, PA_12, 500000); //Frequency either needs to be set for both sides or neither side
}

// void canRX() {
//     CANMessage msg;

//     if (canBus->read(msg)) {
//         receiveQueue.push(msg);
//     }
// }




// chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
