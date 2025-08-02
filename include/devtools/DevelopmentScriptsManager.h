#ifndef DEVELOPMENTSCRIPTSMANAGER_H
#define DEVELOPMENTSCRIPTSMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <memory>
#include <functional>

namespace LegacyStream {

/**
 * @brief Script execution result
 */
struct ScriptResult
{
    QString scriptId;
    QString scriptName;
    bool success = false;
    int exitCode = 0;
    QString output;
    QString error;
    QDateTime startTime;
    QDateTime endTime;
    qint64 duration = 0;
    QString status; // "running", "completed", "failed", "cancelled"
    QJsonObject metadata;
};

/**
 * @brief Script configuration
 */
struct ScriptConfig
{
    QString id;
    QString name;
    QString description;
    QString command;
    QStringList arguments;
    QString workingDirectory;
    QMap<QString, QString> environment;
    bool enabled = true;
    bool runOnStartup = false;
    bool runPeriodically = false;
    int periodicInterval = 3600000; // 1 hour default
    int timeout = 300000; // 5 minutes default
    bool captureOutput = true;
    bool captureError = true;
    QString category; // "build", "test", "deploy", "maintenance", "custom"
    QStringList dependencies;
    QStringList triggers;
};

/**
 * @brief Workflow configuration
 */
struct WorkflowConfig
{
    QString id;
    QString name;
    QString description;
    QStringList scriptIds;
    bool parallelExecution = false;
    bool stopOnFailure = true;
    bool retryOnFailure = false;
    int maxRetries = 3;
    int retryDelay = 5000; // 5 seconds
    bool enabled = true;
    QString category;
    QMap<QString, QString> parameters;
};

/**
 * @brief Development scripts configuration
 */
struct DevelopmentScriptsConfig
{
    QString name;
    bool enableBuildAutomation = true;
    bool enableTestAutomation = true;
    bool enableDeploymentAutomation = true;
    bool enableMaintenanceAutomation = true;
    bool enableCustomScripts = true;
    bool enableWorkflowExecution = true;
    bool enableScheduledExecution = true;
    bool enableLogging = true;
    int maxConcurrentScripts = 5;
    int scriptTimeout = 300000; // 5 minutes
    QString defaultWorkingDirectory;
    QMap<QString, QString> globalEnvironment;
};

/**
 * @brief Development scripts statistics
 */
struct DevelopmentScriptsStats
{
    int totalScripts = 0;
    int activeScripts = 0;
    int successfulExecutions = 0;
    int failedExecutions = 0;
    int totalWorkflows = 0;
    int completedWorkflows = 0;
    double averageExecutionTime = 0.0;
    QDateTime lastExecution;
    QMap<QString, int> executionsByCategory;
    QMap<QString, double> successRatesByCategory;
    QMap<QString, double> averageTimesByCategory;
};

/**
 * @brief Development scripts manager for automation tools
 *
 * Provides comprehensive automation tools for development workflows including
 * build automation, testing automation, deployment scripts, and workflow management.
 */
class DevelopmentScriptsManager : public QObject
{
    Q_OBJECT

public:
    explicit DevelopmentScriptsManager(QObject* parent = nullptr);
    ~DevelopmentScriptsManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Script management
    bool createScript(const QString& name, const ScriptConfig& config);
    void destroyScript(const QString& name);
    bool scriptExists(const QString& name) const;
    QStringList getScriptNames() const;
    ScriptConfig getScript(const QString& name) const;
    void updateScript(const QString& name, const ScriptConfig& config);

    // Script execution
    QString executeScript(const QString& name, const QStringList& arguments = QStringList());
    void executeScriptAsync(const QString& name, const QStringList& arguments = QStringList());
    void cancelScript(const QString& executionId);
    ScriptResult getScriptResult(const QString& executionId) const;
    QList<ScriptResult> getScriptHistory(const QString& name, int count = 50) const;

    // Workflow management
    bool createWorkflow(const QString& name, const WorkflowConfig& config);
    void destroyWorkflow(const QString& name);
    bool workflowExists(const QString& name) const;
    QStringList getWorkflowNames() const;
    WorkflowConfig getWorkflow(const QString& name) const;
    void updateWorkflow(const QString& name, const WorkflowConfig& config);

    // Workflow execution
    QString executeWorkflow(const QString& name, const QMap<QString, QString>& parameters = QMap<QString, QString>());
    void executeWorkflowAsync(const QString& name, const QMap<QString, QString>& parameters = QMap<QString, QString>());
    void cancelWorkflow(const QString& executionId);
    QList<ScriptResult> getWorkflowResults(const QString& executionId) const;

    // Build automation
    void enableBuildAutomation(const QString& name, bool enabled);
    QString executeBuild(const QString& buildType = "release");
    void executeBuildAsync(const QString& buildType = "release");
    QList<ScriptResult> getBuildHistory(const QString& buildType, int count = 20) const;

    // Test automation
    void enableTestAutomation(const QString& name, bool enabled);
    QString executeTests(const QString& testSuite = "all");
    void executeTestsAsync(const QString& testSuite = "all");
    QList<ScriptResult> getTestHistory(const QString& testSuite, int count = 20) const;

    // Deployment automation
    void enableDeploymentAutomation(const QString& name, bool enabled);
    QString executeDeployment(const QString& environment = "production");
    void executeDeploymentAsync(const QString& environment = "production");
    QList<ScriptResult> getDeploymentHistory(const QString& environment, int count = 20) const;

    // Maintenance automation
    void enableMaintenanceAutomation(const QString& name, bool enabled);
    QString executeMaintenance(const QString& task = "cleanup");
    void executeMaintenanceAsync(const QString& task = "cleanup");
    QList<ScriptResult> getMaintenanceHistory(const QString& task, int count = 20) const;

    // Configuration
    void setDevelopmentScriptsConfig(const QString& name, const DevelopmentScriptsConfig& config);
    DevelopmentScriptsConfig getDevelopmentScriptsConfig(const QString& name) const;
    void setMaxConcurrentScripts(const QString& name, int maxScripts);
    void setScriptTimeout(const QString& name, int timeout);

    // Statistics and monitoring
    DevelopmentScriptsStats getDevelopmentScriptsStats(const QString& name) const;
    QJsonObject getAllDevelopmentScriptsStatsJson() const;
    void resetDevelopmentScriptsStats(const QString& name);
    void exportDevelopmentScriptsStats(const QString& filePath) const;

    // Advanced features
    void enableLogging(const QString& name, bool enabled);
    void setLogLevel(const QString& name, const QString& level);
    void enableScheduledExecution(const QString& name, bool enabled);
    void setSchedule(const QString& name, const QString& cronExpression);

    // Utility functions
    bool isScriptRunning(const QString& executionId) const;
    bool isWorkflowRunning(const QString& executionId) const;
    QStringList getRunningScripts() const;
    QStringList getRunningWorkflows() const;

signals:
    void scriptStarted(const QString& executionId, const QString& scriptName);
    void scriptCompleted(const QString& executionId, const ScriptResult& result);
    void scriptFailed(const QString& executionId, const ScriptResult& result);
    void workflowStarted(const QString& executionId, const QString& workflowName);
    void workflowCompleted(const QString& executionId, const QList<ScriptResult>& results);
    void workflowFailed(const QString& executionId, const QString& error);
    void statisticsUpdated(const QString& name, const DevelopmentScriptsStats& stats);

public slots:
    void onScriptTimer();
    void onWorkflowTimer();
    void onStatisticsTimer();

private slots:
    void onScriptFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onScriptError(QProcess::ProcessError error);
    void onScriptOutput();
    void onScriptErrorOutput();

private:
    // Script and workflow management
    struct ScriptExecution
    {
        QString executionId;
        ScriptConfig config;
        QProcess* process = nullptr;
        ScriptResult result;
        QDateTime startTime;
        bool isRunning = false;
        QStringList output;
        QStringList errors;
    };

    struct WorkflowExecution
    {
        QString executionId;
        WorkflowConfig config;
        QList<ScriptExecution> scriptExecutions;
        QMap<QString, QString> parameters;
        QDateTime startTime;
        bool isRunning = false;
        int currentScriptIndex = 0;
        int retryCount = 0;
    };

    struct DevelopmentScripts
    {
        DevelopmentScriptsConfig config;
        DevelopmentScriptsStats stats;
        QMap<QString, ScriptConfig> scripts;
        QMap<QString, WorkflowConfig> workflows;
        QMap<QString, ScriptExecution> scriptExecutions;
        QMap<QString, WorkflowExecution> workflowExecutions;
        QMutex mutex;
        QTimer* scriptTimer = nullptr;
        QTimer* workflowTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        bool isActive = true;
    };

    // Core execution operations
    void executeScriptInternal(DevelopmentScripts& scripts, const QString& name, const QStringList& arguments);
    void executeWorkflowInternal(DevelopmentScripts& scripts, const QString& name, const QMap<QString, QString>& parameters);
    void handleScriptCompletion(DevelopmentScripts& scripts, const QString& executionId, int exitCode);
    void handleWorkflowCompletion(DevelopmentScripts& scripts, const QString& executionId);

    // Script execution functions
    void startScriptExecution(DevelopmentScripts& scripts, const QString& name, const QStringList& arguments);
    void stopScriptExecution(DevelopmentScripts& scripts, const QString& executionId);
    void cleanupScriptExecution(DevelopmentScripts& scripts, const QString& executionId);
    void updateScriptResult(DevelopmentScripts& scripts, const QString& executionId, const ScriptResult& result);

    // Workflow execution functions
    void startWorkflowExecution(DevelopmentScripts& scripts, const QString& name, const QMap<QString, QString>& parameters);
    void stopWorkflowExecution(DevelopmentScripts& scripts, const QString& executionId);
    void executeNextWorkflowScript(DevelopmentScripts& scripts, const QString& executionId);
    void handleWorkflowScriptCompletion(DevelopmentScripts& scripts, const QString& executionId, const QString& scriptExecutionId);

    // Build automation functions
    void executeBuildScript(DevelopmentScripts& scripts, const QString& buildType);
    void handleBuildCompletion(DevelopmentScripts& scripts, const QString& executionId, const ScriptResult& result);
    void validateBuildResult(DevelopmentScripts& scripts, const ScriptResult& result);

    // Test automation functions
    void executeTestScript(DevelopmentScripts& scripts, const QString& testSuite);
    void handleTestCompletion(DevelopmentScripts& scripts, const QString& executionId, const ScriptResult& result);
    void validateTestResult(DevelopmentScripts& scripts, const ScriptResult& result);

    // Deployment automation functions
    void executeDeploymentScript(DevelopmentScripts& scripts, const QString& environment);
    void handleDeploymentCompletion(DevelopmentScripts& scripts, const QString& executionId, const ScriptResult& result);
    void validateDeploymentResult(DevelopmentScripts& scripts, const ScriptResult& result);

    // Maintenance automation functions
    void executeMaintenanceScript(DevelopmentScripts& scripts, const QString& task);
    void handleMaintenanceCompletion(DevelopmentScripts& scripts, const QString& executionId, const ScriptResult& result);
    void validateMaintenanceResult(DevelopmentScripts& scripts, const ScriptResult& result);

    // Statistics
    void updateDevelopmentScriptsStatistics(const QString& name);
    void calculateDevelopmentScriptsMetrics(DevelopmentScripts& scripts);
    void logDevelopmentScriptsEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateExecutionId() const;
    QString generateScriptId() const;
    bool isValidScript(const ScriptConfig& config) const;
    bool isValidWorkflow(const WorkflowConfig& config) const;
    void loadDevelopmentScriptsFromDisk(const QString& name);
    void saveDevelopmentScriptsToDisk(const QString& name);

    // Development scripts storage
    QMap<QString, std::unique_ptr<DevelopmentScripts>> m_developmentScripts;

    // Timers
    QTimer* m_globalScriptTimer = nullptr;
    QTimer* m_globalWorkflowTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_loggingEnabled = true;
    bool m_scheduledExecutionEnabled = true;
    bool m_parallelExecutionEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastExecution;
    QMap<QString, QDateTime> m_lastWorkflow;
    QMap<QString, double> m_executionTimes;

    // Process management
    QMap<QProcess*, QString> m_processExecutions;
    QMap<QString, QProcess*> m_executionProcesses;

    Q_DISABLE_COPY(DevelopmentScriptsManager)
};

} // namespace LegacyStream

#endif // DEVELOPMENTSCRIPTSMANAGER_H 