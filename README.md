[![Library Lint](https://github.com/serenewaffles/OctoPrinter/actions/workflows/arduino-lint.yaml/badge.svg?branch=main)](https://github.com/serenewaffles/OctoPrinter/actions/workflows/arduino-lint.yaml)

# OctoPrinter

### Description

An Arduino library for interfacing with an OctoPrint instance.

# Dependencies
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

# Usage
```cpp
#include <Arduino.h>
#include <OctoPrinter.h>
#include <WiFi.h>

WiFiClient client;
OctoPrinter printerName(API_KEY, HOST_IP, HOST_PORT);

void setup() {
    Serial.begin(115200);
    WiFi.begin(SSID, PASSWORD);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    printerName.begin();
}

void loop() {
    Serial.print("Nozzle temperature actual/target: ");
    Serial.print(printerName.toolActual());
    Serial.print("/");
    Serial.println(printerName.toolTarget());
    delay(1000);
}
```
