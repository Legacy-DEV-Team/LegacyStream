#pragma once

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QReadWriteLock>
#include <memory>
#include <vector>
#include <atomic>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

namespace LegacyStream {

class MemoryPool;
class IOCPOptimizer;
class ResourceMonitor;

class PerformanceManager : public QObject
{
    Q_OBJECT

public:
    static PerformanceManager& instance();
    
    bool initialize();
    void shutdown();
    
    // Memory management
    void* allocateBuffer(size_t size);
    void deallocateBuffer(void* ptr);
    void setBufferSize(size_t size);
    size_t getBufferSize() const { return m_bufferSize; }
    
    // Thread management
    void setIOThreads(int count);
    void setWorkerThreads(int count);
    int getIOThreads() const { return m_ioThreads; }
    int getWorkerThreads() const { return m_workerThreads; }
    
    // Performance monitoring
    struct PerformanceStats {
        double cpuUsage = 0.0;
        double memoryUsage = 0.0;
        quint64 totalMemoryAllocated = 0;
        quint64 peakMemoryUsage = 0;
        quint64 activeConnections = 0;
        quint64 totalConnections = 0;
        double averageResponseTime = 0.0;
        quint64 bytesPerSecond = 0;
        quint64 requestsPerSecond = 0;
    };
    
    PerformanceStats getPerformanceStats() const;
    
    // Resource monitoring
    void startResourceMonitoring();
    void stopResourceMonitoring();
    bool isResourceMonitoringActive() const { return m_resourceMonitoringActive.load(); }
    
    // IOCP optimization
    void optimizeIOCP();
    void setIOCPThreadCount(int count);
    int getIOCPThreadCount() const { return m_iocpThreadCount; }
    
    // Memory pool management
    void createMemoryPool(size_t poolSize, size_t blockSize);
    void* allocateFromPool(size_t size);
    void deallocateToPool(void* ptr);
    
    // Performance tuning
    void enableCompression(bool enabled);
    void setCompressionLevel(int level);
    bool isCompressionEnabled() const { return m_compressionEnabled; }
    int getCompressionLevel() const { return m_compressionLevel; }
    
    // Cache management
    void setCacheSize(size_t size);
    size_t getCacheSize() const { return m_cacheSize; }
    void clearCache();
    
signals:
    void performanceStatsUpdated(const PerformanceStats& stats);
    void resourceUsageChanged(double cpu, double memory);
    void memoryAllocationFailed(size_t requestedSize);
    void performanceWarning(const QString& warning);

private:
    PerformanceManager();
    ~PerformanceManager();
    
    void updatePerformanceStats();
    void monitorSystemResources();
    void optimizeMemoryUsage();
    void handleMemoryPressure();
    
    // Memory pools
    std::unique_ptr<MemoryPool> m_memoryPool;
    std::vector<std::unique_ptr<MemoryPool>> m_specializedPools;
    
    // IOCP optimization
    std::unique_ptr<IOCPOptimizer> m_iocpOptimizer;
    
    // Resource monitoring
    std::unique_ptr<ResourceMonitor> m_resourceMonitor;
    QTimer* m_monitoringTimer;
    
    // Performance settings
    std::atomic<size_t> m_bufferSize{65536};
    std::atomic<int> m_ioThreads{4};
    std::atomic<int> m_workerThreads{8};
    std::atomic<int> m_iocpThreadCount{4};
    std::atomic<bool> m_compressionEnabled{true};
    std::atomic<int> m_compressionLevel{6};
    std::atomic<size_t> m_cacheSize{100 * 1024 * 1024}; // 100MB
    
    // Monitoring state
    std::atomic<bool> m_resourceMonitoringActive{false};
    std::atomic<bool> m_initialized{false};
    
    // Performance stats
    mutable QReadWriteLock m_statsLock;
    PerformanceStats m_currentStats;
    
    // Memory tracking
    std::atomic<quint64> m_totalAllocated{0};
    std::atomic<quint64> m_peakUsage{0};
    
    Q_DISABLE_COPY(PerformanceManager)
};

// Memory Pool for efficient allocation
class MemoryPool
{
public:
    MemoryPool(size_t blockSize, size_t poolSize);
    ~MemoryPool();
    
    void* allocate();
    bool deallocate(void* ptr);
    size_t getBlockSize() const { return m_blockSize; }
    size_t getAvailableBlocks() const;
    size_t getTotalBlocks() const { return m_totalBlocks; }
    
private:
    struct Block {
        Block* next;
        char data[];
    };
    
    void initializePool();
    
    Block* m_freeList;
    std::vector<std::unique_ptr<char[]>> m_poolMemory;
    size_t m_blockSize;
    size_t m_totalBlocks;
    size_t m_availableBlocks;
    mutable QMutex m_mutex;
};

// IOCP Optimizer for Windows
class IOCPOptimizer
{
public:
    IOCPOptimizer();
    ~IOCPOptimizer();
    
    bool initialize(int threadCount);
    void shutdown();
    void optimize();
    
    HANDLE getCompletionPort() const { return m_completionPort; }
    int getThreadCount() const { return m_threadCount; }
    
private:
    HANDLE m_completionPort;
    int m_threadCount;
    std::vector<HANDLE> m_workerThreads;
    std::atomic<bool> m_running{false};
    
    static DWORD WINAPI workerThreadProc(LPVOID param);
    void workerThread();
};

// Resource Monitor for system monitoring
class ResourceMonitor
{
public:
    ResourceMonitor();
    ~ResourceMonitor();
    
    bool initialize();
    void shutdown();
    
    struct SystemResources {
        double cpuUsage;
        double memoryUsage;
        quint64 totalMemory;
        quint64 availableMemory;
        quint64 totalDiskSpace;
        quint64 availableDiskSpace;
        quint64 networkBytesIn;
        quint64 networkBytesOut;
    };
    
    SystemResources getSystemResources() const;
    
private:
    void updateSystemResources();
    
    SystemResources m_currentResources;
    mutable QMutex m_resourcesMutex;
    
#ifdef _WIN32
    HANDLE m_processHandle;
    FILETIME m_lastCpuTime;
    FILETIME m_lastSystemTime;
#endif
};

} // namespace LegacyStream 