#include "fan_control.h"
#include "global_semaphore.h"
#include "global.h"

// Fan initialization function
void fan_init(void) {
    pinMode(FAN_PIN1, OUTPUT);
    fan_off();  // At first, turn off the fan
}

// Fan ON function
void fan_on(void) {
    digitalWrite(FAN_PIN1, HIGH);
}

// Fan OFF function
void fan_off(void) {
    digitalWrite(FAN_PIN1, LOW);
}

// Fan control task
void fan_control_task(void *pvParameters) {
    while (1) {
        int mode = temperature_mode;

        if (mode == 0 || mode == 1) { // (< 35°C)
            fan_off();
            // Wait for semaphore signal or timeout
            xSemaphoreTake(xSemaphoreFan, pdMS_TO_TICKS(100));
        } else if (mode == 2) { // (> 35°C)
            fan_on();
            
            // Check if mode changed (non-blocking)
            if (xSemaphoreTake(xSemaphoreFan, 0) == pdTRUE) {
                // Mode may have changed
                continue;
            }
        }
        
        vTaskDelay(150);
    }
}
