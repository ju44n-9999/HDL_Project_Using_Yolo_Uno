#include "task_wifi.h"
#include "global_semaphore.h"
#include "global.h"

void startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    // Serial.print("AP IP: ");
    // Serial.println(WiFi.softAPIP());
}

void startSTA()
{
    if (WIFI_SSID.isEmpty())
    {
        Serial.println("WiFi SSID is empty!");
        vTaskDelete(NULL);
    }

    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);

    if (WIFI_PASS.isEmpty())
    {
        WiFi.begin(WIFI_SSID.c_str());
    }
    else
    {
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40)
    {
        Serial.print(".");
        vTaskDelay(500 / portTICK_PERIOD_MS);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        // Serial.print("IP: ");
        // Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi");
    }
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    startSTA();
    return false;
}

void task_wifi_handler(void *pvParameters)
{
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Wait for Serial to be ready
    // Type your Wifi
    Serial.println("\n\n========== WiFi Setup ==========");
    Serial.println("Enter WiFi SSID (max 32 characters):");
    Serial.flush();
    
    // Read WiFi SSID
    while (!Serial.available()) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    WIFI_SSID = Serial.readStringUntil('\n');
    WIFI_SSID.trim();
    Serial.print("SSID set to: ");
    Serial.println(WIFI_SSID);
    Serial.flush();
    
    // Type your Wifi Password
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("Enter WiFi Password (max 64 characters, press Enter if no password):");
    Serial.flush();
    
    // Read WiFi Password
    while (!Serial.available()) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    WIFI_PASS = Serial.readStringUntil('\n');
    WIFI_PASS.trim();
    Serial.print("Password set to: ");
    if (WIFI_PASS.isEmpty()) {
        Serial.println("(No password)");
    } else {
        Serial.println("(Hidden)");
    }
    Serial.flush();
    
    Serial.println("================================");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    // Now try to connect
    while (1) {
        if (Wifi_reconnect()) {
            // Serial.println(WiFi.localIP()); // Not so important
            xSemaphoreGive(xBinarySemaphoreInternet);
        } else {
            Serial.println("WiFi not connected, retrying...");
        }
        Serial.flush();
        vTaskDelay(500 / portTICK_PERIOD_MS);  // Check every 0.5 seconds
    }
}
