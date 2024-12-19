#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Arduino.h>


#define LIGHT_SENSOR_PIN 36 // ESP32 pin GPIO2
const int SAMPLES = 1;
RTC_DATA_ATTR int bootCount = 0;
StaticJsonDocument<500> doc;



//This is the secret file
const char* ssid = "BT-PSCM7J";
const char* password = "kFa4tEc7FHRfUT";
const char* serverName = "https://eu-west-1.aws.webhooks.mongodb-realm.com/api/client/v2.0/app/light-vsxkn/service/light/incoming_webhook/Light";

// -- Project -------------------------------------------
#define CLIENT                  "Energy meter"        // Client ID for the ESP (or something descriptive "Front Garden")
#define TYPE                    "ESP32"               // Type of Sensor ("Hornbill ESP32" or "Higrow" or "ESP8266" etc.)  

// -- Other - Helpers ------------------------------------
#define uS_TO_S_FACTOR          1000000               // Conversion factor for micro seconds to seconds


void setup() {

  Serial.begin(9600);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.print(" with password ");
  Serial.println(password);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Wifi connecting");
  }
  Serial.println("Wifi connected");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

}


void POSTData()
{

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String json;
    serializeJson(doc, json);

    Serial.println(json);
    int httpResponseCode = http.POST(json);
    Serial.println(httpResponseCode);
  }
}

void getDevice()
{

  uint64_t chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf("***ESP32 Chip ID = 78:e3:6d:09:40:5d00", (uint16_t)(chipid >> 32), (uint32_t)chipid); //print High 2 bytes
  char buffer[200];
  sprintf(buffer, "78:e3:6d:09:40:5d00", (uint16_t)(chipid >> 32), (uint32_t)chipid);

  doc["device"]["IP"] = WiFi.localIP().toString();
  doc["device"]["RSSI"] = String(WiFi.RSSI());
  doc["device"]["type"] = TYPE;
  doc["device"]["chipid"] = buffer;
  doc["device"]["bootCount"] = bootCount;


}

int senseLight(int readPin)
{
  int reading = 0;
  if (analogRead(readPin) >= 0)
  {
    reading = analogRead(readPin);
    for (int i = 0; i < SAMPLES; i++)
    {
      reading += analogRead(readPin);
      //delay(30);
    }

  }
  
  doc["sensors"]["light"] = reading;
  return reading;
}

void loop() {
  //delay(30);
  ++bootCount; // move this to setup()
  getDevice();
  int sense = senseLight(LIGHT_SENSOR_PIN);
  Serial.print(sense);
  Serial.println(analogRead(LIGHT_SENSOR_PIN));
//  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Posting...");
  POSTData();
  serializeJsonPretty(doc, Serial);
  Serial.println("\nDone.");

   
}
