#include "FreeRTOS.h"
#include "board.h"
#include "drivers/ms5611.h"
#include "hal/gpio.h"
#include "hal/i2c.h"
#include "projdefs.h"
#include "stm32f407xx.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

#define BLINKY_STACK_WORDS 128
static StackType_t blinky_stack[BLINKY_STACK_WORDS];
static StaticTask_t blinky_tcb;

#define MS5611_INIT_STACK_WORDS 128
static StackType_t ms5611_init_stack[MS5611_INIT_STACK_WORDS];
static StaticTask_t ms5611_init_tcb;

void vBlinkyTask(void *pvParameters) {
  bool led_state = false;
  for (;;) {
    if (led_state) {
      orangeLedOff();
      led_state = false;
    } else {
      orangeLedOn();
      led_state = true;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void) {

  // Next step: trigger one of these and make sure we make it into a (no-op) ISR
  // function to establish baseline wiring.

  initLed();

  board_init();
  i2c_init();

  // 3. Enable I2C1_EV in the NVIC (position 31)
  NVIC_EnableIRQ(I2C1_EV_IRQn);
  NVIC_SetPriority(I2C1_EV_IRQn, 5);
  // 4. Enable I2C1_ER in the NVIC (position 32)
  NVIC_EnableIRQ(I2C1_ER_IRQn);
  NVIC_SetPriority(I2C1_ER_IRQn, 5);

  xTaskCreateStatic(vBlinkyTask, "Blinky", BLINKY_STACK_WORDS, NULL, 1,
                    blinky_stack, &blinky_tcb);

  xTaskCreateStatic(vMs5611_Init, "MS5611Init", MS5611_INIT_STACK_WORDS, NULL,
                    1, ms5611_init_stack, &ms5611_init_tcb);

  vTaskStartScheduler();

  for (;;) {
    // cppcheck-suppress misra-c2012-17.3 ; macro, see tool-defects.md TD-001
    // __WFI();
  }
}