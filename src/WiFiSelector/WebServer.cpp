#include "WebServer.h"

Oats::WebServer OatsWebServer;

Oats::WebServer::~WebServer()
{
    stop();
}

void Oats::WebServer::setHandler(Handler& handler, int httpPort, uint16_t wsPort)
{
    delete m_httpServer;
    m_httpServer = new ESP8266WebServer(httpPort);
    handler.setupHttpHandlers(*m_httpServer);
    m_httpServer->begin();

    delete m_wsServer;
    m_wsServer = new WebSocketsServer(wsPort);
    handler.setupWsHandlers(*m_wsServer);
    m_wsServer->begin();
}

void Oats::WebServer::handle() const
{
    if (m_httpServer) {
        m_httpServer->handleClient();
    }

    if (m_wsServer) {
        m_wsServer->loop();
    }
}

void Oats::WebServer::stop()
{
    if (m_httpServer) {
        delete m_httpServer;
        m_httpServer = nullptr;
    }

    if (m_wsServer) {
        delete m_wsServer;
        m_wsServer = nullptr;
    }
}
