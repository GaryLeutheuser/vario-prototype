#include "drivers/ms5611.h"
#include "FreeRTOS.h"
#include "hal/i2c.h"
#include "projdefs.h"
#include "semphr.h"
#include "stm32f407xx.h"
#include "task.h"
#include <stdint.h>

static const uint8_t MS5611_ADDR = UINT8_C(0x77);

void vMs5611_Init(void *pvParameters) {
  // Send reset sequence - MS5611 page 12
  const uint8_t reset_command = UINT8_C(0x1E);
  i2c_write(MS5611_ADDR, &reset_command, 1);

  // Wait for at least 2.8 ms to reset (MS5611 datasheet page 10)
  vTaskDelay(pdMS_TO_TICKS(3));

  // Perform PROM test
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

  // One-shot init task; delete it
  vTaskDelete(NULL);
}

static uint16_t ms5611_read_PROM_value(const uint8_t address) {
  // Retrieve desired PROM data - ms5611 page 12

  // Set PROM Read Mode for given address
  // Command is 0xA<3-bit addr>0
  const uint8_t command =
      (uint8_t)(UINT32_C(0xA0) | (((uint32_t)address << 1u) & UINT32_C(0x1E)));
  i2c_write(MS5611_ADDR, &command, 1);

  uint8_t data[2];
  i2c_read(MS5611_ADDR, data, 2);

  uint16_t value = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
  return value;
}

void ms5611_read_PROM(uint16_t data[8]) {
  for (uint8_t addr = 0; addr < UINT8_C(8); addr++) {
    data[addr] = ms5611_read_PROM_value(addr);
  }
}

// Taken from AN520: Application note for ms5611, which details
// how the MS5611 PROM CRC is computed.
uint8_t ms5611_crc4(uint16_t n_prom[8]) {
  uint32_t cnt;      // simple counter
  uint32_t n_rem;    // crc remainder
  uint32_t crc_read; // original value of the crc
  uint8_t n_bit;
  n_rem = UINT8_C(0x00);

  // Save crc_read
  crc_read = n_prom[7];

  // CRC byte is replaced by 0
  n_prom[7] = (UINT16_C(0xFF00) & (n_prom[7]));

  // For each byte
  for (cnt = 0; cnt < 16u; cnt++) {
    if ((cnt % 2u) == 1u) {
      n_rem ^= (unsigned short)((n_prom[cnt >> 1]) & UINT16_C(0x00FF));
    } else {
      n_rem ^= (unsigned short)(n_prom[cnt >> 1] >> 8);
    }

    for (n_bit = 8; n_bit > UINT8_C(0); n_bit--) {
      if ((n_rem & (UINT32_C(0x8000))) != UINT32_C(0)) {
        n_rem = (n_rem << 1) ^ UINT32_C(0x3000);
      } else {
        n_rem = (n_rem << 1);
      }
    }
  }

  // Final 4-bit remainder is CRC code
  n_rem = (UINT32_C(0x000F) & (n_rem >> 12));

  // Restore crc_read
  n_prom[7] = (uint16_t)crc_read;
  return (uint8_t)(n_rem ^ UINT32_C(0x0));
}