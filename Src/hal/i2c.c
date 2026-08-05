#include "hal/i2c.h"
#include "stm32f407xx.h"

void i2c_write(uint8_t address, const uint8_t *data, size_t len) {
  // Send start (wait for free bus first)
  while ((I2C1->SR2 & I2C_SR2_BUSY_Msk) != UINT32_C(0)) {
  }
  I2C1->CR1 |= I2C_CR1_START;

  // Wait for SB high
  while ((I2C1->SR1 & I2C_SR1_SB_Msk) == UINT32_C(0)) {
  }

  // Unset LSB of address after shifting left once (write mode) + send
  I2C1->DR = (address << 1u) & ~UINT8_C(1);

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
  // Ensure STOP takes before moving on
  while ((I2C1->CR1 & I2C_CR1_STOP_Msk) != UINT32_C(0)) {
  }
}

// Read 1 byte
static void i2c_read_1(uint8_t address, uint8_t *data) {
  // Send start (wait for free bus first)
  while ((I2C1->SR2 & I2C_SR2_BUSY_Msk) != UINT32_C(0)) {
  }
  I2C1->CR1 |= I2C_CR1_START;

  // Poll start
  while ((I2C1->SR1 & I2C_SR1_SB_Msk) == UINT32_C(0)) {
    // wait for SB 1
  }

  // Unset LSB of address after shifting left once (write mode) + send
  I2C1->DR = (address << 1u) | UINT8_C(1);

  // Wait for ADDR high
  while ((I2C1->SR1 & I2C_SR1_ADDR_Msk) == UINT32_C(0)) {
  }

  // No ACK for 1 byte; make sure of that.
  I2C1->CR1 &= ~I2C_CR1_ACK;

  // Finish clearing ADDR by reading SR2
  (void)I2C1->SR2;

  // Send STOP
  I2C1->CR1 |= I2C_CR1_STOP;

  // Wait for RXNE to indicate data available
  while ((I2C1->SR1 & I2C_SR1_RXNE_Msk) == UINT32_C(0)) {
  }

  // Read data
  data[0] = I2C1->DR;

  // Ensure STOP takes before moving on
  while ((I2C1->CR1 & I2C_CR1_STOP_Msk) != UINT32_C(0)) {
  }
}

// Note that 2-byte reception is unique; see page 845 RM0090 Rev 22
static void i2c_read_2(uint8_t address, uint8_t *data) {
  // Send start (wait for free bus first)
  while ((I2C1->SR2 & I2C_SR2_BUSY_Msk) != UINT32_C(0)) {
  }
  I2C1->CR1 |= I2C_CR1_START;
  // Poll start
  while ((I2C1->SR1 & I2C_SR1_SB_Msk) == UINT32_C(0)) {
    // wait for SB 1
  }

  // Unset LSB of address after shifting left once (write mode) + send
  I2C1->DR = (address << 1u) | UINT8_C(1);

  // Wait for ADDR high
  while ((I2C1->SR1 & I2C_SR1_ADDR_Msk) == UINT32_C(0)) {
  }

  // Unset ACK + set POS
  I2C1->CR1 &= ~I2C_CR1_ACK;
  I2C1->CR1 |= I2C_CR1_POS;

  // Finish clearing ADDR by reading SR2
  (void)I2C1->SR2;

  // Wait for BTF high
  while ((I2C1->SR1 & I2C_SR1_BTF_Msk) == UINT32_C(0)) {
  }
  // Set STOP
  I2C1->CR1 |= I2C_CR1_STOP;

  // Clear POS to avoid contaminating future I2C usage
  I2C1->CR1 &= ~I2C_CR1_POS;

  // Read data (DR + shift register behind it are ready)
  data[0] = I2C1->DR;
  data[1] = I2C1->DR;

  // Ensure STOP takes before moving on
  while ((I2C1->CR1 & I2C_CR1_STOP_Msk) != UINT32_C(0)) {
  }
}

// Read n > 2 bytes
static void i2c_read_n(uint8_t address, uint8_t *data, size_t len) {}

void i2c_read(uint8_t address, uint8_t *data, size_t len) {
  // Three cases:
  // 1. Read one byte
  // 2. Read two bytes
  // 3. Read n > 2 bytes
  // This peripheral is touchy, so these different cases
  // are split up in order to make the behavior easy to understand,
  // modify, and validate.

  if (len == 1) {
    i2c_read_1(address, data);
  } else if (len == 2) {
    i2c_read_2(address, data);
  } else if (len > 2) {
    i2c_read_n(address, data, len);
  } else {
    // Probably something smarter we could do here later.
    return;
  }
}