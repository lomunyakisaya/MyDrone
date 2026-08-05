# ESP8266 Library Quick Reference (Extended)

A practical guide to common ESP8266 Arduino-core libraries — what they do, how they work under the hood, where beginners trip up, and a runnable example for each.

---

## 1. `#include <SoftwareSerial.h>`

**Purpose:** Creates an extra UART using digital pins (bit-banged, not hardware UART).

**How it works:** The ESP8266 has only one usable hardware UART for general purpose I/O (the other is often tied up with boot messages). SoftwareSerial gets around this with a GPIO interrupt timer that samples/toggles a pin at the target baud rate, emulating a UART in software. Because it's software-driven, it steals CPU cycles and can miss bits at high baud rates or when interrupts are disabled elsewhere.

**Common functions:**
`begin(9600)`,
`available()`,
`read()`,
`readStringUntil('\n')`,
`print()`
`println()`

**Pitfalls:** Reliable only up to ~9600–19200 baud; avoid running alongside other timing-sensitive libraries (like `Servo`) on the same core.

---

## 2. `#include <ESP8266WiFi.h>`

**Purpose:** Connects to Wi-Fi or creates a hotspot.

**How it works:** Wraps Espressif's SDK Wi-Fi stack. `WiFi.begin()` starts an async connection (association + DHCP); you poll `WiFi.status()` until `WL_CONNECTED`. Credentials are stored in flash, so a bare `WiFi.begin()` later reconnects automatically.

**Common functions:**
`WiFi.begin(ssid, password)`,
`WiFi.status()`, 
`WiFi.localIP()`, 
`WiFi.disconnect()`, 
`WiFi.RSSI()`, 
`WiFi.persistent(false)`

**Modes:** 
`WIFI_STA`(client), 
`WIFI_AP` (access point), 
`WIFI_AP_STA` (both)

**Pitfalls:** Writing credentials to flash on every boot wears it out — use `WiFi.persistent(false)` if setting credentials programmatically each boot.

---

## 3. `#include <ESP8266WebServer.h>`

**Purpose:** Turns the ESP8266 into a synchronous (blocking) web server.

**How it works:** Listens on a TCP socket (port 80 default); each `server.handleClient()` call checks for a pending request, matches the path against handlers registered with `server.on()`, runs the handler, and sends the response. Being synchronous, each request blocks `loop()` until fully handled.

**Common functions:** 
`server.begin()`, Webserver server(80)// default port for http
`server.on(path, handler)`, 
`server.send(status, type, body)`, 
`server.handleClient()`, //in loop 
`server.arg(name)`
`server.onNotFound([]()){ server.send(status, "type", "body");}`

**Pitfalls:** `handleClient()` must run every loop iteration — a stray `delay()` elsewhere freezes the server.

---

## 4. `#include <ArduinoJson.h>`

**Purpose:** Creates and parses JSON.

**How it works:** v6+ uses a `DynamicJsonDocument`/`StaticJsonDocument` — a pre-allocated pool that keys/values are carved from, avoiding heap fragmentation. `deserializeJson()` parses text into the document; `serializeJson()` walks it back out to text.

**Common functions:** 
`serializeJson(doc, output)`, 
`deserializeJson(doc, input)`, 
`doc["key"]`

**Pitfalls:** Size the document correctly (use the ArduinoJson Assistant website) — too small and it silently fails or truncates.

---

## 5. `#include <ESP8266HTTPClient.h>`

**Purpose:** Sends HTTP and HTTPS requests from the ESP8266 to web APIs, cloud services, or local servers.

**How it works:** `HTTPClient` sits on top of a `WiFiClient` or `WiFiClientSecure` connection and handles the request line, headers, response code, and response body for you. It is ideal for simple request/response workflows such as reading sensor data, posting JSON, or calling REST APIs.

**Notes:**
- Use it for GET/POST requests to REST APIs, IoT dashboards, and simple web services.
- It is a good choice when you want a lightweight client without building raw TCP sockets.
- Keep payloads small because the ESP8266 has limited RAM.

**Methods:**
`begin()`, `addHeader()`, `GET()`, `POST()`, `getString()`, `errorToString()`, `end()`

**Example:**
```cpp
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "YourSSID";
const char* password = "YourPassword";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://example.com/api");
    int code = http.GET();

    if (code > 0) {
      String body = http.getString();
      Serial.println(body);
    }

    http.end();
  }
  delay(10000);
}
```

**Typical flow:**
1. Wait for Wi-Fi to connect.
2. Create a `WiFiClient` and `HTTPClient` object.
3. Call `begin()` with the URL.
4. Add headers if needed (`Content-Type`, auth headers, etc.).
5. Send `GET()`/`POST()` and inspect the returned `httpCode`.
6. Read the response body and always call `end()`.

**Common functions:**
`http.begin(url)`,Connect to a URL
`http.begin(client, url)`,
`http.addHeader("Content-Type", "application/json")`,
`http.GET()`,Send a GET request
`http.POST(payload)`,Send a POST request
`http.errorToString(code)`,
`PUT()`	Send a PUT request
`PATCH()`	Send a PATCH request (supported in recent library versions)
`sendRequest()`	Send a custom HTTP method
`addHeader()`	Add an HTTP header
`collectHeaders()`	Collect specific response headers
`header()`	Read a response header
`getString()`	Get the response body as a String
`getSize()`	Get the response size in bytes
`getStream()`	Read the response as a stream
`getStreamPtr()` Get a pointer to the response stream
`errorToString()`	Convert an error code to readable text
`setTimeout()`	Set the request timeout
`setReuse()`	Enable or disable connection reuse
`end()`	Close the connection and free resources
**Pitfalls:**
- `begin()` will fail or behave poorly if the ESP8266 is not yet connected to Wi-Fi — check `WiFi.status()` first.
- `httpCode` can be negative for connection problems, DNS failures, or timeouts — do not assume a non-zero value means success.
- `getString()` buffers the whole response body in RAM; very large responses can cause memory pressure on the ESP8266.
- For HTTPS, you need a valid TLS setup (`WiFiClientSecure`) and often a proper certificate chain; self-signed certs often need extra work.
- Forgetting `end()` leaks sockets and causes strange failures when making many requests.
- POSTing JSON requires the correct header, otherwise many APIs reject the request.
- Avoid long `delay()` calls while waiting for a reply; keep the loop responsive and use timeouts.

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "YourSSID";
const char* password = "YourPassword";
const char* serverUrl = "http://example.com/api/data";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    if (http.begin(client, serverUrl)) {
      http.addHeader("Content-Type", "application/json");

      String payload = "{\"temp\":25.4}";
      int httpCode = http.POST(payload);

      if (httpCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP %d\n%s\n", httpCode, response.c_str());
      } else {
        Serial.printf("POST failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.println("Unable to connect to server");
    }
  }

  delay(10000);
}
```

**Common beginner mistake:**
```cpp
// Bad: starting the request before Wi-Fi is connected
HTTPClient http;
http.begin("http://example.com");
```

**Why it fails:** The ESP8266 may still be trying to associate with Wi-Fi, so the request can hang or return a negative `httpCode`.

### Related networking libraries
- `#include <WiFiClientSecure.h>`: Adds HTTPS/TLS support for protected web services and certificate-based connections.
- `#include <WiFiUdp.h>`: Used for UDP protocols such as NTP, simple packet exchange, or service discovery.
- `#include <ESP8266HTTPUpdate.h>`: Lets the ESP8266 update its firmware from a remote HTTP/HTTPS server.

---

## 6. `#include <Wire.h>`

**Purpose:** I2C communication with sensors, displays, and peripherals.

**How it works:** Implements I2C master protocol on two GPIOs (default SDA=D2, SCL=D1). Every device has a 7-bit address; the master addresses a device then reads/writes bytes. Many sensor libraries (BME280, MPU6050, OLED) sit on top of `Wire`.

**Common functions:** 
`Wire.begin(sda, scl)`, 
`Wire.beginTransmission(address)`, 
`Wire.write(byte)`, 
`Wire.endTransmission()`, 
`Wire.requestFrom(address, length)`, 
`Wire.read()`

**Pitfalls:** Needs pull-up resistors (~4.7kΩ) on SDA/SCL. Address conflicts cause "sensor not found" bugs.

---

## 7. `#include <EEPROM.h>`

**Purpose:** Persist small settings/counters across power cycles.

**How it works:** No true EEPROM hardware — this emulates one in a reserved flash block. `EEPROM.begin(size)` copies that block into RAM; you read/write the RAM buffer, and nothing hits flash until `EEPROM.commit()`.

**Common functions:** 
`EEPROM.begin(size)`, 
`EEPROM.read(addr)` / `write(addr, value)`, 
`EEPROM.get()` / `put()`, 
`EEPROM.commit()`

**Pitfalls:** Flash has ~100,000 erase/write cycles — don't `commit()` in a tight loop. Consider `LittleFS` for new projects.

---

## 8. `#include <LittleFS.h>` (or `<FS.h>`)

**Purpose:** A real file system on flash — config files, logs, or web assets.

**How it works:** Wear-leveling, power-loss-resilient file system addressed by path (`/config.json`), replacing the deprecated SPIFFS.

**Common functions:** 
`LittleFS.begin()`, 
`LittleFS.open(path, mode)`, 
`file.print()` / `readString()`, 
`LittleFS.exists(path)`, 
`LittleFS.remove(path)`

**Pitfalls:** Flash size/partition must match the Arduino IDE board menu setting, or `begin()` fails.

---

## 9. `#include <Ticker.h>`

**Purpose:** Run a function periodically or after a delay, without blocking `loop()`.

**How it works:** Schedules a callback via the hardware timer/interrupt system. `attach()` re-arms repeatedly; `once()` fires a single time.

**Common functions:** `ticker.attach(seconds, callback)`, `ticker.attach_ms(ms, callback)`, `ticker.once(seconds, callback)`, `ticker.detach()`

**Pitfalls:** Callbacks run in interrupt context — keep them short, no `delay()`, heavy `Serial.print()`, or Wi-Fi calls.

---

## 10. `#include <PubSubClient.h>`

**Purpose:** MQTT client for pub/sub messaging to home automation or cloud IoT brokers.

**How it works:** Wraps a `WiFiClient` connection to a broker. `publish()` sends to a topic; `subscribe()` receives via a callback.

**Common functions:** 
`client.setServer(broker_ip, port)`, 
`client.connect(clientId)`, 
`client.publish(topic, payload)`, 
`client.subscribe(topic)`, 
`client.setCallback(function)`, 
`client.loop()`

**Pitfalls:** Default max packet size is 256 bytes — larger JSON payloads silently fail unless you raise `MQTT_MAX_PACKET_SIZE`. `client.loop()` must run frequently.

---

## 11. `#include <ESP8266mDNS.h>`

**Purpose:** Reach the device by name (`http://myesp.local`) instead of an IP.

**How it works:** Broadcasts hostname/IP over multicast UDP; compatible devices resolve `.local` names without central DNS. Can also advertise services (`_http._tcp`).

**Common functions:** 
`MDNS.begin("hostname")`, 
`MDNS.addService(service, protocol, port)`, 
`MDNS.update()`

**Pitfalls:** Some networks block multicast (guest/corporate Wi-Fi) — keep the IP as a fallback.

---

## 12. `#include <ArduinoOTA.h>`

**Purpose:** Upload new firmware over Wi-Fi instead of USB.

**How it works:** Opens a listener the Arduino IDE (or `espota.py`) connects to, receives the binary in chunks, writes it to a spare flash partition, and reboots into it once verified.

**Common functions:** 
`ArduinoOTA.setHostname(name)`, 
`ArduinoOTA.setPassword(password)`, 
`ArduinoOTA.begin()`, 
`ArduinoOTA.handle()`

**Pitfalls:** If `handle()` rarely runs because of blocking code elsewhere, OTA updates time out.

---

# New additions

## 13. `#include <Servo.h>`

**Purpose:** Drives hobby servo motors using a PWM-like pulse (typically 1–2 ms every 20 ms).

**How it works:** On ESP8266, `Servo` uses a software-timed pulse generated off the same hardware timer infrastructure as `Ticker`/`SoftwareSerial`. Because they share timer resources, running several servos plus `SoftwareSerial` at once can cause jitter.

**Common functions:** 
`attach(pin)`, 
`write(angle)`, 
`writeMicroseconds(us)`, 
`read()`, 
`detach()`

**Pitfalls:** Only a handful of pins support glitch-free PWM at once; powering multiple servos from the ESP8266's onboard 3.3V regulator will brown it out — use a separate supply.

```cpp
#include <Servo.h>

Servo myServo;

void setup() {
  myServo.attach(D4);   // GPIO2
}

void loop() {
  myServo.write(0);
  delay(1000);
  myServo.write(90);
  delay(1000);
  myServo.write(180);
  delay(1000);
}
```

---

## 14. `#include <DHT.h>` (DHT sensor library, e.g. by Adafruit)

**Purpose:** Reads temperature and humidity from DHT11/DHT22 sensors.

**How it works:** The DHT protocol is a single-wire, timing-based serial handshake — the library bit-bangs the read by measuring pulse widths on the data pin, then validates a checksum byte.

**Common functions:** 
`begin()`, 
`readTemperature()`, 
`readHumidity()`, 
`readTemperature(true)` (Fahrenheit)

**Pitfalls:** DHT sensors can only be polled every ~2 seconds; polling faster returns stale or `NaN` readings. Needs a pull-up resistor on the data line if your breakout doesn't include one.

```cpp
#include <DHT.h>

#define DHTPIN D4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.printf("Humidity: %.1f%%  Temp: %.1fC\n", h, t);
  }
  delay(2000);
}
```

---

## 15. `#include <OneWire.h>` + `#include <DallasTemperature.h>`

**Purpose:** Reads DS18B20 waterproof temperature probes over the 1-Wire bus.

**How it works:** `OneWire` implements the low-level 1-Wire signaling (each device has a unique 64-bit ROM address, allowing many sensors on one pin). `DallasTemperature` builds the DS18B20-specific conversion commands and scratchpad reads on top of it.

**Common functions:** `sensors.begin()`, `sensors.requestTemperatures()`, `sensors.getTempCByIndex(0)`

**Pitfalls:** Requires a 4.7kΩ pull-up on the data line. `requestTemperatures()` triggers an on-sensor conversion that takes up to 750ms — calling it too often blocks unnecessarily.

```cpp
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS D3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(115200);
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.printf("Temperature: %.2fC\n", tempC);
  delay(1000);
}
```

---

## 16. `#include <Adafruit_NeoPixel.h>`

**Purpose:** Drives addressable RGB LED strips (WS2812/NeoPixel).

**How it works:** Each pixel's color is shifted out as a precisely-timed serial bitstream (no clock line) — the library bit-bangs this with cycle-accurate delays, which is why NeoPixel updates briefly disable interrupts.

**Common functions:** `begin()`, `setPixelColor(n, r, g, b)`, `show()`, `setBrightness(n)`, `Color(r, g, b)`

**Pitfalls:** Because interrupts are briefly disabled during `show()`, it can collide with Wi-Fi timing on ESP8266 — keep strips reasonably short and avoid calling `show()` extremely frequently.

```cpp
#include <Adafruit_NeoPixel.h>

#define PIN D2
#define NUMPIXELS 8

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.setBrightness(50);
  strip.show();
}

void loop() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, strip.Color(0, 150, 255));
  }
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
  delay(500);
}
```

---

## 17. `#include <NTPClient.h>`

**Purpose:** Fetches accurate wall-clock time from an NTP server over Wi-Fi.

**How it works:** Wraps a `WiFiUDP` socket, sends an NTP request packet to a time server, and parses the returned timestamp into epoch seconds, updating on an interval you configure.

**Common functions:** `begin()`, `update()`, `getEpochTime()`, `getFormattedTime()`, `setTimeOffset(seconds)`

**Pitfalls:** `update()` only refreshes on its internal interval, not every call — for a live clock, increment locally with `millis()` between syncs rather than calling `update()` in a tight loop.

```cpp
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  timeClient.begin();
}

void loop() {
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  delay(1000);
}
```

---

## 18. `#include <WiFiManager.h>` (tzapu/WiFiManager)

**Purpose:** Lets a device configure its own Wi-Fi credentials via a captive portal, instead of hardcoding SSID/password.

**How it works:** If it can't connect with stored credentials, it puts the ESP8266 into AP mode with a captive-portal web page; the user picks a network and enters a password from their phone/laptop, which `WiFiManager` then saves and uses to connect in station mode.

**Common functions:** `autoConnect(apName)`, `resetSettings()`, `setConfigPortalTimeout(seconds)`

**Pitfalls:** `autoConnect()` blocks until either a connection succeeds or the portal times out — plan your `setup()` flow (e.g. status LED) around that wait.

```cpp
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

WiFiManager wm;

void setup() {
  Serial.begin(115200);
  bool ok = wm.autoConnect("ESP8266-Setup");
  if (!ok) {
    Serial.println("Failed to connect, restarting...");
    ESP.restart();
  }
  Serial.println("Connected!");
}

void loop() {}
```

---

## 19. `#include <ESPAsyncWebServer.h>` (+ `ESPAsyncTCP`)

**Purpose:** A non-blocking alternative to `ESP8266WebServer` — handles many concurrent connections without freezing `loop()`.

**How it works:** Built on `ESPAsyncTCP`'s event-driven socket layer instead of blocking reads/writes; request handlers are attached as callbacks that fire when data arrives, so `loop()` never has to wait on a client.

**Common functions:** `server.on(path, handler)`, `server.begin()`, `request->send(status, type, body)`, `AsyncWebServerRequest`

**Pitfalls:** Because everything runs from callbacks, doing long/blocking work (like `delay()` or slow sensor reads) inside a handler still stalls the event loop — keep handlers fast and hand off slow work elsewhere.

```cpp
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setup() {
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello from async server!");
  });

  server.begin();
}

void loop() {}
```

---

## 20. `#include <IRremoteESP8266.h>` (+ `IRsend.h` / `IRrecv.h`)

**Purpose:** Sends and receives infrared remote-control signals.

**How it works:** IR remotes encode commands as timed on/off pulses of a modulated carrier (usually 38kHz). `IRsend` generates that carrier and pulse pattern on a GPIO through the hardware PWM/timer; `IRrecv` demodulates an IR receiver module's output and decodes it against known protocol timings (NEC, Sony, Samsung, etc.).

**Common functions:** `IRsend::sendNEC(data, bits)`, `IRrecv::decode(&results)`, `IRrecv::resume()`

**Pitfalls:** IR receive decoding is timing-sensitive, so it competes with Wi-Fi and other interrupt-heavy libraries for CPU — glitchy decodes are common if too much else is running.

```cpp
#include <IRremoteESP8266.h>
#include <IRsend.h>

const uint16_t IR_LED_PIN = D2;
IRsend irsend(IR_LED_PIN);

void setup() {
  irsend.begin();
}

void loop() {
  irsend.sendNEC(0x00FFE01F);  // example NEC power-button code
  delay(5000);
}
```

---

### Notes on combining these

Several of these libraries compete for the same limited hardware timer/interrupt resources (`Servo`, `SoftwareSerial`, `Ticker`, `Adafruit_NeoPixel`, IR send/receive). If a project uses more than one of these together, test for jitter or dropped Wi-Fi packets, and consider moving non-critical work off interrupt-driven paths where possible.