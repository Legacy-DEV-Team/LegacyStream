#ifndef DYNAMICLOADBALANCER_H
#define DYNAMICLOADBALANCER_H

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
 * @brief Server load information
 */
struct ServerLoadInfo
{
    QString serverId;
    QString serverName;
    QString address;
    int port;
    double cpuUsage = 0.0;
    double memoryUsage = 0.0;
    double networkUsage = 0.0;
    int activeConnections = 0;
    int maxConnections = 1000;
    double responseTime = 0.0;
    double throughput = 0.0;
    double errorRate = 0.0;
    bool isHealthy = true;
    bool isAvailable = true;
    QDateTime lastUpdate;
    QDateTime lastHealthCheck;
    int priority = 1;
    double weight = 1.0;
};

/**
 * @brief Load balancing strategy
 */
struct LoadBalancingStrategy
{
    QString name;
    QString algorithm = "weighted_round_robin"; // weighted_round_robin, least_connections, least_response_time, adaptive
    bool enableHealthCheck = true;
    bool enableFailover = true;
    bool enableStickySessions = false;
    int healthCheckInterval = 30000; // 30 seconds
    int failoverTimeout = 60000; // 1 minute
    int stickySessionTimeout = 3600000; // 1 hour
    double cpuThreshold = 80.0;
    double memoryThreshold = 85.0;
    double responseTimeThreshold = 1000.0; // 1 second
    double errorRateThreshold = 0.05; // 5%
    bool enableMetrics = true;
    bool enableLogging = true;
};

/**
 * @brief Load balancing decision
 */
struct LoadBalancingDecision
{
    QString selectedServerId;
    QString reason;
    double confidence = 0.0;
    QDateTime timestamp;
    QJsonObject context;
    QStringList alternativeServers;
    double expectedResponseTime = 0.0;
    double expectedThroughput = 0.0;
};

/**
 * @brief Dynamic load balancer statistics
 */
struct DynamicLoadBalancerStats
{
    int totalRequests = 0;
    int successfulRequests = 0;
    int failedRequests = 0;
    double averageResponseTime = 0.0;
    double averageThroughput = 0.0;
    int activeServers = 0;
    int totalServers = 0;
    QDateTime lastRequest;
    QDateTime lastHealthCheck;
    QMap<QString, int> requestsByServer;
    QMap<QString, double> responseTimesByServer;
    QMap<QString, double> throughputByServer;
    QMap<QString, double> errorRatesByServer;
};

/**
 * @brief Dynamic load balancer for runtime load distribution
 * 
 * Provides intelligent runtime load balancing with adaptive algorithms,
 * health monitoring, failover capabilities, and performance optimization.
 */
class DynamicLoadBalancer : public QObject
{
    Q_OBJECT

public:
    explicit DynamicLoadBalancer(QObject* parent = nullptr);
    ~DynamicLoadBalancer();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Load balancer management
    bool createLoadBalancer(const QString& name, const LoadBalancingStrategy& strategy);
    void destroyLoadBalancer(const QString& name);
    bool loadBalancerExists(const QString& name) const;
    QStringList getLoadBalancerNames() const;

    // Server management
    void addServer(const QString& lbName, const ServerLoadInfo& server);
    void removeServer(const QString& lbName, const QString& serverId);
    void updateServerLoad(const QString& lbName, const ServerLoadInfo& server);
    void enableServer(const QString& lbName, const QString& serverId, bool enabled);
    QList<ServerLoadInfo> getServers(const QString& lbName) const;

    // Load balancing operations
    LoadBalancingDecision selectServer(const QString& lbName, const QString& clientId = QString());
    void reportServerResponse(const QString& lbName, const QString& serverId, double responseTime, bool success);
    void reportServerFailure(const QString& lbName, const QString& serverId);

    // Configuration
    void setLoadBalancingStrategy(const QString& name, const LoadBalancingStrategy& strategy);
    LoadBalancingStrategy getLoadBalancingStrategy(const QString& name) const;
    void setAlgorithm(const QString& name, const QString& algorithm);
    void setHealthCheckInterval(const QString& name, int interval);

    // Health monitoring
    void enableHealthCheck(const QString& name, bool enabled);
    void setHealthCheckTimeout(const QString& name, int timeout);
    void performHealthCheck(const QString& name);
    void performHealthCheckAll();

    // Adaptive features
    void enableAdaptiveLoadBalancing(const QString& name, bool enabled);
    void setAdaptiveThresholds(const QString& name, double cpu, double memory, double responseTime);
    void updateServerWeights(const QString& lbName);
    void recalculateServerPriorities(const QString& lbName);

    // Statistics and monitoring
    DynamicLoadBalancerStats getLoadBalancerStats(const QString& name) const;
    QJsonObject getAllLoadBalancerStatsJson() const;
    void resetLoadBalancerStats(const QString& name);
    void exportLoadBalancerStats(const QString& filePath) const;

    // Advanced features
    void enableMetrics(const QString& name, bool enabled);
    void enableLogging(const QString& name, bool enabled);
    void setFailoverStrategy(const QString& name, const QString& strategy);
    void setStickySessionTimeout(const QString& name, int timeout);

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
    void statisticsUpdated(const QString& lbName, const DynamicLoadBalancerStats& stats);

public slots:
    void onHealthCheckTimer();
    void onStatisticsTimer();
    void onAdaptiveTimer();

private slots:
    void onServerTimeout();
    void onStickySessionExpired();
    void onLoadUpdate();

private:
    // Load balancer management
    struct DynamicLoadBalancer
    {
        LoadBalancingStrategy strategy;
        DynamicLoadBalancerStats stats;
        QList<ServerLoadInfo> servers;
        QMap<QString, QString> stickySessions; // clientId -> serverId
        QMap<QString, QDateTime> sessionTimestamps;
        QMap<QString, double> serverWeights;
        QMap<QString, int> serverPriorities;
        QMutex mutex;
        QTimer* healthCheckTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        QTimer* adaptiveTimer = nullptr;
        bool isHealthy = true;
    };

    // Core load balancer operations
    LoadBalancingDecision selectServerAlgorithm(DynamicLoadBalancer& lb, const QString& clientId);
    void updateServerHealth(DynamicLoadBalancer& lb, const QString& serverId, bool isHealthy);
    void handleServerFailure(DynamicLoadBalancer& lb, const QString& serverId);
    void cleanupStickySessions(DynamicLoadBalancer& lb);

    // Load balancing algorithms
    LoadBalancingDecision weightedRoundRobin(DynamicLoadBalancer& lb, const QString& clientId);
    LoadBalancingDecision leastConnections(DynamicLoadBalancer& lb, const QString& clientId);
    LoadBalancingDecision leastResponseTime(DynamicLoadBalancer& lb, const QString& clientId);
    LoadBalancingDecision adaptiveLoadBalancing(DynamicLoadBalancer& lb, const QString& clientId);

    // Health monitoring
    void performHealthCheck(DynamicLoadBalancer& lb);
    void checkServerHealth(ServerLoadInfo& server);
    void validateServerResponse(const QString& serverId, double responseTime);

    // Adaptive features
    void updateServerWeights(DynamicLoadBalancer& lb);
    void recalculateServerPriorities(DynamicLoadBalancer& lb);
    void adaptToLoadChanges(DynamicLoadBalancer& lb);

    // Statistics
    void updateLoadBalancerStatistics(const QString& name);
    void calculateLoadBalancerMetrics(DynamicLoadBalancer& lb);
    void logLoadBalancerEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateServerId() const;
    bool isServerValid(const ServerLoadInfo& server) const;
    void loadLoadBalancerFromDisk(const QString& name);
    void saveLoadBalancerToDisk(const QString& name);

    // Load balancer storage
    QMap<QString, std::unique_ptr<DynamicLoadBalancer>> m_loadBalancers;

    // Timers
    QTimer* m_globalHealthCheckTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;
    QTimer* m_globalAdaptiveTimer = nullptr;

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

    Q_DISABLE_COPY(DynamicLoadBalancer)
};

} // namespace LegacyStream

#endif // DYNAMICLOADBALANCER_H 