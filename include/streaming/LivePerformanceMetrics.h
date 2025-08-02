#ifndef LIVEPERFORMANCEMETRICS_H
#define LIVEPERFORMANCEMETRICS_H

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
 * @brief Performance metric data
 */
struct PerformanceMetricData
{
    QString metricId;
    QString metricName;
    QString metricType; // "counter", "gauge", "histogram", "summary"
    double value = 0.0;
    double minValue = 0.0;
    double maxValue = 0.0;
    double averageValue = 0.0;
    int count = 0;
    QDateTime timestamp;
    QString unit;
    QMap<QString, QString> labels;
    QJsonObject metadata;
};

/**
 * @brief Performance metrics configuration
 */
struct PerformanceMetricsConfig
{
    QString name;
    int collectionInterval = 1000; // 1 second
    bool enableRealTimeCollection = true;
    bool enableHistoricalData = true;
    bool enableAlerts = true;
    bool enableLogging = true;
    int retentionPeriod = 86400; // 24 hours
    int maxDataPoints = 10000;
    double alertThreshold = 0.8;
    QStringList enabledMetrics;
    QMap<QString, double> metricThresholds;
};

/**
 * @brief Performance alert
 */
struct PerformanceAlert
{
    QString type; // "threshold", "anomaly", "trend"
    QString severity; // "warning", "critical"
    QString message;
    QString metricId;
    double currentValue;
    double threshold;
    QDateTime timestamp;
    QJsonObject context;
};

/**
 * @brief Performance metrics statistics
 */
struct PerformanceMetricsStats
{
    int totalMetrics = 0;
    int activeMetrics = 0;
    int totalAlerts = 0;
    int warningAlerts = 0;
    int criticalAlerts = 0;
    double averagePerformance = 0.0;
    QDateTime lastCollection;
    QDateTime lastAlert;
    QMap<QString, int> metricsByType;
    QMap<QString, double> averageValuesByMetric;
    QMap<QString, int> alertsByMetric;
};

/**
 * @brief Live performance metrics manager for real-time performance data
 * 
 * Provides comprehensive real-time performance metrics collection with
 * historical data, anomaly detection, trend analysis, and automated alerts.
 */
class LivePerformanceMetrics : public QObject
{
    Q_OBJECT

public:
    explicit LivePerformanceMetrics(QObject* parent = nullptr);
    ~LivePerformanceMetrics();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Metrics management
    bool createMetricsCollector(const QString& name, const PerformanceMetricsConfig& config);
    void destroyMetricsCollector(const QString& name);
    bool metricsCollectorExists(const QString& name) const;
    QStringList getMetricsCollectorNames() const;

    // Metrics collection
    void recordMetric(const QString& name, const QString& metricId, double value, const QMap<QString, QString>& labels = QMap<QString, QString>());
    void incrementCounter(const QString& name, const QString& metricId, double increment = 1.0, const QMap<QString, QString>& labels = QMap<QString, QString>());
    void setGauge(const QString& name, const QString& metricId, double value, const QMap<QString, QString>& labels = QMap<QString, QString>());
    void observeHistogram(const QString& name, const QString& metricId, double value, const QMap<QString, QString>& labels = QMap<QString, QString>());

    // Real-time collection
    void enableRealTimeCollection(const QString& name, bool enabled);
    void setCollectionInterval(const QString& name, int interval);
    void setMetricThreshold(const QString& name, const QString& metricId, double threshold);
    void setAlertThreshold(const QString& name, double threshold);

    // Historical data
    void enableHistoricalData(const QString& name, bool enabled);
    void setRetentionPeriod(const QString& name, int period);
    void setMaxDataPoints(const QString& name, int maxPoints);
    QList<PerformanceMetricData> getHistoricalData(const QString& name, const QString& metricId, int count = 100) const;

    // Alerts and notifications
    void enableAlerts(const QString& name, bool enabled);
    void setAlertThresholds(const QString& name, const QMap<QString, double>& thresholds);
    QList<PerformanceAlert> getRecentAlerts(const QString& name, int count = 50) const;
    void clearAlerts(const QString& name);

    // Statistics and monitoring
    PerformanceMetricsStats getMetricsStats(const QString& name) const;
    QJsonObject getAllMetricsStatsJson() const;
    void resetMetricsStats(const QString& name);
    void exportMetricsStats(const QString& filePath) const;

    // Advanced features
    void enableLogging(const QString& name, bool enabled);
    void setLogLevel(const QString& name, const QString& level);
    void enableMetricFilter(const QString& name, const QStringList& metrics);
    void setCollectionMode(const QString& name, const QString& mode);

    // Utility functions
    bool isPerformanceHealthy(const QString& name) const;
    double getAveragePerformance(const QString& name) const;
    double getPeakPerformance(const QString& name) const;
    double getMetricValue(const QString& name, const QString& metricId) const;

signals:
    void metricRecorded(const QString& name, const QString& metricId, double value);
    void performanceAlert(const PerformanceAlert& alert);
    void thresholdWarning(const QString& name, const QString& metricId, double value, double threshold);
    void thresholdCritical(const QString& name, const QString& metricId, double value, double threshold);
    void statisticsUpdated(const QString& name, const PerformanceMetricsStats& stats);

public slots:
    void onCollectionTimer();
    void onAlertTimer();
    void onStatisticsTimer();

private slots:
    void onMetricDataReceived(const QString& name, const QString& metricId, double value);
    void onThresholdCheck();
    void onAnomalyCheck();

private:
    // Metrics collection
    struct MetricsCollector
    {
        PerformanceMetricsConfig config;
        PerformanceMetricsStats stats;
        QMap<QString, PerformanceMetricData> currentMetrics;
        QMap<QString, QList<PerformanceMetricData>> historicalData;
        QList<PerformanceAlert> alerts;
        QMutex mutex;
        QTimer* collectionTimer = nullptr;
        QTimer* alertTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        bool isActive = true;
    };

    // Core collection operations
    void performMetricsCollection(MetricsCollector& collector);
    void updateMetricData(MetricsCollector& collector, const QString& metricId, double value, const QMap<QString, QString>& labels);
    void checkPerformanceAlerts(MetricsCollector& collector);
    void generatePerformanceAlert(MetricsCollector& collector, const QString& type, double value, double threshold, const QString& metricId);

    // Metrics processing
    PerformanceMetricData processMetricData(const QString& metricId, double value, const QMap<QString, QString>& labels);
    void updateHistoricalData(MetricsCollector& collector, const QString& metricId, const PerformanceMetricData& data);
    void calculateMetricStatistics(MetricsCollector& collector, const QString& metricId);

    // Alert management
    void processPerformanceAlert(MetricsCollector& collector, const PerformanceAlert& alert);
    void logPerformanceAlert(const QString& name, const PerformanceAlert& alert);
    bool shouldGenerateAlert(const QString& type, double value, double threshold);

    // Anomaly detection
    bool detectAnomaly(const QString& metricId, double value, const QList<PerformanceMetricData>& historicalData);
    double calculateTrend(const QString& metricId, const QList<PerformanceMetricData>& historicalData);
    bool isValueAnomalous(double value, double average, double standardDeviation);

    // Statistics
    void updateMetricsStatistics(const QString& name);
    void calculateMetricsStatistics(MetricsCollector& collector);
    void logMetricsEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateCollectorId() const;
    bool isMetricValid(const PerformanceMetricData& metric) const;
    void loadCollectorFromDisk(const QString& name);
    void saveCollectorToDisk(const QString& name);

    // Collector storage
    QMap<QString, std::unique_ptr<MetricsCollector>> m_collectors;

    // Timers
    QTimer* m_globalCollectionTimer = nullptr;
    QTimer* m_globalAlertTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_alertsEnabled = true;
    bool m_loggingEnabled = true;
    bool m_realTimeCollectionEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastCollection;
    QMap<QString, QDateTime> m_lastAlert;
    QMap<QString, double> m_averagePerformance;

    // Metrics processing
    QMap<QString, QMap<QString, PerformanceMetricData>> m_currentMetrics;
    QMap<QString, QMap<QString, QList<PerformanceMetricData>>> m_historicalData;
    QMap<QString, QList<PerformanceAlert>> m_alerts;

    Q_DISABLE_COPY(LivePerformanceMetrics)
};

} // namespace LegacyStream

#endif // LIVEPERFORMANCEMETRICS_H 