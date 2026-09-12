#include <ArduinoJson.h>

void setup() {
  Serial.begin(9600);      // USB Serial Monitor
  Serial3.begin(9600);       // UART to ESP8266
}

void loop() {
  //float speed = analogRead(A0);
  StaticJsonDocument<512> doc;
  doc["device_id"] = "LDOS-001";
  doc["temperature"] = random(200, 401) / 10.0;    
  doc["altitude"] = random(10, 200);                
  doc["battery"] = random(40, 101);                 
  doc["timestamp"] = "202 6-08-06 10:55:00";
  doc["speed"] = random(0, 151);                     
  doc["distance"] = random(100, 1000) / 10.0;        
  doc["latitude"] = -1.2921 + (random(-1000, 1000) / 100000.0);
  doc["longitude"] = 36.8219 + (random(-1000, 1000) / 100000.0);
  doc["rssi"] = random(-90, -40);                   
  doc["voltage"] = random(360, 421) / 100.0;        
  doc["current"] = random(0, 300) / 100.0;          
  doc["sats"] = random(4, 16);                      
  doc["uptime"] = millis() / 1000;                 

  serializeJson(doc, Serial3); //send to serial3
  Serial3.println();          // Send newline

  serializeJson(doc, Serial);
  Serial.println();

  delay(1000);
}