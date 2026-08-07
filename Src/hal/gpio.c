// cppcheck-suppress misra-c2012-11.4 ; see deviations.md D-001

#include "hal/gpio.h"
#include "stm32f407xx.h"
#include <stdbool.h>

static void enable_port_clock(const GPIO_TypeDef *port) {
  // Enable port clock
  if (port == GPIOA) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  } else if (port == GPIOB) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  } else if (port == GPIOC) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  } else if (port == GPIOD) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  } else if (port == GPIOE) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
  } else if (port == GPIOF) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
  } else if (port == GPIOG) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
  } else if (port == GPIOH) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
  } else if (port == GPIOI) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
  } else {
  }

  // Errata 2.2.13: Delay after an RCC peripheral clock enabling
  __DSB();
}

void gpio_config_input(GPIO_TypeDef *port, const uint32_t pin,
                       const gpio_pull_t pull_resistor_cfg) {
  enable_port_clock(port);

  // 16 pins on each port
  // Each pin has two MODER/PUPDR bits associated with it
  // You multiply the pin by two (one left shift) to get the start bit
  // e.g. pin 13 == bits 26 + 27
  uint32_t pin_lsb = pin << UINT32_C(1);
  uint32_t pin_msk = UINT32_C(3) << pin_lsb;

  // Input == 00b, clear the MODER bits for the pin
  port->MODER &= ~pin_msk;

  // Set pull-up/pull-down resistor config
  uint32_t pull_value = UINT32_C(0);
  switch (pull_resistor_cfg) {
  case GPIO_PULL_UP:
    pull_value = UINT32_C(1);
    break;
  case GPIO_PULL_DOWN:
    pull_value = UINT32_C(2);
    break;
  default:
    pull_value = UINT32_C(0);
    break;
  }

  port->PUPDR = (port->PUPDR & ~pin_msk) | (pull_value << pin_lsb);
}

void gpio_config_output(GPIO_TypeDef *port, uint32_t pin,
                        gpio_pull_t pull_resistor_cfg, gpio_otype_t otype) {
  enable_port_clock(port);

  // 16 pins on each port
  // Each pin has two MODER/PUPDR bits associated with it
  // You multiply the pin by two (one left shift) to get the start bit
  // e.g. pin 13 == bits 26 + 27
  uint32_t pin_pos = pin << UINT32_C(1);
  uint32_t pin_msk = UINT32_C(3) << pin_pos;

  // Output == 01b, first clear the bits for a clean set
  port->MODER &= ~pin_msk;
  port->MODER |= (1u << pin_pos) & pin_msk;

  // Set output type
  if (otype == GPIO_PUSH_PULL) {
    port->OTYPER &= ~(UINT32_C(1) << pin);
  } else if (otype == GPIO_OPEN_DRAIN) {
    port->OTYPER |= UINT32_C(1) << pin;
  } else {
  }

  // Set pull-up/pull-down resistor config
  uint32_t pull_value = UINT32_C(0);
  switch (pull_resistor_cfg) {
  case GPIO_PULL_UP:
    pull_value = UINT32_C(1);
    break;
  case GPIO_PULL_DOWN:
    pull_value = UINT32_C(2);
    break;
  default:
    pull_value = UINT32_C(0);
    break;
  }

  port->PUPDR = (port->PUPDR & ~pin_msk) | (pull_value << pin_pos);
}

void gpio_config_af(GPIO_TypeDef *port, uint32_t pin, uint16_t af_select,
                    gpio_pull_t pull_resistor_cfg, gpio_otype_t otype) {
  enable_port_clock(port);

  // 16 pins on each port
  // Each pin has two MODER/PUPDR bits associated with it
  // You multiply the pin by two (one left shift) to get the start bit
  // e.g. pin 13 == bits 26 + 27
  uint32_t pin_pos = pin << UINT32_C(1);
  uint32_t pin_msk = UINT32_C(3) << pin_pos;

  // AF == 10b, first clear the bits for a clean set
  port->MODER &= ~pin_msk;
  port->MODER |= (2u << pin_pos) & pin_msk;

  // Set output type
  if (otype == GPIO_PUSH_PULL) {
    port->OTYPER &= ~(UINT32_C(1) << pin);
  } else if (otype == GPIO_OPEN_DRAIN) {
    port->OTYPER |= UINT32_C(1) << pin;
  } else {
  }

  // Set pull-up/pull-down resistor config
  uint32_t pull_value = UINT32_C(0);
  switch (pull_resistor_cfg) {
  case GPIO_PULL_UP:
    pull_value = UINT32_C(1);
    break;
  case GPIO_PULL_DOWN:
    pull_value = UINT32_C(2);
    break;
  default:
    pull_value = UINT32_C(0);
    break;
  }

  port->PUPDR = (port->PUPDR & ~pin_msk) | (pull_value << pin_pos);

  // Select the particular AF
  // Pin 0-7 are on AFRL
  // Pin 8-15 are on AFRH
  // Clear, then set, in case of pin re-use

  if (pin < UINT32_C(8)) {
    uint32_t af_pos = UINT32_C(4) * pin;
    port->AFR[0] &= ~(UINT32_C(0xF) << af_pos);
    port->AFR[0] |= af_select << af_pos;
  } else if (pin <= UINT32_C(15)) {
    uint32_t af_pos = UINT32_C(4) * (pin - UINT32_C(8));
    port->AFR[1] &= ~(UINT32_C(0xF) << af_pos);
    port->AFR[1] |= af_select << af_pos;
  } else {
  }
}

void gpio_write_pin(GPIO_TypeDef *port, uint32_t pin, bool state) {
  if (state) {
    // Set bits are in the low word
    port->BSRR = (UINT32_C(1) << pin);
  } else {
    // Reset bits are in the high word
    port->BSRR = (UINT32_C(1) << (pin + UINT32_C(16)));
  }
}

void initLed(void) {
  // I/O PD13 - orange LED on discovery board
  gpio_config_output(GPIOD, 13, GPIO_NO_PULL, GPIO_PUSH_PULL);
}

void orangeLedOn(void) { gpio_write_pin(GPIOD, 13, true); }

void orangeLedOff(void) { gpio_write_pin(GPIOD, 13, false); }