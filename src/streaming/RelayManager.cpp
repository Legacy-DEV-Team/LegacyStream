#include "streaming/RelayManager.h"
#include "streaming/StreamManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QBuffer>
#include <QTimer>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Q_LOGGING_CATEGORY(relayManager, "relayManager")

namespace LegacyStream {

RelayManager::RelayManager(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_retryTimer(new QTimer(this))
{
    qCDebug(relayManager) << "RelayManager created";
    
    // Set up retry timer
    m_retryTimer->setSingleShot(false);
    m_retryTimer->setInterval(1000); // Check retries every second
    
    // Connect signals
    connect(m_retryTimer, &QTimer::timeout, this, &RelayManager::onRetryTimer);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &RelayManager::onRelayFinished);
}

RelayManager::~RelayManager()
{
    shutdown();
}

bool RelayManager::initialize()
{
    qCDebug(relayManager) << "Initializing RelayManager";
    
    // Initialize statistics
    m_statistics["total_relays"] = 0;
    m_statistics["active_relays"] = 0;
    m_statistics["total_bytes_sent"] = 0;
    m_statistics["total_errors"] = 0;
    m_statistics["start_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    qCInfo(relayManager) << "RelayManager initialized successfully";
    return true;
}

void RelayManager::shutdown()
{
    if (m_isRunning.load()) {
        stop();
    }
    
    // Stop retry timer
    m_retryTimer->stop();
    
    // Disconnect all active relays
    QMutexLocker locker(&m_mutex);
    for (auto it = m_activeRelays.begin(); it != m_activeRelays.end(); ++it) {
        QNetworkReply* reply = it.value();
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    }
    m_activeRelays.clear();
    
    // Clear retry timers
    for (QTimer* timer : m_retryTimers.values()) {
        timer->stop();
        timer->deleteLater();
    }
    m_retryTimers.clear();
    
    qCInfo(relayManager) << "RelayManager shutdown complete";
}

bool RelayManager::start()
{
    if (m_isRunning.load()) {
        qCWarning(relayManager) << "RelayManager already running";
        return true;
    }
    
    if (!m_streamManager) {
        qCCritical(relayManager) << "StreamManager not set";
        return false;
    }
    
    qCInfo(relayManager) << "Starting RelayManager";
    
    // Start retry timer
    m_retryTimer->start();
    
    m_isRunning.store(true);
    m_startTime = QDateTime::currentDateTime();
    
    // Connect enabled relays
    QMutexLocker locker(&m_mutex);
    for (auto it = m_relayConfigs.begin(); it != m_relayConfigs.end(); ++it) {
        const RelayConfig& config = it.value();
        if (config.enabled) {
            connectRelay(config.name);
        }
    }
    
    qCInfo(relayManager) << "RelayManager started successfully";
    emit statusChanged(m_statistics);
    
    return true;
}

void RelayManager::stop()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    qCInfo(relayManager) << "Stopping RelayManager";
    
    // Stop retry timer
    m_retryTimer->stop();
    
    // Disconnect all relays
    QMutexLocker locker(&m_mutex);
    for (auto it = m_activeRelays.begin(); it != m_activeRelays.end(); ++it) {
        disconnectRelay(it.key());
    }
    
    m_isRunning.store(false);
    
    qCInfo(relayManager) << "RelayManager stopped";
    emit statusChanged(m_statistics);
}

void RelayManager::setStreamManager(StreamManager* streamManager)
{
    m_streamManager = streamManager;
}

void RelayManager::addRelay(const RelayConfig& config)
{
    if (!isValidRelayConfig(config)) {
        qCWarning(relayManager) << "Invalid relay config for:" << config.name;
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    m_relayConfigs[config.name] = config;
    m_relayStats[config.name] = RelayStats{config.name};
    
    m_totalRelays++;
    m_statistics["total_relays"] = m_totalRelays;
    
    qCInfo(relayManager) << "Added relay:" << config.name << "->" << config.targetUrl;
    
    // Connect if running and enabled
    if (m_isRunning.load() && config.enabled) {
        connectRelay(config.name);
    }
    
    emit statusChanged(m_statistics);
}

void RelayManager::removeRelay(const QString& name)
{
    QMutexLocker locker(&m_mutex);
    
    // Disconnect if active
    if (m_activeRelays.contains(name)) {
        disconnectRelay(name);
    }
    
    // Remove from configs and stats
    m_relayConfigs.remove(name);
    m_relayStats.remove(name);
    
    // Clean up retry timer
    if (m_retryTimers.contains(name)) {
        m_retryTimers[name]->stop();
        m_retryTimers[name]->deleteLater();
        m_retryTimers.remove(name);
    }
    
    qCInfo(relayManager) << "Removed relay:" << name;
    emit statusChanged(m_statistics);
}

void RelayManager::updateRelay(const QString& name, const RelayConfig& config)
{
    if (!isValidRelayConfig(config)) {
        qCWarning(relayManager) << "Invalid relay config for:" << name;
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Disconnect if currently active
    if (m_activeRelays.contains(name)) {
        disconnectRelay(name);
    }
    
    // Update config
    m_relayConfigs[name] = config;
    
    // Reconnect if running and enabled
    if (m_isRunning.load() && config.enabled) {
        connectRelay(name);
    }
    
    qCInfo(relayManager) << "Updated relay:" << name;
    emit statusChanged(m_statistics);
}

void RelayManager::enableRelay(const QString& name, bool enabled)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_relayConfigs.contains(name)) {
        qCWarning(relayManager) << "Relay not found:" << name;
        return;
    }
    
    RelayConfig& config = m_relayConfigs[name];
    config.enabled = enabled;
    
    if (enabled && m_isRunning.load()) {
        connectRelay(name);
    } else if (!enabled) {
        disconnectRelay(name);
    }
    
    qCInfo(relayManager) << "Relay" << name << (enabled ? "enabled" : "disabled");
    emit statusChanged(m_statistics);
}

bool RelayManager::isRunning() const
{
    return m_isRunning.load();
}

QList<RelayConfig> RelayManager::getRelayConfigs() const
{
    QMutexLocker locker(&m_mutex);
    return m_relayConfigs.values();
}

QMap<QString, RelayStats> RelayManager::getRelayStats() const
{
    QMutexLocker locker(&m_mutex);
    return m_relayStats;
}

QJsonObject RelayManager::getStatusJson() const
{
    QJsonObject status = m_statistics;
    status["running"] = m_isRunning.load();
    status["active_relays_count"] = m_activeRelaysCount;
    
    QJsonArray configs;
    for (const RelayConfig& config : m_relayConfigs.values()) {
        QJsonObject configObj;
        configObj["name"] = config.name;
        configObj["target_url"] = config.targetUrl;
        configObj["mount_point"] = config.mountPoint;
        configObj["enabled"] = config.enabled;
        configObj["bitrate"] = config.bitrate;
        configObj["codec"] = config.codec;
        configs.append(configObj);
    }
    status["relay_configs"] = configs;
    
    return status;
}

void RelayManager::connectRelay(const QString& name)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_relayConfigs.contains(name)) {
        qCWarning(relayManager) << "Relay config not found:" << name;
        return;
    }
    
    const RelayConfig& config = m_relayConfigs[name];
    
    if (!config.enabled) {
        return;
    }
    
    // Create connection request
    QNetworkRequest request = createRelayRequest(config);
    QNetworkReply* reply = m_networkManager->post(request, QByteArray());
    
    if (reply) {
        m_activeRelays[name] = reply;
        m_activeRelaysCount++;
        m_statistics["active_relays"] = m_activeRelaysCount;
        
        // Update stats
        RelayStats& stats = m_relayStats[name];
        stats.connected = false;
        stats.lastConnectTime = QDateTime::currentDateTime();
        stats.active = true;
        
        qCInfo(relayManager) << "Connecting relay:" << name << "->" << config.targetUrl;
    }
}

void RelayManager::disconnectRelay(const QString& name)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_activeRelays.contains(name)) {
        QNetworkReply* reply = m_activeRelays[name];
        if (reply && reply->isRunning()) {
            reply->abort();
        }
        m_activeRelays.remove(name);
        m_activeRelaysCount--;
        m_statistics["active_relays"] = m_activeRelaysCount;
        
        // Update stats
        RelayStats& stats = m_relayStats[name];
        stats.connected = false;
        stats.active = false;
        
        qCInfo(relayManager) << "Disconnected relay:" << name;
    }
}

void RelayManager::reconnectRelay(const QString& name)
{
    disconnectRelay(name);
    connectRelay(name);
}

void RelayManager::onStreamDataReceived(const QString& mountPoint, const QByteArray& data)
{
    if (!m_isRunning.load()) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Find relays for this mount point
    for (auto it = m_relayConfigs.begin(); it != m_relayConfigs.end(); ++it) {
        const RelayConfig& config = it.value();
        if (config.mountPoint == mountPoint && config.enabled) {
            sendRelayData(config.name, config, data);
        }
    }
}

void RelayManager::onRelayFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // Find relay name for this reply
    QString relayName;
    QMutexLocker locker(&m_mutex);
    for (auto it = m_activeRelays.begin(); it != m_activeRelays.end(); ++it) {
        if (it.value() == reply) {
            relayName = it.key();
            break;
        }
    }
    
    if (relayName.isEmpty()) {
        return;
    }
    
    handleRelayResponse(relayName, reply);
}

void RelayManager::onRelayError(QNetworkReply::NetworkError error)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    // Find relay name for this reply
    QString relayName;
    QMutexLocker locker(&m_mutex);
    for (auto it = m_activeRelays.begin(); it != m_activeRelays.end(); ++it) {
        if (it.value() == reply) {
            relayName = it.key();
            break;
        }
    }
    
    if (relayName.isEmpty()) {
        return;
    }
    
    // Update error stats
    RelayStats& stats = m_relayStats[relayName];
    stats.errorCount++;
    stats.lastError = reply->errorString();
    m_totalErrors++;
    m_statistics["total_errors"] = m_totalErrors;
    
    qCWarning(relayManager) << "Relay error:" << relayName << "->" << reply->errorString();
    
    // Schedule retry if configured
    if (m_relayConfigs.contains(relayName)) {
        const RelayConfig& config = m_relayConfigs[relayName];
        if (stats.retryCount < config.retryAttempts) {
            scheduleRetry(relayName, config);
        }
    }
    
    emit relayError(relayName, reply->errorString());
    emit statusChanged(m_statistics);
}

void RelayManager::onRetryTimer()
{
    // Check for retry timers that have expired
    QMutexLocker locker(&m_mutex);
    for (auto it = m_retryTimers.begin(); it != m_retryTimers.end(); ++it) {
        QTimer* timer = it.value();
        if (timer->isActive()) {
            continue;
        }
        
        QString relayName = it.key();
        if (m_relayConfigs.contains(relayName)) {
            const RelayConfig& config = m_relayConfigs[relayName];
            RelayStats& stats = m_relayStats[relayName];
            
            if (stats.retryCount < config.retryAttempts) {
                stats.retryCount++;
                qCInfo(relayManager) << "Retrying relay:" << relayName << "(attempt" << stats.retryCount << ")";
                connectRelay(relayName);
            }
        }
        
        timer->deleteLater();
        m_retryTimers.remove(relayName);
    }
}

void RelayManager::processRelay(const QString& name, const RelayConfig& config)
{
    // This would be called when stream data is available for relay
    // For now, we handle this in onStreamDataReceived
}

void RelayManager::sendRelayData(const QString& name, const RelayConfig& config, const QByteArray& data)
{
    if (!m_activeRelays.contains(name)) {
        return;
    }
    
    QNetworkReply* reply = m_activeRelays[name];
    if (!reply || !reply->isRunning()) {
        return;
    }
    
    // Update stats
    RelayStats& stats = m_relayStats[name];
    stats.bytesSent += data.size();
    stats.lastDataTime = QDateTime::currentDateTime();
    m_totalBytesSent += data.size();
    m_statistics["total_bytes_sent"] = m_totalBytesSent;
    
    emit relayDataSent(name, data.size());
    emit statusChanged(m_statistics);
}

void RelayManager::handleRelayResponse(const QString& name, QNetworkReply* reply)
{
    if (!m_relayConfigs.contains(name)) {
        return;
    }
    
    RelayStats& stats = m_relayStats[name];
    
    if (reply->error() == QNetworkReply::NoError) {
        // Success
        stats.connected = true;
        stats.retryCount = 0;
        stats.lastError.clear();
        
        qCInfo(relayManager) << "Relay connected:" << name;
        emit relayConnected(name);
    } else {
        // Error handled in onRelayError
        stats.connected = false;
    }
    
    // Remove from active relays
    m_activeRelays.remove(name);
    m_activeRelaysCount--;
    m_statistics["active_relays"] = m_activeRelaysCount;
    
    reply->deleteLater();
}

void RelayManager::scheduleRetry(const QString& name, const RelayConfig& config)
{
    if (m_retryTimers.contains(name)) {
        return; // Already scheduled
    }
    
    QTimer* retryTimer = new QTimer(this);
    retryTimer->setSingleShot(true);
    retryTimer->setInterval(config.retryDelay);
    
    m_retryTimers[name] = retryTimer;
    retryTimer->start();
    
    qCDebug(relayManager) << "Scheduled retry for relay:" << name << "in" << config.retryDelay << "ms";
}

QString RelayManager::buildRelayUrl(const RelayConfig& config) const
{
    QUrl url(config.targetUrl);
    if (!url.scheme().isEmpty()) {
        return config.targetUrl;
    }
    
    // Assume HTTP if no scheme specified
    return QString("http://%1").arg(config.targetUrl);
}

QNetworkRequest RelayManager::createRelayRequest(const RelayConfig& config) const
{
    QString url = buildRelayUrl(config);
    QNetworkRequest request(QUrl(url));
    
    // Set headers
    request.setHeader(QNetworkRequest::ContentTypeHeader, "audio/mpeg");
    request.setHeader(QNetworkRequest::UserAgentHeader, "LegacyStream/1.0");
    
    // Add authentication if provided
    if (!config.username.isEmpty() && !config.password.isEmpty()) {
        QString auth = QString("%1:%2").arg(config.username, config.password);
        request.setRawHeader("Authorization", QString("Basic %1").arg(auth.toUtf8().toBase64()).toUtf8());
    }
    
    return request;
}

bool RelayManager::isValidRelayConfig(const RelayConfig& config) const
{
    return !config.name.isEmpty() && 
           !config.targetUrl.isEmpty() && 
           !config.mountPoint.isEmpty();
}

void RelayManager::updateRelayStats(const QString& name, const RelayStats& stats)
{
    m_relayStats[name] = stats;
}

} // namespace LegacyStream 