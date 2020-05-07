#include "BmsConfig.h"
//#include "CanOpenPdo.h"
//#include "common.h"

#include "mbed.h"
#include "rtos.h"

#include "BmsThread.h"
//#include "CANOptions.h"
#include "LTC6811Bus.h"

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

//static constexpr CANConfig cancfg =
//    CANOptions::config<CANOptions::BaudRate::k500k, false>();

Serial* serial;

void initIO();

int main() {
  // Init all io pins
  initIO();
  
  /*
  canStart(&BMS_CAN_DRIVER, &cancfg);
  CANTxFrame txmsg;
  txmsg.IDE = CAN_IDE_STD;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.SID = kFuncIdCellStartup;
  txmsg.DLC = 8;
  uint8_t msg[8] = {'S', 'P', 'I', 'C', 'Y', 'B', 'O', 'I'};
  for (size_t i = 0; i < 8; i++) {
    txmsg.data8[i] = msg[i];
  }
  if (canTransmit(&BMS_CAN_DRIVER, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100)) !=
      MSG_OK) {
    sdWrite(&SD2, (const uint8_t*)"Can startup failed\r\n", 20);
  }
  */

  ThisThread::sleep_for(1000);

  SPI* spiDriver = new SPI(BMS_PIN_SPI_MOSI,
                           BMS_PIN_SPI_MISO,
                           BMS_PIN_SPI_SCLK,
                           BMS_PIN_SPI_SSEL,
                           use_gpio_ssel);
  spiDriver->format(8, 0);
  LTC6811Bus ltcBus = LTC6811Bus(spiDriver);

  BMSThread bmsThread(&ltcBus, 1);
  //bmsThread.start(NORMALPRIO + 1);

  DigitalOut led(LED1);
  // Flash LEDs to indicate startup
  for (int i = 0; i < 4; i++) {
    led = 1;
    ThisThread::sleep_for(50);
    led = 0;
    ThisThread::sleep_for(50);
  }

  while (1) {
    // Sleep 100 secs
    ThisThread::sleep_for(100 * 1000);
  }
}

void initIO() {
  serial = new Serial(USBTX, USBRX);
  serial->printf("INIT\n");
  
  // Set modes for IO
  /*
  palSetLineMode(LINE_BMS_FLT, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_BMS_FLT_LAT, PAL_MODE_INPUT);
  palSetLineMode(LINE_IMD_STATUS, PAL_MODE_INPUT);
  palSetLineMode(LINE_IMD_FLT_LAT, PAL_MODE_INPUT);
  palSetLineMode(LINE_CHARGER_CONTROL, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_SIG_CURRENT, PAL_MODE_INPUT_ANALOG);
  */

  // Reset BMS fault line
  //palSetLine(LINE_BMS_FLT);
  // palClearLine(LINE_BMS_FLT);

  // Enable charging
  //palClearLine(LINE_CHARGER_CONTROL);

  // Set modes for SPI
  /*
    palSetLineMode(LINE_SPI_MISO,
                 PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_SPI_MOSI,
                 PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_SPI_SCLK,
                 PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_SPI_SSEL,
                 PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
  */

  // Set modes for CAN
  //palSetLineMode(LINE_CAN_TX, PAL_MODE_ALTERNATE(9) | PAL_STM32_OSPEED_HIGHEST);
  //palSetLineMode(LINE_CAN_RX, PAL_MODE_ALTERNATE(9));
}
