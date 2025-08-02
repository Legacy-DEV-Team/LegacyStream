#ifndef LOADBALANCERMANAGER_H
#define LOADBALANCERMANAGER_H

#include <QObject>
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
 * @brief Server node information
 */
struct ServerNode
{
    QString id;
    QString name;
    QString address;
    int port;
    bool isActive = true;
    bool isHealthy = true;
    double weight = 1.0;
    int maxConnections = 1000;
    int currentConnections = 0;
    double responseTime = 0.0;
    double cpuUsage = 0.0;
    double memoryUsage = 0.0;
    QDateTime lastHealthCheck;
    QDateTime lastResponse;
    int failureCount = 0;
    int successCount = 0;
    double successRate = 1.0;
};

/**
 * @brief Load balancing configuration
 */
struct LoadBalancerConfig
{
    QString name;
    QString algorithm = "round_robin"; // round_robin, least_connections, weighted, ip_hash, least_response_time
    int healthCheckInterval = 30000; // 30 seconds
    int healthCheckTimeout = 5000; // 5 seconds
    bool enableStickySessions = false;
    int stickySessionTimeout = 3600000; // 1 hour
    bool enableFailover = true;
    int maxFailures = 3;
    int failoverTimeout = 60000; // 1 minute
    bool enableMetrics = true;
    bool enableLogging = true;
};

/**
 * @brief Load balancer statistics
 */
struct LoadBalancerStats
{
    int totalRequests = 0;
    int successfulRequests = 0;
    int failedRequests = 0;
    double averageResponseTime = 0.0;
    int activeServers = 0;
    int totalServers = 0;
    QMap<QString, int> requestsByServer;
    QMap<QString, double> responseTimesByServer;
    QMap<QString, double> successRatesByServer;
    QDateTime lastRequest;
    QDateTime lastHealthCheck;
};

/**
 * @brief Load balancing algorithm interface
 */
class LoadBalancingAlgorithm
{
public:
    virtual ~LoadBalancingAlgorithm() = default;
    virtual QString selectServer(const QList<ServerNode>& servers, const QString& clientId = QString()) = 0;
    virtual void updateServerStats(const QString& serverId, double responseTime, bool success) = 0;
    virtual void reset() = 0;
};

/**
 * @brief Round-robin load balancing algorithm
 */
class RoundRobinAlgorithm : public LoadBalancingAlgorithm
{
public:
    QString selectServer(const QList<ServerNode>& servers, const QString& clientId = QString()) override;
    void updateServerStats(const QString& serverId, double responseTime, bool success) override;
    void reset() override;

private:
    int m_currentIndex = 0;
    QMutex m_mutex;
};

/**
 * @brief Least connections load balancing algorithm
 */
class LeastConnectionsAlgorithm : public LoadBalancingAlgorithm
{
public:
    QString selectServer(const QList<ServerNode>& servers, const QString& clientId = QString()) override;
    void updateServerStats(const QString& serverId, double responseTime, bool success) override;
    void reset() override;
};

/**
 * @brief Weighted load balancing algorithm
 */
class WeightedAlgorithm : public LoadBalancingAlgorithm
{
public:
    QString selectServer(const QList<ServerNode>& servers, const QString& clientId = QString()) override;
    void updateServerStats(const QString& serverId, double responseTime, bool success) override;
    void reset() override;
};

/**
 * @brief IP hash load balancing algorithm
 */
class IPHashAlgorithm : public LoadBalancingAlgorithm
{
public:
    QString selectServer(const QList<ServerNode>& servers, const QString& clientId = QString()) override;
    void updateServerStats(const QString& serverId, double responseTime, bool success) override;
    void reset() override;
};

/**
 * @brief Least response time load balancing algorithm
 */
class LeastResponseTimeAlgorithm : public LoadBalancingAlgorithm
{
public:
    QString selectServer(const QList<ServerNode>& servers, const QString& clientId = QString()) override;
    void updateServerStats(const QString& serverId, double responseTime, bool success) override;
    void reset() override;
};

/**
 * @brief Load balancer manager for multi-server load distribution
 * 
 * Provides sophisticated load balancing with multiple algorithms, health monitoring,
 * failover capabilities, and performance optimization.
 */
class LoadBalancerManager : public QObject
{
    Q_OBJECT

public:
    explicit LoadBalancerManager(QObject* parent = nullptr);
    ~LoadBalancerManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Load balancer management
    bool createLoadBalancer(const QString& name, const LoadBalancerConfig& config);
    void destroyLoadBalancer(const QString& name);
    bool loadBalancerExists(const QString& name) const;
    QStringList getLoadBalancerNames() const;

    // Server management
    void addServer(const QString& lbName, const ServerNode& server);
    void removeServer(const QString& lbName, const QString& serverId);
    void updateServer(const QString& lbName, const ServerNode& server);
    void enableServer(const QString& lbName, const QString& serverId, bool enabled);
    QList<ServerNode> getServers(const QString& lbName) const;

    // Load balancing operations
    QString selectServer(const QString& lbName, const QString& clientId = QString());
    void reportServerResponse(const QString& lbName, const QString& serverId, double responseTime, bool success);
    void reportServerFailure(const QString& lbName, const QString& serverId);

    // Configuration
    void setLoadBalancerConfig(const QString& name, const LoadBalancerConfig& config);
    LoadBalancerConfig getLoadBalancerConfig(const QString& name) const;
    void setAlgorithm(const QString& name, const QString& algorithm);
    void setHealthCheckInterval(const QString& name, int interval);

    // Health monitoring
    void enableHealthCheck(const QString& name, bool enabled);
    void setHealthCheckTimeout(const QString& name, int timeout);
    void performHealthCheck(const QString& name);
    void performHealthCheckAll();

    // Statistics and monitoring
    LoadBalancerStats getLoadBalancerStats(const QString& name) const;
    QJsonObject getAllLoadBalancerStatsJson() const;
    void resetLoadBalancerStats(const QString& name);
    void exportLoadBalancerStats(const QString& filePath) const;

    // Advanced features
    void enableStickySessions(const QString& name, bool enabled);
    void setStickySessionTimeout(const QString& name, int timeout);
    void enableFailover(const QString& name, bool enabled);
    void setMaxFailures(const QString& name, int maxFailures);

    // Utility functions
    int getActiveServers(const QString& name) const;
    int getTotalServers(const QString& name) const;
    double getLoadBalancerUtilization(const QString& name) const;
    bool isLoadBalancerHealthy(const QString& name) const;

signals:
    void serverSelected(const QString& lbName, const QString& serverId);
    void serverAdded(const QString& lbName, const QString& serverId);
    void serverRemoved(const QString& lbName, const QString& serverId);
    void serverFailed(const QString& lbName, const QString& serverId);
    void serverRecovered(const QString& lbName, const QString& serverId);
    void healthCheckFailed(const QString& lbName, const QString& serverId);
    void statisticsUpdated(const QString& lbName, const LoadBalancerStats& stats);

public slots:
    void onHealthCheckTimer();
    void onStatisticsTimer();
    void onFailoverTimer();

private slots:
    void onServerTimeout();
    void onStickySessionExpired();

private:
    // Load balancer management
    struct LoadBalancer
    {
        LoadBalancerConfig config;
        LoadBalancerStats stats;
        QList<ServerNode> servers;
        QMap<QString, QString> stickySessions; // clientId -> serverId
        QMap<QString, QDateTime> sessionTimestamps;
        std::unique_ptr<LoadBalancingAlgorithm> algorithm;
        QMutex mutex;
        QTimer* healthCheckTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        QTimer* failoverTimer = nullptr;
        bool isHealthy = true;
    };

    // Core load balancer operations
    void createAlgorithm(LoadBalancer& lb, const QString& algorithm);
    void updateServerHealth(LoadBalancer& lb, const QString& serverId, bool isHealthy);
    void handleServerFailure(LoadBalancer& lb, const QString& serverId);
    void cleanupStickySessions(LoadBalancer& lb);

    // Health monitoring
    void performHealthCheck(LoadBalancer& lb);
    void checkServerHealth(ServerNode& server);
    void validateServerResponse(const QString& serverId, double responseTime);

    // Statistics
    void updateLoadBalancerStatistics(const QString& name);
    void calculateLoadBalancerMetrics(LoadBalancer& lb);
    void logLoadBalancerEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateServerId() const;
    bool isServerValid(const ServerNode& server) const;
    void loadLoadBalancerFromDisk(const QString& name);
    void saveLoadBalancerToDisk(const QString& name);

    // Load balancer storage
    QMap<QString, std::unique_ptr<LoadBalancer>> m_loadBalancers;

    // Timers
    QTimer* m_globalHealthCheckTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;
    QTimer* m_globalFailoverTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_healthCheckEnabled = true;
    bool m_metricsEnabled = true;
    bool m_loggingEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastHealthCheck;
    QMap<QString, QDateTime> m_lastStatisticsUpdate;
    QMap<QString, double> m_averageResponseTimes;

    Q_DISABLE_COPY(LoadBalancerManager)
};

} // namespace LegacyStream

#endif // LOADBALANCERMANAGER_H 