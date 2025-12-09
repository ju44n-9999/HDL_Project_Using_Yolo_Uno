#ifndef __LED_BLINKY__
#define __LED_BLINKY__
#include <Arduino.h>
#include "global.h"
#include "global_semaphore.h"
#define LED_GPIO 48
void led_blinky(void *pvParameters);


#endif