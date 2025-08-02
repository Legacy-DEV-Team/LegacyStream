#ifndef ADVANCEDCACHEMANAGER_H
#define ADVANCEDCACHEMANAGER_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QCache>
#include <QMutex>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <functional>

namespace LegacyStream {

/**
 * @brief Cache entry with metadata
 */
struct CacheEntry
{
    QByteArray data;
    QDateTime created;
    QDateTime lastAccessed;
    QDateTime expires;
    int accessCount = 0;
    size_t size = 0;
    QString type;
    bool isCompressed = false;
    double priority = 1.0;
    
    // Cache statistics
    qint64 hitCount = 0;
    qint64 missCount = 0;
    double hitRate = 0.0;
};

/**
 * @brief Cache policy configuration
 */
struct CachePolicy
{
    size_t maxSize = 100 * 1024 * 1024; // 100MB default
    size_t maxEntries = 10000;
    int maxAge = 3600; // 1 hour default
    bool enableCompression = true;
    bool enableLRU = true;
    bool enableLFU = false;
    double compressionThreshold = 1024; // Compress entries larger than 1KB
    int cleanupInterval = 300; // 5 minutes
    QString evictionPolicy = "LRU"; // LRU, LFU, FIFO, Random
};

/**
 * @brief Cache statistics
 */
struct CacheStatistics
{
    size_t totalEntries = 0;
    size_t totalSize = 0;
    size_t hitCount = 0;
    size_t missCount = 0;
    size_t evictionCount = 0;
    size_t compressionCount = 0;
    double hitRate = 0.0;
    double averageAccessTime = 0.0;
    QDateTime lastCleanup;
    QMap<QString, size_t> entriesByType;
    QMap<QString, double> hitRatesByType;
};

/**
 * @brief Advanced cache manager for intelligent data caching
 * 
 * Provides sophisticated caching mechanisms with multiple eviction policies,
 * compression, statistics tracking, and adaptive cache sizing.
 */
class AdvancedCacheManager : public QObject
{
    Q_OBJECT

public:
    explicit AdvancedCacheManager(QObject* parent = nullptr);
    ~AdvancedCacheManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Cache operations
    bool put(const QString& key, const QByteArray& data, const QString& type = "general");
    QByteArray get(const QString& key);
    bool remove(const QString& key);
    bool contains(const QString& key) const;
    void clear();
    void clearByType(const QString& type);

    // Cache configuration
    void setCachePolicy(const CachePolicy& policy);
    CachePolicy getCachePolicy() const;
    void setMaxSize(size_t maxSize);
    void setMaxEntries(size_t maxEntries);
    void setMaxAge(int maxAge);
    void enableCompression(bool enabled);
    void setEvictionPolicy(const QString& policy);

    // Advanced features
    void setCompressionThreshold(size_t threshold);
    void setCleanupInterval(int interval);
    void enableLRU(bool enabled);
    void enableLFU(bool enabled);
    void setPriority(const QString& key, double priority);

    // Cache statistics
    CacheStatistics getStatistics() const;
    QJsonObject getStatisticsJson() const;
    void resetStatistics();
    void exportStatistics(const QString& filePath) const;

    // Cache management
    void cleanup();
    void optimize();
    void defragment();
    void preload(const QStringList& keys);
    void warmup(const QString& type);

    // Utility functions
    size_t getSize() const;
    size_t getEntryCount() const;
    QStringList getKeys() const;
    QStringList getKeysByType(const QString& type) const;
    bool isFull() const;
    double getUtilization() const;

signals:
    void cacheHit(const QString& key);
    void cacheMiss(const QString& key);
    void cacheEviction(const QString& key);
    void cacheCompression(const QString& key);
    void statisticsUpdated(const CacheStatistics& stats);
    void cacheFull();
    void cacheCleanup();

public slots:
    void onCleanupTimer();
    void onOptimizationTimer();
    void onStatisticsTimer();

private slots:
    void onMemoryPressure();
    void onLowMemory();

private:
    // Core cache operations
    void addEntry(const QString& key, const QByteArray& data, const QString& type);
    void removeEntry(const QString& key);
    void updateAccess(const QString& key);
    void evictEntries(size_t requiredSpace);

    // Eviction policies
    void evictLRU(size_t count);
    void evictLFU(size_t count);
    void evictFIFO(size_t count);
    void evictRandom(size_t count);
    void evictByPriority(size_t count);

    // Compression
    QByteArray compressData(const QByteArray& data);
    QByteArray decompressData(const QByteArray& data);
    bool shouldCompress(const QByteArray& data) const;

    // Statistics
    void updateStatistics();
    void calculateHitRate();
    void updateAccessTime(const QString& key);

    // Utility functions
    size_t calculateEntrySize(const CacheEntry& entry) const;
    bool isExpired(const CacheEntry& entry) const;
    QString generateCacheKey(const QString& baseKey, const QString& type) const;
    void loadCacheFromDisk();
    void saveCacheToDisk();

    // Cache storage
    QMap<QString, CacheEntry> m_cache;
    QMap<QString, QStringList> m_keysByType;
    QList<QString> m_accessOrder; // For LRU
    QMap<QString, int> m_accessCount; // For LFU

    // Configuration
    CachePolicy m_policy;
    CacheStatistics m_statistics;

    // Timers
    QTimer* m_cleanupTimer = nullptr;
    QTimer* m_optimizationTimer = nullptr;
    QTimer* m_statisticsTimer = nullptr;

    // State management
    QMutex m_mutex;
    bool m_isInitialized = false;
    bool m_compressionEnabled = true;
    size_t m_compressionThreshold = 1024;

    // Performance tracking
    QMap<QString, QDateTime> m_lastAccess;
    QMap<QString, double> m_accessTimes;
    qint64 m_totalAccessTime = 0;
    int m_totalAccessCount = 0;

    Q_DISABLE_COPY(AdvancedCacheManager)
};

} // namespace LegacyStream

#endif // ADVANCEDCACHEMANAGER_H 