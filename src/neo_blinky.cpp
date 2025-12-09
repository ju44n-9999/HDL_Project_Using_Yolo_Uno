#include "neo_blinky.h"
#include "global.h"
#include "global_semaphore.h"


void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    // Initialize semaphores if not already done
    if (xSemaphoreNeoDHT == NULL) {
        xSemaphoreNeoDHT = xSemaphoreCreateBinary();
    }

    bool led_on = false;

    for(;;) {
        int color_mode = neo_color_mode; // read current color mode based on humidity

        // Display color based on humidity level
        uint32_t color = strip.Color(0, 255, 0); // default green
        
        if (color_mode == 1) {
            // yellow (80-85%)
            color = strip.Color(255, 255, 0);
        } else if (color_mode == 2) {
            // orange (85-90%)
            color = strip.Color(255, 165, 0);
        } else if (color_mode == 3) {
            // orange-red (90-95%)
            color = strip.Color(255, 69, 0);
        } else if (color_mode == 4) {
            // red (95-100%)
            color = strip.Color(255, 0, 0);
        } else {
            // green (< 80%)
            color = strip.Color(0, 255, 0);
        }
        
        strip.setPixelColor(0, color);
        strip.show();
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}