#include "BmsConfig.h"

#include "mbed.h"
#include "rtos.h"

//#include "spdlog/spdlog.h"

#include "BmsThread.h"
#include "Can.h"
//#include "CANOptions.h"
#include "Event.h"
#include "LTC681xBus.h"
#include <cstdint>
#include <cstdio>

// When car is off maybe one reading every 10 sec
// when car is on 10 readings per second

// TODO: map 12 cells to number of cells
// TODO: change m_chips to array rather than vector
// TODO: publish fault states
// TODO: SoC tracking using Ah counting and voltage measuring

// Fault states:
// - AMS
// - IMD
//
// * Over temp
// - Under temp
// * Over voltage
// * Under voltage
//
// * Failed init
// * Comm fail

CAN *canBus;
DigitalOut *bmsFault;
DigitalOut *chargerControl;

DigitalOut trigger(PB_6);

void initIO();

int main() {
  // Init all io pins
  initIO();

  trigger = 0;

  canBus->write(BMSCellStartup());

  ThisThread::sleep_for(1s);

  SPI *spiDriver = new SPI(BMS_PIN_SPI_MOSI, BMS_PIN_SPI_MISO, BMS_PIN_SPI_SCLK,
                           BMS_PIN_SPI_SSEL, use_gpio_ssel);

  // SPI *spiDriver = new SPI(BMS_PIN_SPI_MOSI, BMS_PIN_SPI_MISO,
  // BMS_PIN_SPI_SCLK); DigitalOut cs(BMS_PIN_SPI_SSEL);

  spiDriver->format(8, 0);
  //spiDriver->frequency(1250000);
  auto ltcBus = LTC681xParallelBus(spiDriver);

  auto canMailbox = new BmsEventMailbox();
  auto uiMailbox = new BmsEventMailbox();
  std::vector<BmsEventMailbox *> mailboxes = {canMailbox, uiMailbox};

  // ltcBus.WakeupBus();
  // printf("main wakeup1\n");
  // ltcBus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(
  //     StartSelfTestCellVoltage(AdcMode::k7k, SelfTestMode::kSelfTest1)));
  // printf("main Send Command\n");
  // ThisThread::sleep_for(4ms);
  // ltcBus.WakeupBus();
  // printf("main wakeup2\n");

  auto ledCmd =
      LTC681xBus::BuildBroadcastBusCommand(WriteConfigurationGroupA());
  uint8_t statusOn[6] = {0x78, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t statusOff[6] = {0xf8, 0x00, 0x00, 0x00, 0x00, 0x00};

  // while(1);

  std::array<uint16_t, BMS_BANK_COUNT * BMS_BANK_CELL_COUNT> allVoltages;
  ltcBus.WakeupBus();
  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    // if (ltcBus.PollAdcCompletion(LTC681xBus::BuildAddressedBusCommand(
    //         PollADCStatus(), 0)) ==
    //         LTC681xBus::LTC681xBusStatus::PollTimeout) {
    //   printf("Poll timeout.\n");
    // } else {
    //   printf("Poll OK.\n");
    // }

    trigger = 1;
    ThisThread::sleep_for(1ms);
    trigger = 0;

    // ltcBus.WakeupBus();
    // printf("wakeup1\n");
    // ltcBus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(
    //     StartSelfTestCellVoltage(AdcMode::k7k, SelfTestMode::kSelfTest1)));
    // printf("Send Command\n");
    // ThisThread::sleep_for(4ms);
    // ltcBus.WakeupBus();
    // printf("wakeup2\n");
    // for (int i = 0; i < BMS_BANK_COUNT; i++) {
    //   uint16_t rawVoltages[12];
    //   if (ltcBus.SendReadCommand(
    //           LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i),
    //           (uint8_t *)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
    //     printf("Things are not okay. SelfTestVoltageA\n");
    //   }
    //   if (ltcBus.SendReadCommand(
    //           LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i),
    //           (uint8_t *)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
    //     printf("Things are not okay. SelfTestVoltageB\n");
    //   }
    //   if (ltcBus.SendReadCommand(
    //           LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), i),
    //           (uint8_t *)rawVoltages + 12) !=
    //       LTC681xBus::LTC681xBusStatus::Ok) {
    //     printf("Things are not okay. SelfTestVoltageC\n");
    //   }
    //   if (ltcBus.SendReadCommand(
    //           LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), i),
    //           (uint8_t *)rawVoltages + 18) !=
    //       LTC681xBus::LTC681xBusStatus::Ok) {
    //     printf("Things are not okay. SelfTestVoltageD\n");
    //   }

    //   for (int j = 0; j < 12; j++) {
    //     printf("AXST %2d: %4x\n", j, rawVoltages[i]);
    //   }
    // }

    // for (int i = 0; i < 4; i++) {
    //   ltcBus.WakeupBus();
    //   ltcBus.SendDataCommand(ledCmd, statusOn);
    //   ThisThread::sleep_for(100ms);
    //   ltcBus.WakeupBus();
    //   ltcBus.SendDataCommand(ledCmd, statusOff);
    //   ThisThread::sleep_for(100ms);
    // }

    // auto startAdcCmd =
    //     StartCellVoltageADC(AdcMode::k7k, false, CellSelection::kAll);
    // if (ltcBus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(startAdcCmd)) !=
    //     LTC681xBus::LTC681xBusStatus::Ok) {
    //   printf("Things are not okay. StartADC\n");
    // }

    // // Read back values from all chips
    // for (int i = 0; i < BMS_BANK_COUNT; i++) {
    //   if (ltcBus.PollAdcCompletion(
    //           LTC681xBus::BuildAddressedBusCommand(PollADCStatus(), 0)) ==
    //       LTC681xBus::LTC681xBusStatus::PollTimeout) {
    //     printf("Poll timeout.\n");
    //   } else {
    //     printf("Poll OK.\n");
    //   }

    //   uint16_t rawVoltages[12];
    //   if (ltcBus.SendReadCommand(
    //           LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i),
    //           (uint8_t *)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
    //     printf("Things are not okay. VoltageA\n");
    //   }
    //   if (ltcBus.SendReadCommand(
    //           LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i),
    //           (uint8_t *)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
    //     printf("Things are not okay. VoltageB\n");
    //   }
    //   if (ltcBus.SendReadCommand(
    //           LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), i),
    //           (uint8_t *)rawVoltages + 12) !=
    //       LTC681xBus::LTC681xBusStatus::Ok) {
    //     printf("Things are not okay. VoltageC\n");
    //   }
    //   if (ltcBus.SendReadCommand(
    //           LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), i),
    //           (uint8_t *)rawVoltages + 18) !=
    //       LTC681xBus::LTC681xBusStatus::Ok) {
    //     printf("Things are not okay. VoltageD\n");
    //   }

    //   for (int j = 0; j < 12; j++) {
    //     // Endianness of the protocol allows a simple cast :-)
    //     uint16_t voltage = rawVoltages[j] / 10;

    //     int index = BMS_CELL_MAP[j];
    //     if (index != -1) {
    //       allVoltages[(BMS_BANK_CELL_COUNT * i) + index] = voltage;
    //     }
    //   }
    // }

    // for (int i = 0; i < BMS_BANK_COUNT; i++) {
    //   printf("Bank %d V: ", i);
    //   for (int j = 0; j < BMS_BANK_CELL_COUNT; j++) {
    //     printf("%d, ", allVoltages[i * BMS_BANK_CELL_COUNT + j]);
    //   }
    //   printf("\n");
    // }


    while (true) {
    ltcBus.WakeupBus();
    
    // Set all status lights high
    // TODO: This should be in some sort of config class
    uint8_t statusOn[6] = { 0x78, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t statusOff[6] = { 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00 };
    auto ledCmd = LTC681xBus::BuildBroadcastBusCommand(WriteConfigurationGroupA());
    ltcBus.SendDataCommand(ledCmd, statusOn);

    // Start ADC on all chips
    auto startAdcCmd = StartCellVoltageADC(AdcMode::k7k, false, CellSelection::kAll);
    if(ltcBus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(startAdcCmd)) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. StartADC\n");
    }

    // Read back values from all chips
    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      if(ltcBus.PollAdcCompletion(LTC681xBus::BuildAddressedBusCommand(PollADCStatus(), 0)) == LTC681xBus::LTC681xBusStatus::PollTimeout) {
        printf("Poll timeout.\n");
      } else {
        printf("Poll OK.\n");
      }

      uint16_t rawVoltages[12];
      if(ltcBus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i), (uint8_t*)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageA\n");
      }
      if(ltcBus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i), (uint8_t*)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageB\n");
      }
      if(ltcBus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), i), (uint8_t*)rawVoltages + 12) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageC\n");
      }
      if(ltcBus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), i), (uint8_t*)rawVoltages + 18) != LTC681xBus::LTC681xBusStatus::Ok) {
        printf("Things are not okay. VoltageD\n");
      }

      for (int j = 0; j < 12; j++) {
        // Endianness of the protocol allows a simple cast :-)
        uint16_t voltage = rawVoltages[j] / 10;

        int index = BMS_CELL_MAP[j];
        if (index != -1) {
          allVoltages[(BMS_BANK_CELL_COUNT * i) + index] = voltage;
        }
      }
    }

    ltcBus.SendDataCommand(ledCmd, statusOff);


    for (int i = 0; i < BMS_BANK_COUNT; i++) {
      printf("Bank %d V: ", i);
      for (int j = 0; j < BMS_BANK_CELL_COUNT; j++) {
        printf("%d, ", allVoltages[i * BMS_BANK_CELL_COUNT + j]);
      }
      printf("\n");
    }


    ThisThread::sleep_for(500ms);
  }








    while (1);







    uint16_t rawVoltages[12];
    if (ltcBus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(), i),
            (uint8_t *)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. VoltageA\n");
    }

    if (ltcBus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(), i),
            (uint8_t *)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. VoltageB\n");
    }
    if (ltcBus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(), i),
            (uint8_t *)rawVoltages + 12) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. VoltageC\n");
    }
    if (ltcBus.SendReadCommand(
            LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(), i),
            (uint8_t *)rawVoltages + 18) != LTC681xBus::LTC681xBusStatus::Ok) {
      printf("Things are not okay. VoltageD\n");
    }

    for (int j = 0; j < 12; j++) {
      // Endianness of the protocol allows a simple cast :-)
      uint16_t voltage = rawVoltages[j] / 10;

      int index = BMS_CELL_MAP[j];
      if (index != -1) {
        allVoltages[(BMS_BANK_CELL_COUNT * i) + index] = voltage;
      }
    }
  }

  for (int i = 0; i < BMS_BANK_COUNT; i++) {
    printf("Bank %d V: ", i);
    for (int j = 0; j < BMS_BANK_CELL_COUNT; j++) {
      printf("%d, ", allVoltages[i * BMS_BANK_CELL_COUNT + j]);
    }
    printf("\n");
  }

  printf("\n");
  uint8_t configGroup;
  ltcBus.SendReadCommand(
      LTC681xBus::BuildAddressedBusCommand(ReadConfigurationGroupA(), 0),
      &configGroup);
  printf("config group: %d \n", configGroup);

  // while (1);

  // Thread bmsThreadThread;
  BMSThread bmsThread(ltcBus, 1, mailboxes);
  // bmsThreadThread.start(callback(&BMSThread::startThread, &bmsThread));

  DigitalOut led(LED1);
  // Flash LEDs to indicate startup
  for (int i = 0; i < 4; i++) {
    led = 1;
    ThisThread::sleep_for(50ms);
    led = 0;
    ThisThread::sleep_for(50ms);
  }

  /*cs = 1;
  spiDriver->format(8, 3);
  spiDriver->frequency(1000000);
  cs = 0;
  spiDriver->write(0x8F);
  int whoami = spiDriver->write(0x00);
  printf("WHOAMI register = 0x%X\n", whoami);
  Deselect the device
  cs = 1;*/

  // while (true) {

  // Perform self tests

  // Cell Voltage self test
  //   ltcBus.WakeupBus();
  //   printf("wakeup1\n");
  //   ltcBus.SendCommand(LTC681xBus::BuildBroadcastBusCommand(StartSelfTestCellVoltage(AdcMode::k7k,
  //   SelfTestMode::kSelfTest1))); printf("Send Command\n");
  //   ThisThread::sleep_for(4ms);
  //   ltcBus.WakeupBus();
  //   printf("wakeup2\n");
  //   for (int i = 0; i < BMS_BANK_COUNT; i++) {
  //     uint16_t rawVoltages[12];
  //     if(ltcBus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupA(),
  //     i), (uint8_t*)rawVoltages) != LTC681xBus::LTC681xBusStatus::Ok) {
  //       printf("Things are not okay. SelfTestVoltageA\n");
  //     }
  //     if(ltcBus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupB(),
  //     i), (uint8_t*)rawVoltages + 6) != LTC681xBus::LTC681xBusStatus::Ok) {
  //       printf("Things are not okay. SelfTestVoltageB\n");
  //     }
  //     if(ltcBus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupC(),
  //     i), (uint8_t*)rawVoltages + 12) != LTC681xBus::LTC681xBusStatus::Ok) {
  //       printf("Things are not okay. SelfTestVoltageC\n");
  //     }
  //     if(ltcBus.SendReadCommand(LTC681xBus::BuildAddressedBusCommand(ReadCellVoltageGroupD(),
  //     i), (uint8_t*)rawVoltages + 18) != LTC681xBus::LTC681xBusStatus::Ok) {
  //       printf("Things are not okay. SelfTestVoltageD\n");
  //     }

  //   }
  //}
  // bmsThreadThread.join();
}

void initIO() {
  printf("INIT\n");

  canBus = new CAN(BMS_PIN_CAN_RX, BMS_PIN_CAN_TX, BMS_CAN_FREQUENCY);

  bmsFault = new DigitalOut(BMS_PIN_BMS_FLT);

  chargerControl = new DigitalOut(BMS_PIN_CHARGER_CONTROL);

  // Set modes for IO
  /*
  palSetLineMode(LINE_BMS_FLT_LAT, PAL_MODE_INPUT);
  palSetLineMode(LINE_IMD_STATUS, PAL_MODE_INPUT);
  palSetLineMode(LINE_IMD_FLT_LAT, PAL_MODE_INPUT);
  palSetLineMode(LINE_SIG_CURRENT, PAL_MODE_INPUT_ANALOG);
  */

  // Reset BMS fault line
  bmsFault->write(1);
  bmsFault->write(0);

  // Enable charging
  chargerControl->write(0);
}