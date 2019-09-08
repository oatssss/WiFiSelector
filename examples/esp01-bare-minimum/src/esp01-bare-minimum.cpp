#include <WifiSelector.h>

void setup() {
  Serial.begin(9600);
  WifiSelector.reconnect("esp8266-ssid", "esp8266-pass");

  // WifiSelector::reconnect blocks here and attempts to connect to a saved access point
  // If unsuccessful, the board enables AP mode and creates the network "esp8266-ssid"
  // protected with password "esp8266-pass"
  // A DNS server intercepts HTTP requests to any domain and serves the WiFi Selector landing page
  // The call only unblocks when the board successfully connects to an AP
}

void loop() {
  const String msg = "Connected to " + WiFi.SSID();
  Serial.println(msg.c_str());
}
