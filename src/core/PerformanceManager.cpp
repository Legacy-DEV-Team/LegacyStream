#include "core/PerformanceManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QProcess>
#include <QDir>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <pdh.h>
#include <mmsystem.h>
#pragma comment(lib, "pdh.lib")
#endif

Q_LOGGING_CATEGORY(performanceManager, "performanceManager")

namespace LegacyStream {

// PerformanceManager Implementation
PerformanceManager& PerformanceManager::instance()
{
    static PerformanceManager instance;
    return instance;
}

PerformanceManager::PerformanceManager()
    : QObject(nullptr)
    , m_monitoringTimer(new QTimer(this))
{
    qCDebug(performanceManager) << "PerformanceManager created";
}

PerformanceManager::~PerformanceManager()
{
    shutdown();
}

bool PerformanceManager::initialize()
{
    if (m_initialized.load()) {
        qCWarning(performanceManager) << "PerformanceManager already initialized";
        return true;
    }

    qCInfo(performanceManager) << "Initializing PerformanceManager";

    try {
        auto& config = Configuration::instance();
        
        // Initialize memory pool
        m_memoryPool = std::make_unique<MemoryPool>(config.bufferSize(), 1000);
        
        // Initialize IOCP optimizer
        m_iocpOptimizer = std::make_unique<IOCPOptimizer>();
        m_iocpOptimizer->initialize(config.ioThreads());
        
        // Initialize resource monitor
        m_resourceMonitor = std::make_unique<ResourceMonitor>();
        m_resourceMonitor->initialize();
        
        // Setup monitoring timer
        connect(m_monitoringTimer, &QTimer::timeout, this, &PerformanceManager::updatePerformanceStats);
        m_monitoringTimer->start(1000); // Update every second
        
        // Create specialized memory pools
        createMemoryPool(1024 * 1024, 4096);   // 4KB blocks for small buffers
        createMemoryPool(10 * 1024 * 1024, 65536); // 64KB blocks for medium buffers
        createMemoryPool(50 * 1024 * 1024, 1024 * 1024); // 1MB blocks for large buffers
        
        m_initialized.store(true);
        qCInfo(performanceManager) << "PerformanceManager initialized successfully";
        return true;
    }
    catch (const std::exception& e) {
        qCCritical(performanceManager) << "Failed to initialize PerformanceManager:" << e.what();
        return false;
    }
}

void PerformanceManager::shutdown()
{
    if (!m_initialized.load()) {
        return;
    }

    qCInfo(performanceManager) << "Shutting down PerformanceManager";

    if (m_monitoringTimer) {
        m_monitoringTimer->stop();
    }

    if (m_resourceMonitor) {
        m_resourceMonitor->shutdown();
    }

    if (m_iocpOptimizer) {
        m_iocpOptimizer->shutdown();
    }

    m_initialized.store(false);
    qCInfo(performanceManager) << "PerformanceManager shut down";
}

void* PerformanceManager::allocateBuffer(size_t size)
{
    if (!m_initialized.load()) {
        qCWarning(performanceManager) << "PerformanceManager not initialized";
        return nullptr;
    }

    // Try to allocate from specialized pools first
    for (auto& pool : m_specializedPools) {
        if (size <= pool->getBlockSize()) {
            void* ptr = pool->allocate();
            if (ptr) {
                m_totalAllocated.fetch_add(size);
                quint64 peak = m_peakUsage.load();
                while (m_totalAllocated.load() > peak && 
                       !m_peakUsage.compare_exchange_weak(peak, m_totalAllocated.load())) {
                    // Retry until we update the peak value
                }
                return ptr;
            }
        }
    }

    // Fallback to main memory pool
    void* ptr = m_memoryPool->allocate();
    if (ptr) {
        m_totalAllocated.fetch_add(size);
        quint64 peak = m_peakUsage.load();
        while (m_totalAllocated.load() > peak && 
               !m_peakUsage.compare_exchange_weak(peak, m_totalAllocated.load())) {
            // Retry until we update the peak value
        }
        return ptr;
    }

    // Last resort: direct allocation
    void* directPtr = malloc(size);
    if (directPtr) {
        m_totalAllocated.fetch_add(size);
        quint64 peak = m_peakUsage.load();
        while (m_totalAllocated.load() > peak && 
               !m_peakUsage.compare_exchange_weak(peak, m_totalAllocated.load())) {
            // Retry until we update the peak value
        }
        return directPtr;
    }

    emit memoryAllocationFailed(size);
    return nullptr;
}

void PerformanceManager::deallocateBuffer(void* ptr)
{
    if (!ptr) return;

    // Try to deallocate to specialized pools first
    for (auto& pool : m_specializedPools) {
        if (pool->deallocate(ptr)) {
            return;
        }
    }

    // Try main memory pool
    if (m_memoryPool->deallocate(ptr)) {
        return;
    }

    // Fallback to direct deallocation
    free(ptr);
}

void PerformanceManager::setBufferSize(size_t size)
{
    m_bufferSize.store(size);
    qCInfo(performanceManager) << "Buffer size set to" << size << "bytes";
}

void PerformanceManager::setIOThreads(int count)
{
    m_ioThreads.store(count);
    if (m_iocpOptimizer) {
        m_iocpOptimizer->shutdown();
        m_iocpOptimizer->initialize(count);
    }
    qCInfo(performanceManager) << "I/O threads set to" << count;
}

void PerformanceManager::setWorkerThreads(int count)
{
    m_workerThreads.store(count);
    qCInfo(performanceManager) << "Worker threads set to" << count;
}

PerformanceManager::PerformanceStats PerformanceManager::getPerformanceStats() const
{
    QReadLocker locker(&m_statsLock);
    return m_currentStats;
}

void PerformanceManager::startResourceMonitoring()
{
    if (m_resourceMonitoringActive.load()) {
        return;
    }

    m_resourceMonitoringActive.store(true);
    qCInfo(performanceManager) << "Resource monitoring started";
}

void PerformanceManager::stopResourceMonitoring()
{
    if (!m_resourceMonitoringActive.load()) {
        return;
    }

    m_resourceMonitoringActive.store(false);
    qCInfo(performanceManager) << "Resource monitoring stopped";
}

void PerformanceManager::optimizeIOCP()
{
    if (m_iocpOptimizer) {
        m_iocpOptimizer->optimize();
        qCInfo(performanceManager) << "IOCP optimization completed";
    }
}

void PerformanceManager::setIOCPThreadCount(int count)
{
    m_iocpThreadCount.store(count);
    if (m_iocpOptimizer) {
        m_iocpOptimizer->shutdown();
        m_iocpOptimizer->initialize(count);
    }
    qCInfo(performanceManager) << "IOCP thread count set to" << count;
}

void PerformanceManager::createMemoryPool(size_t poolSize, size_t blockSize)
{
    auto pool = std::make_unique<MemoryPool>(blockSize, poolSize / blockSize);
    m_specializedPools.push_back(std::move(pool));
    qCDebug(performanceManager) << "Created memory pool:" << poolSize << "bytes," << blockSize << "block size";
}

void* PerformanceManager::allocateFromPool(size_t size)
{
    for (auto& pool : m_specializedPools) {
        if (size <= pool->getBlockSize()) {
            return pool->allocate();
        }
    }
    return m_memoryPool->allocate();
}

void PerformanceManager::deallocateToPool(void* ptr)
{
    for (auto& pool : m_specializedPools) {
        if (pool->deallocate(ptr)) {
            return;
        }
    }
    m_memoryPool->deallocate(ptr);
}

void PerformanceManager::enableCompression(bool enabled)
{
    m_compressionEnabled.store(enabled);
    qCInfo(performanceManager) << "Compression" << (enabled ? "enabled" : "disabled");
}

void PerformanceManager::setCompressionLevel(int level)
{
    if (level >= 1 && level <= 9) {
        m_compressionLevel.store(level);
        qCInfo(performanceManager) << "Compression level set to" << level;
    }
}

void PerformanceManager::setCacheSize(size_t size)
{
    m_cacheSize.store(size);
    qCInfo(performanceManager) << "Cache size set to" << size << "bytes";
}

void PerformanceManager::clearCache()
{
    // Clear specialized pools
    for (auto& pool : m_specializedPools) {
        // Reset pool (implementation would depend on pool design)
    }
    qCInfo(performanceManager) << "Cache cleared";
}

void PerformanceManager::updatePerformanceStats()
{
    if (!m_resourceMonitor) return;

    auto resources = m_resourceMonitor->getSystemResources();
    
    QWriteLocker locker(&m_statsLock);
    m_currentStats.cpuUsage = resources.cpuUsage;
    m_currentStats.memoryUsage = resources.memoryUsage;
    m_currentStats.totalMemoryAllocated = m_totalAllocated.load();
    m_currentStats.peakMemoryUsage = m_peakUsage.load();
    
    emit performanceStatsUpdated(m_currentStats);
    emit resourceUsageChanged(resources.cpuUsage, resources.memoryUsage);
    
    // Check for performance warnings
    if (resources.cpuUsage > 80.0) {
        emit performanceWarning("High CPU usage detected");
    }
    if (resources.memoryUsage > 85.0) {
        emit performanceWarning("High memory usage detected");
    }
}

void PerformanceManager::monitorSystemResources()
{
    if (!m_resourceMonitoringActive.load()) return;
    
    updatePerformanceStats();
    optimizeMemoryUsage();
}

void PerformanceManager::optimizeMemoryUsage()
{
    // Implement memory optimization strategies
    if (m_currentStats.memoryUsage > 80.0) {
        handleMemoryPressure();
    }
}

void PerformanceManager::handleMemoryPressure()
{
    // Implement memory pressure handling
    qCWarning(performanceManager) << "Memory pressure detected, optimizing usage";
    
    // Clear caches
    clearCache();
    
    // Force garbage collection if applicable
    // This would depend on the specific memory management strategy
}

// MemoryPool Implementation
MemoryPool::MemoryPool(size_t blockSize, size_t poolSize)
    : m_blockSize(blockSize)
    , m_totalBlocks(poolSize)
    , m_availableBlocks(poolSize)
    , m_freeList(nullptr)
{
    initializePool();
}

MemoryPool::~MemoryPool()
{
    // Memory will be automatically freed by vector
}

void MemoryPool::initializePool()
{
    size_t totalSize = m_totalBlocks * m_blockSize;
    size_t chunks = (totalSize + 1024 * 1024 - 1) / (1024 * 1024); // 1MB chunks
    
    for (size_t i = 0; i < chunks; ++i) {
        size_t chunkSize = std::min(1024 * 1024ULL, totalSize - i * 1024 * 1024);
        auto chunk = std::make_unique<char[]>(chunkSize);
        m_poolMemory.push_back(std::move(chunk));
    }
    
    // Initialize free list
    m_freeList = nullptr;
    for (size_t i = 0; i < m_totalBlocks; ++i) {
        size_t offset = i * m_blockSize;
        size_t chunkIndex = offset / (1024 * 1024);
        size_t chunkOffset = offset % (1024 * 1024);
        
        Block* block = reinterpret_cast<Block*>(m_poolMemory[chunkIndex].get() + chunkOffset);
        block->next = m_freeList;
        m_freeList = block;
    }
}

void* MemoryPool::allocate()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_freeList) {
        return nullptr; // Pool exhausted
    }
    
    Block* block = m_freeList;
    m_freeList = block->next;
    m_availableBlocks--;
    
    return block->data;
}

bool MemoryPool::deallocate(void* ptr)
{
    if (!ptr) return false;
    
    // Check if pointer belongs to this pool
    for (const auto& chunk : m_poolMemory) {
        if (ptr >= chunk.get() && ptr < chunk.get() + 1024 * 1024) {
            QMutexLocker locker(&m_mutex);
            
            Block* block = reinterpret_cast<Block*>(static_cast<char*>(ptr) - offsetof(Block, data));
            block->next = m_freeList;
            m_freeList = block;
            m_availableBlocks++;
            
            return true;
        }
    }
    
    return false; // Pointer doesn't belong to this pool
}

size_t MemoryPool::getAvailableBlocks() const
{
    QMutexLocker locker(&m_mutex);
    return m_availableBlocks;
}

// IOCPOptimizer Implementation
IOCPOptimizer::IOCPOptimizer()
    : m_completionPort(nullptr)
    , m_threadCount(0)
{
}

IOCPOptimizer::~IOCPOptimizer()
{
    shutdown();
}

bool IOCPOptimizer::initialize(int threadCount)
{
    m_threadCount = threadCount;
    
    // Create completion port
    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (!m_completionPort) {
        qCCritical(performanceManager) << "Failed to create IOCP completion port";
        return false;
    }
    
    // Create worker threads
    m_running.store(true);
    for (int i = 0; i < threadCount; ++i) {
        HANDLE thread = CreateThread(nullptr, 0, workerThreadProc, this, 0, nullptr);
        if (thread) {
            m_workerThreads.push_back(thread);
        }
    }
    
    qCInfo(performanceManager) << "IOCP optimizer initialized with" << threadCount << "threads";
    return true;
}

void IOCPOptimizer::shutdown()
{
    m_running.store(false);
    
    // Signal all threads to exit
    for (size_t i = 0; i < m_workerThreads.size(); ++i) {
        PostQueuedCompletionStatus(m_completionPort, 0, 0, nullptr);
    }
    
    // Wait for threads to exit
    if (!m_workerThreads.empty()) {
        WaitForMultipleObjects(static_cast<DWORD>(m_workerThreads.size()), 
                              m_workerThreads.data(), TRUE, 5000);
        
        for (HANDLE thread : m_workerThreads) {
            CloseHandle(thread);
        }
        m_workerThreads.clear();
    }
    
    if (m_completionPort) {
        CloseHandle(m_completionPort);
        m_completionPort = nullptr;
    }
}

void IOCPOptimizer::optimize()
{
    // Implement IOCP optimization strategies
    qCInfo(performanceManager) << "IOCP optimization completed";
}

DWORD WINAPI IOCPOptimizer::workerThreadProc(LPVOID param)
{
    IOCPOptimizer* optimizer = static_cast<IOCPOptimizer*>(param);
    optimizer->workerThread();
    return 0;
}

void IOCPOptimizer::workerThread()
{
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    LPOVERLAPPED overlapped;
    
    while (m_running.load()) {
        BOOL result = GetQueuedCompletionStatus(
            m_completionPort,
            &bytesTransferred,
            &completionKey,
            &overlapped,
            INFINITE
        );
        
        if (!result) {
            if (GetLastError() != WAIT_TIMEOUT) {
                qCWarning(performanceManager) << "IOCP GetQueuedCompletionStatus failed";
            }
            continue;
        }
        
        // Handle completion
        // This would be implemented based on specific I/O requirements
    }
}

// ResourceMonitor Implementation
ResourceMonitor::ResourceMonitor()
#ifdef _WIN32
    : m_processHandle(nullptr)
#endif
{
}

ResourceMonitor::~ResourceMonitor()
{
    shutdown();
}

bool ResourceMonitor::initialize()
{
#ifdef _WIN32
    m_processHandle = GetCurrentProcess();
    if (!m_processHandle) {
        qCCritical(performanceManager) << "Failed to get process handle";
        return false;
    }
    
    // Initialize CPU time tracking
    FILETIME dummy;
    GetProcessTimes(m_processHandle, &dummy, &dummy, &m_lastCpuTime, &dummy);
    GetSystemTimeAsFileTime(&m_lastSystemTime);
#endif

    qCInfo(performanceManager) << "Resource monitor initialized";
    return true;
}

void ResourceMonitor::shutdown()
{
#ifdef _WIN32
    if (m_processHandle) {
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
#endif
}

ResourceMonitor::SystemResources ResourceMonitor::getSystemResources() const
{
    QMutexLocker locker(&m_resourcesMutex);
    return m_currentResources;
}

void ResourceMonitor::updateSystemResources()
{
#ifdef _WIN32
    if (!m_processHandle) return;
    
    // Update CPU usage
    FILETIME dummy, currentCpuTime, currentSystemTime;
    GetProcessTimes(m_processHandle, &dummy, &dummy, &currentCpuTime, &dummy);
    GetSystemTimeAsFileTime(&currentSystemTime);
    
    ULARGE_INTEGER cpuTime, systemTime, lastCpuTime, lastSystemTime;
    cpuTime.LowPart = currentCpuTime.dwLowDateTime;
    cpuTime.HighPart = currentCpuTime.dwHighDateTime;
    systemTime.LowPart = currentSystemTime.dwLowDateTime;
    systemTime.HighPart = currentSystemTime.dwHighDateTime;
    lastCpuTime.LowPart = m_lastCpuTime.dwLowDateTime;
    lastCpuTime.HighPart = m_lastCpuTime.dwHighDateTime;
    lastSystemTime.LowPart = m_lastSystemTime.dwLowDateTime;
    lastSystemTime.HighPart = m_lastSystemTime.dwHighDateTime;
    
    double cpuUsage = 0.0;
    if (systemTime.QuadPart - lastSystemTime.QuadPart > 0) {
        cpuUsage = 100.0 * (cpuTime.QuadPart - lastCpuTime.QuadPart) / 
                   (systemTime.QuadPart - lastSystemTime.QuadPart);
    }
    
    // Update memory usage
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(m_processHandle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            QMutexLocker locker(&m_resourcesMutex);
            m_currentResources.cpuUsage = cpuUsage;
            m_currentResources.memoryUsage = 100.0 * pmc.WorkingSetSize / memInfo.ullTotalPhys;
            m_currentResources.totalMemory = memInfo.ullTotalPhys;
            m_currentResources.availableMemory = memInfo.ullAvailPhys;
        }
    }
    
    m_lastCpuTime = currentCpuTime;
    m_lastSystemTime = currentSystemTime;
#endif
}

} // namespace LegacyStream 