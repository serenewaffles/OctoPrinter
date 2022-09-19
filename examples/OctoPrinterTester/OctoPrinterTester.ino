#include <Arduino.h>
#include <OctoPrinter.h>
#include <WiFi.h>

const String API_KEY = "your api key here";
const String HOST_IP = "your host's IP here";
const int HOST_PORT = 80;

const char* SSID = "MyWiFiSSID";
const char* PASSWORD = "MyWiFiPassword";

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
