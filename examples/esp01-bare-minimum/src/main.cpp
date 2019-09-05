#include <WifiSelector.h>

void setup() {
  Serial.begin(9600);
  WifiSelector.reconnect("esp8266-ssid", "esp8266-pass");
}

void loop() {
  const String msg = "Connected to " + WiFi.SSID();
  Serial.println(msg.c_str());
}