#ifndef __GLOBAL_SEMAPHORE__
#define __GLOBAL_SEMAPHORE__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Semaphore để notify LED task về thay đổi nhiệt độ
extern SemaphoreHandle_t xSemaphoreLED;

// Semaphore để notify NeoPixel task về thay đổi độ ẩm
extern SemaphoreHandle_t xSemaphoreNeoDHT;

// Semaphore để notify Fan task về thay đổi nhiệt độ
extern SemaphoreHandle_t xSemaphoreFan;

extern SemaphoreHandle_t xBinarySemaphoreInternet;

void init_semaphores();

#endif
