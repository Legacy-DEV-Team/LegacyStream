#ifndef CODEQUALITYMANAGER_H
#define CODEQUALITYMANAGER_H

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
 * @brief Code quality issue
 */
struct CodeQualityIssue
{
    QString type; // "error", "warning", "info", "style"
    QString severity; // "low", "medium", "high", "critical"
    QString message;
    QString file;
    int line = 0;
    int column = 0;
    QString rule;
    QString description;
    QString suggestion;
    QDateTime timestamp;
    QString category; // "naming", "complexity", "performance", "security", "style"
};

/**
 * @brief Code quality configuration
 */
struct CodeQualityConfig
{
    QString name;
    bool enableLinting = true;
    bool enableFormatting = true;
    bool enableAnalysis = true;
    bool enableAutoFix = false;
    int maxLineLength = 120;
    int maxFunctionLength = 50;
    int maxComplexity = 10;
    bool enableNamingConventions = true;
    bool enablePerformanceChecks = true;
    bool enableSecurityChecks = true;
    bool enableStyleChecks = true;
    QStringList ignoredFiles;
    QStringList ignoredPatterns;
    bool enableLogging = true;
};

/**
 * @brief Code quality statistics
 */
struct CodeQualityStats
{
    int totalIssues = 0;
    int errorCount = 0;
    int warningCount = 0;
    int infoCount = 0;
    int styleCount = 0;
    double averageComplexity = 0.0;
    double codeCoverage = 0.0;
    double maintainabilityIndex = 0.0;
    QDateTime lastAnalysis;
    QMap<QString, int> issuesByCategory;
    QMap<QString, int> issuesBySeverity;
    QMap<QString, int> issuesByFile;
};

/**
 * @brief Code quality manager for comprehensive code analysis
 *
 * Provides sophisticated code quality tools including linting, formatting,
 * complexity analysis, and automated code improvement suggestions.
 */
class CodeQualityManager : public QObject
{
    Q_OBJECT

public:
    explicit CodeQualityManager(QObject* parent = nullptr);
    ~CodeQualityManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Code quality analysis
    bool createAnalyzer(const QString& name, const CodeQualityConfig& config);
    void destroyAnalyzer(const QString& name);
    bool analyzerExists(const QString& name) const;
    QStringList getAnalyzerNames() const;

    // Code analysis
    QList<CodeQualityIssue> analyzeFile(const QString& filePath, const QString& analyzerName = QString());
    QList<CodeQualityIssue> analyzeDirectory(const QString& directory, const QString& analyzerName = QString());
    QList<CodeQualityIssue> analyzeProject(const QString& projectPath, const QString& analyzerName = QString());
    void analyzeCodeInBackground(const QString& path, const QString& analyzerName = QString());

    // Code formatting
    bool formatFile(const QString& filePath, const QString& style = "default");
    bool formatDirectory(const QString& directory, const QString& style = "default");
    QString formatCode(const QString& code, const QString& language, const QString& style = "default");
    void setFormattingStyle(const QString& name, const QJsonObject& style);

    // Linting
    QList<CodeQualityIssue> lintFile(const QString& filePath);
    QList<CodeQualityIssue> lintDirectory(const QString& directory);
    void enableLinting(const QString& name, bool enabled);
    void setLintingRules(const QString& name, const QJsonObject& rules);

    // Code analysis
    void enableAnalysis(const QString& name, bool enabled);
    void setAnalysisRules(const QString& name, const QJsonObject& rules);
    double calculateComplexity(const QString& filePath);
    double calculateMaintainabilityIndex(const QString& filePath);
    double calculateCodeCoverage(const QString& filePath);

    // Auto-fix capabilities
    void enableAutoFix(const QString& name, bool enabled);
    bool autoFixIssues(const QString& filePath, const QList<CodeQualityIssue>& issues);
    QList<CodeQualityIssue> getAutoFixableIssues(const QList<CodeQualityIssue>& issues);

    // Configuration
    void setCodeQualityConfig(const QString& name, const CodeQualityConfig& config);
    CodeQualityConfig getCodeQualityConfig(const QString& name) const;
    void setMaxLineLength(const QString& name, int length);
    void setMaxFunctionLength(const QString& name, int length);
    void setMaxComplexity(const QString& name, int complexity);

    // Statistics and monitoring
    CodeQualityStats getAnalyzerStats(const QString& name) const;
    QJsonObject getAllAnalyzerStatsJson() const;
    void resetAnalyzerStats(const QString& name);
    void exportAnalyzerStats(const QString& filePath) const;

    // Advanced features
    void enableLogging(const QString& name, bool enabled);
    void setLogLevel(const QString& name, const QString& level);
    void enableCustomRules(const QString& name, bool enabled);
    void addCustomRule(const QString& name, const QJsonObject& rule);

    // Utility functions
    bool isCodeHealthy(const QString& filePath) const;
    double getOverallQuality(const QString& filePath) const;
    QStringList getSupportedLanguages() const;
    QStringList getSupportedStyles() const;

signals:
    void analysisCompleted(const QString& analyzerName, const QList<CodeQualityIssue>& issues);
    void formattingCompleted(const QString& filePath, bool success);
    void lintingCompleted(const QString& filePath, const QList<CodeQualityIssue>& issues);
    void qualityIssue(const CodeQualityIssue& issue);
    void statisticsUpdated(const QString& name, const CodeQualityStats& stats);

public slots:
    void onAnalysisTimer();
    void onBackgroundAnalysis();
    void onStatisticsTimer();

private slots:
    void onFileChanged(const QString& filePath);
    void onDirectoryChanged(const QString& directory);
    void onProjectChanged(const QString& projectPath);

private:
    // Code quality analysis
    struct CodeQualityAnalyzer
    {
        CodeQualityConfig config;
        CodeQualityStats stats;
        QMap<QString, QList<CodeQualityIssue>> fileIssues;
        QMap<QString, QJsonObject> formattingStyles;
        QMap<QString, QJsonObject> lintingRules;
        QMap<QString, QJsonObject> analysisRules;
        QMutex mutex;
        QTimer* analysisTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        bool isActive = true;
    };

    // Core analysis operations
    void performCodeAnalysis(CodeQualityAnalyzer& analyzer, const QString& filePath);
    void performCodeFormatting(CodeQualityAnalyzer& analyzer, const QString& filePath);
    void performCodeLinting(CodeQualityAnalyzer& analyzer, const QString& filePath);
    void generateQualityIssues(CodeQualityAnalyzer& analyzer, const QString& filePath);

    // Code analysis functions
    QList<CodeQualityIssue> analyzeCodeStyle(const QString& code, const QString& language);
    QList<CodeQualityIssue> analyzeCodeComplexity(const QString& code, const QString& language);
    QList<CodeQualityIssue> analyzeCodePerformance(const QString& code, const QString& language);
    QList<CodeQualityIssue> analyzeCodeSecurity(const QString& code, const QString& language);

    // Formatting functions
    QString formatCodeStyle(const QString& code, const QString& language, const QJsonObject& style);
    QString formatCodeIndentation(const QString& code, const QString& language);
    QString formatCodeSpacing(const QString& code, const QString& language);
    QString formatCodeNaming(const QString& code, const QString& language);

    // Linting functions
    QList<CodeQualityIssue> lintCodeStyle(const QString& code, const QString& language);
    QList<CodeQualityIssue> lintCodeNaming(const QString& code, const QString& language);
    QList<CodeQualityIssue> lintCodeComplexity(const QString& code, const QString& language);
    QList<CodeQualityIssue> lintCodeSecurity(const QString& code, const QString& language);

    // Statistics
    void updateAnalyzerStatistics(const QString& name);
    void calculateAnalyzerMetrics(CodeQualityAnalyzer& analyzer);
    void logAnalyzerEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateAnalyzerId() const;
    bool isFileSupported(const QString& filePath) const;
    QString detectLanguage(const QString& filePath) const;
    void loadAnalyzerFromDisk(const QString& name);
    void saveAnalyzerToDisk(const QString& name);

    // Analyzer storage
    QMap<QString, std::unique_ptr<CodeQualityAnalyzer>> m_analyzers;

    // Timers
    QTimer* m_globalAnalysisTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;
    QTimer* m_backgroundAnalysisTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_loggingEnabled = true;
    bool m_autoFixEnabled = false;
    bool m_backgroundAnalysisEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastAnalysis;
    QMap<QString, QDateTime> m_lastFormatting;
    QMap<QString, double> m_analysisTimes;

    // File watchers
    QMap<QString, QFileSystemWatcher*> m_fileWatchers;
    QMap<QString, QStringList> m_watchedFiles;

    Q_DISABLE_COPY(CodeQualityManager)
};

} // namespace LegacyStream

#endif // CODEQUALITYMANAGER_H 