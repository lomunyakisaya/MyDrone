#include "websocket.h"

void WebSocket::begin() {
    server.begin();
    server.onEvent(onEvent);
    Serial.println("WebSocket server started on port 81");
}

void WebSocket::loop() {
    server.loop();
}

void WebSocket::sendData(const String& data) {
    server.broadcastTXT(data);
    Serial.print("Broadcast: ");
    Serial.println(data);
}

void WebSocket::onEvent(
    uint8_t clientNum,
    WStype_t type,
    uint8_t* payload,
    size_t length
) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.printf("Client %u connected\n", clientNum);
            break;
        case WStype_DISCONNECTED:
            Serial.printf("Client %u disconnected\n", clientNum);
            break;
        case WStype_TEXT:
            Serial.printf("Received from %u: %s\n", clientNum, payload ? reinterpret_cast<char*>(payload) : "");
            break;
        default:
            break;
    }
}