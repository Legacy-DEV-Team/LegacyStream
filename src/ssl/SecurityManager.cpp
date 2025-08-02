#include "ssl/SecurityManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(securityManager, "securityManager")

namespace LegacyStream {

SecurityManager::SecurityManager(QObject* parent)
    : QObject(parent)
    , m_cleanupTimer(new QTimer(this))
    , m_statsUpdateTimer(new QTimer(this))
{
    qCDebug(securityManager) << "SecurityManager created";
    
    // Set up timers
    m_cleanupTimer->setSingleShot(false);
    m_cleanupTimer->setInterval(60000); // Clean up every minute
    m_statsUpdateTimer->setSingleShot(false);
    m_statsUpdateTimer->setInterval(5000); // Update stats every 5 seconds
    
    // Connect signals
    connect(m_cleanupTimer, &QTimer::timeout, this, &SecurityManager::onCleanupTimer);
    connect(m_statsUpdateTimer, &QTimer::timeout, this, &SecurityManager::onStatsUpdateTimer);
}

SecurityManager::~SecurityManager()
{
    shutdown();
}

bool SecurityManager::initialize()
{
    qCDebug(securityManager) << "Initializing SecurityManager";
    
    // Initialize statistics
    m_securityStats.lastUpdate = QDateTime::currentDateTime();
    
    qDebug() << "SecurityManager initialized successfully";
    return true;
}

void SecurityManager::shutdown()
{
    if (m_isRunning.load()) {
        stop();
    }
    
    // Stop timers
    m_cleanupTimer->stop();
    m_statsUpdateTimer->stop();
    
    // Clear tracking data
    QMutexLocker locker(&m_mutex);
    m_requestTimestamps.clear();
    m_connectionCounts.clear();
    m_connectionTimestamps.clear();
    m_blockedIPs.clear();
    m_suspiciousIPs.clear();
    
    qDebug() << "SecurityManager shutdown complete";
}

bool SecurityManager::start()
{
    if (m_isRunning.load()) {
        qCWarning(securityManager) << "SecurityManager already running";
        return true;
    }
    
    qDebug() << "Starting SecurityManager";
    
    // Start timers
    m_cleanupTimer->start();
    m_statsUpdateTimer->start();
    
    m_isRunning.store(true);
    m_startTime = QDateTime::currentDateTime();
    
    qDebug() << "SecurityManager started successfully";
    return true;
}

void SecurityManager::stop()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    qDebug() << "Stopping SecurityManager";
    
    // Stop timers
    m_cleanupTimer->stop();
    m_statsUpdateTimer->stop();
    
    m_isRunning.store(false);
    
    qDebug() << "SecurityManager stopped";
}

void SecurityManager::setRateLimitConfig(const RateLimitConfig& config)
{
    QMutexLocker locker(&m_mutex);
    m_rateLimitConfig = config;
    qDebug() << "Rate limit config updated";
}

bool SecurityManager::checkRateLimit(const QHostAddress& clientIP)
{
    if (!m_securityEnabled || !m_rateLimitConfig.enabled) {
        return true;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString ipStr = ipToString(clientIP);
    int currentTime = getCurrentTimestamp();
    
    // Check if IP is blocked
    if (m_blockedIPs.contains(ipStr)) {
        int blockExpiry = m_blockedIPs[ipStr];
        if (currentTime < blockExpiry) {
            m_securityStats.blockedRequests++;
            return false;
        } else {
            // Block expired
            m_blockedIPs.remove(ipStr);
            emit ipUnblocked(clientIP);
        }
    }
    
    // Check rate limit
    QList<int>& timestamps = m_requestTimestamps[ipStr];
    
    // Remove old timestamps outside the window
    while (!timestamps.isEmpty() && timestamps.first() < currentTime - m_rateLimitConfig.windowSize) {
        timestamps.removeFirst();
    }
    
    // Check if rate limit exceeded
    if (timestamps.size() >= m_rateLimitConfig.maxRequestsPerMinute) {
        m_securityStats.rateLimitedRequests++;
        emit rateLimitExceeded(clientIP);
        return false;
    }
    
    // Add current timestamp
    timestamps.append(currentTime);
    
    return true;
}

void SecurityManager::recordRequest(const QHostAddress& clientIP)
{
    if (!m_securityEnabled) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString ipStr = ipToString(clientIP);
    
    // Update request count
    updateRequestCount(clientIP);
    
    // Check for DDoS attack
    checkForDDoSAttack(clientIP);
    
    // Update statistics
    m_securityStats.totalRequests++;
    m_securityStats.requestsByIP[ipStr]++;
}

void SecurityManager::setDDoSProtectionConfig(const DDoSProtectionConfig& config)
{
    QMutexLocker locker(&m_mutex);
    m_ddosConfig = config;
    
    // Update IP lists
    m_allowedIPs.clear();
    m_blockedIPs.clear();
    
    for (const QString& ip : config.allowedIPs) {
        if (isValidIP(ip)) {
            m_allowedIPs.insert(ip);
        }
    }
    
    for (const QString& ip : config.blockedIPs) {
        if (isValidIP(ip)) {
            m_blockedIPs.insert(ip);
        }
    }
    
    qDebug() << "DDoS protection config updated";
}

bool SecurityManager::checkDDoSProtection(const QHostAddress& clientIP)
{
    if (!m_securityEnabled || !m_ddosConfig.enabled) {
        return true;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString ipStr = ipToString(clientIP);
    
    // Check if IP is explicitly allowed
    if (m_allowedIPs.contains(ipStr)) {
        return true;
    }
    
    // Check if IP is explicitly blocked
    if (m_blockedIPs.contains(ipStr)) {
        m_securityStats.blockedRequests++;
        return false;
    }
    
    // Check connection rate
    QList<int>& timestamps = m_connectionTimestamps[ipStr];
    int currentTime = getCurrentTimestamp();
    
    // Remove old timestamps (older than 1 second)
    while (!timestamps.isEmpty() && timestamps.first() < currentTime - 1) {
        timestamps.removeFirst();
    }
    
    // Check if connection rate exceeded
    if (timestamps.size() >= m_ddosConfig.maxConnectionsPerSecond) {
        m_securityStats.ddosBlockedRequests++;
        emit ddosAttackDetected(clientIP);
        blockIP(clientIP, m_ddosConfig.blockDuration);
        return false;
    }
    
    // Add current timestamp
    timestamps.append(currentTime);
    
    return true;
}

void SecurityManager::blockIP(const QHostAddress& clientIP, int duration)
{
    QMutexLocker locker(&m_mutex);
    
    QString ipStr = ipToString(clientIP);
    int expiryTime = getCurrentTimestamp() + duration;
    
    m_blockedIPs[ipStr] = expiryTime;
    
    qDebug() << "IP blocked:" << ipStr << "for" << duration << "seconds";
    emit ipBlocked(clientIP, "Rate limit or DDoS protection");
}

void SecurityManager::unblockIP(const QHostAddress& clientIP)
{
    QMutexLocker locker(&m_mutex);
    
    QString ipStr = ipToString(clientIP);
    m_blockedIPs.remove(ipStr);
    
    qDebug() << "IP unblocked:" << ipStr;
    emit ipUnblocked(clientIP);
}

void SecurityManager::addAllowedIP(const QString& ip)
{
    if (isValidIP(ip)) {
        QMutexLocker locker(&m_mutex);
        m_allowedIPs.insert(ip);
        qDebug() << "Added allowed IP:" << ip;
    }
}

void SecurityManager::removeAllowedIP(const QString& ip)
{
    QMutexLocker locker(&m_mutex);
    m_allowedIPs.remove(ip);
    qDebug() << "Removed allowed IP:" << ip;
}

void SecurityManager::addBlockedIP(const QString& ip)
{
    if (isValidIP(ip)) {
        QMutexLocker locker(&m_mutex);
        m_blockedIPs.insert(ip);
        qDebug() << "Added blocked IP:" << ip;
    }
}

void SecurityManager::removeBlockedIP(const QString& ip)
{
    QMutexLocker locker(&m_mutex);
    m_blockedIPs.remove(ip);
    qDebug() << "Removed blocked IP:" << ip;
}

bool SecurityManager::isIPAllowed(const QHostAddress& clientIP) const
{
    QMutexLocker locker(&m_mutex);
    QString ipStr = ipToString(clientIP);
    return m_allowedIPs.contains(ipStr);
}

bool SecurityManager::isIPBlocked(const QHostAddress& clientIP) const
{
    QMutexLocker locker(&m_mutex);
    QString ipStr = ipToString(clientIP);
    return m_blockedIPs.contains(ipStr);
}

SecurityStats SecurityManager::getSecurityStats() const
{
    QMutexLocker locker(&m_mutex);
    return m_securityStats;
}

QJsonObject SecurityManager::getSecurityStatsJson() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject stats;
    stats["total_requests"] = m_securityStats.totalRequests;
    stats["blocked_requests"] = m_securityStats.blockedRequests;
    stats["rate_limited_requests"] = m_securityStats.rateLimitedRequests;
    stats["ddos_blocked_requests"] = m_securityStats.ddosBlockedRequests;
    stats["last_update"] = m_securityStats.lastUpdate.toString(Qt::ISODate);
    
    QJsonObject requestsByIP;
    for (auto it = m_securityStats.requestsByIP.begin(); it != m_securityStats.requestsByIP.end(); ++it) {
        requestsByIP[it.key()] = it.value();
    }
    stats["requests_by_ip"] = requestsByIP;
    
    QJsonObject blockedByIP;
    for (auto it = m_securityStats.blockedByIP.begin(); it != m_securityStats.blockedByIP.end(); ++it) {
        blockedByIP[it.key()] = it.value();
    }
    stats["blocked_by_ip"] = blockedByIP;
    
    return stats;
}

void SecurityManager::clearStats()
{
    QMutexLocker locker(&m_mutex);
    m_securityStats = SecurityStats{};
    m_securityStats.lastUpdate = QDateTime::currentDateTime();
    qDebug() << "Security statistics cleared";
}

void SecurityManager::setSecurityEnabled(bool enabled)
{
    m_securityEnabled = enabled;
    qDebug() << "Security" << (enabled ? "enabled" : "disabled");
}

bool SecurityManager::isSecurityEnabled() const
{
    return m_securityEnabled;
}

void SecurityManager::onCleanupTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    cleanupExpiredBlocks();
}

void SecurityManager::onStatsUpdateTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    updateSecurityStats();
}

void SecurityManager::updateRequestCount(const QHostAddress& clientIP)
{
    QString ipStr = ipToString(clientIP);
    int currentTime = getCurrentTimestamp();
    
    // Add current timestamp to request history
    m_requestTimestamps[ipStr].append(currentTime);
    
    // Remove old timestamps outside the window
    QList<int>& timestamps = m_requestTimestamps[ipStr];
    while (!timestamps.isEmpty() && timestamps.first() < currentTime - m_rateLimitConfig.windowSize) {
        timestamps.removeFirst();
    }
    
    // Update connection count
    m_connectionCounts[ipStr]++;
    
    // Check for DDoS attack patterns
    checkForDDoSAttack(clientIP);
}

void SecurityManager::checkForDDoSAttack(const QHostAddress& clientIP)
{
    QString ipStr = ipToString(clientIP);
    int currentTime = getCurrentTimestamp();
    
    // Add current timestamp to connection history
    m_connectionTimestamps[ipStr].append(currentTime);
    
    // Remove old timestamps outside the window
    QList<int>& timestamps = m_connectionTimestamps[ipStr];
    while (!timestamps.isEmpty() && timestamps.first() < currentTime - m_ddosConfig.connectionTimeout) {
        timestamps.removeFirst();
    }
    
    // Check connection rate
    if (timestamps.size() > m_ddosConfig.maxConnectionsPerSecond) {
        qCWarning(securityManager) << "DDoS attack detected from IP:" << ipStr;
        emit ddosAttackDetected(clientIP);
        blockIP(clientIP, m_ddosConfig.blockDuration);
        return;
    }
    
    // Check for suspicious patterns
    int suspiciousCount = m_suspiciousIPs.value(ipStr, 0);
    if (timestamps.size() > m_ddosConfig.suspiciousThreshold) {
        suspiciousCount++;
        m_suspiciousIPs[ipStr] = suspiciousCount;
        
        if (suspiciousCount >= 5) { // 5 suspicious events trigger block
            qCWarning(securityManager) << "Suspicious activity detected from IP:" << ipStr;
            emit securityAlert(QString("Suspicious activity detected from IP: %1").arg(ipStr));
            blockIP(clientIP, m_ddosConfig.blockDuration);
        }
    } else {
        // Reset suspicious count if behavior is normal
        m_suspiciousIPs[ipStr] = qMax(0, suspiciousCount - 1);
    }
}

void SecurityManager::cleanupExpiredBlocks()
{
    QMutexLocker locker(&m_mutex);
    
    int currentTime = getCurrentTimestamp();
    QStringList expiredIPs;
    
    // Clean up blocked IPs
    for (auto it = m_blockedIPs.begin(); it != m_blockedIPs.end(); ++it) {
        if (it.value() < currentTime) {
            expiredIPs.append(it.key());
        }
    }
    
    for (const QString& ip : expiredIPs) {
        m_blockedIPs.remove(ip);
        QHostAddress clientIP = stringToIP(ip);
        emit ipUnblocked(clientIP);
        qCDebug(securityManager) << "IP block expired:" << ip;
    }
    
    // Clean up old request timestamps
    for (auto it = m_requestTimestamps.begin(); it != m_requestTimestamps.end(); ++it) {
        QList<int>& timestamps = it.value();
        while (!timestamps.isEmpty() && timestamps.first() < currentTime - m_rateLimitConfig.windowSize) {
            timestamps.removeFirst();
        }
        
        // Remove empty entries
        if (timestamps.isEmpty()) {
            it = m_requestTimestamps.erase(it);
        }
    }
    
    // Clean up old connection timestamps
    for (auto it = m_connectionTimestamps.begin(); it != m_connectionTimestamps.end(); ++it) {
        QList<int>& timestamps = it.value();
        while (!timestamps.isEmpty() && timestamps.first() < currentTime - m_ddosConfig.connectionTimeout) {
            timestamps.removeFirst();
        }
        
        // Remove empty entries
        if (timestamps.isEmpty()) {
            it = m_connectionTimestamps.erase(it);
        }
    }
}

void SecurityManager::updateSecurityStats()
{
    QMutexLocker locker(&m_mutex);
    
    // Update total requests
    m_securityStats.totalRequests++;
    
    // Update requests by IP
    for (auto it = m_requestTimestamps.begin(); it != m_requestTimestamps.end(); ++it) {
        m_securityStats.requestsByIP[it.key()] = it.value().size();
    }
    
    // Update blocked requests by IP
    for (auto it = m_blockedIPs.begin(); it != m_blockedIPs.end(); ++it) {
        m_securityStats.blockedByIP[it.key()]++;
    }
    
    m_securityStats.lastUpdate = QDateTime::currentDateTime();
    emit statsUpdated(m_securityStats);
}

QString SecurityManager::ipToString(const QHostAddress& ip) const
{
    return ip.toString();
}

QHostAddress SecurityManager::stringToIP(const QString& ip) const
{
    return QHostAddress(ip);
}

bool SecurityManager::isValidIP(const QString& ip) const
{
    QHostAddress testIP(ip);
    return !testIP.isNull();
}

int SecurityManager::getCurrentTimestamp() const
{
    return QDateTime::currentSecsSinceEpoch();
}

} // namespace LegacyStream 
