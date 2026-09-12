#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>


SoftwareSerial megaSerial(5, 4);
ESP8266WebServer server(80);
WiFiClient client;

const char* ssid = "lomunyak";
const char* password = "Ilm..1703";
float temparature;
float altitude;
float battery;

void setup(){

  //UART SETUP
  Serial.begin(9600);
  megaSerial.begin(9600);

//WIFI configs
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(200);
    Serial.print(".");
  }
  Serial.print("Esp8266 ip: ");
  Serial.println(WiFi.localIP());

   //Web Server
  server.begin(); 
  Serial.println("Server Started");
  Serial.println("Connected");

  //Server endpoints configs
  server.on("/", [](){server.send(200, "text/plain", "Hello World");
  });
  server.onNotFound([]() {
  server.send(404, "text/plain", "The page you requested was not found!");
});
}
void loop() {
  server.handleClient();  
    if (megaSerial.available()) {

        String json = megaSerial.readStringUntil('\n');


        //JSON TO ESP8266 SERIAL MONITOR
        Serial.println(json);
        
        //JSON TO HTTP
        HTTPClient http;
        http.begin(client, "http://192.168.0.103:8000/esp8266");
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(json);
        if (httpCode > 0) {
            Serial.print("HTTP Code: ");
            Serial.println(httpCode);
            Serial.println(http.getString());
        } 
        else {
            Serial.print("HTTP Error: ");
            Serial.println(http.errorToString(httpCode));
        }

        http.end();
    }
}

