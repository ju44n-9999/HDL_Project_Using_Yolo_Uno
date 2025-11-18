#include "temp_humi_monitor.h"
#include "global.h"

DHT20 dht20;
LiquidCrystal_I2C lcd(33, 16, 2);


void temp_humi_monitor(void *pvParameters){

    Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();

    lcd.begin();        // initialize display
    lcd.backlight();    // turn on backlight
    lcd.clear();
    lcd.setCursor(0, 0);

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
        glob_humidity = humidity;

        // Determine desired NeoPixel mode and signal Neo task if changed.
        // Behavior depends on current `dht_display_mode`:
        // Mode 0 (normal): steady green and display "Hello World!"
        // Mode 1 (temperature): red blink thresholds at 30 (500ms) and 35 (100ms)
        // Mode 2 (humidity): blue blink thresholds at 80 (500ms) and 90 (100ms)
        int new_mode = 0;

        if (dht_display_mode == 0) {
            // Normal mode: no special Neo behaviour
            new_mode = 0;
        } else if (dht_display_mode == 1) {
            // Temperature mode
            if (temperature >= 35.0) {
                new_mode = 2; // red fast
            } else if (temperature >= 30.0) {
                new_mode = 1; // red slow
            } else {
                new_mode = 0; // normal
            }
        } else if (dht_display_mode == 2) {
            // Humidity mode
            if (humidity >= 90.0) {
                new_mode = 4; // blue fast
            } else if (humidity >= 80.0) {
                new_mode = 3; // blue slow
            } else {
                new_mode = 0; // normal
            }
        }

        if (new_mode != neo_mode) {
            neo_mode = new_mode;
            // notify Neo task of the change; don't block if semaphore already full
            xSemaphoreGive(xSemaphoreNeo);
        }
        /*
        // Print the results to Terminal
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print("%  Temperature: ");
        Serial.print(temperature);
        Serial.println("°C");
        */

        // If a DHT display toggle was requested, it was already applied by the button task
        // Take the semaphore if available so we can react immediately (non-blocking)
        if (xSemaphoreTake(xSemaphoreDHTToggle, 0) == pdTRUE) {
            // nothing to do here: dht_display_mode is updated by the button task
        }

        // Display based on current `dht_display_mode`:
        // 0 = normal: "Hello World!"
        // 1 = Temperature mode: first line label, second line temperature
        // 2 = Humidity mode: first line label, second line humidity
        switch (dht_display_mode) {
            case 1: // Temperature mode
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Temperature");
                lcd.setCursor(0, 1);
                lcd.print(temperature, 2);
                break;
            case 2: // Humidity mode
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Humidity (%)");
                lcd.setCursor(0, 1);
                lcd.print(humidity, 2);
                break;
            case 0: // Normal
            default:
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Hello World!");
                lcd.setCursor(0, 1);
                lcd.print(" ");
                break;
        }

        vTaskDelay(250);
    }
    
}