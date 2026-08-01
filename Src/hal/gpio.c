// cppcheck-suppress misra-c2012-11.4 ; see deviations.md D-001

#include "hal/gpio.h"
#include "stm32f407xx.h"

void initLed(void) {
  // I/O PD13 - orange LED on discovery board

  // Enable GPIOD clock
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

  // PD13 -- output
  GPIOD->MODER |=
      (UINT32_C(1) << GPIO_MODER_MODER13_Pos) & GPIO_MODER_MODER13_Msk;
}

void orangeLedOn(void) { GPIOD->BSRR = GPIO_BSRR_BS13_Msk; }

void orangeLedOff(void) { GPIOD->BSRR = GPIO_BSRR_BR13_Msk; }