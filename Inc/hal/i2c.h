#include <stddef.h>
#include <stdint.h>

// Bytes written from data pointer
void i2c_write(uint8_t address, const uint8_t *data, size_t len);

// Bytes read to data pointer
void i2c_read(uint8_t address, uint8_t *data, size_t len);