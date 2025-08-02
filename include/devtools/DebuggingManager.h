#ifndef DEBUGGINGMANAGER_H
#define DEBUGGINGMANAGER_H

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
 * @brief Breakpoint information
 */
struct Breakpoint
{
    QString id;
    QString file;
    int line = 0;
    int column = 0;
    QString condition;
    QString expression;
    bool enabled = true;
    bool temporary = false;
    int hitCount = 0;
    QString action; // "stop", "log", "evaluate"
    QDateTime created;
    QDateTime lastHit;
    QString description;
};

/**
 * @brief Call stack frame
 */
struct CallStackFrame
{
    QString function;
    QString file;
    int line = 0;
    int column = 0;
    QString address;
    QString module;
    QMap<QString, QString> localVariables;
    QMap<QString, QString> arguments;
    QString returnValue;
    bool isInlined = false;
    QString sourceCode;
};

/**
 * @brief Variable information
 */
struct VariableInfo
{
    QString name;
    QString type;
    QString value;
    QString address;
    int size = 0;
    bool isConst = false;
    bool isReference = false;
    bool isPointer = false;
    QList<VariableInfo> children; // For structs/objects
    QString description;
    QString expression;
};

/**
 * @brief Debugging configuration
 */
struct DebuggingConfig
{
    QString name;
    bool enableBreakpoints = true;
    bool enableCallStack = true;
    bool enableVariableInspection = true;
    bool enableMemoryAnalysis = true;
    bool enablePerformanceProfiling = true;
    bool enableExceptionHandling = true;
    bool enableLogging = true;
    int maxCallStackDepth = 100;
    int maxVariableDisplayDepth = 5;
    bool enableRealTimeDebugging = true;
    bool enableRemoteDebugging = false;
    QString remoteHost;
    int remotePort = 12345;
};

/**
 * @brief Debugging session statistics
 */
struct DebuggingStats
{
    int totalBreakpoints = 0;
    int activeBreakpoints = 0;
    int breakpointHits = 0;
    int exceptionsCaught = 0;
    int memoryLeaksDetected = 0;
    double averageResponseTime = 0.0;
    QDateTime sessionStart;
    QDateTime lastBreakpoint;
    QMap<QString, int> breakpointsByFile;
    QMap<QString, int> exceptionsByType;
    QMap<QString, double> performanceByFunction;
};

/**
 * @brief Debugging manager for advanced debugging features
 *
 * Provides sophisticated debugging capabilities including breakpoints,
 * call stack analysis, variable inspection, and memory analysis.
 */
class DebuggingManager : public QObject
{
    Q_OBJECT

public:
    explicit DebuggingManager(QObject* parent = nullptr);
    ~DebuggingManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Debugging session management
    bool createDebugSession(const QString& name, const DebuggingConfig& config);
    void destroyDebugSession(const QString& name);
    bool debugSessionExists(const QString& name) const;
    QStringList getDebugSessionNames() const;

    // Breakpoint management
    QString addBreakpoint(const QString& sessionName, const QString& file, int line, const QString& condition = QString());
    void removeBreakpoint(const QString& sessionName, const QString& breakpointId);
    void enableBreakpoint(const QString& sessionName, const QString& breakpointId, bool enabled);
    void updateBreakpoint(const QString& sessionName, const QString& breakpointId, const Breakpoint& breakpoint);
    QList<Breakpoint> getBreakpoints(const QString& sessionName) const;

    // Call stack analysis
    QList<CallStackFrame> getCallStack(const QString& sessionName) const;
    CallStackFrame getCurrentFrame(const QString& sessionName) const;
    void setCurrentFrame(const QString& sessionName, int frameIndex);
    void expandFrame(const QString& sessionName, int frameIndex);

    // Variable inspection
    QList<VariableInfo> getLocalVariables(const QString& sessionName, int frameIndex = -1) const;
    QList<VariableInfo> getGlobalVariables(const QString& sessionName) const;
    VariableInfo getVariable(const QString& sessionName, const QString& variableName, int frameIndex = -1) const;
    QString evaluateExpression(const QString& sessionName, const QString& expression, int frameIndex = -1);

    // Memory analysis
    void enableMemoryAnalysis(const QString& sessionName, bool enabled);
    QJsonObject getMemoryUsage(const QString& sessionName) const;
    QList<QJsonObject> getMemoryLeaks(const QString& sessionName) const;
    void analyzeMemoryLeaks(const QString& sessionName);
    void setMemoryBreakpoint(const QString& sessionName, const QString& address, size_t size);

    // Performance profiling
    void enablePerformanceProfiling(const QString& sessionName, bool enabled);
    QJsonObject getPerformanceProfile(const QString& sessionName) const;
    void startProfiling(const QString& sessionName);
    void stopProfiling(const QString& sessionName);
    void clearProfilingData(const QString& sessionName);

    // Exception handling
    void enableExceptionHandling(const QString& sessionName, bool enabled);
    void setExceptionBreakpoint(const QString& sessionName, const QString& exceptionType);
    void removeExceptionBreakpoint(const QString& sessionName, const QString& exceptionType);
    QList<QString> getCaughtExceptions(const QString& sessionName) const;

    // Configuration
    void setDebuggingConfig(const QString& name, const DebuggingConfig& config);
    DebuggingConfig getDebuggingConfig(const QString& name) const;
    void setMaxCallStackDepth(const QString& name, int depth);
    void setMaxVariableDisplayDepth(const QString& name, int depth);

    // Statistics and monitoring
    DebuggingStats getDebugSessionStats(const QString& name) const;
    QJsonObject getAllDebugSessionStatsJson() const;
    void resetDebugSessionStats(const QString& name);
    void exportDebugSessionStats(const QString& filePath) const;

    // Advanced features
    void enableLogging(const QString& name, bool enabled);
    void setLogLevel(const QString& name, const QString& level);
    void enableRemoteDebugging(const QString& name, bool enabled);
    void setRemoteConnection(const QString& name, const QString& host, int port);

    // Utility functions
    bool isDebuggingActive(const QString& sessionName) const;
    bool isBreakpointHit(const QString& sessionName, const QString& breakpointId) const;
    QString getCurrentFunction(const QString& sessionName) const;
    QString getCurrentFile(const QString& sessionName) const;

signals:
    void breakpointHit(const QString& sessionName, const QString& breakpointId);
    void exceptionCaught(const QString& sessionName, const QString& exceptionType);
    void memoryLeakDetected(const QString& sessionName, const QString& address);
    void callStackChanged(const QString& sessionName, const QList<CallStackFrame>& frames);
    void variableChanged(const QString& sessionName, const QString& variableName, const QString& newValue);
    void statisticsUpdated(const QString& name, const DebuggingStats& stats);

public slots:
    void onDebugTimer();
    void onMemoryAnalysisTimer();
    void onStatisticsTimer();

private slots:
    void onBreakpointTriggered(const QString& breakpointId);
    void onExceptionThrown(const QString& exceptionType);
    void onMemoryAllocated(const QString& address, size_t size);
    void onMemoryFreed(const QString& address);

private:
    // Debugging session management
    struct DebugSession
    {
        DebuggingConfig config;
        DebuggingStats stats;
        QMap<QString, Breakpoint> breakpoints;
        QList<CallStackFrame> callStack;
        QMap<QString, VariableInfo> localVariables;
        QMap<QString, VariableInfo> globalVariables;
        QMap<QString, QJsonObject> memoryUsage;
        QList<QJsonObject> memoryLeaks;
        QJsonObject performanceProfile;
        QList<QString> caughtExceptions;
        QMutex mutex;
        QTimer* debugTimer = nullptr;
        QTimer* memoryAnalysisTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        bool isActive = true;
        int currentFrameIndex = 0;
    };

    // Core debugging operations
    void handleBreakpoint(DebugSession& session, const QString& breakpointId);
    void handleException(DebugSession& session, const QString& exceptionType);
    void analyzeCallStack(DebugSession& session);
    void inspectVariables(DebugSession& session, int frameIndex);

    // Breakpoint management
    void addBreakpointToSession(DebugSession& session, const Breakpoint& breakpoint);
    void removeBreakpointFromSession(DebugSession& session, const QString& breakpointId);
    void updateBreakpointInSession(DebugSession& session, const QString& breakpointId, const Breakpoint& breakpoint);
    bool isBreakpointConditionMet(const Breakpoint& breakpoint, const QMap<QString, QString>& context);

    // Call stack analysis
    void captureCallStack(DebugSession& session);
    void expandCallStackFrame(DebugSession& session, int frameIndex);
    CallStackFrame createCallStackFrame(const QString& function, const QString& file, int line);

    // Variable inspection
    void captureLocalVariables(DebugSession& session, int frameIndex);
    void captureGlobalVariables(DebugSession& session);
    VariableInfo inspectVariable(const QString& name, const QString& type, const QString& value);
    QString evaluateVariableExpression(const QString& expression, const QMap<QString, QString>& context);

    // Memory analysis
    void performMemoryAnalysis(DebugSession& session);
    void detectMemoryLeaks(DebugSession& session);
    void trackMemoryAllocation(DebugSession& session, const QString& address, size_t size);
    void trackMemoryDeallocation(DebugSession& session, const QString& address);

    // Performance profiling
    void startPerformanceProfiling(DebugSession& session);
    void stopPerformanceProfiling(DebugSession& session);
    void capturePerformanceData(DebugSession& session);
    void analyzePerformanceProfile(DebugSession& session);

    // Statistics
    void updateDebugSessionStatistics(const QString& name);
    void calculateDebugSessionMetrics(DebugSession& session);
    void logDebugSessionEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateBreakpointId() const;
    QString generateSessionId() const;
    bool isValidBreakpoint(const Breakpoint& breakpoint) const;
    void loadDebugSessionFromDisk(const QString& name);
    void saveDebugSessionToDisk(const QString& name);

    // Debug session storage
    QMap<QString, std::unique_ptr<DebugSession>> m_debugSessions;

    // Timers
    QTimer* m_globalDebugTimer = nullptr;
    QTimer* m_globalMemoryAnalysisTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_loggingEnabled = true;
    bool m_memoryAnalysisEnabled = true;
    bool m_performanceProfilingEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastBreakpoint;
    QMap<QString, QDateTime> m_lastException;
    QMap<QString, double> m_debugResponseTimes;

    // Platform-specific debugging
#ifdef _WIN32
    void* m_debugProcess = nullptr;
    void* m_debugThread = nullptr;
    void* m_debugEvent = nullptr;
#endif

    Q_DISABLE_COPY(DebuggingManager)
};

} // namespace LegacyStream

#endif // DEBUGGINGMANAGER_H 