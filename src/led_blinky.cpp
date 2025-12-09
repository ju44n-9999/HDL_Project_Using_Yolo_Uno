#include "led_blinky.h"
#include "global_semaphore.h"

void led_blinky(void *pvParameters){
    pinMode(LED_GPIO, OUTPUT);
    
    // Initialize semaphores if not already done
    if (xSemaphoreLED == NULL) {
        xSemaphoreLED = xSemaphoreCreateBinary();
    }
  
    while(1) {
        int mode = temperature_mode;
        
        if (mode == 0) {
            // Steady on (< 30°C)
            digitalWrite(LED_GPIO, HIGH);
            // Wait for semaphore signal or timeout
            xSemaphoreTake(xSemaphoreLED, pdMS_TO_TICKS(100));
        } else if (mode == 1) {
            // Blink 500ms (30-35°C)
            digitalWrite(LED_GPIO, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(LED_GPIO, LOW);
            vTaskDelay(pdMS_TO_TICKS(500));
            
            // Check if mode changed (non-blocking)
            if (xSemaphoreTake(xSemaphoreLED, 0) == pdTRUE) {
                // Mode may have changed
                continue;
            }
        } else if (mode == 2) {
            // Blink 100ms (> 35°C)
            digitalWrite(LED_GPIO, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(LED_GPIO, LOW);
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Check if mode changed (non-blocking)
            if (xSemaphoreTake(xSemaphoreLED, 0) == pdTRUE) {
                // Mode may have changed
                continue;
            }
        }
    }
}