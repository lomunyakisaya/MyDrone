#include <ArduinoJson.h>

void setup() {
  Serial.begin(9600);      // USB Serial Monitor
  Serial3.begin(9600);       // UART to ESP8266
}

void loop() {
  //float speed = analogRead(A0);
  StaticJsonDocument<512> doc;
  doc["device_id"] = "LDOS-001";
  doc["temperature"] = random(200, 401) / 10.0;      // 20.0 - 40.0 °C
  doc["altitude"] = random(10, 200);                 // meters
  doc["battery"] = random(40, 101);                  // %
  doc["timestamp"] = "202 6-08-06 10:55:00";
  doc["speed"] = random(0, 151);                     // km/h
  doc["distance"] = random(100, 1000) / 10.0;        // 10.0 - 99.9 km
  doc["latitude"] = -1.2921 + (random(-1000, 1000) / 100000.0);
  doc["longitude"] = 36.8219 + (random(-1000, 1000) / 100000.0);
  doc["rssi"] = random(-90, -40);                    // dBm
  doc["voltage"] = random(360, 421) / 100.0;         // 3.60 - 4.20 V
  doc["current"] = random(0, 300) / 100.0;           // 0.00 - 3.00 A
  doc["sats"] = random(4, 16);                       // GPS satellites
  doc["uptime"] = millis() / 1000;                   // seconds

  serializeJson(doc, Serial3); //send to serial3
  Serial3.println();          // Send newline

  serializeJson(doc, Serial);
  Serial.println();

  delay(1000);
}