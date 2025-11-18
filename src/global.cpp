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
String wifi_ssid = "abcde";
String wifi_password = "123456789";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();
// Semaphore used to notify NeoPixel task about NeoPixel-mode changes
SemaphoreHandle_t xSemaphoreNeo = xSemaphoreCreateBinary();
// neo_mode mapping:
// 0 = normal (steady green)
// 1 = red blink 500 ms (temperature threshold 1)
// 2 = red blink 100 ms (temperature threshold 2)
// 3 = blue blink 500 ms (humidity threshold 1)
// 4 = blue blink 100 ms (humidity threshold 2)
volatile int neo_mode = 0;
// Semaphore used to toggle DHT display mode (short press)
SemaphoreHandle_t xSemaphoreDHTToggle = xSemaphoreCreateBinary();
// dht_display_mode: 0 = both (default), 1 = temperature only, 2 = humidity only
volatile int dht_display_mode = 0;