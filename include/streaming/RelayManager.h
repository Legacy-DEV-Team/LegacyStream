#ifndef RELAYMANAGER_H
#define RELAYMANAGER_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QMutex>
#include <QAtomicInt>
#include <QJsonObject>
#include <QJsonArray>

class StreamManager;

namespace LegacyStream {

/**
 * @brief Stream relay configuration
 */
struct RelayConfig
{
    QString name;
    QString targetUrl;
    QString mountPoint;
    QString username;
    QString password;
    int bitrate = 128;
    QString codec = "mp3";
    bool enabled = true;
    int retryAttempts = 3;
    int retryDelay = 5000; // ms
    int timeout = 30000; // ms
};

/**
 * @brief Relay statistics
 */
struct RelayStats
{
    QString name;
    bool connected = false;
    qint64 bytesSent = 0;
    qint64 bytesReceived = 0;
    int errorCount = 0;
    QString lastError;
    QDateTime lastConnectTime;
    QDateTime lastDataTime;
    int retryCount = 0;
    bool active = false;
};

/**
 * @brief RelayManager for stream mirroring
 * 
 * Manages stream relay connections to external servers.
 * Supports multiple simultaneous relays with automatic reconnection.
 */
class RelayManager : public QObject
{
    Q_OBJECT

public:
    explicit RelayManager(QObject* parent = nullptr);
    ~RelayManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool start();
    void stop();

    // Configuration
    void setStreamManager(StreamManager* streamManager);
    void addRelay(const RelayConfig& config);
    void removeRelay(const QString& name);
    void updateRelay(const QString& name, const RelayConfig& config);
    void enableRelay(const QString& name, bool enabled);

    // Status and information
    bool isRunning() const;
    QList<RelayConfig> getRelayConfigs() const;
    QMap<QString, RelayStats> getRelayStats() const;
    QJsonObject getStatusJson() const;

    // Manual control
    void connectRelay(const QString& name);
    void disconnectRelay(const QString& name);
    void reconnectRelay(const QString& name);

signals:
    void relayConnected(const QString& name);
    void relayDisconnected(const QString& name);
    void relayError(const QString& name, const QString& error);
    void relayDataSent(const QString& name, qint64 bytes);
    void statusChanged(const QJsonObject& status);

private slots:
    void onStreamDataReceived(const QString& mountPoint, const QByteArray& data);
    void onRelayFinished();
    void onRelayError(QNetworkReply::NetworkError error);
    void onRetryTimer();

private:
    // Core functionality
    void processRelay(const QString& name, const RelayConfig& config);
    void sendRelayData(const QString& name, const RelayConfig& config, const QByteArray& data);
    void handleRelayResponse(const QString& name, QNetworkReply* reply);
    void scheduleRetry(const QString& name, const RelayConfig& config);

    // Utility functions
    QString buildRelayUrl(const RelayConfig& config) const;
    QNetworkRequest createRelayRequest(const RelayConfig& config) const;
    bool isValidRelayConfig(const RelayConfig& config) const;
    void updateRelayStats(const QString& name, const RelayStats& stats);

    // Configuration
    StreamManager* m_streamManager = nullptr;
    QMap<QString, RelayConfig> m_relayConfigs;
    QMap<QString, RelayStats> m_relayStats;
    QMap<QString, QNetworkReply*> m_activeRelays;

    // State management
    QAtomicInt m_isRunning = 0;
    QTimer* m_retryTimer = nullptr;
    QMutex m_mutex;

    // Network management
    QNetworkAccessManager* m_networkManager = nullptr;
    QMap<QString, QTimer*> m_retryTimers;

    // Statistics
    QJsonObject m_statistics;
    QDateTime m_startTime;
    int m_totalRelays = 0;
    int m_activeRelaysCount = 0;
    qint64 m_totalBytesSent = 0;
    int m_totalErrors = 0;

    Q_DISABLE_COPY(RelayManager)
};

} // namespace LegacyStream

#endif // RELAYMANAGER_H 