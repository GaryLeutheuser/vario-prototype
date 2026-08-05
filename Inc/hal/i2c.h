#include <stddef.h>
#include <stdint.h>

// Note: addresses must be the 7-bit I2C device address. MSB will be discarded.
// The functions handle adding the R/W bit.

// Bytes written from data pointer
void i2c_write(uint8_t address, const uint8_t *data, size_t len);

// Bytes read to data pointer
void i2c_read(uint8_t address, uint8_t *data, size_t len);