#include "task_webserver.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "global.h"
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool webserver_isrunning = false;

// ========== CONFIGURATION ==========
// Pin definitions (adjust to your hardware)
#define LED1_PIN 2          // Single LED from Task 1
#define NEOPIXEL_PIN 48     // NeoPixel from Task 2
#define NEOPIXEL_COUNT 1    // Number of NeoPixels

// External semaphores from other tasks
extern SemaphoreHandle_t xSemaphore_LED;        // From Task 1
extern SemaphoreHandle_t xSemaphore_NeoPixel;   // From Task 2
extern SemaphoreHandle_t xSemaphore_Sensor;     // From Task 3

// External state variables
extern bool led1_manual_control;     // Manual control flag for LED
extern bool neopixel_manual_control; // Manual control flag for NeoPixel
extern float glob_temperature;       // Global temperature variable
extern float glob_humidity;          // Global humidity variable

// Local device states
bool led1State = false;
bool neopixel_on = false;
uint32_t neopixel_color = 0x000000; // RGB color

// NeoPixel object
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ========== HELPER FUNCTIONS ==========

// Convert RGB to uint32_t color
uint32_t colorRGB(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Send current status to all connected clients
void sendCurrentStatus()
{
    StaticJsonDocument<512> doc;
    
    // Device states
    doc["LED1"] = led1State ? "ON" : "OFF";
    doc["NEOPIXEL"] = neopixel_on ? "ON" : "OFF";
    
    // NeoPixel color in hex format
    char colorHex[8];
    sprintf(colorHex, "#%02X%02X%02X", 
            (neopixel_color >> 16) & 0xFF,
            (neopixel_color >> 8) & 0xFF,
            neopixel_color & 0xFF);
    doc["COLOR"] = colorHex;
    
    // Sensor data
    doc["temperature"] = String(glob_temperature, 1);
    doc["humidity"] = String(glob_humidity, 1);
    
    // Manual control status
    doc["led1_manual"] = led1_manual_control;
    doc["neopixel_manual"] = neopixel_manual_control;
    
    doc["timestamp"] = millis();
    
    String output;
    serializeJson(doc, output);
    Webserver_sendata(output);
}

// Control LED1 (Task 1 device)
void controlLED1(bool state)
{
    if (xSemaphoreTake(xSemaphore_LED, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        led1State = state;
        digitalWrite(LED1_PIN, state ? HIGH : LOW);
        
        // Enable manual control mode
        led1_manual_control = true;
        
        xSemaphoreGive(xSemaphore_LED);
        
        Serial.printf("✅ LED1 set to %s (Manual Control)\n", state ? "ON" : "OFF");
    }
    else
    {
        Serial.println("⚠️ Failed to acquire LED semaphore");
    }
}

// Control NeoPixel (Task 2 device)
void controlNeoPixel(bool state, uint32_t color = 0)
{
    if (xSemaphoreTake(xSemaphore_NeoPixel, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        neopixel_on = state;
        
        if (state && color != 0) {
            neopixel_color = color;
        }
        
        // Update NeoPixel
        if (neopixel_on) {
            pixels.setPixelColor(0, neopixel_color);
            pixels.show();
        } else {
            pixels.clear();
            pixels.show();
        }
        
        // Enable manual control mode
        neopixel_manual_control = true;
        
        xSemaphoreGive(xSemaphore_NeoPixel);
        
        Serial.printf("✅ NeoPixel set to %s with color 0x%06X (Manual Control)\n", 
                      state ? "ON" : "OFF", neopixel_color);
    }
    else
    {
        Serial.println("⚠️ Failed to acquire NeoPixel semaphore");
    }
}

// Restore automatic control (let Task 1 & 2 control devices based on sensors)
void restoreAutoControl()
{
    if (xSemaphoreTake(xSemaphore_LED, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        led1_manual_control = false;
        xSemaphoreGive(xSemaphore_LED);
        Serial.println("✅ LED1 restored to automatic control");
    }
    
    if (xSemaphoreTake(xSemaphore_NeoPixel, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        neopixel_manual_control = false;
        xSemaphoreGive(xSemaphore_NeoPixel);
        Serial.println("✅ NeoPixel restored to automatic control");
    }
}

// ========== WEBSOCKET MESSAGE HANDLER ==========
void handleWebSocketMessage(String message)
{
    Serial.println("📩 Received: " + message);
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.print("❌ JSON parsing failed: ");
        Serial.println(error.c_str());
        return;
    }
    
    // Support messages using the older "page" format (task_handler style)
    if (doc.containsKey("page")) {
        if (doc["page"] == "device") {
            JsonObject value = doc["value"];
            if (!value.containsKey("gpio") || !value.containsKey("status")) {
                Serial.println("⚠️ JSON thiếu thông tin gpio hoặc status");
                return;
            }

            int gpio = value["gpio"];
            String status = value["status"].as<String>();

            Serial.printf("⚙️ Điều khiển GPIO %d → %s\n", gpio, status.c_str());
            pinMode(gpio, OUTPUT);
            if (status.equalsIgnoreCase("ON")) {
                digitalWrite(gpio, HIGH);
                Serial.printf("🔆 GPIO %d ON\n", gpio);
            } else if (status.equalsIgnoreCase("OFF")) {
                digitalWrite(gpio, LOW);
                Serial.printf("💤 GPIO %d OFF\n", gpio);
            }
            return; // done
        }

        if (doc["page"] == "setting") {
            String WIFI_SSID = doc["value"]["ssid"].as<String>();
            String WIFI_PASS = doc["value"]["password"].as<String>();
            String CORE_IOT_TOKEN = doc["value"]["token"].as<String>();
            String CORE_IOT_SERVER = doc["value"]["server"].as<String>();
            String CORE_IOT_PORT = doc["value"]["port"].as<String>();

            Serial.println("📥 Nhận cấu hình từ WebSocket:");
            Serial.println("SSID: " + WIFI_SSID);
            Serial.println("PASS: " + WIFI_PASS);
            Serial.println("TOKEN: " + CORE_IOT_TOKEN);
            Serial.println("SERVER: " + CORE_IOT_SERVER);
            Serial.println("PORT: " + CORE_IOT_PORT);

            Save_info_File(WIFI_SSID, WIFI_PASS, CORE_IOT_TOKEN, CORE_IOT_SERVER, CORE_IOT_PORT);

            String msg = "{\"status\":\"ok\",\"page\":\"setting_saved\"}";
            ws.textAll(msg);
            return;
        }
    }

    String device = doc["device"] | "";
    String action = doc["action"] | "";
    
    // ===== LED1 Control =====
    if (device == "LED1") {
        if (action == "ON") {
            controlLED1(true);
        } else if (action == "OFF") {
            controlLED1(false);
        } else if (action == "TOGGLE") {
            controlLED1(!led1State);
        }
    }
    
    // ===== NeoPixel Control =====
    else if (device == "NEOPIXEL" || device == "LED2") {
        if (action == "ON") {
            // Default color if not specified
            uint32_t color = neopixel_color != 0 ? neopixel_color : colorRGB(255, 255, 255);
            controlNeoPixel(true, color);
        } 
        else if (action == "OFF") {
            controlNeoPixel(false);
        }
        else if (action == "COLOR") {
            // Expect color in format: {"device":"NEOPIXEL", "action":"COLOR", "color":"#FF0000"}
            String colorStr = doc["color"] | "#FFFFFF";
            
            // Parse hex color
            long colorValue = strtol(colorStr.c_str() + 1, NULL, 16);
            uint32_t color = (uint32_t)colorValue;
            
            controlNeoPixel(true, color);
        }
        else if (action == "PRESET") {
            // Preset colors
            String preset = doc["preset"] | "white";
            uint32_t color;
            
            if (preset == "red") color = colorRGB(255, 0, 0);
            else if (preset == "green") color = colorRGB(0, 255, 0);
            else if (preset == "blue") color = colorRGB(0, 0, 255);
            else if (preset == "yellow") color = colorRGB(255, 255, 0);
            else if (preset == "cyan") color = colorRGB(0, 255, 255);
            else if (preset == "magenta") color = colorRGB(255, 0, 255);
            else color = colorRGB(255, 255, 255); // white
            
            controlNeoPixel(true, color);
        }
    }
    
    // ===== Both Devices Control =====
    else if (device == "BOTH") {
        bool state = (action == "ON");
        controlLED1(state);
        controlNeoPixel(state, state ? colorRGB(255, 255, 255) : 0);
    }
    
    // ===== Restore Automatic Control =====
    else if (action == "AUTO") {
        restoreAutoControl();
    }
    
    // ===== Get Current Status =====
    else if (action == "GET_STATUS") {
        sendCurrentStatus();
        return;
    }
    
    // Send updated status to all clients
    sendCurrentStatus();
}

// ========== WEBSOCKET EVENTS ==========
void Webserver_sendata(String data)
{
    if (ws.count() > 0)
    {
        ws.textAll(data);
        Serial.println("📤 Sent: " + data);
    }
    else
    {
        Serial.println("⚠️ No WebSocket clients connected");
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
             AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("✅ WebSocket client #%u connected from %s\n", 
                      client->id(), client->remoteIP().toString().c_str());
        
        // Send current status to newly connected client
        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to ensure connection is stable
        sendCurrentStatus();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("❌ WebSocket client #%u disconnected\n", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            handleWebSocketMessage(message);
        }
    }
}

// ========== SERVER SETUP ==========
void connnectWSV()
{
    // Initialize hardware
    pinMode(LED1_PIN, OUTPUT);
    digitalWrite(LED1_PIN, LOW);
    
    pixels.begin();
    pixels.clear();
    pixels.show();
    
    // Setup WebSocket
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    
    // Serve static files from LittleFS
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/script.js", "application/javascript");
    });
    
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/styles.css", "text/css");
    });
    
    // Start server
    server.begin();
    ElegantOTA.begin(&server);
    webserver_isrunning = true;
    
    Serial.println("✅ Web Server started!");
    Serial.print("🌐 Access at: http://");
    Serial.println(WiFi.localIP());
}

void Webserver_stop()
{
    ws.closeAll();
    server.end();
    webserver_isrunning = false;
    Serial.println("❌ Web Server stopped");
}

void Webserver_reconnect()
{
    if (!webserver_isrunning)
    {
        connnectWSV();
    }
    ElegantOTA.loop();
}

// ========== PERIODIC STATUS UPDATE TASK ==========
void task_webserver_update(void *pvParameters)
{
    const TickType_t xFrequency = pdMS_TO_TICKS(2000); // Update every 2 seconds
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1)
    {
        if (webserver_isrunning && ws.count() > 0)
        {
            sendCurrentStatus();
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}