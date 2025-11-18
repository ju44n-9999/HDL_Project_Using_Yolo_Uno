#include "neo_blinky.h"
#include "global.h"


void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    bool led_on = false;

    for(;;) {
        int mode = neo_mode; // read current mode

        if (mode == 0) {
            // steady green
            strip.setPixelColor(0, strip.Color(0, 255, 0));
            strip.show();
            // wait indefinitely until temp/humi task signals a mode change
            if (xSemaphoreTake(xSemaphoreNeo, portMAX_DELAY) == pdTRUE) {
                continue;
            }
        } else {
            uint32_t interval_ms = 500;
            uint32_t r = 255, g = 0, b = 0; // default red

            if (mode == 1) { // red 500ms
                interval_ms = 500;
                r = 255; g = 0; b = 0;
            } else if (mode == 2) { // red 100ms
                interval_ms = 100;
                r = 255; g = 0; b = 0;
            } else if (mode == 3) { // blue 500ms
                interval_ms = 500;
                r = 0; g = 0; b = 255;
            } else if (mode == 4) { // blue 100ms
                interval_ms = 100;
                r = 0; g = 0; b = 255;
            }

            if (led_on) {
                strip.setPixelColor(0, strip.Color(0, 0, 0));
                led_on = false;
            } else {
                strip.setPixelColor(0, strip.Color(r, g, b));
                led_on = true;
            }
            strip.show();

            // wait for either the blink interval or a semaphore from monitor task
            if (xSemaphoreTake(xSemaphoreNeo, pdMS_TO_TICKS(interval_ms)) == pdTRUE) {
                // mode may have changed; loop will read new neo_mode
                continue;
            }
        }
    }
}