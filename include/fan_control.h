#ifndef __FAN_CONTROL__
#define __FAN_CONTROL__
#include <Arduino.h>
#include "global.h"
#include "global_semaphore.h"

// Fan control pins (D4-D3)
#define FAN_PIN1 6 // D3

void fan_init(void);
void fan_on(void);
void fan_off(void);
void fan_control_task(void *pvParameters);

#endif
