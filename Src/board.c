#include "board.h"
#include "hal/gpio.h"

void board_init(void) {
  // I2C1 - PB8: SCL, PB9: SDA
  gpio_config_af(GPIOB, 8, 4, GPIO_PULL_UP, GPIO_OPEN_DRAIN);
  gpio_config_af(GPIOB, 9, 4, GPIO_PULL_UP, GPIO_OPEN_DRAIN);
}