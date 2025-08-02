#include "streaming/HttpServer.h"
#include <QDebug>

namespace LegacyStream {

HttpServer::HttpServer(QObject *parent)
    : QObject(parent)
    , m_isRunning(false)
{
    qDebug() << "HttpServer initialized";
}

HttpServer::~HttpServer()
{
    qDebug() << "HttpServer destroyed";
}

bool HttpServer::start(int port)
{
    qDebug() << "HttpServer: Starting on port" << port;
    m_isRunning = true;
    return true;
}

void HttpServer::stop()
{
    qDebug() << "HttpServer: Stopping";
    m_isRunning = false;
}

bool HttpServer::isRunning() const
{
    return m_isRunning;
}

} // namespace LegacyStream 