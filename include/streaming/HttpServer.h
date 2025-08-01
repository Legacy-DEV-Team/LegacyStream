#pragma once

#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QThreadPool>
#include <QMutex>
#include <QHash>
#include <QTimer>
#include <atomic>
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#endif

namespace LegacyStream {

class HttpConnection;
class StreamManager;
class SSLManager;

namespace Protocols {
    class IceCastHandler;
    class SHOUTcastHandler;
    class HLSHandler;
}

class HttpServer : public QObject
{
    Q_OBJECT

public:
    explicit HttpServer(QObject* parent = nullptr);
    ~HttpServer();

    enum class ConnectionType {
        HTTP,
        HTTPS,
        IceCast,
        SHOUTcast,
        HLS
    };

    struct ServerStats {
        std::atomic<quint64> totalConnections{0};
        std::atomic<quint64> activeConnections{0};
        std::atomic<quint64> bytesServed{0};
        std::atomic<quint64> requestsPerSecond{0};
        std::atomic<double> averageResponseTime{0.0};
        std::atomic<int> errorCount{0};
    };

    // Server management
    bool startServer(quint16 httpPort, quint16 httpsPort = 0);
    void stopServer();
    bool isRunning() const { return m_isRunning.load(); }
    
    // Statistics
    ServerStats getStats() const { return m_stats; }
    void resetStats();
    
    // Configuration
    void setMaxConnections(int maxConnections) { m_maxConnections = maxConnections; }
    int maxConnections() const { return m_maxConnections; }
    
    void setConnectionTimeout(int timeout) { m_connectionTimeout = timeout; }
    int connectionTimeout() const { return m_connectionTimeout; }
    
    void setKeepAliveTimeout(int timeout) { m_keepAliveTimeout = timeout; }
    int keepAliveTimeout() const { return m_keepAliveTimeout; }
    
    // SSL support
    void setSSLManager(SSLManager* sslManager) { m_sslManager = sslManager; }
    bool isSSLEnabled() const;
    
    // Protocol handlers
    void setStreamManager(StreamManager* streamManager) { m_streamManager = streamManager; }

signals:
    void connectionAccepted(const QString& clientIP, ConnectionType type);
    void connectionClosed(const QString& clientIP, ConnectionType type);
    void requestReceived(const QString& method, const QString& path, const QString& clientIP);
    void errorOccurred(const QString& error);
    void statsUpdated(const ServerStats& stats);

private slots:
    void onNewHttpConnection();
    void onNewHttpsConnection();
    void onConnectionClosed();
    void updateStats();
    void cleanupConnections();

private:
    // Server initialization
    bool setupHttpServer(quint16 port);
    bool setupHttpsServer(quint16 port);
    bool setupIOCP();
    void setupThreadPool();
    
    // Connection management
    void handleNewConnection(qintptr socketDescriptor, ConnectionType type);
    bool acceptConnection(HttpConnection* connection);
    void removeConnection(HttpConnection* connection);
    
    // Windows IOCP implementation
#ifdef _WIN32
    bool initializeIOCP();
    void shutdownIOCP();
    static DWORD WINAPI iocpWorkerThread(LPVOID param);
    void processIOCPEvent(DWORD bytesTransferred, ULONG_PTR completionKey, LPOVERLAPPED overlapped);
    
    HANDLE m_iocpHandle = INVALID_HANDLE_VALUE;
    std::vector<HANDLE> m_workerThreads;
    std::atomic<bool> m_iocpShutdown{false};
    static constexpr int IOCP_WORKER_THREADS = 8;
#endif
    
    // Server sockets
    std::unique_ptr<QTcpServer> m_httpServer;
    std::unique_ptr<QTcpServer> m_httpsServer;
    
    // Thread management
    std::unique_ptr<QThreadPool> m_threadPool;
    
    // Connection tracking
    QHash<qintptr, std::unique_ptr<HttpConnection>> m_connections;
    QMutex m_connectionsMutex;
    std::atomic<int> m_activeConnectionCount{0};
    
    // Configuration
    int m_maxConnections = 100000;
    int m_connectionTimeout = 30000; // 30 seconds
    int m_keepAliveTimeout = 60000; // 60 seconds
    quint16 m_httpPort = 0;
    quint16 m_httpsPort = 0;
    
    // Protocol handlers
    std::unique_ptr<Protocols::IceCastHandler> m_iceCastHandler;
    std::unique_ptr<Protocols::SHOUTcastHandler> m_shoutCastHandler;
    std::unique_ptr<Protocols::HLSHandler> m_hlsHandler;
    
    // Managers
    StreamManager* m_streamManager = nullptr;
    SSLManager* m_sslManager = nullptr;
    
    // Statistics
    ServerStats m_stats;
    QTimer* m_statsTimer;
    QTimer* m_cleanupTimer;
    
    // State
    std::atomic<bool> m_isRunning{false};
    std::atomic<bool> m_shutdownRequested{false};
    
    Q_DISABLE_COPY(HttpServer)
};

} // namespace LegacyStream