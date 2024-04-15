#include <array>
#include <cstdio>
#include <vector>
#include <iostream>

#include "mbed.h"

#include "LTC681xParallelBus.h"
#include "BmsThread.h"


CAN* canBus;

void initIO();

CircularBuffer<CANMessage, 32> canqueue;

uint8_t canCount;

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
}