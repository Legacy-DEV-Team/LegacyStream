#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <memory>

namespace LegacyStream {

class StreamManager;

namespace WebInterface {
    class WebInterface;
}

class HttpServer : public QObject
{
    Q_OBJECT

public:
    explicit HttpServer(QObject* parent = nullptr);
    ~HttpServer();

    // Initialization
    bool initialize(int port = 8080, const QString& host = "0.0.0.0");
    void shutdown();

    // Configuration
    void setPort(int port);
    void setHost(const QString& host);
    void setWebInterface(WebInterface::WebInterface* webInterface);
    void setStreamManager(StreamManager* streamManager);

    // Server control
    bool isRunning() const;
    int getPort() const;
    QString getHost() const;

signals:
    void clientConnected(const QString& clientIP);
    void clientDisconnected(const QString& clientIP);
    void requestReceived(const QString& method, const QString& path, const QString& clientIP);
    void errorOccurred(const QString& error);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientReadyRead();

private:
    // HTTP request handling
    void handleHttpRequest(QTcpSocket* socket, const QString& request);
    void sendHttpResponse(QTcpSocket* socket, int statusCode, const QString& statusText,
                         const QString& contentType, const QByteArray& body);
    void sendErrorResponse(QTcpSocket* socket, int statusCode, const QString& message);
    
    // Request parsing
    bool parseHttpRequest(const QString& request, QString& method, QString& path,
                         QMap<QString, QString>& headers, QString& body);
    QString extractPath(const QString& requestLine);
    QMap<QString, QString> extractHeaders(const QStringList& lines);
    
    // Route handling
    void handleRoute(QTcpSocket* socket, const QString& method, const QString& path,
                    const QMap<QString, QString>& headers, const QString& body);
    void handleStaticFile(QTcpSocket* socket, const QString& path);
    void handleApiRequest(QTcpSocket* socket, const QString& method, const QString& path,
                         const QMap<QString, QString>& headers, const QString& body);
    void handleWebInterfaceRequest(QTcpSocket* socket, const QString& method, const QString& path,
                                 const QMap<QString, QString>& headers, const QString& body);

    // Utility methods
    QString getMimeType(const QString& filename) const;
    QString urlDecode(const QString& encoded) const;
    QString getClientIP(QTcpSocket* socket) const;
    bool isStaticFile(const QString& path) const;

    // Server components
    QTcpServer* m_tcpServer = nullptr;
    QList<QTcpSocket*> m_clients;
    
    // Configuration
    int m_port = 8080;
    QString m_host = "0.0.0.0";
    bool m_isRunning = false;
    
    // Component references
    WebInterface::WebInterface* m_webInterface = nullptr;
    StreamManager* m_streamManager = nullptr;
    
    // Static file handling
    QString m_staticFilesPath = "static";
    QMap<QString, QString> m_mimeTypes;
    
    // Request statistics
    qint64 m_totalRequests = 0;
    qint64 m_totalBytesServed = 0;
    QMap<QString, int> m_requestCounts;

    Q_DISABLE_COPY(HttpServer)
};

} // namespace LegacyStream 