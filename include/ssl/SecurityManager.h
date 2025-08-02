#ifndef SECURITYMANAGER_H
#define SECURITYMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QMutex>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>

namespace LegacyStream {

/**
 * @brief Rate limiting configuration
 */
struct RateLimitConfig
{
    int maxRequestsPerMinute = 1000;
    int maxConnectionsPerIP = 100;
    int burstLimit = 50;
    int windowSize = 60; // seconds
    bool enabled = true;
};

/**
 * @brief DDoS protection configuration
 */
struct DDoSProtectionConfig
{
    int maxConnectionsPerSecond = 100;
    int connectionTimeout = 30;
    int requestTimeout = 60;
    int suspiciousThreshold = 10;
    int blockDuration = 300; // seconds
    bool enabled = true;
    QStringList allowedIPs;
    QStringList blockedIPs;
};

/**
 * @brief Security statistics
 */
struct SecurityStats
{
    int totalRequests = 0;
    int blockedRequests = 0;
    int rateLimitedRequests = 0;
    int ddosBlockedRequests = 0;
    QMap<QString, int> requestsByIP;
    QMap<QString, int> blockedByIP;
    QDateTime lastUpdate;
};

/**
 * @brief SecurityManager for advanced security features
 * 
 * Provides rate limiting, DDoS protection, IP filtering, and security monitoring.
 */
class SecurityManager : public QObject
{
    Q_OBJECT

public:
    explicit SecurityManager(QObject* parent = nullptr);
    ~SecurityManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool start();
    void stop();

    // Rate limiting
    void setRateLimitConfig(const RateLimitConfig& config);
    bool checkRateLimit(const QHostAddress& clientIP);
    void recordRequest(const QHostAddress& clientIP);

    // DDoS protection
    void setDDoSProtectionConfig(const DDoSProtectionConfig& config);
    bool checkDDoSProtection(const QHostAddress& clientIP);
    void blockIP(const QHostAddress& clientIP, int duration = 300);
    void unblockIP(const QHostAddress& clientIP);

    // IP filtering
    void addAllowedIP(const QString& ip);
    void removeAllowedIP(const QString& ip);
    void addBlockedIP(const QString& ip);
    void removeBlockedIP(const QString& ip);
    bool isIPAllowed(const QHostAddress& clientIP) const;
    bool isIPBlocked(const QHostAddress& clientIP) const;

    // Security monitoring
    SecurityStats getSecurityStats() const;
    QJsonObject getSecurityStatsJson() const;
    void clearStats();

    // Configuration
    void setSecurityEnabled(bool enabled);
    bool isSecurityEnabled() const;

signals:
    void ipBlocked(const QHostAddress& ip, const QString& reason);
    void ipUnblocked(const QHostAddress& ip);
    void rateLimitExceeded(const QHostAddress& ip);
    void ddosAttackDetected(const QHostAddress& ip);
    void securityAlert(const QString& alert);
    void statsUpdated(const SecurityStats& stats);

private slots:
    void onCleanupTimer();
    void onStatsUpdateTimer();

private:
    // Core functionality
    void updateRequestCount(const QHostAddress& clientIP);
    void checkForDDoSAttack(const QHostAddress& clientIP);
    void cleanupExpiredBlocks();
    void updateSecurityStats();

    // Utility functions
    QString ipToString(const QHostAddress& ip) const;
    QHostAddress stringToIP(const QString& ip) const;
    bool isValidIP(const QString& ip) const;
    int getCurrentTimestamp() const;

    // Configuration
    RateLimitConfig m_rateLimitConfig;
    DDoSProtectionConfig m_ddosConfig;
    bool m_securityEnabled = true;

    // State management
    QAtomicInt m_isRunning = 0;
    QTimer* m_cleanupTimer = nullptr;
    QTimer* m_statsUpdateTimer = nullptr;
    QMutex m_mutex;

    // Rate limiting tracking
    QMap<QString, QList<int>> m_requestTimestamps; // IP -> list of timestamps
    QMap<QString, int> m_connectionCounts; // IP -> connection count

    // DDoS protection tracking
    QMap<QString, QList<int>> m_connectionTimestamps; // IP -> list of connection timestamps
    QMap<QString, int> m_blockedIPs; // IP -> block expiration timestamp
    QMap<QString, int> m_suspiciousIPs; // IP -> suspicious activity count

    // IP filtering
    QSet<QString> m_allowedIPs;
    QSet<QString> m_blockedIPs;

    // Statistics
    SecurityStats m_securityStats;
    QDateTime m_startTime;

    Q_DISABLE_COPY(SecurityManager)
};

} // namespace LegacyStream

#endif // SECURITYMANAGER_H 