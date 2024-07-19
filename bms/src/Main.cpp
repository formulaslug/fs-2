#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <memory>
#include <vector>
#include <iostream>

#include "mbed.h"

#include "LTC681xParallelBus.h"
#include "BmsThread.h"

#include "Can.h"


CAN* canBus;

void initIO();
void canRX();

void canBootupTX();
void canBoardStateTX();
void canTempTX(uint8_t seg);
void canTempTX0();
void canTempTX1();
void canTempTX2();
void canTempTX3();
void canVoltTX(uint8_t seg);
void canVoltTX0();
void canVoltTX1();
void canVoltTX2();
void canVoltTX3();
void canCurrentLimTX();



EventQueue queue(32*EVENTS_EVENT_SIZE);// creates an eventqueue which is thread and ISR safe. EVENTS_EVENT_SIZE is the size of the buffer allocated



CircularBuffer<CANMessage, 32> canqueue;

uint8_t canCount;


DigitalIn shutdown_measure_pin(ACC_SHUTDOWN_MEASURE);
DigitalIn imd_status_pin(ACC_IMD_STATUS);
DigitalIn charge_state_pin(ACC_CHARGE_STATE);


DigitalOut fan_control_pin(ACC_FAN_CONTROL);
DigitalOut charge_enable_pin(ACC_CHARGE_ENABLE);
DigitalOut bms_fault_pin(ACC_BMS_FAULT);
DigitalOut precharge_control_pin(ACC_PRECHARGE_CONTROL);


AnalogIn current_vref_pin(ACC_BUFFERED_C_VREF);
AnalogIn current_sense_pin(ACC_BUFFERED_C_OUT);
AnalogIn glv_voltage_pin(ACC_GLV_VOLTAGE);

bool prechargeDone = false;
bool hasBmsFault = true;
bool isCharging = false;
bool hasFansOn = false;
bool isBalancing = false;

uint16_t dcBusVoltage;
uint32_t tsVoltagemV;
//uint16_t tsVoltage;
uint8_t glvVoltage;
uint16_t tsCurrent;

uint16_t allVoltages[BMS_BANK_COUNT*BMS_BANK_CELL_COUNT];
int8_t allTemps[BMS_BANK_COUNT*BMS_BANK_TEMP_COUNT];

int8_t minCellVolt; // factor of 0.02 so 255 is 5.1 volts
int8_t maxCellVolt; // factor of 0.02 so 255 is 5.1 volts

int main() {

  printf("main\n");
  initIO();
  printf("initIO\n");


  SPI* spiDriver = new SPI(BMS_PIN_SPI_MOSI,
                           BMS_PIN_SPI_MISO,
                           BMS_PIN_SPI_SCLK,
                           BMS_PIN_SPI_SSEL,
                           use_gpio_ssel);
  spiDriver->format(8, 0);
  auto ltcBus = LTC681xParallelBus(spiDriver);

  BmsEventMailbox* bmsMailbox = new BmsEventMailbox();
  MainToBMSMailbox* mainToBMSMailbox = new MainToBMSMailbox();

  Thread bmsThreadThread;
  BMSThread bmsThread(ltcBus, 1, bmsMailbox, mainToBMSMailbox);
  bmsThreadThread.start(callback(&BMSThread::startThread, &bmsThread));

  osThreadSetPriority(osThreadGetId(), osPriorityHigh7);
  Timer t;
  t.start();
  while (1) {
    glvVoltage = (uint8_t)(glv_voltage_pin * 185.3); // in mV
    //printf("GLV voltage: %d mV\n", glvVoltage * 100);

    while (!bmsMailbox->empty()) {
        BmsEvent *bmsEvent;
        
        osEvent evt = bmsMailbox->get();
        if (evt.status == osEventMessage) {
            bmsEvent = (BmsEvent*)evt.value.p;
        } else {
            continue;
        }

        switch (bmsEvent->bmsState) {
            case BMSThreadState::BMSStartup:
                break;
            case BMSThreadState::BMSIdle:
                hasBmsFault = false;

                minCellVolt = bmsEvent->minVolt;
                maxCellVolt = bmsEvent->maxVolt;
                isBalancing = bmsEvent->isBalancing;

                tsVoltagemV = 0;

                for (int i = 0; i < BMS_BANK_COUNT*BMS_BANK_CELL_COUNT; i++) {
                    allVoltages[i] = bmsEvent->voltageValues[i];
                    tsVoltagemV += allVoltages[i];
                    //printf("%d, V: %d\n", i, allVoltages[i]);
                }
                for (int i = 0; i < BMS_BANK_COUNT*BMS_BANK_TEMP_COUNT; i++) {
                    allTemps[i] = bmsEvent->temperatureValues[i];
                    // printf("%d, T: %d\n", i, allTemps[i]);
                }

                break;
            case BMSThreadState::BMSFault:
                printf("*** BMS FAULT ***\n");
                hasBmsFault = true;
                break;
            default:
                break;
        }
        delete bmsEvent;
    }

    while (!canqueue.empty()) {
        CANMessage msg;
        canqueue.pop(msg);

        uint32_t id = msg.id;
        unsigned char* data = msg.data;

        switch(id) {
          case 0x682: // temperature message from MC
            dcBusVoltage = (data[2] | (data[3] << 8)); // TODO: check if this is correct
            break;
          default:
            break;
        }
    }

    if (!mainToBMSMailbox->full()) {
        MainToBMSEvent* mainToBMSEvent = new MainToBMSEvent();
        mainToBMSEvent->balanceAllowed = shutdown_measure_pin;
        mainToBMSEvent->charging = isCharging;
        mainToBMSMailbox->put(mainToBMSEvent);
    }






    if (dcBusVoltage >= (uint16_t)(tsVoltagemV/100.0) * PRECHARGE_PERCENT && tsVoltagemV >= 60000) {
        prechargeDone = true;
    } else if (dcBusVoltage < 60000) {
        prechargeDone = false;
    }





    bms_fault_pin = !hasBmsFault;

    

    isCharging = charge_state_pin;
    // printf("charge state: %x\n", isCharging);

    precharge_control_pin = isCharging || prechargeDone /*false*/;

    hasFansOn = prechargeDone || isCharging;

    charge_enable_pin = isCharging && !hasBmsFault && shutdown_measure_pin;
    fan_control_pin = hasFansOn;
    // printf("charge state: %x, hasBmsFault: %x, shutdown_measure: %x\n", isCharging, hasBmsFault, true && shutdown_measure_pin);

    // times 1.5 to change from 3.3v to 5v
    // divided by 0.625 for how the current sensor works :/
    // divided by 300 because that's the nominal current reading of the sensor (ie baseline)
    // multiplied by 10 and cast to a uint16 for 1 decimal place
    tsCurrent = ((uint16_t)((current_sense_pin-current_vref_pin)/125.0))*10;
    
    // printf("Ts current: %d\n", tsCurrent);

    // printf("Error Rx %d - tx %d\n", canBus->rderror(),canBus->tderror());

    queue.dispatch_once();
    ThisThread::sleep_for(50 - (t.read_ms()%50));
  }
}

void initIO() {
    canBus = new CAN(BMS_PIN_CAN_RX, BMS_PIN_CAN_TX, BMS_CAN_FREQUENCY);
    // canBus->frequency(BMS_CAN_FREQUENCY);
    // canBus->reset();
    canBus->attach(canRX);

    queue.call(&canBootupTX);
    queue.dispatch_once();

    queue.call_every(100ms, &canBoardStateTX);
    queue.call_every( 40ms, &canCurrentLimTX);
    queue.call_every(200ms, &canVoltTX0);
    queue.call_every(200ms, &canVoltTX1);
    queue.call_every(200ms, &canVoltTX2);
    queue.call_every(200ms, &canVoltTX3);
    queue.call_every(200ms, &canTempTX0);
    queue.call_every(200ms, &canTempTX1);
    queue.call_every(200ms, &canTempTX2);
    queue.call_every(200ms, &canTempTX3);


    fan_control_pin = 0; // turn fans off at start
    charge_enable_pin = 0; // charge not allowed at start
    bms_fault_pin = 0; // assume fault at start, low means fault
    precharge_control_pin = 0; // positive AIR open at start

}

void canRX() {
    CANMessage msg;
    
    if (canBus->read(msg)) {
        canqueue.push(msg);
    }
}

void canBootupTX() {
    canBus->write(accBoardBootup());
}

void canBoardStateTX() {
    canBus->write(accBoardState(
        glvVoltage,
        (uint16_t)(tsVoltagemV/100.0),
        hasBmsFault,
        isBalancing,
        prechargeDone,
        isCharging,
        hasFansOn,
        shutdown_measure_pin,
        false,
        false,
        minCellVolt,
        maxCellVolt,
        tsCurrent
    ));
    ThisThread::sleep_for(1ms);
}

void canTempTX(uint8_t segment) {
    int8_t temps[7] = {
            allTemps[(segment * BMS_BANK_CELL_COUNT)],
            allTemps[(segment * BMS_BANK_CELL_COUNT) + 1],
            allTemps[(segment * BMS_BANK_CELL_COUNT) + 2],
            allTemps[(segment * BMS_BANK_CELL_COUNT) + 3],
            allTemps[(segment * BMS_BANK_CELL_COUNT) + 4],
            allTemps[(segment * BMS_BANK_CELL_COUNT) + 5],
            allTemps[(segment * BMS_BANK_CELL_COUNT) + 6]
    };
    canBus->write(accBoardTemp(segment, temps));
    ThisThread::sleep_for(1ms);
}

void canVoltTX(uint8_t segment) {
    uint16_t volts[7] = {
            allVoltages[(segment * BMS_BANK_CELL_COUNT)],
            allVoltages[(segment * BMS_BANK_CELL_COUNT) + 1],
            allVoltages[(segment * BMS_BANK_CELL_COUNT) + 2],
            allVoltages[(segment * BMS_BANK_CELL_COUNT) + 3],
            allVoltages[(segment * BMS_BANK_CELL_COUNT) + 4],
            allVoltages[(segment * BMS_BANK_CELL_COUNT) + 5],
            allVoltages[(segment * BMS_BANK_CELL_COUNT) + 6]
    };
    canBus->write(accBoardVolt(segment, volts));
    ThisThread::sleep_for(1ms);
}

void canCurrentLimTX() {
    uint16_t chargeCurrentLimit = 0x0000;
    uint16_t dischargeCurrentLimit = (uint16_t)(((CAR_MAX_POWER/(tsVoltagemV/1000.0))*CAR_POWER_PERCENT < CAR_CURRENT_MAX) ? (CAR_MAX_POWER/(tsVoltagemV/1000.0)*CAR_POWER_PERCENT) : CAR_CURRENT_MAX);
    canBus->write(motorControllerCurrentLim(chargeCurrentLimit, dischargeCurrentLimit));
    ThisThread::sleep_for(1ms);
}

void canVoltTX0() {
    canVoltTX(0);
}

void canVoltTX1() {
    canVoltTX(1);
}

void canVoltTX2() {
    canVoltTX(2);
}

void canVoltTX3() {
    canVoltTX(3);
}

void canTempTX0() {
    canTempTX(0);
}

void canTempTX1() {
    canTempTX(1);
}

void canTempTX2() {
    canTempTX(2);
}

void canTempTX3() {
    canTempTX(3);
}
