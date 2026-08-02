#include <stdint.h>

static void i2c_start(void);
static void i2c_stop(void);

// Probably want a form where we can read/write more data from buffers
void i2c_write(uint8_t address, uint8_t data);
uint8_t i2c_read(uint8_t address);

// full cycle:
// 1. send start (SDA low -> SCL low)
// 2. send addr (clock out 7 bits, r/w, ack)
// 3. send / receieve data frame