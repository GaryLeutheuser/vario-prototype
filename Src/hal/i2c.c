#include "hal/i2c.h"
#include "stm32f407xx.h"

void i2c_write(uint8_t address, const uint8_t *data, size_t len) {
  // Send start
  I2C1->CR1 |= I2C_CR1_START;

  // Wait for SB high
  while ((I2C1->SR1 & I2C_SR1_SB_Msk) == UINT32_C(0)) {
  }

  // Unset LSB of address (write mode) + send
  I2C1->DR = address & ~UINT8_C(1);

  // Wait for ADDR high
  while ((I2C1->SR1 & I2C_SR1_ADDR_Msk) == UINT32_C(0)) {
  }

  // Read SR2 to clear ADDR
  (void)I2C1->SR2;

  // Transmit data
  for (size_t index = 0; index < len; index++) {
    // Wait for TXE high
    while ((I2C1->SR1 & I2C_SR1_TXE_Msk) == UINT32_C(0)) {
    }
    // Send byte
    I2C1->DR = data[index];
  }

  // Wait for BTF high
  while ((I2C1->SR1 & I2C_SR1_BTF_Msk) == UINT32_C(0)) {
  }
  // Wait for TXE high
  while ((I2C1->SR1 & I2C_SR1_TXE_Msk) == UINT32_C(0)) {
  }

  // Send stop
  I2C1->CR1 |= I2C_CR1_STOP;
}