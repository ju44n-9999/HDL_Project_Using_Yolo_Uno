#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern float glob_temperature;
extern float glob_humidity;

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

extern boolean isWifiConnected;

// LED modes for temperature
// 0 = steady on (< 30°C)
// 1 = blink 500ms (30-40°C)
// 2 = blink 100ms (> 40°C)
extern volatile int temperature_mode;

// NeoPixel color modes for humidity
// 0 = green (< 80%)
// 1 = yellow (80-85%)
// 2 = orange (85-90%)
// 3 = orange-red (90-95%)
// 4 = red (95-100%)
extern volatile int neo_color_mode;

extern volatile int dht_display_mode;

#endif