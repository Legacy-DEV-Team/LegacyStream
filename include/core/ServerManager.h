#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QNetworkAccessManager>
#include <memory>
#include <vector>
#include <atomic>

// CertificateManager is in global namespace
class CertificateManager;

namespace LegacyStream {

class HttpServer;
class StreamManager;
class RelayManager;
class MetadataManager;
class SSLManager;
class HLSGenerator;

namespace StatisticRelay {
    class StatisticRelayManager;
}

namespace WebInterface {
    class WebInterface;
}

namespace Protocols {
    // Protocol servers not implemented yet
    // class IceCastServer;
    // class SHOUTcastServer;
}

class ServerManager : public QObject
{
    Q_OBJECT

public:
    static ServerManager& instance();
    
    bool initialize();
    void shutdown();
    
    bool isRunning() const { return m_isRunning.load(); }
    
    // Server management
    bool startServers();
    void stopServers();
    void restartServers();
    
    // Statistics
    struct ServerStats {
        quint64 totalConnections = 0;
        quint64 activeStreams = 0;
        quint64 totalBytesServed = 0;
        quint64 currentListeners = 0;
        double cpuUsage = 0.0;
        double memoryUsage = 0.0;
        qint64 uptime = 0;
    };
    
    ServerStats getStats() const;
    
    // Component access
    HttpServer* httpServer() const { return m_httpServer.get(); }
    StreamManager* streamManager() const { return m_streamManager.get(); }
    RelayManager* relayManager() const { return m_relayManager.get(); }
    MetadataManager* metadataManager() const { return m_metadataManager.get(); }
    SSLManager* sslManager() const { return m_sslManager.get(); }
    HLSGenerator* hlsGenerator() const { return m_hlsGenerator.get(); }
    WebInterface::WebInterface* webInterface() const { return m_webInterface.get(); }
    StatisticRelay::StatisticRelayManager* statisticRelayManager() const { return m_statisticRelayManager.get(); }
    
signals:
    void serverStarted();
    void serverStopped();
    void serverError(const QString& error);
    void statsUpdated(const ServerStats& stats);
    void streamConnected(const QString& mountPoint);
    void streamDisconnected(const QString& mountPoint);
    void listenerConnected(const QString& mountPoint, const QString& clientIP);
    void listenerDisconnected(const QString& mountPoint, const QString& clientIP);

private slots:
    void updateStats();
    void handleServerError(const QString& error);

private:
    ServerManager();
    ~ServerManager();
    
    void initializeComponents();
    void setupStatisticsTimer();
    
    std::atomic<bool> m_isRunning{false};
    std::atomic<bool> m_initialized{false};
    
    // Core components
    std::unique_ptr<HttpServer> m_httpServer;
    std::unique_ptr<StreamManager> m_streamManager;
    std::unique_ptr<RelayManager> m_relayManager;
    std::unique_ptr<MetadataManager> m_metadataManager;
    std::unique_ptr<SSLManager> m_sslManager;
    std::unique_ptr<HLSGenerator> m_hlsGenerator;
    std::unique_ptr<WebInterface::WebInterface> m_webInterface;
    std::unique_ptr<StatisticRelay::StatisticRelayManager> m_statisticRelayManager;
    
    // Protocol servers (not implemented yet)
    // std::unique_ptr<Protocols::IceCastServer> m_iceCastServer;
    // std::unique_ptr<Protocols::SHOUTcastServer> m_shoutCastServer;
    
    // SSL certificate management
    std::unique_ptr<CertificateManager> m_certificateManager;
    
    // Statistics
    QTimer* m_statsTimer;
    ServerStats m_currentStats;
    qint64 m_startTime;
    
    // Network
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    
    Q_DISABLE_COPY(ServerManager)
};

} // namespace LegacyStream