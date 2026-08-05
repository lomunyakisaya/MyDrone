#include <ArduinoJson.h>

void setup() {
  Serial.begin(9600);      // USB Serial Monitor
  Serial3.begin(9600);       // UART to ESP8266
}

void loop() {
  float speed = analogRead(A0);
  StaticJsonDocument<256> doc;
  doc["device_id"] = "LDOS-001";
  doc["temperature"] = 34.7;
  doc["altitude"] = 23;
  doc["battery"] = 78;
  doc["timestamp"] = "2023-07-15 12:34:56";
  doc["speed"] = speed;
  doc["distance"] = 120.3;
  doc["latitude"] = 37.7749;
  doc["longitude"] = -122.4194;
  doc["rssi"] = -65;
  doc["voltage"] = 3.7;
  doc["current"] = 0.5;
  doc["sats"] = 7;
  doc["uptime"]= 123456;

  serializeJson(doc, Serial3); //send to serial3
  Serial3.println();          // Send newline

  serializeJson(doc, Serial);
  Serial.println();

  delay(1000);
}