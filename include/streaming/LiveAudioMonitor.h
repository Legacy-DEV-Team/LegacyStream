#ifndef LIVEAUDIOMONITOR_H
#define LIVEAUDIOMONITOR_H

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
 * @brief Audio analysis data
 */
struct AudioAnalysisData
{
    double rms = 0.0;
    double peak = 0.0;
    double crest = 0.0;
    double dynamicRange = 0.0;
    double frequency = 0.0;
    double phase = 0.0;
    double distortion = 0.0;
    double noise = 0.0;
    QList<double> spectrum;
    QList<double> waveform;
    QDateTime timestamp;
    QString streamId;
    QString mountPoint;
};

/**
 * @brief Audio quality metrics
 */
struct AudioQualityMetrics
{
    double overallQuality = 0.0;
    double clarity = 0.0;
    double loudness = 0.0;
    double balance = 0.0;
    double depth = 0.0;
    double width = 0.0;
    double stereo = 0.0;
    double bass = 0.0;
    double mid = 0.0;
    double treble = 0.0;
    QDateTime timestamp;
    QString streamId;
};

/**
 * @brief Audio monitoring configuration
 */
struct AudioMonitorConfig
{
    QString name;
    int sampleRate = 44100;
    int channels = 2;
    int bufferSize = 4096;
    int fftSize = 2048;
    bool enableRealTimeAnalysis = true;
    bool enableQualityMetrics = true;
    bool enableSpectrumAnalysis = true;
    bool enableWaveformAnalysis = true;
    bool enableAlerts = true;
    double qualityThreshold = 0.7;
    double volumeThreshold = -20.0;
    int analysisInterval = 100; // 100ms
    bool enableLogging = true;
};

/**
 * @brief Audio alert
 */
struct AudioAlert
{
    QString type; // "quality", "volume", "distortion", "noise"
    QString severity; // "warning", "critical"
    QString message;
    double currentValue;
    double threshold;
    QDateTime timestamp;
    QString streamId;
    QJsonObject context;
};

/**
 * @brief Audio monitoring statistics
 */
struct AudioMonitorStats
{
    int totalAnalyses = 0;
    int qualityAlerts = 0;
    int volumeAlerts = 0;
    int distortionAlerts = 0;
    double averageQuality = 0.0;
    double averageVolume = 0.0;
    double peakVolume = 0.0;
    QDateTime lastAnalysis;
    QDateTime lastAlert;
    QMap<QString, int> alertsByType;
    QMap<QString, double> qualityByStream;
    QMap<QString, double> volumeByStream;
};

/**
 * @brief Live audio monitor for real-time audio analysis
 * 
 * Provides comprehensive real-time audio monitoring with quality metrics,
 * spectrum analysis, waveform analysis, and automated alerts.
 */
class LiveAudioMonitor : public QObject
{
    Q_OBJECT

public:
    explicit LiveAudioMonitor(QObject* parent = nullptr);
    ~LiveAudioMonitor();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Audio monitoring
    bool createMonitor(const QString& name, const AudioMonitorConfig& config);
    void destroyMonitor(const QString& name);
    bool monitorExists(const QString& name) const;
    QStringList getMonitorNames() const;

    // Audio analysis
    void processAudioData(const QByteArray& audioData, const QString& streamId);
    void analyzeAudioBuffer(const QByteArray& buffer, const QString& streamId);
    AudioAnalysisData getLatestAnalysis(const QString& streamId) const;
    AudioQualityMetrics getLatestQualityMetrics(const QString& streamId) const;

    // Real-time analysis
    void enableRealTimeAnalysis(const QString& name, bool enabled);
    void setAnalysisInterval(const QString& name, int interval);
    void setQualityThreshold(const QString& name, double threshold);
    void setVolumeThreshold(const QString& name, double threshold);

    // Quality metrics
    void enableQualityMetrics(const QString& name, bool enabled);
    void calculateQualityMetrics(const QString& streamId);
    double getOverallQuality(const QString& streamId) const;
    double getClarity(const QString& streamId) const;
    double getLoudness(const QString& streamId) const;

    // Spectrum analysis
    void enableSpectrumAnalysis(const QString& name, bool enabled);
    void setFFTSize(const QString& name, int size);
    QList<double> getSpectrum(const QString& streamId) const;
    QList<double> getWaveform(const QString& streamId) const;

    // Alerts and notifications
    void enableAlerts(const QString& name, bool enabled);
    void setAlertThresholds(const QString& name, double quality, double volume);
    QList<AudioAlert> getRecentAlerts(const QString& name, int count = 50) const;
    void clearAlerts(const QString& name);

    // Statistics and monitoring
    AudioMonitorStats getMonitorStats(const QString& name) const;
    QJsonObject getAllMonitorStatsJson() const;
    void resetMonitorStats(const QString& name);
    void exportMonitorStats(const QString& filePath) const;

    // Advanced features
    void enableLogging(const QString& name, bool enabled);
    void setLogLevel(const QString& name, const QString& level);
    void enableStreamFilter(const QString& name, const QStringList& streams);
    void setAnalysisMode(const QString& name, const QString& mode);

    // Utility functions
    bool isAudioHealthy(const QString& streamId) const;
    double getAverageVolume(const QString& streamId) const;
    double getPeakVolume(const QString& streamId) const;
    double getDistortionLevel(const QString& streamId) const;

signals:
    void audioAnalysisCompleted(const QString& streamId, const AudioAnalysisData& data);
    void qualityMetricsUpdated(const QString& streamId, const AudioQualityMetrics& metrics);
    void audioAlert(const AudioAlert& alert);
    void qualityWarning(const QString& streamId, double quality, double threshold);
    void volumeWarning(const QString& streamId, double volume, double threshold);
    void statisticsUpdated(const QString& name, const AudioMonitorStats& stats);

public slots:
    void onAnalysisTimer();
    void onAlertTimer();
    void onStatisticsTimer();

private slots:
    void onAudioDataReceived(const QByteArray& data, const QString& streamId);
    void onQualityCheck();
    void onVolumeCheck();

private:
    // Audio monitoring
    struct AudioMonitor
    {
        AudioMonitorConfig config;
        AudioMonitorStats stats;
        QMap<QString, AudioAnalysisData> latestAnalyses;
        QMap<QString, AudioQualityMetrics> latestQualityMetrics;
        QMap<QString, QList<double>> audioBuffers;
        QList<AudioAlert> alerts;
        QMutex mutex;
        QTimer* analysisTimer = nullptr;
        QTimer* alertTimer = nullptr;
        QTimer* statisticsTimer = nullptr;
        bool isActive = true;
    };

    // Core monitoring operations
    void performAudioAnalysis(AudioMonitor& monitor, const QString& streamId);
    void calculateQualityMetrics(AudioMonitor& monitor, const QString& streamId);
    void checkAudioAlerts(AudioMonitor& monitor, const QString& streamId);
    void generateAudioAlert(AudioMonitor& monitor, const QString& type, double value, double threshold, const QString& streamId);

    // Audio analysis
    AudioAnalysisData analyzeAudioBuffer(const QByteArray& buffer);
    AudioQualityMetrics calculateQualityMetrics(const AudioAnalysisData& analysis);
    QList<double> performFFT(const QByteArray& buffer);
    QList<double> extractWaveform(const QByteArray& buffer);

    // Quality calculations
    double calculateRMS(const QByteArray& buffer);
    double calculatePeak(const QByteArray& buffer);
    double calculateCrest(const QByteArray& buffer);
    double calculateDynamicRange(const QByteArray& buffer);
    double calculateFrequency(const QByteArray& buffer);
    double calculateDistortion(const QByteArray& buffer);
    double calculateNoise(const QByteArray& buffer);

    // Alert management
    void processAudioAlert(AudioMonitor& monitor, const AudioAlert& alert);
    void logAudioAlert(const QString& name, const AudioAlert& alert);
    bool shouldGenerateAlert(const QString& type, double value, double threshold);

    // Statistics
    void updateMonitorStatistics(const QString& name);
    void calculateMonitorMetrics(AudioMonitor& monitor);
    void logMonitorEvent(const QString& name, const QString& event);

    // Utility functions
    QString generateMonitorId() const;
    bool isAudioValid(const AudioAnalysisData& analysis) const;
    void loadMonitorFromDisk(const QString& name);
    void saveMonitorToDisk(const QString& name);

    // Monitor storage
    QMap<QString, std::unique_ptr<AudioMonitor>> m_monitors;

    // Timers
    QTimer* m_globalAnalysisTimer = nullptr;
    QTimer* m_globalAlertTimer = nullptr;
    QTimer* m_globalStatisticsTimer = nullptr;

    // State management
    QMutex m_globalMutex;
    bool m_isInitialized = false;
    bool m_alertsEnabled = true;
    bool m_loggingEnabled = true;
    bool m_realTimeAnalysisEnabled = true;

    // Performance tracking
    QMap<QString, QDateTime> m_lastAnalysis;
    QMap<QString, QDateTime> m_lastAlert;
    QMap<QString, double> m_averageQuality;

    // Audio processing
    QMap<QString, QByteArray> m_audioBuffers;
    QMap<QString, QList<double>> m_spectrumData;
    QMap<QString, QList<double>> m_waveformData;

    Q_DISABLE_COPY(LiveAudioMonitor)
};

} // namespace LegacyStream

#endif // LIVEAUDIOMONITOR_H 