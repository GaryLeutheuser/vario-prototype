#include <stdint.h>

void vMs5611_Init(void *pvParameters);

// Read all PROM values from MS5611 into the provided data
void ms5611_read_PROM(uint16_t data[8]);

uint8_t ms5611_crc4(uint16_t n_prom[8]);