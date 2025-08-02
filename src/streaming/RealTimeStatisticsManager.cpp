#include "streaming/RealTimeStatisticsManager.h"
#include "streaming/StreamManager.h"
#include "core/PerformanceManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QProcess>
#include <QNetworkInterface>

Q_LOGGING_CATEGORY(realTimeStats, "realTimeStats")

namespace LegacyStream {

RealTimeStatisticsManager::RealTimeStatisticsManager(QObject* parent)
    : QObject(parent)
    , m_collectionTimer(new QTimer(this))
    , m_performanceTimer(new QTimer(this))
    , m_alertCheckTimer(new QTimer(this))
{
    qCDebug(realTimeStats) << "RealTimeStatisticsManager created";
    
    // Set up timers
    m_collectionTimer->setSingleShot(false);
    m_collectionTimer->setInterval(5000); // Collect every 5 seconds
    
    m_performanceTimer->setSingleShot(false);
    m_performanceTimer->setInterval(10000); // Performance metrics every 10 seconds
    
    m_alertCheckTimer->setSingleShot(false);
    m_alertCheckTimer->setInterval(30000); // Check alerts every 30 seconds
    
    // Connect signals
    connect(m_collectionTimer, &QTimer::timeout, this, &RealTimeStatisticsManager::onCollectionTimer);
    connect(m_performanceTimer, &QTimer::timeout, this, &RealTimeStatisticsManager::onPerformanceTimer);
    connect(m_alertCheckTimer, &QTimer::timeout, this, &RealTimeStatisticsManager::onAlertCheckTimer);
}

RealTimeStatisticsManager::~RealTimeStatisticsManager()
{
    shutdown();
}

bool RealTimeStatisticsManager::initialize(StreamManager* streamManager, PerformanceManager* performanceManager)
{
    if (!streamManager || !performanceManager) {
        qCCritical(realTimeStats) << "Invalid dependencies";
        return false;
    }

    m_streamManager = streamManager;
    m_performanceManager = performanceManager;
    
    // Connect to manager updates
    connect(m_streamManager, &StreamManager::statusChanged, this, &RealTimeStatisticsManager::onStreamManagerUpdate);
    
    // Initialize default alerts
    setupDefaultAlerts();
    
    // Initialize performance thresholds
    setupPerformanceThresholds();
    
    m_startTime = QDateTime::currentDateTime();
    
    qDebug() << "RealTimeStatisticsManager initialized successfully";
    return true;
}

void RealTimeStatisticsManager::shutdown()
{
    if (m_isRunning.load()) {
        stop();
    }
    
    // Stop timers
    m_collectionTimer->stop();
    m_performanceTimer->stop();
    m_alertCheckTimer->stop();
    
    // Clear data
    QMutexLocker locker(&m_mutex);
    m_realTimeStats.clear();
    m_historicalData.clear();
    m_performanceHistory.clear();
    m_recentAlerts.clear();
    
    qDebug() << "RealTimeStatisticsManager shutdown complete";
}

bool RealTimeStatisticsManager::start()
{
    if (m_isRunning.load()) {
        qCWarning(realTimeStats) << "RealTimeStatisticsManager already running";
        return true;
    }
    
    qDebug() << "Starting RealTimeStatisticsManager";
    
    // Start timers
    m_collectionTimer->start();
    m_performanceTimer->start();
    m_alertCheckTimer->start();
    
    m_isRunning.store(true);
    
    qDebug() << "RealTimeStatisticsManager started successfully";
    return true;
}

void RealTimeStatisticsManager::stop()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    qDebug() << "Stopping RealTimeStatisticsManager";
    
    // Stop timers
    m_collectionTimer->stop();
    m_performanceTimer->stop();
    m_alertCheckTimer->stop();
    
    m_isRunning.store(false);
    
    qDebug() << "RealTimeStatisticsManager stopped";
}

void RealTimeStatisticsManager::collectStreamStatistics()
{
    if (!m_streamManager) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Collect stream statistics
    QList<StreamInfo> streams = m_streamManager->getStreams();
    QJsonArray streamsArray;
    
    for (const StreamInfo& stream : streams) {
        QJsonObject streamObj;
        streamObj["mount_point"] = stream.mountPoint;
        streamObj["codec"] = stream.codec;
        streamObj["bitrate"] = stream.bitrate;
        streamObj["sample_rate"] = stream.sampleRate;
        streamObj["channels"] = stream.channels;
        streamObj["active"] = stream.active;
        streamObj["listeners"] = stream.listeners;
        streamObj["bytes_received"] = stream.bytesReceived;
        streamObj["bytes_sent"] = stream.bytesSent;
        streamObj["start_time"] = stream.startTime.toString(Qt::ISODate);
        streamObj["metadata"] = stream.metadata;
        
        streamsArray.append(streamObj);
    }
    
    m_realTimeStats["streams"] = streamsArray;
    m_realTimeStats["total_streams"] = streams.size();
    m_realTimeStats["active_streams"] = m_streamManager->getActiveStreams();
    m_realTimeStats["total_listeners"] = m_streamManager->getTotalListeners();
    m_realTimeStats["total_bytes_received"] = m_streamManager->getTotalBytesReceived();
    m_realTimeStats["total_bytes_sent"] = m_streamManager->getTotalBytesSent();
    
    // Add to historical data
    addHistoricalDataPoint("streams", m_realTimeStats);
    
    emit statisticsUpdated(m_realTimeStats);
}

void RealTimeStatisticsManager::collectPerformanceMetrics()
{
    if (!m_performanceManager) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Collect performance metrics
    m_currentMetrics.cpuUsage = m_performanceManager->getCpuUsage();
    m_currentMetrics.memoryUsage = m_performanceManager->getMemoryUsage();
    m_currentMetrics.networkUsage = m_performanceManager->getNetworkUsage();
    m_currentMetrics.activeConnections = m_performanceManager->getActiveConnections();
    m_currentMetrics.totalRequests = m_performanceManager->getTotalRequests();
    m_currentMetrics.responseTime = m_performanceManager->getAverageResponseTime();
    m_currentMetrics.throughput = m_performanceManager->getThroughput();
    m_currentMetrics.timestamp = QDateTime::currentDateTime();
    
    // Add to performance history
    m_performanceHistory.append(m_currentMetrics);
    
    // Limit history size
    while (m_performanceHistory.size() > m_maxPerformanceHistory) {
        m_performanceHistory.removeFirst();
    }
    
    // Add to historical data
    QJsonObject metricsObj;
    metricsObj["cpu_usage"] = m_currentMetrics.cpuUsage;
    metricsObj["memory_usage"] = m_currentMetrics.memoryUsage;
    metricsObj["network_usage"] = m_currentMetrics.networkUsage;
    metricsObj["active_connections"] = m_currentMetrics.activeConnections;
    metricsObj["total_requests"] = m_currentMetrics.totalRequests;
    metricsObj["response_time"] = m_currentMetrics.responseTime;
    metricsObj["throughput"] = m_currentMetrics.throughput;
    metricsObj["timestamp"] = m_currentMetrics.timestamp.toString(Qt::ISODate);
    
    addHistoricalDataPoint("performance", metricsObj);
    
    emit performanceMetricsUpdated(m_currentMetrics);
}

void RealTimeStatisticsManager::collectNetworkStatistics()
{
    QMutexLocker locker(&m_mutex);
    
    // Collect network interface statistics
    QJsonArray interfacesArray;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface& interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) && 
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            
            QJsonObject interfaceObj;
            interfaceObj["name"] = interface.name();
            interfaceObj["address"] = interface.addressEntries().first().ip().toString();
            interfaceObj["netmask"] = interface.addressEntries().first().netmask().toString();
            interfaceObj["broadcast"] = interface.addressEntries().first().broadcast().toString();
            
            interfacesArray.append(interfaceObj);
        }
    }
    
    m_realTimeStats["network_interfaces"] = interfacesArray;
    
    // Add to historical data
    addHistoricalDataPoint("network", m_realTimeStats);
}

void RealTimeStatisticsManager::collectSystemStatistics()
{
    QMutexLocker locker(&m_mutex);
    
    // Collect system information
    QJsonObject systemInfo;
    systemInfo["uptime"] = m_startTime.secsTo(QDateTime::currentDateTime());
    systemInfo["process_id"] = QApplication::applicationPid();
    systemInfo["version"] = "1.0.0"; // TODO: Get from configuration
    systemInfo["platform"] = QSysInfo::prettyProductName();
    systemInfo["architecture"] = QSysInfo::currentCpuArchitecture();
    
    m_realTimeStats["system"] = systemInfo;
    
    // Add to historical data
    addHistoricalDataPoint("system", systemInfo);
}

void RealTimeStatisticsManager::addHistoricalDataPoint(const QString& category, const QJsonObject& data)
{
    QMutexLocker locker(&m_mutex);
    
    HistoricalDataPoint point;
    point.timestamp = QDateTime::currentDateTime();
    point.data = data;
    point.category = category;
    
    m_historicalData[category].append(point);
    
    // Limit data points per category
    while (m_historicalData[category].size() > m_maxHistoricalPoints) {
        m_historicalData[category].removeFirst();
    }
    
    emit historicalDataUpdated(category);
}

QJsonArray RealTimeStatisticsManager::getHistoricalData(const QString& category, int hours) const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonArray result;
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-hours * 3600);
    
    if (m_historicalData.contains(category)) {
        for (const HistoricalDataPoint& point : m_historicalData[category]) {
            if (point.timestamp >= cutoff) {
                QJsonObject dataPoint;
                dataPoint["timestamp"] = point.timestamp.toString(Qt::ISODate);
                dataPoint["data"] = point.data;
                dataPoint["category"] = point.category;
                result.append(dataPoint);
            }
        }
    }
    
    return result;
}

QJsonObject RealTimeStatisticsManager::getAnalyticsSummary() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject summary;
    
    // Calculate trends
    QList<double> cpuValues, memoryValues, listenerValues;
    
    for (const PerformanceMetrics& metrics : m_performanceHistory) {
        cpuValues.append(metrics.cpuUsage);
        memoryValues.append(metrics.memoryUsage);
    }
    
    // Stream statistics
    if (m_streamManager) {
        QList<StreamInfo> streams = m_streamManager->getStreams();
        for (const StreamInfo& stream : streams) {
            listenerValues.append(stream.listeners);
        }
    }
    
    summary["cpu_trend"] = calculateTrend(cpuValues);
    summary["memory_trend"] = calculateTrend(memoryValues);
    summary["listener_trend"] = calculateTrend(listenerValues);
    summary["peak_connections"] = m_peakConnections;
    summary["average_response_time"] = m_averageResponseTime;
    summary["total_requests"] = m_totalRequests;
    summary["uptime"] = m_startTime.secsTo(QDateTime::currentDateTime());
    
    return summary;
}

QJsonObject RealTimeStatisticsManager::getTrendAnalysis(const QString& metric, int hours) const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject analysis;
    QList<double> values;
    
    // Get historical data for the metric
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-hours * 3600);
    
    if (m_historicalData.contains(metric)) {
        for (const HistoricalDataPoint& point : m_historicalData[metric]) {
            if (point.timestamp >= cutoff) {
                if (point.data.contains("value")) {
                    values.append(point.data["value"].toDouble());
                }
            }
        }
    }
    
    if (!values.isEmpty()) {
        analysis["average"] = calculateAverage(values);
        analysis["trend"] = calculateTrend(values);
        analysis["percentiles"] = calculatePercentiles(values);
        analysis["anomaly"] = detectAnomaly(values);
        analysis["data_points"] = values.size();
    }
    
    return analysis;
}

QJsonObject RealTimeStatisticsManager::getPerformanceReport() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject report;
    
    if (!m_performanceHistory.isEmpty()) {
        const PerformanceMetrics& latest = m_performanceHistory.last();
        
        report["current_cpu"] = latest.cpuUsage;
        report["current_memory"] = latest.memoryUsage;
        report["current_network"] = latest.networkUsage;
        report["active_connections"] = latest.activeConnections;
        report["response_time"] = latest.responseTime;
        report["throughput"] = latest.throughput;
        
        // Calculate averages
        QList<double> cpuValues, memoryValues, responseValues;
        for (const PerformanceMetrics& metrics : m_performanceHistory) {
            cpuValues.append(metrics.cpuUsage);
            memoryValues.append(metrics.memoryUsage);
            responseValues.append(metrics.responseTime);
        }
        
        report["avg_cpu"] = calculateAverage(cpuValues);
        report["avg_memory"] = calculateAverage(memoryValues);
        report["avg_response_time"] = calculateAverage(responseValues);
    }
    
    return report;
}

QJsonObject RealTimeStatisticsManager::getStreamAnalytics() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject analytics;
    
    if (m_streamManager) {
        QList<StreamInfo> streams = m_streamManager->getStreams();
        
        analytics["total_streams"] = streams.size();
        analytics["active_streams"] = m_streamManager->getActiveStreams();
        analytics["total_listeners"] = m_streamManager->getTotalListeners();
        analytics["total_bytes_received"] = m_streamManager->getTotalBytesReceived();
        analytics["total_bytes_sent"] = m_streamManager->getTotalBytesSent();
        
        // Calculate per-stream analytics
        QJsonArray streamAnalytics;
        for (const StreamInfo& stream : streams) {
            QJsonObject streamAnalytic;
            streamAnalytic["mount_point"] = stream.mountPoint;
            streamAnalytic["listeners"] = stream.listeners;
            streamAnalytic["bytes_sent"] = stream.bytesSent;
            streamAnalytic["uptime"] = stream.startTime.secsTo(QDateTime::currentDateTime());
            streamAnalytic["active"] = stream.active;
            
            streamAnalytics.append(streamAnalytic);
        }
        
        analytics["streams"] = streamAnalytics;
    }
    
    return analytics;
}

PerformanceMetrics RealTimeStatisticsManager::getCurrentPerformanceMetrics() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentMetrics;
}

QJsonObject RealTimeStatisticsManager::getPerformanceMetricsJson() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject metrics;
    metrics["cpu_usage"] = m_currentMetrics.cpuUsage;
    metrics["memory_usage"] = m_currentMetrics.memoryUsage;
    metrics["network_usage"] = m_currentMetrics.networkUsage;
    metrics["active_connections"] = m_currentMetrics.activeConnections;
    metrics["total_requests"] = m_currentMetrics.totalRequests;
    metrics["response_time"] = m_currentMetrics.responseTime;
    metrics["throughput"] = m_currentMetrics.throughput;
    metrics["timestamp"] = m_currentMetrics.timestamp.toString(Qt::ISODate);
    
    return metrics;
}

void RealTimeStatisticsManager::addAlert(const AlertConfig& alert)
{
    QMutexLocker locker(&m_mutex);
    m_alerts[alert.name] = alert;
    qDebug() << "Added alert:" << alert.name;
}

void RealTimeStatisticsManager::removeAlert(const QString& name)
{
    QMutexLocker locker(&m_mutex);
    m_alerts.remove(name);
    qDebug() << "Removed alert:" << name;
}

void RealTimeStatisticsManager::enableAlert(const QString& name, bool enabled)
{
    QMutexLocker locker(&m_mutex);
    if (m_alerts.contains(name)) {
        m_alerts[name].enabled = enabled;
        qDebug() << "Alert" << name << (enabled ? "enabled" : "disabled");
    }
}

QList<AlertEvent> RealTimeStatisticsManager::getRecentAlerts(int count) const
{
    QMutexLocker locker(&m_mutex);
    
    QList<AlertEvent> result;
    int startIndex = qMax(0, m_recentAlerts.size() - count);
    
    for (int i = startIndex; i < m_recentAlerts.size(); ++i) {
        result.append(m_recentAlerts[i]);
    }
    
    return result;
}

QJsonObject RealTimeStatisticsManager::getRealTimeStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return m_realTimeStats;
}

QJsonObject RealTimeStatisticsManager::getStatisticsSummary() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject summary;
    summary["uptime"] = m_startTime.secsTo(QDateTime::currentDateTime());
    summary["total_requests"] = m_totalRequests;
    summary["total_bytes_received"] = m_totalBytesReceived;
    summary["total_bytes_sent"] = m_totalBytesSent;
    summary["peak_connections"] = m_peakConnections;
    summary["average_response_time"] = m_averageResponseTime;
    summary["active_alerts"] = m_alerts.size();
    summary["recent_alerts"] = m_recentAlerts.size();
    
    return summary;
}

void RealTimeStatisticsManager::onCollectionTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    collectStreamStatistics();
    collectNetworkStatistics();
    collectSystemStatistics();
}

void RealTimeStatisticsManager::onPerformanceTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    collectPerformanceMetrics();
}

void RealTimeStatisticsManager::onAlertCheckTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    checkAlerts();
}

void RealTimeStatisticsManager::onStreamManagerUpdate()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    // Update statistics when stream manager changes
    collectStreamStatistics();
}

void RealTimeStatisticsManager::onPerformanceManagerUpdate()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    // Update performance metrics when performance manager changes
    collectPerformanceMetrics();
}

void RealTimeStatisticsManager::checkAlerts()
{
    QMutexLocker locker(&m_mutex);
    
    for (const AlertConfig& alert : m_alerts.values()) {
        if (!alert.enabled) {
            continue;
        }
        
        // Check cooldown
        QDateTime lastAlert = m_lastAlertTime.value(alert.name);
        if (lastAlert.isValid() && lastAlert.secsTo(QDateTime::currentDateTime()) < alert.cooldown) {
            continue;
        }
        
        // Check conditions
        bool shouldTrigger = false;
        QJsonObject context;
        
        if (alert.condition == "cpu_high" && m_currentMetrics.cpuUsage > alert.threshold) {
            shouldTrigger = true;
            context["current_value"] = m_currentMetrics.cpuUsage;
            context["threshold"] = alert.threshold;
        } else if (alert.condition == "memory_high" && m_currentMetrics.memoryUsage > alert.threshold) {
            shouldTrigger = true;
            context["current_value"] = m_currentMetrics.memoryUsage;
            context["threshold"] = alert.threshold;
        } else if (alert.condition == "response_time_high" && m_currentMetrics.responseTime > alert.threshold) {
            shouldTrigger = true;
            context["current_value"] = m_currentMetrics.responseTime;
            context["threshold"] = alert.threshold;
        }
        
        if (shouldTrigger) {
            triggerAlert(alert, context);
        }
    }
}

void RealTimeStatisticsManager::triggerAlert(const AlertConfig& alert, const QJsonObject& context)
{
    AlertEvent event;
    event.name = alert.name;
    event.message = alert.message;
    event.severity = alert.severity;
    event.timestamp = QDateTime::currentDateTime();
    event.context = context;
    
    m_recentAlerts.append(event);
    m_lastAlertTime[alert.name] = event.timestamp;
    
    // Limit recent alerts
    while (m_recentAlerts.size() > m_maxRecentAlerts) {
        m_recentAlerts.removeFirst();
    }
    
    qCWarning(realTimeStats) << "Alert triggered:" << alert.name << "-" << alert.message;
    emit alertTriggered(event);
}

void RealTimeStatisticsManager::setupDefaultAlerts()
{
    // CPU usage alert
    AlertConfig cpuAlert;
    cpuAlert.name = "high_cpu_usage";
    cpuAlert.condition = "cpu_high";
    cpuAlert.threshold = 80.0;
    cpuAlert.severity = "warning";
    cpuAlert.message = "CPU usage is high";
    addAlert(cpuAlert);
    
    // Memory usage alert
    AlertConfig memoryAlert;
    memoryAlert.name = "high_memory_usage";
    memoryAlert.condition = "memory_high";
    memoryAlert.threshold = 85.0;
    memoryAlert.severity = "warning";
    memoryAlert.message = "Memory usage is high";
    addAlert(memoryAlert);
    
    // Response time alert
    AlertConfig responseAlert;
    responseAlert.name = "high_response_time";
    responseAlert.condition = "response_time_high";
    responseAlert.threshold = 1000.0; // 1 second
    responseAlert.severity = "critical";
    responseAlert.message = "Response time is high";
    addAlert(responseAlert);
}

void RealTimeStatisticsManager::setupPerformanceThresholds()
{
    m_performanceThresholds["cpu_warning"] = 70.0;
    m_performanceThresholds["cpu_critical"] = 90.0;
    m_performanceThresholds["memory_warning"] = 80.0;
    m_performanceThresholds["memory_critical"] = 95.0;
    m_performanceThresholds["response_time_warning"] = 500.0;
    m_performanceThresholds["response_time_critical"] = 2000.0;
}

double RealTimeStatisticsManager::calculateAverage(const QList<double>& values) const
{
    if (values.isEmpty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double value : values) {
        sum += value;
    }
    
    return sum / values.size();
}

double RealTimeStatisticsManager::calculateTrend(const QList<double>& values) const
{
    if (values.size() < 2) {
        return 0.0;
    }
    
    // Simple linear trend calculation
    double first = values.first();
    double last = values.last();
    double timeSpan = values.size() - 1;
    
    return (last - first) / timeSpan;
}

QJsonObject RealTimeStatisticsManager::calculatePercentiles(const QList<double>& values) const
{
    QJsonObject percentiles;
    
    if (values.isEmpty()) {
        return percentiles;
    }
    
    QList<double> sortedValues = values;
    std::sort(sortedValues.begin(), sortedValues.end());
    
    int size = sortedValues.size();
    
    percentiles["min"] = sortedValues.first();
    percentiles["max"] = sortedValues.last();
    percentiles["median"] = sortedValues[size / 2];
    percentiles["p25"] = sortedValues[size * 25 / 100];
    percentiles["p75"] = sortedValues[size * 75 / 100];
    percentiles["p95"] = sortedValues[size * 95 / 100];
    
    return percentiles;
}

QString RealTimeStatisticsManager::detectAnomaly(const QList<double>& values) const
{
    if (values.size() < 10) {
        return "insufficient_data";
    }
    
    // Simple anomaly detection using standard deviation
    double mean = calculateAverage(values);
    double variance = 0.0;
    
    for (double value : values) {
        variance += (value - mean) * (value - mean);
    }
    variance /= values.size();
    
    double stdDev = sqrt(variance);
    double lastValue = values.last();
    
    if (abs(lastValue - mean) > 2 * stdDev) {
        return "anomaly_detected";
    }
    
    return "normal";
}

QString RealTimeStatisticsManager::formatBytes(qint64 bytes) const
{
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;
    
    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', 2).arg(units[unitIndex]);
}

QString RealTimeStatisticsManager::formatDuration(qint64 seconds) const
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}

QDateTime RealTimeStatisticsManager::getTimestamp() const
{
    return QDateTime::currentDateTime();
}

void RealTimeStatisticsManager::clearHistoricalData(const QString& category)
{
    QMutexLocker locker(&m_mutex);
    
    if (category.isEmpty()) {
        m_historicalData.clear();
    } else {
        m_historicalData.remove(category);
    }
}

void RealTimeStatisticsManager::exportHistoricalData(const QString& category, const QString& filePath) const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonArray data = getHistoricalData(category, 24);
    QJsonDocument doc(data);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        qDebug() << "Exported historical data to:" << filePath;
    } else {
        qCWarning(realTimeStats) << "Failed to export historical data to:" << filePath;
    }
}

void RealTimeStatisticsManager::setPerformanceThresholds(const QMap<QString, double>& thresholds)
{
    QMutexLocker locker(&m_mutex);
    m_performanceThresholds = thresholds;
}

void RealTimeStatisticsManager::clearAlerts()
{
    QMutexLocker locker(&m_mutex);
    m_recentAlerts.clear();
    m_lastAlertTime.clear();
}

void RealTimeStatisticsManager::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    
    m_totalRequests = 0;
    m_totalBytesReceived = 0;
    m_totalBytesSent = 0;
    m_peakConnections = 0;
    m_averageResponseTime = 0.0;
    m_startTime = QDateTime::currentDateTime();
    
    qDebug() << "Statistics reset";
}

} // namespace LegacyStream 
