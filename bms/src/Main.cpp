#include "BmsConfig.h"

#include "mbed.h"
#include "rtos.h"

//#include "spdlog/spdlog.h"

#include "BmsThread.h"
#include "Can.h"
//#include "CANOptions.h"
#include "LTC681xBus.h"
#include "Event.h"
#include <cstdint>

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


CAN* canBus;
DigitalOut* bmsFault;
DigitalOut* chargerControl;

void initIO();

int main() {
  // Init all io pins
  initIO();
  
  canBus->write(BMSCellStartup());

  ThisThread::sleep_for(1s);

  SPI* spiDriver = new SPI(BMS_PIN_SPI_MOSI,
                           BMS_PIN_SPI_MISO,
                           BMS_PIN_SPI_SCLK,
                           BMS_PIN_SPI_SSEL,
                           use_gpio_ssel);
  spiDriver->format(8, 0);
  auto ltcBus = LTC681xParallelBus(spiDriver);

  auto canMailbox = new BmsEventMailbox();
  auto uiMailbox = new BmsEventMailbox();
  std::vector<BmsEventMailbox*> mailboxes = { canMailbox, uiMailbox };

  Thread bmsThreadThread;
  BMSThread bmsThread(ltcBus, 1, mailboxes);
  bmsThreadThread.start(callback(&BMSThread::startThread, &bmsThread));

  DigitalOut led(LED1);
  // Flash LEDs to indicate startup
  for (int i = 0; i < 4; i++) {
    led = 1;
    ThisThread::sleep_for(50ms);
    led = 0;
    ThisThread::sleep_for(50ms);
  }

  while(true) {
    auto event = canMailbox->get();
    if (event.status == osEventMessage) {
      BmsEvent* msg = (BmsEvent*)event.value.p;

      switch(msg->getType()) {
      case BmsEventType::VoltageMeasurement: {
        auto converted = static_cast<VoltageMeasurement*>(msg);
        
        printf("Voltages: \n");
        for (int i = 0; i < BMS_BANK_COUNT; i++) {
          for(int j = 0; j < BMS_BANK_CELL_COUNT; j++) {
            printf("%4dmV ", converted->voltageValues[(i * BMS_BANK_CELL_COUNT) + j]);
          }
          printf("\n");
        }
        converted->~VoltageMeasurement();
        break;
      }
      case BmsEventType::TemperatureMeasurement: {
        auto converted = static_cast<TemperatureMeasurement*>(msg);
        
        printf("Temperatures: \n");
        for (int i = 0; i < BMS_BANK_COUNT; i++) {
          for(int j = 0; j < BMS_BANK_TEMP_COUNT; j++) {
            int8_t val = converted->temperatureValues[(i * BMS_BANK_CELL_COUNT) + j];
            if(val) {
              printf("%2d ", val);
            } else {
              printf("XX ");
            }
          }
          printf("\n");
        }
        converted->~TemperatureMeasurement();
        break;
      }
      default:
        break;
      }
      delete msg;
    }
  }
  bmsThreadThread.join();
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
