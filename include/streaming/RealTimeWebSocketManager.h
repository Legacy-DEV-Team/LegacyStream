#ifndef REALTIMEWEBSOCKETMANAGER_H
#define REALTIMEWEBSOCKETMANAGER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <functional>

namespace LegacyStream {

/**
 * @brief WebSocket client information
 */
struct WebSocketClient
{
    QString id;
    QWebSocket* socket = nullptr;
    QString sessionId;
    QString userId;
    QString userRole;
    QDateTime connectedAt;
    QDateTime lastActivity;
    QSet<QString> subscribedTopics;
    QMap<QString, QVariant> metadata;
    bool isAuthenticated = false;
    int messageCount = 0;
    int errorCount = 0;
    QString userAgent;
    QString remoteAddress;
    int port = 0;
};

/**
 * @brief WebSocket message structure
 */
struct WebSocketMessage
{
    QString type;
    QString topic;
    QString action;
    QJsonObject data;
    QJsonObject metadata;
    QDateTime timestamp;
    QString messageId;
    QString clientId;
    QString sessionId;
    int sequenceNumber = 0;
    bool requiresAck = false;
    int ttl = 0; // Time to live in seconds
};

/**
 * @brief WebSocket server configuration
 */
struct WebSocketServerConfig
{
    QString name;
    int port = 8080;
    QString host = "localhost";
    QString path = "/ws";
    int maxConnections = 1000;
    int maxMessageSize = 1024 * 1024; // 1MB
    int heartbeatInterval = 30000; // 30 seconds
    int connectionTimeout = 300000; // 5 minutes
    bool enableSSL = false;
    QString sslCertificate;
    QString sslPrivateKey;
    bool enableCompression = true;
    bool enableLogging = true;
    QString allowedOrigins = "*";
};

/**
 * @brief WebSocket server statistics
 */
struct WebSocketServerStats
{
    int totalConnections = 0;
    int activeConnections = 0;
    int totalMessages = 0;
    int messagesPerSecond = 0;
    int totalErrors = 0;
    double averageResponseTime = 0.0;
    QDateTime lastMessage;
    QDateTime lastError;
    QMap<QString, int> messagesByType;
    QMap<QString, int> connectionsByOrigin;
    QMap<QString, double> responseTimesByClient;
};

/**
 * @brief Real-time WebSocket manager for advanced real-time communication
 * 
 * Provides sophisticated WebSocket management with authentication, topic-based
 * subscriptions, message queuing, and real-time data distribution.
 */
class RealTimeWebSocketManager : public QObject
{
    Q_OBJECT

public:
    explicit RealTimeWebSocketManager(QObject* parent = nullptr);
    ~RealTimeWebSocketManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Server management
    bool startServer(const WebSocketServerConfig& config);
    void stopServer();
    bool isServerRunning() const;
    WebSocketServerConfig getServerConfig() const;
    void setServerConfig(const WebSocketServerConfig& config);

    // Client management
    QList<WebSocketClient> getConnectedClients() const;
    WebSocketClient* getClient(const QString& clientId);
    void disconnectClient(const QString& clientId);
    void disconnectAllClients();
    int getClientCount() const;

    // Message broadcasting
    void broadcastMessage(const QJsonObject& data, const QString& topic = QString());
    void sendToClient(const QString& clientId, const QJsonObject& data);
    void sendToClients(const QStringList& clientIds, const QJsonObject& data);
    void sendToTopic(const QString& topic, const QJsonObject& data);
    void sendToRole(const QString& role, const QJsonObject& data);

    // Topic management
    void subscribeClientToTopic(const QString& clientId, const QString& topic);
    void unsubscribeClientFromTopic(const QString& clientId, const QString& topic);
    QStringList getClientTopics(const QString& clientId) const;
    QStringList getTopicSubscribers(const QString& topic) const;

    // Authentication and security
    void authenticateClient(const QString& clientId, const QString& userId, const QString& role);
    void deauthenticateClient(const QString& clientId);
    bool isClientAuthenticated(const QString& clientId) const;
    void setClientMetadata(const QString& clientId, const QMap<QString, QVariant>& metadata);

    // Statistics and monitoring
    WebSocketServerStats getServerStats() const;
    QJsonObject getServerStatsJson() const;
    void resetServerStats();
    void exportServerStats(const QString& filePath) const;

    // Advanced features
    void enableHeartbeat(bool enabled);
    void setHeartbeatInterval(int interval);
    void enableCompression(bool enabled);
    void setMaxMessageSize(int size);
    void setConnectionTimeout(int timeout);

    // Message queuing
    void queueMessage(const WebSocketMessage& message);
    void processMessageQueue();
    void clearMessageQueue();

    // Utility functions
    QString generateClientId() const;
    QString generateMessageId() const;
    bool isValidTopic(const QString& topic) const;
    bool isValidClientId(const QString& clientId) const;

signals:
    void clientConnected(const QString& clientId);
    void clientDisconnected(const QString& clientId);
    void clientAuthenticated(const QString& clientId, const QString& userId);
    void messageReceived(const QString& clientId, const QJsonObject& message);
    void messageSent(const QString& clientId, const QJsonObject& message);
    void errorOccurred(const QString& error);
    void statisticsUpdated(const WebSocketServerStats& stats);

public slots:
    void onHeartbeatTimer();
    void onCleanupTimer();
    void onStatisticsTimer();

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& message);
    void onError(QAbstractSocket::SocketError error);

private:
    // WebSocket server management
    struct WebSocketServer
    {
        WebSocketServerConfig config;
        WebSocketServerStats stats;
        QWebSocketServer* server = nullptr;
        QMap<QWebSocket*, WebSocketClient> clients;
        QMap<QString, QSet<QString>> topicSubscribers; // topic -> set of client IDs
        QMap<QString, QList<WebSocketMessage>> messageQueue;
        QMutex mutex;
        QTimer* heartbeatTimer = nullptr;
        QTimer* cleanupTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        bool isRunning = false;
    };

    // Core server operations
    void handleNewConnection(QWebSocket* socket);
    void handleClientDisconnection(QWebSocket* socket);
    void processClientMessage(QWebSocket* socket, const QString& message);
    void processBinaryMessage(QWebSocket* socket, const QByteArray& message);

    // Message handling
    void handleTextMessage(const QString& clientId, const QJsonObject& message);
    void handleBinaryMessage(const QString& clientId, const QByteArray& message);
    void handleAuthentication(const QString& clientId, const QJsonObject& data);
    void handleSubscription(const QString& clientId, const QJsonObject& data);
    void handleHeartbeat(const QString& clientId);

    // Client management
    void addClient(QWebSocket* socket);
    void removeClient(QWebSocket* socket);
    void updateClientActivity(const QString& clientId);
    void cleanupInactiveClients();

    // Message broadcasting
    void broadcastToClients(const QJsonObject& data, const QStringList& clientIds);
    void broadcastToTopic(const QJsonObject& data, const QString& topic);
    void queueMessageForClient(const QString& clientId, const WebSocketMessage& message);

    // Statistics
    void updateServerStatistics();
    void calculateServerMetrics(WebSocketServer& server);
    void logServerEvent(const QString& event);

    // Utility functions
    QJsonObject createMessageResponse(const QString& type, const QJsonObject& data);
    QJsonObject createErrorResponse(const QString& error, const QString& details);
    bool validateMessage(const QJsonObject& message);
    void logMessage(const QString& clientId, const QJsonObject& message);

    // Server storage
    std::unique_ptr<WebSocketServer> m_server;

    // Timers
    QTimer* m_globalHeartbeatTimer = nullptr;
    QTimer* m_globalCleanupTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_heartbeatEnabled = true;
    bool m_compressionEnabled = true;
    bool m_loggingEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastActivity;
    QMap<QString, QDateTime> m_lastHeartbeat;
    QMap<QString, double> m_responseTimes;

    Q_DISABLE_COPY(RealTimeWebSocketManager)
};

} // namespace LegacyStream

#endif // REALTIMEWEBSOCKETMANAGER_H 