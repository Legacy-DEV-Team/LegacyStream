#ifndef CONNECTIONPOOLMANAGER_H
#define CONNECTIONPOOLMANAGER_H

#include <QObject>
#include <QMap>
#include <QQueue>
#include <QMutex>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <functional>

namespace LegacyStream {

/**
 * @brief Connection pool configuration
 */
struct ConnectionPoolConfig
{
    QString name;
    int minConnections = 5;
    int maxConnections = 20;
    int initialConnections = 10;
    int connectionTimeout = 30000; // 30 seconds
    int idleTimeout = 600000; // 10 minutes
    int maxLifetime = 3600000; // 1 hour
    bool enableHealthCheck = true;
    int healthCheckInterval = 60000; // 1 minute
    QString connectionString;
    QString driver;
    bool enableMetrics = true;
    bool enableLogging = true;
};

/**
 * @brief Connection pool statistics
 */
struct ConnectionPoolStats
{
    int totalConnections = 0;
    int activeConnections = 0;
    int idleConnections = 0;
    int waitingRequests = 0;
    double averageWaitTime = 0.0;
    double averageConnectionTime = 0.0;
    int connectionErrors = 0;
    int healthCheckFailures = 0;
    QDateTime lastHealthCheck;
    QMap<QString, int> connectionsByType;
    QMap<QString, double> responseTimesByType;
};

/**
 * @brief Connection wrapper
 */
class PooledConnection
{
public:
    explicit PooledConnection(const QString& id);
    ~PooledConnection();

    // Connection management
    bool connect(const QString& connectionString);
    void disconnect();
    bool isConnected() const;
    bool isHealthy() const;

    // Connection metadata
    QString getId() const { return m_id; }
    QDateTime getCreated() const { return m_created; }
    QDateTime getLastUsed() const { return m_lastUsed; }
    int getUseCount() const { return m_useCount; }
    bool isInUse() const { return m_inUse; }

    // Connection operations
    void markAsUsed();
    void markAsIdle();
    void updateHealth();
    bool isExpired(int maxLifetime) const;

private:
    QString m_id;
    QDateTime m_created;
    QDateTime m_lastUsed;
    QDateTime m_lastHealthCheck;
    int m_useCount = 0;
    bool m_inUse = false;
    bool m_connected = false;
    bool m_healthy = true;
    void* m_nativeConnection = nullptr; // Platform-specific connection handle
};

/**
 * @brief Connection pool manager for database and network connection pooling
 * 
 * Provides efficient connection pooling with health monitoring, load balancing,
 * and automatic connection lifecycle management.
 */
class ConnectionPoolManager : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionPoolManager(QObject* parent = nullptr);
    ~ConnectionPoolManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Pool management
    bool createPool(const QString& name, const ConnectionPoolConfig& config);
    void destroyPool(const QString& name);
    bool poolExists(const QString& name) const;
    QStringList getPoolNames() const;

    // Connection operations
    std::shared_ptr<PooledConnection> getConnection(const QString& poolName);
    void returnConnection(const QString& poolName, std::shared_ptr<PooledConnection> connection);
    void closeConnection(const QString& poolName, std::shared_ptr<PooledConnection> connection);

    // Pool configuration
    void setPoolConfig(const QString& name, const ConnectionPoolConfig& config);
    ConnectionPoolConfig getPoolConfig(const QString& name) const;
    void setMaxConnections(const QString& name, int maxConnections);
    void setConnectionTimeout(const QString& name, int timeout);

    // Health monitoring
    void enableHealthCheck(const QString& name, bool enabled);
    void setHealthCheckInterval(const QString& name, int interval);
    void performHealthCheck(const QString& name);
    void performHealthCheckAll();

    // Statistics and monitoring
    ConnectionPoolStats getPoolStats(const QString& name) const;
    QJsonObject getAllPoolStatsJson() const;
    void resetPoolStats(const QString& name);
    void exportPoolStats(const QString& filePath) const;

    // Advanced features
    void enableMetrics(const QString& name, bool enabled);
    void enableLogging(const QString& name, bool enabled);
    void setLoadBalancing(const QString& name, const QString& strategy);
    void setConnectionValidation(const QString& name, const QString& query);

    // Utility functions
    int getActiveConnections(const QString& name) const;
    int getIdleConnections(const QString& name) const;
    int getTotalConnections(const QString& name) const;
    double getPoolUtilization(const QString& name) const;
    bool isPoolHealthy(const QString& name) const;

signals:
    void connectionAcquired(const QString& poolName, const QString& connectionId);
    void connectionReleased(const QString& poolName, const QString& connectionId);
    void connectionCreated(const QString& poolName, const QString& connectionId);
    void connectionDestroyed(const QString& poolName, const QString& connectionId);
    void connectionError(const QString& poolName, const QString& error);
    void healthCheckFailed(const QString& poolName, const QString& connectionId);
    void poolExhausted(const QString& poolName);
    void statisticsUpdated(const QString& poolName, const ConnectionPoolStats& stats);

public slots:
    void onHealthCheckTimer();
    void onCleanupTimer();
    void onStatisticsTimer();

private slots:
    void onConnectionTimeout();
    void onIdleTimeout();
    void onLifetimeExpired();

private:
    // Pool management
    struct ConnectionPool
    {
        ConnectionPoolConfig config;
        ConnectionPoolStats stats;
        QQueue<std::shared_ptr<PooledConnection>> idleConnections;
        QMap<QString, std::shared_ptr<PooledConnection>> activeConnections;
        QQueue<std::function<void(std::shared_ptr<PooledConnection>)>> waitingRequests;
        QMutex mutex;
        QTimer* healthCheckTimer = nullptr;
        QTimer* cleanupTimer = nullptr;
        bool isHealthy = true;
    };

    // Core pool operations
    std::shared_ptr<PooledConnection> createConnection(const QString& poolName);
    void destroyConnection(const QString& poolName, std::shared_ptr<PooledConnection> connection);
    void addToIdlePool(const QString& poolName, std::shared_ptr<PooledConnection> connection);
    std::shared_ptr<PooledConnection> getFromIdlePool(const QString& poolName);
    void processWaitingRequests(const QString& poolName);

    // Health monitoring
    void performHealthCheck(ConnectionPool& pool);
    void cleanupExpiredConnections(ConnectionPool& pool);
    void validateConnection(std::shared_ptr<PooledConnection> connection);

    // Statistics
    void updatePoolStatistics(const QString& poolName);
    void calculatePoolMetrics(ConnectionPool& pool);
    void logPoolEvent(const QString& poolName, const QString& event);

    // Utility functions
    QString generateConnectionId() const;
    bool isConnectionValid(std::shared_ptr<PooledConnection> connection) const;
    void loadPoolFromDisk(const QString& name);
    void savePoolToDisk(const QString& name);

    // Pool storage
    QMap<QString, std::unique_ptr<ConnectionPool>> m_pools;

    // Timers
    QTimer* m_globalHealthCheckTimer = nullptr;
    QTimer* m_globalCleanupTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_healthCheckEnabled = true;
    bool m_metricsEnabled = true;
    bool m_loggingEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastHealthCheck;
    QMap<QString, QDateTime> m_lastCleanup;
    QMap<QString, double> m_averageResponseTimes;

    Q_DISABLE_COPY(ConnectionPoolManager)
};

} // namespace LegacyStream

#endif // CONNECTIONPOOLMANAGER_H 