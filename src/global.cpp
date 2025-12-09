#include "global.h"
float glob_temperature = 0;
float glob_humidity = 0;

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER;
String CORE_IOT_PORT;

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid = "1417";
String wifi_password = "080305@@";
boolean isWifiConnected = false;

// LED mode for temperature feedback
// 0 = steady on (< 30°C) && fan off
// 1 = blink 500ms (30-35°C) && fan on
// 2 = blink 100ms (> 35°C) && fan on
volatile int temperature_mode = 0;

// NeoPixel color mode for humidity feedback
// 0 = green (< 80%)
// 1 = yellow (80-85%)
// 2 = orange (85-90%)
// 3 = orange-red (90-95%)
// 4 = red (95-100%)
volatile int neo_color_mode = 0;

volatile int dht_display_mode = 1; // 1 = temperature, 2 = humidity