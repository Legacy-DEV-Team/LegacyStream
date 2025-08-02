#ifndef AUDIOPROCESSOR_H
#define AUDIOPROCESSOR_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QAudioFormat>
#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioOutput>
#include <QAudioInput>
#include <QAudioDeviceInfo>

namespace LegacyStream {

/**
 * @brief Audio effect types
 */
enum class AudioEffectType
{
    NONE,
    EQUALIZER,
    COMPRESSOR,
    REVERB,
    DELAY,
    CHORUS,
    FLANGER,
    DISTORTION,
    FILTER_LOW_PASS,
    FILTER_HIGH_PASS,
    FILTER_BAND_PASS,
    NORMALIZER,
    NOISE_REDUCTION
};

/**
 * @brief Audio filter configuration
 */
struct AudioFilterConfig
{
    AudioEffectType type = AudioEffectType::NONE;
    bool enabled = false;
    double intensity = 1.0; // 0.0 to 1.0
    QMap<QString, double> parameters;
    
    // Equalizer specific
    QMap<int, double> frequencyBands; // frequency -> gain
    
    // Compressor specific
    double threshold = -20.0; // dB
    double ratio = 4.0;
    double attack = 10.0; // ms
    double release = 100.0; // ms
    
    // Filter specific
    double cutoffFrequency = 1000.0; // Hz
    double resonance = 0.5;
    
    // Reverb specific
    double roomSize = 0.5;
    double damping = 0.5;
    double wetLevel = 0.33;
    double dryLevel = 0.4;
};
/**
 * @brief Audio analysis results
 */
struct AudioAnalysis
{
    double rms = 0.0; // Root Mean Square
    double peak = 0.0; // Peak amplitude
    double dynamicRange = 0.0; // Dynamic range in dB
    double spectralCentroid = 0.0; // Spectral centroid frequency
    double spectralRolloff = 0.0; // Spectral rolloff frequency
    double zeroCrossingRate = 0.0; // Zero crossing rate
    QMap<int, double> spectrum; // Frequency spectrum
    QMap<int, double> mfcc; // Mel-frequency cepstral coefficients
    bool isClipping = false;
    double snr = 0.0; // Signal-to-noise ratio
    QDateTime timestamp;
};

/**
 * @brief Audio format information
 */
struct AudioFormatInfo
{
    QString format; // "mp3", "aac", "ogg", "flac", "wav", etc.
    int sampleRate = 44100;
    int channels = 2;
    int bitDepth = 16;
    int bitrate = 128000;
    double duration = 0.0; // seconds
    qint64 fileSize = 0;
    QString codec;
    bool isLossless = false;
};

/**
 * @brief Audio synchronization information
 */
struct AudioSyncInfo
{
    QString streamId;
    qint64 timestamp = 0;
    double offset = 0.0; // seconds
    double drift = 0.0; // seconds per minute
    bool isSynchronized = false;
    QDateTime lastSync;
};

/**
 * @brief AudioProcessor for advanced audio processing and analysis
 * 
 * Provides advanced audio processing features including effects, filters,
 * real-time analysis, format conversion, and multi-stream synchronization.
 */
class AudioProcessor : public QObject
{
    Q_OBJECT

public:
    explicit AudioProcessor(QObject* parent = nullptr);
    ~AudioProcessor();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool start();
    void stop();

    // Audio processing
    QByteArray processAudio(const QByteArray& inputData, const QString& streamId);
    QByteArray applyEffects(const QByteArray& audioData, const QString& streamId);
    QByteArray applyFilter(const QByteArray& audioData, const AudioFilterConfig& filter);
    QByteArray normalizeAudio(const QByteArray& audioData);
    QByteArray reduceNoise(const QByteArray& audioData);

    // Effects management
    void addEffect(const QString& streamId, const AudioFilterConfig& effect);
    void removeEffect(const QString& streamId, AudioEffectType effectType);
    void updateEffect(const QString& streamId, const AudioFilterConfig& effect);
    QList<AudioFilterConfig> getEffects(const QString& streamId) const;
    void clearEffects(const QString& streamId);

    // Audio analysis
    AudioAnalysis analyzeAudio(const QByteArray& audioData, const QString& streamId);
    QJsonObject getAnalysisJson(const AudioAnalysis& analysis) const;
    void startRealTimeAnalysis(const QString& streamId, bool enabled);
    bool isRealTimeAnalysisEnabled(const QString& streamId) const;

    // Format conversion
    QByteArray convertFormat(const QByteArray& audioData, const QString& fromFormat, 
                            const QString& toFormat, const AudioFormatInfo& targetFormat);
    AudioFormatInfo detectFormat(const QByteArray& audioData);
    QJsonObject getFormatInfoJson(const AudioFormatInfo& format) const;
    bool isFormatSupported(const QString& format) const;

    // Audio synchronization
    void synchronizeStreams(const QStringList& streamIds);
    void setStreamOffset(const QString& streamId, double offset);
    double getStreamOffset(const QString& streamId) const;
    AudioSyncInfo getSyncInfo(const QString& streamId) const;
    void resetSynchronization(const QString& streamId);

    // Audio quality control
    void setQualitySettings(const QString& streamId, const QJsonObject& settings);
    QJsonObject getQualitySettings(const QString& streamId) const;
    void enableQualityMonitoring(const QString& streamId, bool enabled);
    bool isQualityMonitoringEnabled(const QString& streamId) const;

    // Statistics and monitoring
    QJsonObject getProcessingStats() const;
    QJsonObject getStreamProcessingStats(const QString& streamId) const;
    void resetStats();

signals:
    void audioProcessed(const QString& streamId, const QByteArray& processedData);
    void analysisUpdated(const QString& streamId, const AudioAnalysis& analysis);
    void effectApplied(const QString& streamId, AudioEffectType effectType);
    void formatConverted(const QString& streamId, const QString& fromFormat, const QString& toFormat);
    void streamsSynchronized(const QStringList& streamIds);
    void qualityAlert(const QString& streamId, const QString& alert);
    void processingError(const QString& streamId, const QString& error);

private slots:
    void onAnalysisTimer();
    void onSyncTimer();
    void onQualityCheckTimer();

private:
    // Core processing functions
    QByteArray applyEqualizer(const QByteArray& audioData, const AudioFilterConfig& config);
    QByteArray applyCompressor(const QByteArray& audioData, const AudioFilterConfig& config);
    QByteArray applyReverb(const QByteArray& audioData, const AudioFilterConfig& config);
    QByteArray applyDelay(const QByteArray& audioData, const AudioFilterConfig& config);
    QByteArray applyFilter(const QByteArray& audioData, const AudioFilterConfig& config);
    QByteArray applyChorus(const QByteArray& audioData, const AudioFilterConfig& config);
    QByteArray applyFlanger(const QByteArray& audioData, const AudioFilterConfig& config);
    QByteArray applyDistortion(const QByteArray& audioData, const AudioFilterConfig& config);

    // Analysis functions
    double calculateRMS(const QByteArray& audioData);
    double calculatePeak(const QByteArray& audioData);
    double calculateDynamicRange(const QByteArray& audioData);
    double calculateSpectralCentroid(const QByteArray& audioData);
    double calculateSpectralRolloff(const QByteArray& audioData);
    double calculateZeroCrossingRate(const QByteArray& audioData);
    QMap<int, double> calculateSpectrum(const QByteArray& audioData);
    QMap<int, double> calculateMFCC(const QByteArray& audioData);
    bool detectClipping(const QByteArray& audioData);
    double calculateSNR(const QByteArray& audioData);

    // Utility functions
    QByteArray resampleAudio(const QByteArray& audioData, int fromSampleRate, int toSampleRate);
    QByteArray convertChannels(const QByteArray& audioData, int fromChannels, int toChannels);
    QByteArray convertBitDepth(const QByteArray& audioData, int fromBitDepth, int toBitDepth);
    QString formatToString(AudioEffectType effect) const;
    AudioEffectType stringToFormat(const QString& format) const;

    // Timers
    QTimer* m_analysisTimer = nullptr;
    QTimer* m_syncTimer = nullptr;
    QTimer* m_qualityCheckTimer = nullptr;

    // State management
    QAtomicInt m_isRunning = 0;
    QMutex m_mutex;

    // Effects and processing
    QMap<QString, QList<AudioFilterConfig>> m_streamEffects;
    QMap<QString, QByteArray> m_effectBuffers; // For effects that need state

    // Analysis
    QMap<QString, bool> m_realTimeAnalysisEnabled;
    QMap<QString, AudioAnalysis> m_lastAnalysis;
    QMap<QString, QList<AudioAnalysis>> m_analysisHistory;

    // Synchronization
    QMap<QString, AudioSyncInfo> m_syncInfo;
    QStringList m_synchronizedStreams;

    // Quality monitoring
    QMap<QString, bool> m_qualityMonitoringEnabled;
    QMap<QString, QJsonObject> m_qualitySettings;
    QMap<QString, QList<double>> m_qualityHistory;

    // Statistics
    QMap<QString, qint64> m_processedBytes;
    QMap<QString, qint64> m_processingTime;
    QMap<QString, int> m_effectApplications;
    QMap<QString, int> m_formatConversions;

    // Audio devices
    QAudioDeviceInfo m_inputDevice;
    QAudioDeviceInfo m_outputDevice;
    QAudioFormat m_audioFormat;

    // Processing settings
    int m_bufferSize = 4096;
    int m_sampleRate = 44100;
    int m_channels = 2;
    int m_bitDepth = 16;

    Q_DISABLE_COPY(AudioProcessor)
};

} // namespace LegacyStream

#endif // AUDIOPROCESSOR_H 