#include "global_semaphore.h"

// Semaphore noticer for LED task
SemaphoreHandle_t xSemaphoreLED = NULL;

// Semaphore noticer for NeoPixel and DHT task
SemaphoreHandle_t xSemaphoreNeoDHT = NULL;

// Semaphore noticer for Fan task
SemaphoreHandle_t xSemaphoreFan = NULL;

// Semaphore noticer for Internet connection
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

void init_semaphores() {
    xSemaphoreLED = xSemaphoreCreateBinary();
    xSemaphoreNeoDHT = xSemaphoreCreateBinary();
    xSemaphoreFan = xSemaphoreCreateBinary();
}
