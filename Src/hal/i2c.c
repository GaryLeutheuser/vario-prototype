#include "hal/i2c.h"
#include "FreeRTOS.h"
#include "stm32f407xx.h"
#include "task.h"
#include <stdbool.h>

typedef enum {
  I2C_STATE_IDLE,
  I2C_STATE_START_SENT,
  I2C_STATE_ADDR_SENT,
  I2C_STATE_READING,
  I2C_STATE_WRITING,
} i2c_state_t;

typedef struct {
  i2c_state_t state;
  uint8_t address;
  bool reading;
  uint8_t *buffer;
  size_t length;
  size_t index;
  // i2c_status_t status;
  TaskHandle_t waiting_task;
} i2c_xfer_t;

static volatile i2c_xfer_t xfer;

void I2C1_EV_IRQHandler(void) {
  switch (xfer.state) {
  case I2C_STATE_START_SENT:
    if ((I2C1->SR1 & I2C_SR1_SB) == UINT32_C(0)) {
      // err?
    }

    if (xfer.reading) {
      I2C1->DR = ((uint32_t)xfer.address << 1u) | UINT8_C(1);
    } else {
      I2C1->DR = ((uint32_t)xfer.address << 1U) & ~UINT32_C(1);
    }

    xfer.state = I2C_STATE_ADDR_SENT;
    break;

  case I2C_STATE_ADDR_SENT:
    if ((I2C1->SR1 & I2C_SR1_ADDR) == UINT32_C(0)) {
      // err?
    }

    if (xfer.reading && xfer.length == 1) {
      // No ACK for 1 byte reads; make sure of that.
      I2C1->CR1 &= ~I2C_CR1_ACK;
    } else if (xfer.reading && xfer.length == 2) {
      // Unset ACK + set POS for 2 byte reception
      I2C1->CR1 &= ~I2C_CR1_ACK;
      I2C1->CR1 |= I2C_CR1_POS;
    }

    // Finish clearing ADDR by reading SR2
    (void)I2C1->SR2;

    if (xfer.reading && xfer.length == 1) {
      // Send STOP
      I2C1->CR1 |= I2C_CR1_STOP;
    }

    // Wait for data in buffer (next event)
    if (xfer.reading) {
      xfer.state = I2C_STATE_READING;
    }
    // Send first data byte (TXE already free)
    else {
      I2C1->DR = xfer.buffer[xfer.index++];
      xfer.state = I2C_STATE_WRITING;
    }
    break;

  case I2C_STATE_WRITING:
    if (xfer.index == xfer.length) {
      // We are done
      I2C1->CR1 |= I2C_CR1_STOP;
      xfer.state = I2C_STATE_IDLE;
    } else {
      if ((I2C1->SR1 & I2C_SR1_BTF) == UINT32_C(0)) {
        // err?
      }

      I2C1->DR = xfer.buffer[xfer.index++];
      xfer.state = I2C_STATE_WRITING;
    }
    break;

  case I2C_STATE_READING:
    // 2 byte receive
    if (xfer.length == 2) {
      // Set STOP
      I2C1->CR1 |= I2C_CR1_STOP;
      // Clear POS to avoid contaminating future I2C usage
      I2C1->CR1 &= ~I2C_CR1_POS;
      xfer.buffer[0] = (uint8_t)I2C1->DR;
      xfer.buffer[1] = (uint8_t)I2C1->DR;
    }
    // 1 byte receive
    else {
      xfer.buffer[0] = (uint8_t)(I2C1->DR);
      I2C1->CR1 |= I2C_CR1_STOP;
    }

    xfer.state = I2C_STATE_IDLE;
    break;

  case I2C_STATE_IDLE:
    break;
  }
}

void I2C1_ER_IRQHandler(void) {}

void i2c_init() {
  // Turn on I2C1 module clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

  // Configure I2C1
  // Set FREQ to 16 MHz
  I2C1->CR2 |= (UINT32_C(16) << I2C_CR2_FREQ_Pos) & I2C_CR2_FREQ_Msk;
  // Set CCR
  I2C1->CCR |= (UINT32_C(80) << I2C_CCR_CCR_Pos) & I2C_CCR_CCR_Msk;
  // Set TRISE
  I2C1->TRISE = UINT32_C(17); // 1000 ns / 63 ns == 15.87 == 16 + 1 = 17

  // Interrupts
  I2C1->CR2 |= I2C_CR2_ITEVTEN | I2C_CR2_ITERREN;

  // Enable I2C1
  I2C1->CR1 |= I2C_CR1_PE;
}

void i2c_write_isr(uint8_t address, uint8_t *data, size_t len) {
  xfer.address = address;
  xfer.buffer = data;
  xfer.length = len;
  xfer.reading = false;
  xfer.state = I2C_STATE_START_SENT;
  xfer.index = 0;
  I2C1->CR1 |= I2C_CR1_START;
}

void i2c_read_isr(uint8_t address, uint8_t *data, size_t len) {
  xfer.address = address;
  xfer.buffer = data;
  xfer.length = len;
  xfer.reading = true;
  xfer.state = I2C_STATE_START_SENT;
  xfer.index = 0;
  I2C1->CR1 |= I2C_CR1_START;
}

void i2c_write(uint8_t address, const uint8_t *data, size_t len) {
  // Send start (wait for free bus first)
  while ((I2C1->SR2 & I2C_SR2_BUSY_Msk) != UINT32_C(0)) {
  }
  I2C1->CR1 |= I2C_CR1_START;

  // Wait for SB high
  while ((I2C1->SR1 & I2C_SR1_SB_Msk) == UINT32_C(0)) {
  }

  // Unset LSB of address after shifting left once (write mode) + send
  I2C1->DR = ((uint32_t)address << 1U) & ~UINT32_C(1);

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
  data[0] = (uint8_t)(I2C1->DR);

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
  data[0] = (uint8_t)I2C1->DR;
  data[1] = (uint8_t)I2C1->DR;

  // Ensure STOP takes before moving on
  while ((I2C1->CR1 & I2C_CR1_STOP_Msk) != UINT32_C(0)) {
  }
}

// Read n > 2 bytes
// static void i2c_read_n(uint8_t address, const uint8_t *data, size_t len) {}

void i2c_read(uint8_t address, uint8_t *data, size_t len) {
  // Three cases:
  // 1. Read one byte
  // 2. Read two bytes
  // 3. Read n > 2 bytes
  // This peripheral is touchy, so these different cases
  // are split up in order to make the behavior easy to understand,
  // modify, and validate.

  if (len == (size_t)(1)) {
    i2c_read_1(address, data);
  } else if (len == (size_t)(2)) {
    i2c_read_2(address, data);
  }
  // else if (len > (size_t)(2)) {
  //   i2c_read_n(address, data, len);
  // }
  else {
  }
}