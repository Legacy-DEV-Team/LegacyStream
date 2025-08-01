# Performance Manager Documentation

## Overview

The `PerformanceManager` is a critical component of LegacyStream that provides advanced performance optimization, memory management, and resource monitoring capabilities. It ensures the streaming server operates at maximum efficiency while maintaining stability under high load conditions.

## Features

### 🚀 **Advanced Performance Optimization**
- **Windows IOCP Integration** - High-performance I/O completion ports
- **Thread Management** - Configurable I/O and worker threads
- **Memory Pooling** - Efficient memory allocation with specialized pools
- **Resource Monitoring** - Real-time CPU and memory usage tracking
- **Performance Tuning** - Dynamic optimization based on system resources

### 💾 **Memory Management**
- **Memory Pools** - Specialized pools for different buffer sizes
- **Lock-free Operations** - Minimized contention in high-concurrency scenarios
- **Memory Pressure Handling** - Automatic optimization under memory constraints
- **Peak Usage Tracking** - Monitor memory allocation patterns

### 📊 **Resource Monitoring**
- **CPU Usage Tracking** - Real-time CPU utilization monitoring
- **Memory Usage Monitoring** - Process and system memory tracking
- **Performance Statistics** - Comprehensive performance metrics
- **Warning System** - Automated alerts for performance issues

## Architecture

### Core Components

#### PerformanceManager
The main singleton class that coordinates all performance optimization features.

```cpp
class PerformanceManager : public QObject
{
    // Memory management
    void* allocateBuffer(size_t size);
    void deallocateBuffer(void* ptr);
    
    // Thread management
    void setIOThreads(int count);
    void setWorkerThreads(int count);
    
    // Performance monitoring
    PerformanceStats getPerformanceStats() const;
    
    // Resource monitoring
    void startResourceMonitoring();
    void stopResourceMonitoring();
};
```

#### MemoryPool
Efficient memory allocation with specialized pools for different buffer sizes.

```cpp
class MemoryPool
{
    void* allocate();
    bool deallocate(void* ptr);
    size_t getAvailableBlocks() const;
};
```

#### IOCPOptimizer
Windows-specific I/O completion port optimization.

```cpp
class IOCPOptimizer
{
    bool initialize(int threadCount);
    void optimize();
    HANDLE getCompletionPort() const;
};
```

#### ResourceMonitor
System resource monitoring and statistics collection.

```cpp
class ResourceMonitor
{
    SystemResources getSystemResources() const;
    bool initialize();
    void updateSystemResources();
};
```

## Configuration

### Performance Settings

The PerformanceManager integrates with the main Configuration system:

```ini
[Performance]
# Thread configuration
IOThreads=4
WorkerThreads=8
IOCPThreadCount=4

# Memory configuration
BufferSize=65536
CacheSize=104857600

# Compression settings
EnableCompression=true
CompressionLevel=6

# Monitoring settings
ResourceMonitoringEnabled=true
MonitoringInterval=1000
```

### Memory Pool Configuration

Specialized memory pools are automatically created:

- **Small Buffers**: 4KB blocks (1MB pool)
- **Medium Buffers**: 64KB blocks (10MB pool)  
- **Large Buffers**: 1MB blocks (50MB pool)
- **Main Pool**: Configurable block size (default 64KB)

## Usage

### Initialization

```cpp
// Initialize performance manager
auto& perfManager = PerformanceManager::instance();
if (!perfManager.initialize()) {
    qCCritical() << "Failed to initialize PerformanceManager";
    return false;
}

// Start resource monitoring
perfManager.startResourceMonitoring();
perfManager.optimizeIOCP();
```

### Memory Allocation

```cpp
// Allocate buffer from performance-optimized pools
void* buffer = perfManager.allocateBuffer(8192);
if (buffer) {
    // Use buffer
    // ...
    perfManager.deallocateBuffer(buffer);
}
```

### Performance Monitoring

```cpp
// Get current performance statistics
auto stats = perfManager.getPerformanceStats();
qDebug() << "CPU Usage:" << stats.cpuUsage << "%";
qDebug() << "Memory Usage:" << stats.memoryUsage << "%";
qDebug() << "Total Allocated:" << stats.totalMemoryAllocated << "bytes";
```

### Thread Management

```cpp
// Configure thread counts
perfManager.setIOThreads(8);
perfManager.setWorkerThreads(16);
perfManager.setIOCPThreadCount(4);
```

## Performance Statistics

### PerformanceStats Structure

```cpp
struct PerformanceStats {
    double cpuUsage = 0.0;              // CPU usage percentage
    double memoryUsage = 0.0;           // Memory usage percentage
    quint64 totalMemoryAllocated = 0;   // Total allocated memory
    quint64 peakMemoryUsage = 0;        // Peak memory usage
    quint64 activeConnections = 0;      // Current active connections
    quint64 totalConnections = 0;       // Total connections handled
    double averageResponseTime = 0.0;   // Average response time
    quint64 bytesPerSecond = 0;         // Data transfer rate
    quint64 requestsPerSecond = 0;      // Request processing rate
};
```

### SystemResources Structure

```cpp
struct SystemResources {
    double cpuUsage;           // Current CPU usage
    double memoryUsage;        // Current memory usage
    quint64 totalMemory;       // Total system memory
    quint64 availableMemory;   // Available system memory
    quint64 totalDiskSpace;    // Total disk space
    quint64 availableDiskSpace; // Available disk space
    quint64 networkBytesIn;    // Network bytes received
    quint64 networkBytesOut;   // Network bytes sent
};
```

## Integration with ServerManager

The PerformanceManager is automatically integrated into the ServerManager lifecycle:

### Initialization Phase
1. PerformanceManager is initialized first
2. Memory pools are created based on configuration
3. IOCP optimizer is configured
4. Resource monitoring is started

### Runtime Phase
1. Real-time performance statistics are collected
2. Memory allocation is optimized through pools
3. Resource usage is monitored continuously
4. Performance warnings are emitted when thresholds are exceeded

### Shutdown Phase
1. Resource monitoring is stopped
2. Memory pools are cleaned up
3. IOCP optimizer is shut down
4. Performance statistics are finalized

## Performance Optimization Strategies

### Memory Management
- **Pool-based Allocation**: Reduces fragmentation and allocation overhead
- **Size-specific Pools**: Optimized pools for common buffer sizes
- **Lock-free Operations**: Minimizes contention in high-concurrency scenarios
- **Automatic Cleanup**: Memory pressure detection and response

### I/O Optimization
- **IOCP Thread Pool**: Configurable thread count for I/O operations
- **Completion Port Management**: Efficient handling of I/O completions
- **Buffer Reuse**: Minimizes allocation/deallocation overhead
- **Async Operations**: Non-blocking I/O for maximum throughput

### Resource Monitoring
- **Real-time Tracking**: Continuous monitoring of system resources
- **Threshold Alerts**: Automatic warnings for performance issues
- **Historical Data**: Performance trend analysis
- **Adaptive Optimization**: Dynamic adjustment based on resource usage

## Monitoring and Alerts

### Performance Warnings
The PerformanceManager emits signals for performance issues:

```cpp
// Connect to performance warning signals
connect(&perfManager, &PerformanceManager::performanceWarning,
        this, &MainWindow::handlePerformanceWarning);

connect(&perfManager, &PerformanceManager::memoryAllocationFailed,
        this, &MainWindow::handleMemoryAllocationFailed);
```

### Resource Usage Signals
```cpp
// Monitor resource usage changes
connect(&perfManager, &PerformanceManager::resourceUsageChanged,
        this, &MainWindow::updateResourceDisplay);

connect(&perfManager, &PerformanceManager::performanceStatsUpdated,
        this, &MainWindow::updatePerformanceDisplay);
```

## Best Practices

### Configuration
- Set appropriate thread counts based on CPU cores
- Configure memory pool sizes based on expected load
- Enable resource monitoring for production environments
- Set reasonable performance thresholds

### Memory Management
- Use `allocateBuffer()` instead of direct `malloc()`
- Always pair allocations with deallocations
- Monitor memory usage patterns
- Respond to memory pressure warnings

### Performance Monitoring
- Regularly check performance statistics
- Respond to performance warnings promptly
- Monitor resource usage trends
- Adjust configuration based on usage patterns

## Troubleshooting

### Common Issues

#### High Memory Usage
- Check for memory leaks in custom code
- Review memory pool configuration
- Monitor allocation patterns
- Consider increasing pool sizes

#### High CPU Usage
- Review thread count configuration
- Check for inefficient algorithms
- Monitor I/O patterns
- Consider load balancing

#### Performance Warnings
- Review resource usage thresholds
- Check system resource availability
- Monitor application load
- Consider scaling strategies

### Debug Information

Enable debug logging for detailed performance information:

```cpp
// Enable performance manager debug logging
QLoggingCategory::setFilterRules("performanceManager.debug=true");
```

## API Reference

### PerformanceManager Methods

#### Initialization
- `bool initialize()` - Initialize the performance manager
- `void shutdown()` - Shutdown the performance manager

#### Memory Management
- `void* allocateBuffer(size_t size)` - Allocate optimized buffer
- `void deallocateBuffer(void* ptr)` - Deallocate buffer
- `void setBufferSize(size_t size)` - Set default buffer size
- `size_t getBufferSize() const` - Get current buffer size

#### Thread Management
- `void setIOThreads(int count)` - Set I/O thread count
- `void setWorkerThreads(int count)` - Set worker thread count
- `int getIOThreads() const` - Get current I/O thread count
- `int getWorkerThreads() const` - Get current worker thread count

#### Performance Monitoring
- `PerformanceStats getPerformanceStats() const` - Get performance statistics
- `void startResourceMonitoring()` - Start resource monitoring
- `void stopResourceMonitoring()` - Stop resource monitoring
- `bool isResourceMonitoringActive() const` - Check monitoring status

#### IOCP Optimization
- `void optimizeIOCP()` - Optimize IOCP configuration
- `void setIOCPThreadCount(int count)` - Set IOCP thread count
- `int getIOCPThreadCount() const` - Get IOCP thread count

#### Memory Pool Management
- `void createMemoryPool(size_t poolSize, size_t blockSize)` - Create memory pool
- `void* allocateFromPool(size_t size)` - Allocate from pool
- `void deallocateToPool(void* ptr)` - Deallocate to pool

#### Performance Tuning
- `void enableCompression(bool enabled)` - Enable/disable compression
- `void setCompressionLevel(int level)` - Set compression level
- `bool isCompressionEnabled() const` - Check compression status
- `int getCompressionLevel() const` - Get compression level

#### Cache Management
- `void setCacheSize(size_t size)` - Set cache size
- `size_t getCacheSize() const` - Get cache size
- `void clearCache()` - Clear all caches

### Signals

- `void performanceStatsUpdated(const PerformanceStats& stats)` - Performance stats updated
- `void resourceUsageChanged(double cpu, double memory)` - Resource usage changed
- `void memoryAllocationFailed(size_t requestedSize)` - Memory allocation failed
- `void performanceWarning(const QString& warning)` - Performance warning

## Conclusion

The PerformanceManager provides a comprehensive solution for performance optimization in LegacyStream. It ensures efficient resource utilization, provides real-time monitoring capabilities, and enables dynamic optimization based on system conditions. This component is essential for maintaining high-performance streaming operations under various load conditions. 