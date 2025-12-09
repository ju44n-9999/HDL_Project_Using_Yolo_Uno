#include "temp_humi_monitor.h"
#include "global.h"
#include "global_semaphore.h"

DHT20 dht20;
LiquidCrystal_I2C lcd(33, 16, 2);

void temp_humi_monitor(void *pvParameters){

    // Initialize DHT20 sensor and LCD only once
    static bool initialized = false;
    if (!initialized) {
        dht20.begin();
        lcd.begin();        // initialize display
        lcd.backlight();    // turn on backlight
        lcd.clear();
        lcd.setCursor(0, 0);
        initialized = true;
    }

    while (1){
        /* code */
        
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity =  -1;
            //return;
        }

        //Update global variables for temperature and humidity
        glob_temperature = temperature;
        /*
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print("\n");
        */
        glob_humidity = humidity;
        /*
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("\n");
        */

        // Determine LED mode based on temperature
        int new_temperature_mode = 0;
        if (temperature > 35.0) {
            new_temperature_mode = 2; // blink 100ms && fan on
        } else if (temperature >= 30.0) {
            new_temperature_mode = 1; // blink 500ms && fan on
        } else {
            new_temperature_mode = 0; // steady on && fan off
        }
        
        if (new_temperature_mode != temperature_mode) {
            temperature_mode = new_temperature_mode;
            xSemaphoreGive(xSemaphoreLED);
            xSemaphoreGive(xSemaphoreFan);
        }

        // Determine NeoPixel color mode based on humidity
        int new_neo_color_mode = 0;
        if (humidity >= 95.0) {
            new_neo_color_mode = 4; // red
        } else if (humidity >= 90.0) {
            new_neo_color_mode = 3; // orange-red
        } else if (humidity >= 85.0) {
            new_neo_color_mode = 2; // orange
        } else if (humidity >= 80.0) {
            new_neo_color_mode = 1; // yellow
        } else {
            new_neo_color_mode = 0; // green
        }
        
        if (new_neo_color_mode != neo_color_mode) {
            neo_color_mode = new_neo_color_mode;
            xSemaphoreGive(xSemaphoreNeoDHT);
        }

        // Display based on current `dht_display_mode`:
        // 1 = Temperature mode: first line label, second line temperature
        // 2 = Humidity mode: first line label, second line humidity
        switch (dht_display_mode) {
            case 1: // Temperature mode
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Temperature:");
                lcd.setCursor(0, 1);
                lcd.print(temperature, 2);
                break;
            case 2: // Humidity mode
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Humidity (%):");
                lcd.setCursor(0, 1);
                lcd.print(humidity, 2);
                break;
            default:
                break;
        }

        vTaskDelay(150);
    }
    
}