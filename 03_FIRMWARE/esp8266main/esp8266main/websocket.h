#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <Arduino.h>
#include <WebSocketsServer.h>

class WebSocket {
public:
    void begin();
    void loop();
    void sendData(const String& data);

private:
    WebSocketsServer server = WebSocketsServer(81);

    static void onEvent(
        uint8_t clientNum,
        WStype_t type,
        uint8_t* payload,
        size_t length
    );
};

#endif