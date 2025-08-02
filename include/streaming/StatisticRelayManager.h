#pragma once

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QString>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <memory>

namespace LegacyStream {

class StreamManager;

namespace StatisticRelay {

struct ShoutcastRelayConfig {
    QString host;
    int port = 8000;
    QString password;
    QString sid;  // For Shoutcast v2
    bool enabled = false;
    int updateInterval = 30; // seconds
    QString mountPoint;
};

struct IcecastRelayConfig {
    QString host;
    int port = 8000;
    QString username;
    QString password;
    QString alias;  // e.g., "/live"
    bool enabled = false;
    int updateInterval = 30; // seconds
    QString mountPoint;
};

struct RelayStatistics {
    QString mountPoint;
    QString protocol;
    int currentListeners = 0;
    int peakListeners = 0;
    qint64 bytesServed = 0;
    qint64 uptime = 0;
    QString currentSong;
    QString currentArtist;
    QString currentAlbum;
    QString currentGenre;
    QString codec;
    QString bitrate;
    QString sampleRate;
    QString channels;
    bool isLive = false;
    QDateTime lastUpdate;
};

/**
 * @brief WebSocket client information (disabled - not available in current Qt installation)
 */
struct WebSocketClient {
    // QWebSocket* socket;  // Disabled
    QString id;
    QDateTime connectedAt;
    QStringList subscribedTopics;
};

class StatisticRelayManager : public QObject
{
    Q_OBJECT

public:
    explicit StatisticRelayManager(QObject* parent = nullptr);
    ~StatisticRelayManager();

    bool initialize(StreamManager* streamManager);
    void shutdown();

    // Configuration management
    void addShoutcastRelay(const QString& name, const ShoutcastRelayConfig& config);
    void addIcecastRelay(const QString& name, const IcecastRelayConfig& config);
    void removeRelay(const QString& name);
    void updateRelayConfig(const QString& name, const ShoutcastRelayConfig& config);
    void updateRelayConfig(const QString& name, const IcecastRelayConfig& config);
    void enableRelay(const QString& name, bool enabled);

    // Statistics
    QMap<QString, RelayStatistics> getRelayStatistics() const;
    QJsonObject getRelayStatisticsJson() const;

    // Real-time statistics collection
    void enableRealTimeCollection(bool enabled);
    void setRealTimeUpdateInterval(int seconds);
    QJsonObject getRealTimeStatistics() const;
    void broadcastStatistics(const QJsonObject& statistics);

    // WebSocket management
    void startWebSocketServer(int port = 8080);
    void stopWebSocketServer();
    bool isWebSocketServerRunning() const;
    int getWebSocketClientCount() const;
    void broadcastToWebSocketClients(const QJsonObject& data, const QString& topic = QString());

    // Control
    void start();
    void stop();
    bool isRunning() const { return m_isRunning; }

signals:
    void relayConnected(const QString& name, const QString& type);
    void relayDisconnected(const QString& name, const QString& type);
    void relayError(const QString& name, const QString& error);
    void statisticsRelayed(const QString& name, const QString& type);
    void relayStatusChanged(const QString& name, bool connected);
    void realTimeStatisticsUpdated(const QJsonObject& statistics);
    void webSocketClientConnected(const QString& clientId);
    void webSocketClientDisconnected(const QString& clientId);

private slots:
    void updateStatistics();
    void onShoutcastRelayFinished();
    void onIcecastRelayFinished();
    void onNetworkError(QNetworkReply::NetworkError error);
    void onRealTimeCollectionTimer();
    void onWebSocketNewConnection();
    void onWebSocketClientDisconnected();
    void onWebSocketTextMessageReceived(const QString& message);

private:
    // Relay management
    void processShoutcastRelay(const QString& name, const ShoutcastRelayConfig& config);
    void processIcecastRelay(const QString& name, const IcecastRelayConfig& config);
    void sendShoutcastStatistics(const QString& name, const ShoutcastRelayConfig& config, const RelayStatistics& stats);
    void sendIcecastStatistics(const QString& name, const IcecastRelayConfig& config, const RelayStatistics& stats);

    // Statistics collection
    RelayStatistics collectMountPointStatistics(const QString& mountPoint) const;
    QString formatShoutcastRequest(const ShoutcastRelayConfig& config, const RelayStatistics& stats) const;
    QString formatIcecastRequest(const IcecastRelayConfig& config, const RelayStatistics& stats) const;

    // Real-time statistics
    void collectRealTimeStatistics();
    QJsonObject buildRealTimeStatisticsJson() const;
    void processWebSocketMessage(const QJsonObject& message, WebSocketClient* client);

    // Utility methods
    QString buildShoutcastUrl(const ShoutcastRelayConfig& config) const;
    QString buildIcecastUrl(const IcecastRelayConfig& config) const;
    QString escapeUrl(const QString& text) const;
    QString formatBytes(qint64 bytes) const;
    QString formatDuration(qint64 seconds) const;
    QString generateClientId() const;

    StreamManager* m_streamManager = nullptr;
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    QTimer* m_updateTimer;
    QTimer* m_realTimeCollectionTimer;

    // WebSocket server
    std::unique_ptr<QWebSocketServer> m_webSocketServer;
    QMap<QWebSocket*, WebSocketClient> m_webSocketClients;
    int m_webSocketPort = 8080;

    // Real-time statistics
    bool m_realTimeCollectionEnabled = false;
    int m_realTimeUpdateInterval = 5; // seconds
    QJsonObject m_realTimeStatistics;

    // Relay configurations
    QMap<QString, ShoutcastRelayConfig> m_shoutcastRelays;
    QMap<QString, IcecastRelayConfig> m_icecastRelays;
    
    // Relay statistics
    QMap<QString, RelayStatistics> m_relayStatistics;
    QMap<QString, bool> m_relayConnectionStatus;
    
    // Network replies tracking
    QMap<QNetworkReply*, QString> m_activeReplies;
    
    // State
    bool m_isRunning = false;
    int m_updateInterval = 30; // seconds
    int m_maxRetries = 3;
    int m_retryDelay = 5; // seconds

    Q_DISABLE_COPY(StatisticRelayManager)
};

} // namespace StatisticRelay
} // namespace LegacyStream 