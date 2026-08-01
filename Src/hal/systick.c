// cppcheck-suppress misra-c2012-11.4 ; see deviations.md D-001

#include "hal/systick.h"
#include "stm32f407xx.h"

void systick_init(void) {
  // 16000 counts @ 16 MHz clock == 1 ms cycle
  SysTick->LOAD = 15999u;

  // Internal clock, enable tick interrupt
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;

  // Set unknown initial count to zero
  SysTick->VAL = 0u;

  // Enable the counter
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

static volatile uint32_t tick = 0u;

// cppcheck-suppress misra-c2012-8.7 ; ISR, see deviations.md D-002
void SysTick_Handler(void) { tick++; } // 1 ms tick

uint32_t systick_get_ms(void) { return tick; }