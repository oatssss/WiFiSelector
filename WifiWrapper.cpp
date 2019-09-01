#include "WifiWrapper.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

_WifiWrapper WifiWrapper;

bool _WifiWrapper::connect(const String& ssid, const String& pass, unsigned long timeoutMs) const
{
    //wifi_station_disconnect();

    unsigned long timeoutStart = millis();

    if (ssid == "") {
        WiFi.begin();
    }
    else {
        WiFi.begin(ssid, pass);
    }

    if (timeoutMs == 0) {
        return WL_CONNECTED == WiFi.waitForConnectResult();
    }

    while (WiFi.status() != WL_CONNECTED) {

        if (WiFi.status() == WL_CONNECT_FAILED ||
            WiFi.status() == WL_NO_SSID_AVAIL  ||
            millis() - timeoutStart > timeoutMs)
        {
            //wifi_station_disconnect();
            return false;
        }
        delay(10);
    }

    return true;
}

int8_t _WifiWrapper::scanNetworks()
{
    return m_numNetworks = WiFi.scanNetworks();
}

int sort_desc(const void *cmp1, const void *cmp2)
{
    int idxA = *(static_cast<const int*>(cmp1));
    int idxB = *(static_cast<const int*>(cmp2));
    return WiFi.RSSI(idxB) - WiFi.RSSI(idxA);
}

String _WifiWrapper::getCachedNetworkList() const
{
    // Sort SSID indices descending by signal strength
    int* indices = new int[m_numNetworks];
    for (int i = 0; i < m_numNetworks; i++) {
        indices[i] = i;
    }
    qsort(indices, m_numNetworks, sizeof(int), sort_desc);

    String networks = R"({"networks":[)";

    for(int i = 0; i < m_numNetworks; i++) {
        // Serialize the SSID to get proper escaping
        StaticJsonDocument<128> ssid;
        JsonVariant var = ssid.to<JsonVariant>();
        var.set(WiFi.SSID(indices[i]));
        String serialized;
        serializeJson(ssid, serialized);

        if (i == 0) {
            networks.concat(serialized);
        }
        else {
            networks.concat(", " + serialized);
        }
    }

    delete indices;

    networks.concat("]}");

    return networks;
}

bool _WifiWrapper::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}
