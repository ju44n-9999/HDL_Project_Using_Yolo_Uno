#include "coreiot.h"
#include "global_semaphore.h"

// ----------- CONFIGURE THESE! -----------
const char* coreIOT_Server = "app.coreiot.io";  
const char* coreIOT_Token = "rVFapWxNLQwxeuXBQ9jR";   // Device Access Token
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    if (client.connect("ESP32Client", coreIOT_Token, NULL)) {
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      client.subscribe("v1/devices/me/attributes");
      Serial.println("Subscribed to RPC and Attributes");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Check if this is RPC or Attributes
  if (strstr(topic, "rpc/request") != NULL) {
    // Handle RPC commands
    const char* method = doc["method"];
    
    // Toggle display mode (Temperature <-> Humidity)
    if (strcmp(method, "toggleDisplayMode") == 0) {
      dht_display_mode = (dht_display_mode == 1) ? 2 : 1;
      Serial.print("Display mode toggled to: ");
      Serial.println(dht_display_mode == 1 ? "Temperature" : "Humidity");
    } 
    // Old RPC command for LED control (I'm not gonna use it, but keep for reference)
    else if (strcmp(method, "setStateLED") == 0) {
      const char* params = doc["params"];
      if (strcmp(params, "ON") == 0) {
        Serial.println("Device turned ON.");
      } else {   
        Serial.println("Device turned OFF.");
      }
    } 
    else {
      Serial.print("Unknown method: ");
      Serial.println(method);
    }
  }
  else if (strstr(topic, "attributes") != NULL) {
    // Handle Attributes from CoreIOT
    // Example: {"ledBrightness": 100, "displayMode": 1}
    
    if (doc.containsKey("ledBrightness")) {
      int brightness = doc["ledBrightness"];
      Serial.print("LED Brightness set to: ");
      Serial.println(brightness);
      // TODO: Implement LED brightness control
    }
    
    if (doc.containsKey("displayMode")) {
      int mode = doc["displayMode"];
      dht_display_mode = mode;
      Serial.print("Display mode set to: ");
      Serial.println(mode == 1 ? "Temperature" : "Humidity");
    }
  }
}


void setup_coreiot(){

  //Serial.print("Connecting to WiFi...");
  //WiFi.begin(wifi_ssid, wifi_password);
  //while (WiFi.status() != WL_CONNECTED) {
  
  // while (isWifiConnected == false) {
  //   delay(500);
  //   Serial.print(".");
  // }

  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }


  Serial.println(" Connected!");

  client.setServer(coreIOT_Server, mqttPort);
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters){

    setup_coreiot();

    while(1){

        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        // Sample payload, publish to 'v1/devices/me/telemetry'
        String payload = "{\"temperature\":" + String(glob_temperature) +  ",\"humidity\":" + String(glob_humidity) + "}";
        
        client.publish("v1/devices/me/telemetry", payload.c_str());

        Serial.println("Published payload: " + payload);
        vTaskDelay(10000);  // Publish every 10 seconds
    }
}