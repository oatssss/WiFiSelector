#pragma once

#include <Arduino.h>

class _WifiWrapper
{
    int8_t m_numNetworks = 0;

public:
    /**
     * @brief Attempt to connect to an AP
     *
     * @param ssid Optional - The name of the access point, if unspecified, will use previous connection details
     * @param pass Optional - The WPA2 pass of the AP
     * @param timeoutMs Optional - Duration in ms to attempt the connection
     *
     * @return true If successfully connected to the AP before the timeout expired
     * @return false If connection failed or the timeout expired
     */
    bool connect(const String& ssid = "", const String& pass = "", unsigned long timeoutMs = 0) const;

    /**
     * @brief Scan for the available networks
     *
     * @return int8_t The number of networks found
     */
    int8_t scanNetworks();

    /**
     * @brief Get the networks found in the last scan
     *
     * @return String A JSON string with the networks: {networks:["network1","network2"]}
     */
    String getCachedNetworkList() const;

    /**
     * @brief Get the ESP station status
     *
     * @return wl_status_t
     */
    bool isConnected() const;
};

extern _WifiWrapper WifiWrapper;
