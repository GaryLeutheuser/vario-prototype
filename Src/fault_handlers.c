#include "FreeRTOS.h"
#include "task.h"

volatile TaskHandle_t overflow_task;
volatile char *overflow_name;

// Callback for stack overflow detection.
// We just want to sit here so we can use the recorded information to diagnose
// what caused the overflow and fix it.
// Don't leave this in for prod; watchdog is all we've got at that point.
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  overflow_task = xTask;
  overflow_name = pcTaskName;
  taskDISABLE_INTERRUPTS();
  for (;;) {
  }
}
