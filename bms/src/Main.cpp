#include <array>
#include <cstdio>
#include <vector>
#include <iostream>

#include "mbed.h"

#include "LTC681xParallelBus.h"
#include "BmsThread.h"


CAN* canBus;

void initIO();
void canRX();

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

  auto canMailbox = new BmsEventMailbox();
  std::vector<BmsEventMailbox *> mailboxes = {canMailbox};

  Thread bmsThreadThread;
  BMSThread bmsThread(ltcBus, 1, mailboxes);
  bmsThreadThread.start(callback(&BMSThread::startThread, &bmsThread));

  std::array<int8_t, BMS_BANK_COUNT * BMS_BANK_TEMP_COUNT> allTemps;
  osThreadSetPriority(osThreadGetId(), osPriorityHigh7);
  Timer t;
  t.start();
  while (1) {
    int glv_voltage = glv_voltage_pin * 18530; // in mV
    //printf("GLV voltage: %d mV\n", glv);

    while (!canqueue.empty()) {
        CANMessage msg;
        canqueue.pop(msg);
        switch(msg.id) {
          default:
            break;
        }
    }
    ThisThread::sleep_for(50 - (t.read_ms()%50));
  }
}

void initIO() {
    canBus = new CAN(BMS_PIN_CAN_RX, BMS_PIN_CAN_TX, BMS_CAN_FREQUENCY);
    canBus->attach(canRX);


    fan_control_pin = 0; // turn fans off at start
    charge_enable_pin = 0; // charge not allowed at start
    bms_fault_pin = 0; // assume fault at start
    precharge_control_pin = 0; // positive AIR open at start

}

void canRX() {
    CANMessage msg;
    
    if (canBus->read(msg)) {
        canqueue.push(msg);
    }
}