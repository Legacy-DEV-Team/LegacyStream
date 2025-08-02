#ifndef RESOURCEMONITORMANAGER_H
#define RESOURCEMONITORMANAGER_H

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
 * @brief System resource information
 */
struct SystemResources
{
    double cpuUsage = 0.0;
    double memoryUsage = 0.0;
    double diskUsage = 0.0;
    double networkUsage = 0.0;
    quint64 totalMemory = 0;
    quint64 availableMemory = 0;
    quint64 totalDiskSpace = 0;
    quint64 availableDiskSpace = 0;
    quint64 networkBytesIn = 0;
    quint64 networkBytesOut = 0;
    int activeConnections = 0;
    int totalProcesses = 0;
    double systemLoad = 0.0;
    QDateTime timestamp;
};

/**
 * @brief Process resource information
 */
struct ProcessResources
{
    QString processId;
    QString processName;
    double cpuUsage = 0.0;
    double memoryUsage = 0.0;
    quint64 memoryRSS = 0;
    quint64 memoryVMS = 0;
    int threadCount = 0;
    int handleCount = 0;
    double ioReadBytes = 0.0;
    double ioWriteBytes = 0.0;
    QDateTime startTime;
    QDateTime lastUpdate;
};

/**
 * @brief Resource monitoring configuration
 */
struct ResourceMonitorConfig
{
    QString name;
    int updateInterval = 1000; // 1 second
    bool monitorCPU = true;
    bool monitorMemory = true;
    bool monitorDisk = true;
    bool monitorNetwork = true;
    bool monitorProcesses = true;
    bool enableAlerts = true;
    bool enableLogging = true;
    double cpuThreshold = 80.0;
    double memoryThreshold = 85.0;
    double diskThreshold = 90.0;
    double networkThreshold = 70.0;
};

/**
 * @brief Resource alert
 */
struct ResourceAlert
{
    QString type; // "cpu", "memory", "disk", "network"
    QString severity; // "warning", "critical"
    QString message;
    double currentValue;
    double threshold;
    QDateTime timestamp;
    QJsonObject context;
};

/**
 * @brief Resource monitoring statistics
 */
struct ResourceMonitorStats
{
    int totalAlerts = 0;
    int warningAlerts = 0;
    int criticalAlerts = 0;
    double averageCPU = 0.0;
    double averageMemory = 0.0;
    double averageDisk = 0.0;
    double averageNetwork = 0.0;
    QDateTime lastAlert;
    QMap<QString, int> alertsByType;
    QMap<QString, double> peakValues;
    QList<ResourceAlert> recentAlerts;
};

/**
 * @brief Resource monitoring manager for real-time resource tracking
 * 
 * Provides comprehensive system and process resource monitoring with alerts,
 * logging, and performance optimization recommendations.
 */
class ResourceMonitorManager : public QObject
{
    Q_OBJECT

public:
    explicit ResourceMonitorManager(QObject* parent = nullptr);
    ~ResourceMonitorManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Resource monitoring
    bool createMonitor(const QString& name, const ResourceMonitorConfig& config);
    void destroyMonitor(const QString& name);
    bool monitorExists(const QString& name) const;
    QStringList getMonitorNames() const;

    // Resource data
    SystemResources getSystemResources() const;
    QList<ProcessResources> getProcessResources() const;
    ProcessResources getProcessResources(const QString& processId) const;
    QJsonObject getSystemResourcesJson() const;
    QJsonObject getProcessResourcesJson() const;

    // Configuration
    void setMonitorConfig(const QString& name, const ResourceMonitorConfig& config);
    ResourceMonitorConfig getMonitorConfig(const QString& name) const;
    void setUpdateInterval(const QString& name, int interval);
    void setThresholds(const QString& name, double cpu, double memory, double disk, double network);

    // Alerts and notifications
    void enableAlerts(const QString& name, bool enabled);
    void setAlertThresholds(const QString& name, double cpu, double memory, double disk, double network);
    QList<ResourceAlert> getRecentAlerts(const QString& name, int count = 50) const;
    void clearAlerts(const QString& name);

    // Statistics and monitoring
    ResourceMonitorStats getMonitorStats(const QString& name) const;
    QJsonObject getAllMonitorStatsJson() const;
    void resetMonitorStats(const QString& name);
    void exportMonitorStats(const QString& filePath) const;

    // Advanced features
    void enableLogging(const QString& name, bool enabled);
    void setLogLevel(const QString& name, const QString& level);
    void enableProcessMonitoring(const QString& name, bool enabled);
    void setProcessFilter(const QString& name, const QStringList& processes);

    // Utility functions
    bool isSystemHealthy() const;
    double getSystemLoad() const;
    double getMemoryPressure() const;
    double getDiskPressure() const;
    double getNetworkPressure() const;

signals:
    void systemResourcesUpdated(const SystemResources& resources);
    void processResourcesUpdated(const QList<ProcessResources>& processes);
    void resourceAlert(const ResourceAlert& alert);
    void resourceWarning(const QString& type, double value, double threshold);
    void resourceCritical(const QString& type, double value, double threshold);
    void statisticsUpdated(const QString& name, const ResourceMonitorStats& stats);

public slots:
    void onUpdateTimer();
    void onAlertTimer();
    void onStatisticsTimer();

private slots:
    void onSystemResourceUpdate();
    void onProcessResourceUpdate();
    void onAlertCheck();

private:
    // Resource monitoring
    struct ResourceMonitor
    {
        ResourceMonitorConfig config;
        ResourceMonitorStats stats;
        SystemResources lastSystemResources;
        QList<ProcessResources> lastProcessResources;
        QList<ResourceAlert> alerts;
        QMutex mutex;
        QTimer* updateTimer = nullptr;
        QTimer* alertTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        bool isActive = true;
    };

    // Core monitoring operations
    void updateSystemResources(ResourceMonitor& monitor);
    void updateProcessResources(ResourceMonitor& monitor);
    void checkResourceAlerts(ResourceMonitor& monitor);
    void generateResourceAlert(ResourceMonitor& monitor, const QString& type, double value, double threshold);

    // Resource collection
    SystemResources collectSystemResources();
    QList<ProcessResources> collectProcessResources();
    ProcessResources collectProcessResource(const QString& processId);
    double calculateCPUUsage();
    double calculateMemoryUsage();
    double calculateDiskUsage();
    double calculateNetworkUsage();

    // Alert management
    void processResourceAlert(ResourceMonitor& monitor, const ResourceAlert& alert);
    void logResourceAlert(const QString& name, const ResourceAlert& alert);
    bool shouldGenerateAlert(const QString& type, double value, double threshold);

    // Statistics
    void updateMonitorStatistics(const QString& name);
    void calculateMonitorMetrics(ResourceMonitor& monitor);
    void logMonitorEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateMonitorId() const;
    bool isResourceValid(const SystemResources& resources) const;
    void loadMonitorFromDisk(const QString& name);
    void saveMonitorToDisk(const QString& name);

    // Monitor storage
    QMap<QString, std::unique_ptr<ResourceMonitor>> m_monitors;

    // Timers
    QTimer* m_globalUpdateTimer = nullptr;
    QTimer* m_globalAlertTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_alertsEnabled = true;
    bool m_loggingEnabled = true;
    bool m_processMonitoringEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastUpdate;
    QMap<QString, QDateTime> m_lastAlert;
    QMap<QString, double> m_peakValues;

    // Platform-specific
#ifdef _WIN32
    void* m_performanceCounter = nullptr;
    void* m_memoryCounter = nullptr;
    void* m_diskCounter = nullptr;
    void* m_networkCounter = nullptr;
#endif

    Q_DISABLE_COPY(ResourceMonitorManager)
};

} // namespace LegacyStream

#endif // RESOURCEMONITORMANAGER_H 