#include "stm32f407xx.h"
#include <stdbool.h>
#include <stdint.h>

void initLed(void);
void orangeLedOn(void);
void orangeLedOff(void);

typedef enum { GPIO_PUSH_PULL = 0, GPIO_OPEN_DRAIN = 1 } gpio_otype_t;

typedef enum {
  GPIO_NO_PULL = 0,
  GPIO_PULL_UP = 1,
  GPIO_PULL_DOWN = 2
} gpio_pull_t;

void gpio_config_output(GPIO_TypeDef *port, uint32_t pin, gpio_pull_t pull,
                        gpio_otype_t otype);

void gpio_config_input(GPIO_TypeDef *port, uint32_t pin, gpio_pull_t pull);

// Alternate function configuration (for use with peripherals)
void gpio_config_af(GPIO_TypeDef *port, uint32_t pin, uint16_t af_select,
                    gpio_pull_t pull, gpio_otype_t otype);

void gpio_write_pin(GPIO_TypeDef *port, uint32_t pin, bool state);