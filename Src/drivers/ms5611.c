#include "drivers/ms5611.h"
#include "hal/i2c.h"
#include "hal/systick.h"
#include "stm32f407xx.h"
#include <stdint.h>

static const uint8_t MS5611_ADDR = UINT16_C(0x77);

void ms5611_init() {
  // TODO: Move this GPIO stuff into GPIO HAL and call it from here

  // Hardware configuration
  // PB9 - IC21 SDA
  // PB8 - IC21 SCL
  // PS pulled high for I2C
  // CSB pulled LOW, so final addr bit is HIGH: 0x111011C1

  // Set up I2C pins
  // Open-drain + pull-up (I2C requirements) + select AF4 (see table 9 of
  // the datasheet doc DS8626)

  // Enable GPIOB
  // Enable GPIOD clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

  // Set AF on the pins
  GPIOB->MODER |=
      ((UINT32_C(2) << GPIO_MODER_MODER8_Pos) & GPIO_MODER_MODER8_Msk) |
      ((UINT32_C(2) << GPIO_MODER_MODER9_Pos) & GPIO_MODER_MODER9_Msk);

  // Set open-drain on the pins
  GPIOB->OTYPER |= GPIO_OTYPER_OT8_Msk | GPIO_OTYPER_OT9_Msk;

  // Set pull-up resistors on the pins
  GPIOB->PUPDR |=
      ((UINT32_C(1) << GPIO_PUPDR_PUPD8_Pos) & GPIO_PUPDR_PUPD8_Msk) |
      ((UINT32_C(1) << GPIO_PUPDR_PUPD9_Pos) & GPIO_PUPDR_PUPD9_Msk);

  // Pick AF4 on each pin
  GPIOB->AFR[1] |=
      ((UINT32_C(4) << GPIO_AFRH_AFSEL8_Pos) & GPIO_AFRH_AFSEL8_Msk) |
      ((UINT32_C(4) << GPIO_AFRH_AFSEL9_Pos) & GPIO_AFRH_AFSEL9_Msk);

  // Turn on I2C1 module clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

  // Configure I2C1
  // Set FREQ to 16 MHz
  I2C1->CR2 |= (UINT32_C(16) << I2C_CR2_FREQ_Pos) & I2C_CR2_FREQ_Msk;
  // Set CCR
  I2C1->CCR |= (UINT32_C(80) << I2C_CCR_CCR_Pos) & I2C_CCR_CCR_Msk;
  // Set TRISE
  I2C1->TRISE = UINT32_C(17); // 1000 ns / 63 ns == 15.87 == 16 + 1 = 17
  // Set controller mode

  // Enable I2C1
  I2C1->CR1 |= I2C_CR1_PE;

  // Send reset sequence - MS5611 page 12
  const uint8_t reset_command = UINT8_C(0x1E);
  i2c_write(MS5611_ADDR, &reset_command, 1);

  // Wait for at least a tick to allow the MS5611 to reset.
  uint32_t reset_wait_start = systick_get_ms();
  while (systick_get_ms() - reset_wait_start < 10) {
    // wait
  }
}

static uint16_t ms5611_read_PROM_value(const uint8_t address) {
  // Retrieve desired PROM data - ms5611 page 12

  // Set PROM Read Mode for given address
  // Command is 0xA<3-bit addr>0
  const uint8_t command = 0xA0 | ((address << 1u) & 0x1E);
  i2c_write(MS5611_ADDR, &command, 1);

  uint8_t data[2];
  i2c_read(MS5611_ADDR, data, 2);

  uint16_t value = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
  return value;
}

void ms5611_read_PROM(uint16_t data[8]) {
  for (uint8_t addr = 0; addr < 8; addr++) {
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
  for (cnt = 0; cnt < 16; cnt++) {
    if (cnt % 2 == 1) {
      n_rem ^= (unsigned short)((n_prom[cnt >> 1]) & UINT16_C(0x00FF));
    } else {
      n_rem ^= (unsigned short)(n_prom[cnt >> 1] >> 8);
    }

    for (n_bit = 8; n_bit > 0; n_bit--) {
      if (n_rem & (UINT32_C(0x8000))) {
        n_rem = (n_rem << 1) ^ UINT32_C(0x3000);
      } else {
        n_rem = (n_rem << 1);
      }
    }
  }

  // Final 4-bit remainder is CRC code
  n_rem = (UINT32_C(0x000F) & (n_rem >> 12));

  // Restore crc_read
  n_prom[7] = crc_read;
  return (n_rem ^ UINT32_C(0x0));
}