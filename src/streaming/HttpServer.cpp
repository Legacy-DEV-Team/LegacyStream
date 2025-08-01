#include "streaming/HttpServer.h"
#include "streaming/WebInterface.h"
#include "streaming/StreamManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(httpServer, "httpServer")

namespace LegacyStream {

HttpServer::HttpServer(QObject* parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
{
    qCDebug(httpServer) << "HttpServer created";
    
    // Initialize MIME types
    m_mimeTypes[".html"] = "text/html";
    m_mimeTypes[".htm"] = "text/html";
    m_mimeTypes[".css"] = "text/css";
    m_mimeTypes[".js"] = "application/javascript";
    m_mimeTypes[".json"] = "application/json";
    m_mimeTypes[".png"] = "image/png";
    m_mimeTypes[".jpg"] = "image/jpeg";
    m_mimeTypes[".jpeg"] = "image/jpeg";
    m_mimeTypes[".gif"] = "image/gif";
    m_mimeTypes[".ico"] = "image/x-icon";
    m_mimeTypes[".svg"] = "image/svg+xml";
    m_mimeTypes[".mp3"] = "audio/mpeg";
    m_mimeTypes[".ogg"] = "audio/ogg";
    m_mimeTypes[".wav"] = "audio/wav";
    m_mimeTypes[".txt"] = "text/plain";
    m_mimeTypes[".xml"] = "application/xml";
}

HttpServer::~HttpServer()
{
    shutdown();
}

bool HttpServer::initialize(int port, const QString& host)
{
    m_port = port;
    m_host = host;
    
    if (!m_tcpServer->listen(QHostAddress(m_host), m_port)) {
        qCCritical(httpServer) << "Failed to start HTTP server on" << m_host << ":" << m_port;
        return false;
    }
    
    connect(m_tcpServer, &QTcpServer::newConnection,
            this, &HttpServer::onNewConnection);
    
    m_isRunning = true;
    qCInfo(httpServer) << "HTTP server started on" << m_host << ":" << m_port;
    return true;
}

void HttpServer::shutdown()
{
    if (m_tcpServer) {
        m_tcpServer->close();
    }
    
    // Close all client connections
    for (QTcpSocket* client : m_clients) {
        client->close();
        client->deleteLater();
    }
    m_clients.clear();
    
    m_isRunning = false;
    qCInfo(httpServer) << "HTTP server shut down";
}

void HttpServer::setPort(int port)
{
    m_port = port;
}

void HttpServer::setHost(const QString& host)
{
    m_host = host;
}

void HttpServer::setWebInterface(WebInterface::WebInterface* webInterface)
{
    m_webInterface = webInterface;
}

void HttpServer::setStreamManager(StreamManager* streamManager)
{
    m_streamManager = streamManager;
}

bool HttpServer::isRunning() const
{
    return m_isRunning;
}

int HttpServer::getPort() const
{
    return m_port;
}

QString HttpServer::getHost() const
{
    return m_host;
}

void HttpServer::onNewConnection()
{
    QTcpSocket* client = m_tcpServer->nextPendingConnection();
    if (client) {
        connect(client, &QTcpSocket::readyRead,
                this, &HttpServer::onClientReadyRead);
        connect(client, &QTcpSocket::disconnected,
                this, &HttpServer::onClientDisconnected);
        
        m_clients.append(client);
        QString clientIP = getClientIP(client);
        emit clientConnected(clientIP);
        
        qCDebug(httpServer) << "New client connected:" << clientIP;
    }
}

void HttpServer::onClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        QString clientIP = getClientIP(client);
        m_clients.removeOne(client);
        client->deleteLater();
        emit clientDisconnected(clientIP);
        
        qCDebug(httpServer) << "Client disconnected:" << clientIP;
    }
}

void HttpServer::onClientReadyRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    QByteArray data = client->readAll();
    QString request = QString::fromUtf8(data);
    
    if (!request.isEmpty()) {
        handleHttpRequest(client, request);
    }
}

void HttpServer::handleHttpRequest(QTcpSocket* socket, const QString& request)
{
    QString method, path, body;
    QMap<QString, QString> headers;
    
    if (!parseHttpRequest(request, method, path, headers, body)) {
        sendErrorResponse(socket, 400, "Bad Request");
        return;
    }
    
    QString clientIP = getClientIP(socket);
    emit requestReceived(method, path, clientIP);
    
    // Update statistics
    m_totalRequests++;
    m_requestCounts[method]++;
    
    handleRoute(socket, method, path, headers, body);
}

bool HttpServer::parseHttpRequest(const QString& request, QString& method, QString& path,
                                 QMap<QString, QString>& headers, QString& body)
{
    QStringList lines = request.split("\r\n");
    if (lines.isEmpty()) return false;
    
    // Parse request line
    QString requestLine = lines.takeFirst();
    QStringList parts = requestLine.split(" ");
    if (parts.size() < 2) return false;
    
    method = parts[0];
    path = extractPath(requestLine);
    
    // Parse headers
    headers = extractHeaders(lines);
    
    // Parse body (if present)
    body = "";
    bool inBody = false;
    for (const QString& line : lines) {
        if (inBody) {
            body += line + "\r\n";
        } else if (line.isEmpty()) {
            inBody = true;
        }
    }
    
    return true;
}

QString HttpServer::extractPath(const QString& requestLine)
{
    QStringList parts = requestLine.split(" ");
    if (parts.size() < 2) return "/";
    
    QString path = parts[1];
    QUrl url(path);
    return url.path();
}

QMap<QString, QString> HttpServer::extractHeaders(const QStringList& lines)
{
    QMap<QString, QString> headers;
    
    for (const QString& line : lines) {
        if (line.isEmpty()) break;
        
        int colonPos = line.indexOf(':');
        if (colonPos > 0) {
            QString key = line.left(colonPos).trimmed().toLower();
            QString value = line.mid(colonPos + 1).trimmed();
            headers[key] = value;
        }
    }
    
    return headers;
}

void HttpServer::handleRoute(QTcpSocket* socket, const QString& method, const QString& path,
                            const QMap<QString, QString>& headers, const QString& body)
{
    // Handle API requests
    if (path.startsWith("/api/")) {
        handleApiRequest(socket, method, path, headers, body);
        return;
    }
    
    // Handle WebSocket upgrade
    if (path == "/ws" && headers["upgrade"] == "websocket") {
        // WebSocket upgrade handling would go here
        sendErrorResponse(socket, 501, "WebSocket not implemented yet");
        return;
    }
    
    // Handle static files
    if (isStaticFile(path)) {
        handleStaticFile(socket, path);
        return;
    }
    
    // Handle web interface requests
    handleWebInterfaceRequest(socket, method, path, headers, body);
}

void HttpServer::handleApiRequest(QTcpSocket* socket, const QString& method, const QString& path,
                                 const QMap<QString, QString>& headers, const QString& body)
{
    Q_UNUSED(headers)
    
    if (method != "GET" && method != "POST") {
        sendErrorResponse(socket, 405, "Method Not Allowed");
        return;
    }
    
    QString response;
    QString contentType = "application/json";
    
    if (path == "/api/stats") {
        if (m_webInterface) {
            QJsonObject stats = m_webInterface->getServerStatsJson();
            response = QJsonDocument(stats).toJson();
        } else {
            response = "{\"error\": \"Web interface not available\"}";
        }
    } else if (path == "/api/mountpoints") {
        if (m_webInterface) {
            QJsonObject mountPoints = m_webInterface->getMountPointsJson();
            response = QJsonDocument(mountPoints).toJson();
        } else {
            response = "{\"error\": \"Web interface not available\"}";
        }
    } else if (path == "/api/analytics") {
        if (m_webInterface) {
            QJsonObject analytics = m_webInterface->getAnalyticsData();
            response = QJsonDocument(analytics).toJson();
        } else {
            response = "{\"error\": \"Web interface not available\"}";
        }
    } else if (path == "/api/relay") {
        if (m_webInterface) {
            QJsonObject relayStats = m_webInterface->getRelayStatsJson();
            response = QJsonDocument(relayStats).toJson();
        } else {
            response = "{\"error\": \"Web interface not available\"}";
        }
    } else if (path == "/api/control" && method == "POST") {
        // Handle stream control requests
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject request = doc.object();
            QString mountPoint = request["mountPoint"].toString();
            QString action = request["action"].toString();
            
            if (m_webInterface) {
                if (action == "start") {
                    m_webInterface->startStream(mountPoint);
                } else if (action == "stop") {
                    m_webInterface->stopStream(mountPoint);
                } else if (action == "restart") {
                    m_webInterface->restartStream(mountPoint);
                }
                
                response = "{\"status\": \"success\", \"action\": \"" + action + "\", \"mountPoint\": \"" + mountPoint + "\"}";
            } else {
                response = "{\"error\": \"Web interface not available\"}";
            }
        } else {
            response = "{\"error\": \"Invalid JSON\"}";
        }
    } else {
        sendErrorResponse(socket, 404, "API endpoint not found");
        return;
    }
    
    sendHttpResponse(socket, 200, "OK", contentType, response.toUtf8());
}

void HttpServer::handleWebInterfaceRequest(QTcpSocket* socket, const QString& method, const QString& path,
                                         const QMap<QString, QString>& headers, const QString& body)
{
    Q_UNUSED(method)
    Q_UNUSED(headers)
    Q_UNUSED(body)
    
    QString response;
    QString contentType = "text/html";
    
    if (path == "/" || path == "/index.html") {
        if (m_webInterface) {
            response = m_webInterface->generateStatusPage();
        } else {
            response = "<h1>LegacyStream Server</h1><p>Web interface not available</p>";
        }
    } else if (path == "/mountpoints") {
        if (m_webInterface) {
            response = m_webInterface->generateMountPointsPage();
        } else {
            response = "<h1>Mount Points</h1><p>Web interface not available</p>";
        }
    } else if (path == "/analytics") {
        if (m_webInterface) {
            response = m_webInterface->generateAnalyticsPage();
        } else {
            response = "<h1>Analytics</h1><p>Web interface not available</p>";
        }
    } else if (path == "/mobile") {
        if (m_webInterface) {
            response = m_webInterface->generateMobilePage();
        } else {
            response = "<h1>Mobile Interface</h1><p>Web interface not available</p>";
        }
    } else if (path.startsWith("/stream/")) {
        QString mountPoint = path.mid(8); // Remove "/stream/" prefix
        if (m_webInterface) {
            response = m_webInterface->generateStreamPage(mountPoint);
        } else {
            response = "<h1>Stream Details</h1><p>Web interface not available</p>";
        }
    } else {
        sendErrorResponse(socket, 404, "Page not found");
        return;
    }
    
    sendHttpResponse(socket, 200, "OK", contentType, response.toUtf8());
}

void HttpServer::handleStaticFile(QTcpSocket* socket, const QString& path)
{
    QString filePath = m_staticFilesPath + path;
    QFile file(filePath);
    
    if (!file.exists()) {
        sendErrorResponse(socket, 404, "File not found");
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        sendErrorResponse(socket, 500, "Internal server error");
        return;
    }
    
    QByteArray data = file.readAll();
    QString contentType = getMimeType(path);
    
    sendHttpResponse(socket, 200, "OK", contentType, data);
}

void HttpServer::sendHttpResponse(QTcpSocket* socket, int statusCode, const QString& statusText,
                                 const QString& contentType, const QByteArray& body)
{
    QString response = QString("HTTP/1.1 %1 %2\r\n"
                              "Content-Type: %3\r\n"
                              "Content-Length: %4\r\n"
                              "Connection: close\r\n"
                              "\r\n")
                       .arg(statusCode)
                       .arg(statusText)
                       .arg(contentType)
                       .arg(body.size());
    
    socket->write(response.toUtf8() + body);
    socket->flush();
    
    m_totalBytesServed += body.size();
}

void HttpServer::sendErrorResponse(QTcpSocket* socket, int statusCode, const QString& message)
{
    QString html = QString("<html><head><title>%1</title></head>"
                          "<body><h1>%1</h1><p>%2</p></body></html>")
                   .arg(statusCode)
                   .arg(message);
    
    sendHttpResponse(socket, statusCode, message, "text/html", html.toUtf8());
}

QString HttpServer::getMimeType(const QString& filename) const
{
    QFileInfo fileInfo(filename);
    QString extension = fileInfo.suffix().toLower();
    
    if (m_mimeTypes.contains("." + extension)) {
        return m_mimeTypes["." + extension];
    }
    
    return "application/octet-stream";
}

QString HttpServer::urlDecode(const QString& encoded) const
{
    return QUrl::fromPercentEncoding(encoded.toUtf8());
}

QString HttpServer::getClientIP(QTcpSocket* socket) const
{
    if (!socket) return "unknown";
    return socket->peerAddress().toString();
}

bool HttpServer::isStaticFile(const QString& path) const
{
    return path.contains('.') && !path.startsWith("/api/");
}

} // namespace LegacyStream 