#include "board.h"
#include "drivers/ms5611.h"
#include "hal/gpio.h"
#include "hal/i2c.h"
#include "hal/systick.h"
#include "stm32f407xx.h"
#include <stdint.h>

int main(void) {
  initLed();
  systick_init();
  board_init();
  i2c_init();
  ms5611_init();

  // Start with test values from AN520 to validate CRC algorithm
  uint16_t nprom[] = {0x3132, 0x3334, 0x3536, 0x3738,
                      0x3940, 0x4142, 0x4344, 0x4500};
  uint8_t test_crc = ms5611_crc4(nprom);
  (void)test_crc;

  // Zero nprom variable
  for (uint8_t i = 0; i < UINT8_C(8); i++) {
    nprom[i] = 0x0000;
  }

  // Compute actual PROM of factory written data
  ms5611_read_PROM(nprom);
  test_crc = ms5611_crc4(nprom);
  (void)test_crc;

  // Blinky for now
  uint8_t led_on = 0;
  uint32_t start_tick = 0u;

  for (;;) {
    static const uint32_t FLASH_PERIOD_MS = 1000u;
    if ((systick_get_ms() - start_tick) >= FLASH_PERIOD_MS) {
      start_tick += FLASH_PERIOD_MS;
      if (led_on == 0u) {
        orangeLedOn();
        led_on = 1;
      } else {
        orangeLedOff();
        led_on = 0;
      }
    }

    // cppcheck-suppress misra-c2012-17.3 ; macro, see tool-defects.md TD-001
    __WFI();
  }
}