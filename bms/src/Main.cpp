#include "BmsConfig.h"

#include "mbed.h"
#include "rtos.h"

//#include "spdlog/spdlog.h"

#include "BmsThread.h"
#include "Can.h"
//#include "CANOptions.h"
#include "LTC681xBus.h"
#include "Event.h"

#ifdef TARGET_NUCLEO_F303K8
  // Required for setting MISO to pull up on STM32
  #include <targets/TARGET_STM/TARGET_STM32F3/STM32Cube_FW/STM32F3xx_HAL_Driver/stm32f3xx_ll_gpio.h>
#endif

CAN canBus = CAN(BMS_PIN_CAN_RX, BMS_PIN_CAN_TX, BMS_CAN_FREQUENCY);
DigitalOut bmsFault = DigitalOut(BMS_PIN_BMS_FLT);
DigitalOut chargerControl = DigitalOut(BMS_PIN_CHARGER_CONTROL);

void initIO();

// TODO: We need a better way of statically allocating thread stack
constexpr size_t STACK_SIZE = 1024;
unsigned char bmsThreadStack[STACK_SIZE];
Thread bmsThreadThread = Thread(osPriorityNormal, STACK_SIZE, bmsThreadStack);

int main() {
  // Init all io pins
  initIO();
  
  canBus.write(BMSCellStartup());

  SPI spiDriver = SPI(BMS_PIN_SPI_MOSI,
                      BMS_PIN_SPI_MISO,
                      BMS_PIN_SPI_SCLK,
                      BMS_PIN_SPI_SSEL,
                      use_gpio_ssel);
  
#ifdef TARGET_NUCLEO_F303K8
  // This is a hack required to set the MISO pin to use a pull up resistor
  LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_6, LL_GPIO_PULL_UP);
#endif

  spiDriver.format(8, 0);
  auto ltcBus = LTC681xParallelBus(&spiDriver);

  auto canMailbox = BmsEventMailbox();
  auto uiMailbox = BmsEventMailbox();
  std::array<BmsEventMailbox*, 2> mailboxes = { &canMailbox, &uiMailbox };

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
    auto event = canMailbox.get();
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
            auto val = converted->temperatureValues[(i * BMS_BANK_CELL_COUNT) + j];
            if(val.has_value()) {
              printf("%2d ", val.value());
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
  
  // Set modes for IO
  /*
  palSetLineMode(LINE_BMS_FLT_LAT, PAL_MODE_INPUT);
  palSetLineMode(LINE_IMD_STATUS, PAL_MODE_INPUT);
  palSetLineMode(LINE_IMD_FLT_LAT, PAL_MODE_INPUT);
  palSetLineMode(LINE_SIG_CURRENT, PAL_MODE_INPUT_ANALOG);
  */

  // Reset BMS fault line
  bmsFault.write(1);
  bmsFault.write(0);

  // Enable charging
  chargerControl.write(0);
}
