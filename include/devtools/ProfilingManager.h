#ifndef PROFILINGMANAGER_H
#define PROFILINGMANAGER_H

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
 * @brief Performance profile data
 */
struct PerformanceProfile
{
    QString function;
    QString file;
    int line = 0;
    qint64 totalTime = 0;
    qint64 averageTime = 0;
    qint64 minTime = 0;
    qint64 maxTime = 0;
    int callCount = 0;
    double cpuUsage = 0.0;
    qint64 memoryUsage = 0;
    qint64 peakMemoryUsage = 0;
    QDateTime firstCall;
    QDateTime lastCall;
    QList<qint64> callTimes;
    QString parentFunction;
    QList<QString> childFunctions;
};

/**
 * @brief Memory allocation profile
 */
struct MemoryProfile
{
    QString allocationSite;
    QString file;
    int line = 0;
    size_t totalAllocated = 0;
    size_t currentAllocated = 0;
    size_t peakAllocated = 0;
    int allocationCount = 0;
    int deallocationCount = 0;
    double fragmentation = 0.0;
    QDateTime firstAllocation;
    QDateTime lastAllocation;
    QList<size_t> allocationSizes;
    QString function;
    QString stackTrace;
};

/**
 * @brief Profiling configuration
 */
struct ProfilingConfig
{
    QString name;
    bool enableCPUProfiling = true;
    bool enableMemoryProfiling = true;
    bool enableNetworkProfiling = true;
    bool enableDiskProfiling = true;
    bool enableRealTimeProfiling = true;
    bool enableHistoricalProfiling = true;
    int samplingInterval = 1000; // 1 second
    int maxProfileDepth = 100;
    int maxMemoryTracking = 1000;
    bool enableHotspotDetection = true;
    bool enableBottleneckAnalysis = true;
    bool enableOptimizationSuggestions = true;
    bool enableLogging = true;
};

/**
 * @brief Profiling session statistics
 */
struct ProfilingStats
{
    int totalFunctions = 0;
    int profiledFunctions = 0;
    int memoryAllocations = 0;
    int memoryDeallocations = 0;
    double averageCPUUsage = 0.0;
    double averageMemoryUsage = 0.0;
    qint64 totalProfilingTime = 0;
    QDateTime sessionStart;
    QDateTime lastProfile;
    QMap<QString, int> functionsByFile;
    QMap<QString, double> cpuUsageByFunction;
    QMap<QString, qint64> memoryUsageByFunction;
};

/**
 * @brief Performance hotspot
 */
struct PerformanceHotspot
{
    QString function;
    QString file;
    int line = 0;
    double cpuUsage = 0.0;
    qint64 memoryUsage = 0;
    qint64 executionTime = 0;
    int callFrequency = 0;
    QString severity; // "low", "medium", "high", "critical"
    QString description;
    QString suggestion;
    QDateTime detected;
};

/**
 * @brief Profiling manager for comprehensive performance analysis
 *
 * Provides sophisticated performance profiling tools including CPU profiling,
 * memory profiling, network profiling, and performance optimization recommendations.
 */
class ProfilingManager : public QObject
{
    Q_OBJECT

public:
    explicit ProfilingManager(QObject* parent = nullptr);
    ~ProfilingManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Profiling session management
    bool createProfilingSession(const QString& name, const ProfilingConfig& config);
    void destroyProfilingSession(const QString& name);
    bool profilingSessionExists(const QString& name) const;
    QStringList getProfilingSessionNames() const;

    // CPU profiling
    void startCPUProfiling(const QString& sessionName);
    void stopCPUProfiling(const QString& sessionName);
    void profileFunction(const QString& sessionName, const QString& function, const QString& file, int line);
    void endFunctionProfile(const QString& sessionName, const QString& function);
    QList<PerformanceProfile> getCPUProfiles(const QString& sessionName) const;

    // Memory profiling
    void startMemoryProfiling(const QString& sessionName);
    void stopMemoryProfiling(const QString& sessionName);
    void trackMemoryAllocation(const QString& sessionName, const QString& site, size_t size, const QString& function);
    void trackMemoryDeallocation(const QString& sessionName, const QString& site, size_t size);
    QList<MemoryProfile> getMemoryProfiles(const QString& sessionName) const;

    // Network profiling
    void startNetworkProfiling(const QString& sessionName);
    void stopNetworkProfiling(const QString& sessionName);
    void profileNetworkCall(const QString& sessionName, const QString& endpoint, qint64 bytes, qint64 duration);
    QJsonObject getNetworkProfiles(const QString& sessionName) const;

    // Disk profiling
    void startDiskProfiling(const QString& sessionName);
    void stopDiskProfiling(const QString& sessionName);
    void profileDiskOperation(const QString& sessionName, const QString& operation, const QString& file, qint64 bytes, qint64 duration);
    QJsonObject getDiskProfiles(const QString& sessionName) const;

    // Real-time profiling
    void enableRealTimeProfiling(const QString& sessionName, bool enabled);
    void setSamplingInterval(const QString& sessionName, int interval);
    void startRealTimeProfiling(const QString& sessionName);
    void stopRealTimeProfiling(const QString& sessionName);

    // Hotspot detection
    void enableHotspotDetection(const QString& sessionName, bool enabled);
    QList<PerformanceHotspot> detectHotspots(const QString& sessionName);
    void analyzePerformanceBottlenecks(const QString& sessionName);
    void generateOptimizationSuggestions(const QString& sessionName);

    // Configuration
    void setProfilingConfig(const QString& name, const ProfilingConfig& config);
    ProfilingConfig getProfilingConfig(const QString& name) const;
    void setSamplingInterval(const QString& name, int interval);
    void setMaxProfileDepth(const QString& name, int depth);

    // Statistics and monitoring
    ProfilingStats getProfilingSessionStats(const QString& name) const;
    QJsonObject getAllProfilingSessionStatsJson() const;
    void resetProfilingSessionStats(const QString& name);
    void exportProfilingSessionStats(const QString& filePath) const;

    // Advanced features
    void enableLogging(const QString& name, bool enabled);
    void setLogLevel(const QString& name, const QString& level);
    void enableHistoricalProfiling(const QString& name, bool enabled);
    void setHistoricalDataRetention(const QString& name, int days);

    // Utility functions
    bool isProfilingActive(const QString& sessionName) const;
    double getOverallPerformance(const QString& sessionName) const;
    QStringList getTopFunctions(const QString& sessionName, int count = 10) const;
    QStringList getMemoryLeaks(const QString& sessionName) const;

signals:
    void profilingStarted(const QString& sessionName);
    void profilingStopped(const QString& sessionName);
    void hotspotDetected(const QString& sessionName, const PerformanceHotspot& hotspot);
    void performanceBottleneck(const QString& sessionName, const QString& function);
    void optimizationSuggestion(const QString& sessionName, const QString& suggestion);
    void statisticsUpdated(const QString& name, const ProfilingStats& stats);

public slots:
    void onProfilingTimer();
    void onHotspotDetectionTimer();
    void onStatisticsTimer();

private slots:
    void onFunctionEntered(const QString& function);
    void onFunctionExited(const QString& function);
    void onMemoryAllocated(const QString& site, size_t size);
    void onMemoryFreed(const QString& site, size_t size);

private:
    // Profiling session management
    struct ProfilingSession
    {
        ProfilingConfig config;
        ProfilingStats stats;
        QMap<QString, PerformanceProfile> cpuProfiles;
        QMap<QString, MemoryProfile> memoryProfiles;
        QJsonObject networkProfiles;
        QJsonObject diskProfiles;
        QList<PerformanceHotspot> hotspots;
        QMap<QString, qint64> functionStartTimes;
        QMap<QString, QList<qint64>> functionCallStacks;
        QMutex mutex;
        QTimer* profilingTimer = nullptr;
        QTimer* hotspotDetectionTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        bool isActive = true;
        bool cpuProfilingActive = false;
        bool memoryProfilingActive = false;
        bool networkProfilingActive = false;
        bool diskProfilingActive = false;
    };

    // Core profiling operations
    void performCPUProfiling(ProfilingSession& session);
    void performMemoryProfiling(ProfilingSession& session);
    void performNetworkProfiling(ProfilingSession& session);
    void performDiskProfiling(ProfilingSession& session);

    // CPU profiling functions
    void startFunctionProfile(ProfilingSession& session, const QString& function, const QString& file, int line);
    void endFunctionProfile(ProfilingSession& session, const QString& function);
    void updateFunctionProfile(ProfilingSession& session, const QString& function, qint64 duration);
    void calculateFunctionMetrics(ProfilingSession& session, const QString& function);

    // Memory profiling functions
    void trackMemoryAllocation(ProfilingSession& session, const QString& site, size_t size, const QString& function);
    void trackMemoryDeallocation(ProfilingSession& session, const QString& site, size_t size);
    void updateMemoryProfile(ProfilingSession& session, const QString& site, size_t size, bool isAllocation);
    void calculateMemoryMetrics(ProfilingSession& session, const QString& site);

    // Network profiling functions
    void trackNetworkCall(ProfilingSession& session, const QString& endpoint, qint64 bytes, qint64 duration);
    void updateNetworkProfile(ProfilingSession& session, const QString& endpoint, qint64 bytes, qint64 duration);
    void calculateNetworkMetrics(ProfilingSession& session, const QString& endpoint);

    // Disk profiling functions
    void trackDiskOperation(ProfilingSession& session, const QString& operation, const QString& file, qint64 bytes, qint64 duration);
    void updateDiskProfile(ProfilingSession& session, const QString& operation, const QString& file, qint64 bytes, qint64 duration);
    void calculateDiskMetrics(ProfilingSession& session, const QString& operation);

    // Hotspot detection
    void detectPerformanceHotspots(ProfilingSession& session);
    void analyzeBottlenecks(ProfilingSession& session);
    void generateOptimizationSuggestions(ProfilingSession& session);
    PerformanceHotspot createHotspot(const QString& function, const QString& file, int line, double cpuUsage, qint64 memoryUsage);

    // Statistics
    void updateProfilingSessionStatistics(const QString& name);
    void calculateProfilingSessionMetrics(ProfilingSession& session);
    void logProfilingSessionEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateSessionId() const;
    QString generateProfileId() const;
    bool isValidProfile(const PerformanceProfile& profile) const;
    void loadProfilingSessionFromDisk(const QString& name);
    void saveProfilingSessionToDisk(const QString& name);

    // Profiling session storage
    QMap<QString, std::unique_ptr<ProfilingSession>> m_profilingSessions;

    // Timers
    QTimer* m_globalProfilingTimer = nullptr;
    QTimer* m_globalHotspotDetectionTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_loggingEnabled = true;
    bool m_hotspotDetectionEnabled = true;
    bool m_optimizationSuggestionsEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastProfile;
    QMap<QString, QDateTime> m_lastHotspot;
    QMap<QString, double> m_profilingOverhead;

    // Platform-specific profiling
#ifdef _WIN32
    void* m_performanceCounter = nullptr;
    void* m_memoryCounter = nullptr;
    void* m_networkCounter = nullptr;
    void* m_diskCounter = nullptr;
#endif

    Q_DISABLE_COPY(ProfilingManager)
};

} // namespace LegacyStream

#endif // PROFILINGMANAGER_H 