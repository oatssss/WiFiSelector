#include "WifiSelector.h"
#include "WiFiSelector/WifiWrapper.h"
#include "WiFiSelector/Debugging.h"

#include <FS.h>
#include <DNSServer.h>

WifiSelectorHandler WifiSelector;

WifiSelectorHandler::WifiSelectorHandler()
{
    SPIFFS.begin();
    WifiWrapper.scanNetworks();
}

namespace {
    String getContentType(const String& filename) { // convert the file extension to the MIME type
        if (filename.endsWith(".html")) return "text/html";
        if (filename.endsWith(".css")) return "text/css";
        if (filename.endsWith(".js")) return "application/javascript";
        return "text/plain";
    }

    bool handleUrlRequest(ESP8266WebServer& httpServer) { // send the right file to the client (if it exists)

        String path = httpServer.uri();
        PRINT_LN("handleUrlRequest: %s", path.c_str());

        if (path.endsWith("/")) {
            path += "wifi-selector.html";
        }

        String contentType = getContentType(path);
        if (SPIFFS.exists(path)) {
            File file = SPIFFS.open(path, "r");
            httpServer.streamFile(file, contentType);
            file.close();
            return true;
        }

        if (path.endsWith("wifi-selector.html")) {
            PRINT_LN("\twifi-selector.html wasn't found, please make sure you've uploaded the contents of `WiFiSelector/basic-front-end` to the SPIFFS image");
            httpServer.send(404, "text/plain", "wifi-selector.html wasn't found, please make sure you've uploaded the contents of `WiFiSelector/basic-front-end` to the SPIFFS image");
        }
        else {
            PRINT_LN("\tFile Not Found");
            httpServer.send(404, "text/plain", "404: Not Found");
        }

        return false;
    }
}

void WifiSelectorHandler::setupHttpHandlers(ESP8266WebServer& httpServer)
{
#ifndef NDEBUG
    PRINT_LN("SPIFFS files:");
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
        String str = "\t";
        str += dir.fileName();
        str += " / ";
        str += (unsigned int)dir.fileSize();
        PRINT_LN(str.c_str());
    }
#endif

    httpServer.on("/scan", [&httpServer](){
        WifiWrapper.scanNetworks();
        httpServer.send(200, "text/json", WifiWrapper.getCachedNetworkList());
    });

    httpServer.onNotFound([&httpServer](){
        handleUrlRequest(httpServer);
    });
}

struct Command
{
    String name;
    String args[2];
};

bool parseCommand(const String& text, Command& output);
void handleClientConnected(WebSocketsServer& wsServer, uint8_t id);
void handleScan(WebSocketsServer& wsServer, uint8_t id);
void handleAuth(WebSocketsServer& wsServer, uint8_t id, const String& ssid, const String& pass);

void WifiSelectorHandler::setupWsHandlers(WebSocketsServer& wsServer)
{
    wsServer.onEvent([this, &wsServer](uint8_t id, WStype_t type, uint8_t* payload, size_t length){
        switch(type) {
            case WStype_DISCONNECTED: {
                PRINT_LN("[%u] Disconnected!", id);
                break;
            }
            case WStype_CONNECTED: {
                // POTENTIAL TODO: Setup ping/pong to terminate lingering dead clients
                IPAddress ip = wsServer.remoteIP(id);
                PRINT_LN("[%u] Connected from %d.%d.%d.%d url: %s", id, ip[0], ip[1], ip[2], ip[3], reinterpret_cast<const char*>(payload));
                handleClientConnected(wsServer, id);
                break;
            }
            case WStype_TEXT: {
                PRINT_LN("[%u] get Text: %s", id, payload);

                // Handle the command sent
                Command cmd = {};
                bool isCmd = parseCommand(String(reinterpret_cast<char*>(payload)), cmd);
                if (isCmd && cmd.name == "scan") {
                    handleScan(wsServer, id);
                }
                if (isCmd && cmd.name == "auth") {
                    handleAuth(wsServer, id, cmd.args[0], cmd.args[1]);
                }

                break;
            }
            default:
                break;
        }
    });
}

bool parseCommand(const String& text, Command& output)
{
    // Return if text isn't a command
    int colonIdx = text.indexOf(':');
    if (colonIdx < 0) {
        return false;
    }

    output = {};

    // Text up to first colon is cmd name
    output.name = text.substring(0, colonIdx);

    // Following text is the argument list delimited by colons
    for (uint8_t i = 0; i < sizeof(output.args)/sizeof(output.args[0]); i++) {
        size_t prevColonIdx = colonIdx;
        if ((colonIdx = text.indexOf(':', prevColonIdx+1)) < 0) {
            break;
        }
        output.args[i] = text.substring(prevColonIdx+1, colonIdx);
    }

    return true;
}

void handleClientConnected(WebSocketsServer& wsServer, uint8_t id)
{
    handleScan(wsServer, id);

    if (WifiWrapper.isConnected()) {
        wsServer.sendTXT(id, R"({"authorized":true})");
    }
    else {
        wsServer.sendTXT(id, R"({"authorized":false})");
    }
}

void handleScan(WebSocketsServer& wsServer, uint8_t id)
{
    PRINT_LN("handleScan - start");
    WifiWrapper.scanNetworks();
    String networks = WifiWrapper.getCachedNetworkList();
    wsServer.sendTXT(id, networks);
    PRINT_LN("handleScan - end");
}

void handleAuth(WebSocketsServer& wsServer, uint8_t id, const String& ssid, const String& pass)
{
    // TODO: Set a flag when authorizing that gets cleared when sending the client the web page.
    //       If it's set when the client requests the page, send an
    //       additional msg to the client that the previous auth failed.
    //       See if it's possible to get client's IP/MAC, then set flag per IP, and only send
    //       failure msg to clients with matching IP/MAC
    PRINT_LN("handleAuth(%s, %s) - start", ssid.c_str(), pass.c_str());
    const bool connected = WifiWrapper.connect(ssid, pass);
    if (!connected) {
        wsServer.close();
        wsServer.begin();
        PRINT_LN("handleAuth - error end");
        return;
    }

    wsServer.close();
    wsServer.begin();
    PRINT_LN("handleAuth - success end");
    // Since we're connected to a network now, we don't need the wifi selector or webserver
    OatsWebServer.stop();
}

void WifiSelectorHandler::reconnect(const String& portalSsid, const String& portalPass, size_t timeoutMs, int httpPort, uint16_t wsPort)
{
    // Attempt to reconnect to the previous network
    if (WifiWrapper.connect("", "", timeoutMs)) {
        return;
    }

    if (!WiFi.softAP(portalSsid, portalPass)) {
        PRINT_LN("Unable to start soft Access Point, rebooting...");
        ESP.restart();
        delay(2000);
    }

    // Upon failure, show the Wifi Selector
    OatsWebServer.setHandler(WifiSelector, httpPort, wsPort);

    // Route all traffic to 192.168.4.1 (default IP of esp)
    DNSServer dnsServer;
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1)); // Port 53, match requests to any domain, send to 192.168.4.1 (itself)

    // Block until connected to an AP
    // Process just dns/http/websocket requests
    while (!WifiWrapper.isConnected()) {
        dnsServer.processNextRequest();
        OatsWebServer.handle();
    }
}
