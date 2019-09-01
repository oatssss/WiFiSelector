#pragma once

#include <Arduino.h>
#include "WebServer.h"

class WifiSelectorHandler : public Oats::WebServer::Handler
{
public:
    WifiSelectorHandler();
    ~WifiSelectorHandler() = default;

    WifiSelectorHandler(const WifiSelectorHandler&) = delete;
    WifiSelectorHandler& operator=(const WifiSelectorHandler&) = delete;

protected:
    void setupHttpHandlers(ESP8266WebServer& httpServer) override;
    void setupWsHandlers(WebSocketsServer& wsServer) override;

public:
    /**
     * @brief Attempt to connect to a previously saved network.
     *        If unable to within the timeout, AP mode will be enabled with the given ssid/pass and
     *        HTTP requests to the ESP will be sent a WiFi selector page
     *
     * @remarks This is a blocking synchronous call.
     *          The function returns when the ESP successully connects to
     *          a network specified through the WiFi selector page
     *
     * @param ssid Optional - The name the ESP should use as its network name, if not specified, defaults to "esp8266-oats"
     * @param pass Optional - A password to protect the network, defaults to no password
     * @param timeoutMs Optional - The duration in ms to attempt initial connection.
     *                  If connection fails, connection attempts will stop
     *                  and the WiFi selector will be available via HTTP.
     *                  Defaults to 10 seconds
     */
    void reconnect(const String& portalSsid = "esp8266-oats", const String& portalPass = "", size_t timeoutMs = 10000);
};

extern WifiSelectorHandler WifiSelector;
