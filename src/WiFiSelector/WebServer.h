#pragma once

#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

namespace Oats
{
class WebServer
{
public:
    WebServer() = default;
    ~WebServer();

    WebServer(const WebServer&) = delete;
    WebServer& operator=(const WebServer&) = delete;

private:
    ESP8266WebServer* m_httpServer = nullptr;
    WebSocketsServer* m_wsServer = nullptr;

public:
    class Handler
    {
        friend class WebServer;

    protected:
        Handler() = default;
        virtual ~Handler() = default;

        virtual void setupHttpHandlers(ESP8266WebServer& httpServer) = 0;
        virtual void setupWsHandlers(WebSocketsServer& wsServer) {}
    };

    void setHandler(Handler& handler, int httpPort = 80, uint16_t wsPort = 8080);
    void handle() const;
    void stop();
};
}

extern Oats::WebServer OatsWebServer;
