#ifndef REALTIMESTATISTICSMANAGER_H
#define REALTIMESTATISTICSMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QHostAddress>

namespace LegacyStream {

class StreamManager;
class PerformanceManager;

/**
 * @brief Performance metrics structure
 */
struct PerformanceMetrics
{
    double cpuUsage = 0.0;
    double memoryUsage = 0.0;
    double networkUsage = 0.0;
    int activeConnections = 0;
    int totalRequests = 0;
    double responseTime = 0.0;
    double throughput = 0.0;
    QDateTime timestamp;
};

/**
 * @brief Historical data point
 */
struct HistoricalDataPoint
{
    QDateTime timestamp;
    QJsonObject data;
    QString category;
};

/**
 * @brief Alert configuration
 */
struct AlertConfig
{
    QString name;
    QString condition;
    double threshold;
    QString severity; // "info", "warning", "critical"
    bool enabled = true;
    int cooldown = 300; // seconds
    QString message;
};

/**
 * @brief Alert event
 */
struct AlertEvent
{
    QString name;
    QString message;
    QString severity;
    QDateTime timestamp;
    QJsonObject context;
};

/**
 * @brief RealTimeStatisticsManager for advanced statistics and monitoring
 * 
 * Provides real-time statistics collection, historical analytics,
 * performance monitoring, and automated alerting system.
 */
class RealTimeStatisticsManager : public QObject
{
    Q_OBJECT

public:
    explicit RealTimeStatisticsManager(QObject* parent = nullptr);
    ~RealTimeStatisticsManager();

    // Initialization and lifecycle
    bool initialize(StreamManager* streamManager, PerformanceManager* performanceManager);
    void shutdown();
    bool start();
    void stop();

    // Real-time statistics collection
    void collectStreamStatistics();
    void collectPerformanceMetrics();
    void collectNetworkStatistics();
    void collectSystemStatistics();

    // Historical data management
    void addHistoricalDataPoint(const QString& category, const QJsonObject& data);
    QJsonArray getHistoricalData(const QString& category, int hours = 24) const;
    void clearHistoricalData(const QString& category = QString());
    void exportHistoricalData(const QString& category, const QString& filePath) const;

    // Advanced analytics
    QJsonObject getAnalyticsSummary() const;
    QJsonObject getTrendAnalysis(const QString& metric, int hours = 24) const;
    QJsonObject getPerformanceReport() const;
    QJsonObject getStreamAnalytics() const;

    // Performance monitoring
    PerformanceMetrics getCurrentPerformanceMetrics() const;
    QJsonObject getPerformanceMetricsJson() const;
    void setPerformanceThresholds(const QMap<QString, double>& thresholds);

    // Alert system
    void addAlert(const AlertConfig& alert);
    void removeAlert(const QString& name);
    void enableAlert(const QString& name, bool enabled);
    QList<AlertEvent> getRecentAlerts(int count = 50) const;
    void clearAlerts();

    // Statistics access
    QJsonObject getRealTimeStatistics() const;
    QJsonObject getStatisticsSummary() const;
    void resetStatistics();

signals:
    void statisticsUpdated(const QJsonObject& statistics);
    void performanceMetricsUpdated(const PerformanceMetrics& metrics);
    void alertTriggered(const AlertEvent& alert);
    void alertResolved(const QString& alertName);
    void historicalDataUpdated(const QString& category);

private slots:
    void onCollectionTimer();
    void onPerformanceTimer();
    void onAlertCheckTimer();
    void onStreamManagerUpdate();
    void onPerformanceManagerUpdate();

private:
    // Core functionality
    void processStreamData();
    void processPerformanceData();
    void checkAlerts();
    void triggerAlert(const AlertConfig& alert, const QJsonObject& context);
    void resolveAlert(const QString& alertName);

    // Analytics functions
    double calculateAverage(const QList<double>& values) const;
    double calculateTrend(const QList<double>& values) const;
    QJsonObject calculatePercentiles(const QList<double>& values) const;
    QString detectAnomaly(const QList<double>& values) const;

    // Utility functions
    QString formatBytes(qint64 bytes) const;
    QString formatDuration(qint64 seconds) const;
    QDateTime getTimestamp() const;

    // Dependencies
    StreamManager* m_streamManager = nullptr;
    PerformanceManager* m_performanceManager = nullptr;

    // Timers
    QTimer* m_collectionTimer = nullptr;
    QTimer* m_performanceTimer = nullptr;
    QTimer* m_alertCheckTimer = nullptr;

    // State management
    QAtomicInt m_isRunning = 0;
    QMutex m_mutex;

    // Real-time statistics
    QJsonObject m_realTimeStats;
    PerformanceMetrics m_currentMetrics;
    QMap<QString, QJsonObject> m_streamStats;

    // Historical data
    QMap<QString, QList<HistoricalDataPoint>> m_historicalData;
    int m_maxHistoricalPoints = 10000; // Maximum data points per category

    // Performance monitoring
    QMap<QString, double> m_performanceThresholds;
    QList<PerformanceMetrics> m_performanceHistory;
    int m_maxPerformanceHistory = 1000;

    // Alert system
    QMap<QString, AlertConfig> m_alerts;
    QList<AlertEvent> m_recentAlerts;
    QMap<QString, QDateTime> m_lastAlertTime;
    int m_maxRecentAlerts = 100;

    // Statistics tracking
    QDateTime m_startTime;
    qint64 m_totalRequests = 0;
    qint64 m_totalBytesReceived = 0;
    qint64 m_totalBytesSent = 0;
    int m_peakConnections = 0;
    double m_averageResponseTime = 0.0;

    Q_DISABLE_COPY(RealTimeStatisticsManager)
};

} // namespace LegacyStream

#endif // REALTIMESTATISTICSMANAGER_H 