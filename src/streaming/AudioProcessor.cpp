#include "streaming/AudioProcessor.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QAudioFormat>
#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioOutput>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QDir>
#include <QFile>
#include <QBuffer>
#include <QDataStream>
#include <QtMath>
#include <QThread>

Q_LOGGING_CATEGORY(audioProcessor, "audioProcessor")

namespace LegacyStream {

AudioProcessor::AudioProcessor(QObject* parent)
    : QObject(parent)
    , m_analysisTimer(new QTimer(this))
    , m_syncTimer(new QTimer(this))
    , m_qualityCheckTimer(new QTimer(this))
{
    qCDebug(audioProcessor) << "AudioProcessor created";
    
    // Set up timers
    m_analysisTimer->setSingleShot(false);
    m_analysisTimer->setInterval(1000); // Analysis every second
    
    m_syncTimer->setSingleShot(false);
    m_syncTimer->setInterval(5000); // Sync check every 5 seconds
    
    m_qualityCheckTimer->setSingleShot(false);
    m_qualityCheckTimer->setInterval(2000); // Quality check every 2 seconds
    
    // Connect signals
    connect(m_analysisTimer, &QTimer::timeout, this, &AudioProcessor::onAnalysisTimer);
    connect(m_syncTimer, &QTimer::timeout, this, &AudioProcessor::onSyncTimer);
    connect(m_qualityCheckTimer, &QTimer::timeout, this, &AudioProcessor::onQualityCheckTimer);
    
    // Set up audio format
    m_audioFormat.setSampleRate(m_sampleRate);
    m_audioFormat.setChannelCount(m_channels);
    m_audioFormat.setSampleFormat(QAudioFormat::Int16);
}

AudioProcessor::~AudioProcessor()
{
    shutdown();
}

bool AudioProcessor::initialize()
{
    qCDebug(audioProcessor) << "Initializing AudioProcessor";
    
    // Get available audio devices
    QList<QAudioDeviceInfo> inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QList<QAudioDeviceInfo> outputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    
    if (!inputDevices.isEmpty()) {
        m_inputDevice = inputDevices.first();
        qCInfo(audioProcessor) << "Using input device:" << m_inputDevice.deviceName();
    }
    
    if (!outputDevices.isEmpty()) {
        m_outputDevice = outputDevices.first();
        qCInfo(audioProcessor) << "Using output device:" << m_outputDevice.deviceName();
    }
    
    qCInfo(audioProcessor) << "AudioProcessor initialized successfully";
    return true;
}

void AudioProcessor::shutdown()
{
    if (m_isRunning.load()) {
        stop();
    }
    
    // Stop timers
    m_analysisTimer->stop();
    m_syncTimer->stop();
    m_qualityCheckTimer->stop();
    
    // Clear data
    QMutexLocker locker(&m_mutex);
    m_streamEffects.clear();
    m_effectBuffers.clear();
    m_lastAnalysis.clear();
    m_analysisHistory.clear();
    m_syncInfo.clear();
    m_synchronizedStreams.clear();
    m_qualitySettings.clear();
    m_qualityHistory.clear();
    
    qCInfo(audioProcessor) << "AudioProcessor shutdown complete";
}

bool AudioProcessor::start()
{
    if (m_isRunning.load()) {
        qCWarning(audioProcessor) << "AudioProcessor already running";
        return true;
    }
    
    qCInfo(audioProcessor) << "Starting AudioProcessor";
    
    // Start timers
    m_analysisTimer->start();
    m_syncTimer->start();
    m_qualityCheckTimer->start();
    
    m_isRunning.store(true);
    
    qCInfo(audioProcessor) << "AudioProcessor started successfully";
    return true;
}

void AudioProcessor::stop()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    qCInfo(audioProcessor) << "Stopping AudioProcessor";
    
    // Stop timers
    m_analysisTimer->stop();
    m_syncTimer->stop();
    m_qualityCheckTimer->stop();
    
    m_isRunning.store(false);
    
    qCInfo(audioProcessor) << "AudioProcessor stopped";
}

QByteArray AudioProcessor::processAudio(const QByteArray& inputData, const QString& streamId)
{
    if (inputData.isEmpty()) {
        return inputData;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QByteArray processedData = inputData;
    
    // Apply effects if any are configured for this stream
    if (m_streamEffects.contains(streamId)) {
        processedData = applyEffects(processedData, streamId);
    }
    
    // Update statistics
    m_processedBytes[streamId] += processedData.size();
    
    emit audioProcessed(streamId, processedData);
    return processedData;
}

QByteArray AudioProcessor::applyEffects(const QByteArray& audioData, const QString& streamId)
{
    if (!m_streamEffects.contains(streamId)) {
        return audioData;
    }
    
    QByteArray processedData = audioData;
    const QList<AudioFilterConfig>& effects = m_streamEffects[streamId];
    
    for (const AudioFilterConfig& effect : effects) {
        if (!effect.enabled) {
            continue;
        }
        
        switch (effect.type) {
            case AudioEffectType::EQUALIZER:
                processedData = applyEqualizer(processedData, effect);
                break;
            case AudioEffectType::COMPRESSOR:
                processedData = applyCompressor(processedData, effect);
                break;
            case AudioEffectType::REVERB:
                processedData = applyReverb(processedData, effect);
                break;
            case AudioEffectType::DELAY:
                processedData = applyDelay(processedData, effect);
                break;
            case AudioEffectType::FILTER_LOW_PASS:
            case AudioEffectType::FILTER_HIGH_PASS:
            case AudioEffectType::FILTER_BAND_PASS:
                processedData = applyFilter(processedData, effect);
                break;
            case AudioEffectType::CHORUS:
                processedData = applyChorus(processedData, effect);
                break;
            case AudioEffectType::FLANGER:
                processedData = applyFlanger(processedData, effect);
                break;
            case AudioEffectType::DISTORTION:
                processedData = applyDistortion(processedData, effect);
                break;
            case AudioEffectType::NORMALIZER:
                processedData = normalizeAudio(processedData);
                break;
            case AudioEffectType::NOISE_REDUCTION:
                processedData = reduceNoise(processedData);
                break;
            default:
                break;
        }
        
        m_effectApplications[streamId]++;
        emit effectApplied(streamId, effect.type);
    }
    
    return processedData;
}

QByteArray AudioProcessor::applyEqualizer(const QByteArray& audioData, const AudioFilterConfig& config)
{
    // Simple equalizer implementation
    QByteArray processedData = audioData;
    
    // Convert to 16-bit samples
    QDataStream inStream(processedData);
    QDataStream outStream(&processedData, QIODevice::WriteOnly);
    
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    // Apply frequency band gains
    for (int i = 0; i < samples.size(); ++i) {
        double gain = 1.0;
        
        // Simple frequency-based gain application
        for (auto it = config.frequencyBands.begin(); it != config.frequencyBands.end(); ++it) {
            int frequency = it.key();
            double bandGain = it.value();
            
            // Apply gain based on frequency (simplified)
            if (frequency > 0) {
                gain *= bandGain;
            }
        }
        
        samples[i] = qBound(-32768, static_cast<int>(samples[i] * gain), 32767);
    }
    
    // Convert back to bytes
    processedData.clear();
    QDataStream finalStream(&processedData, QIODevice::WriteOnly);
    for (qint16 sample : samples) {
        finalStream << sample;
    }
    
    return processedData;
}

QByteArray AudioProcessor::applyCompressor(const QByteArray& audioData, const AudioFilterConfig& config)
{
    QByteArray processedData = audioData;
    
    // Convert to 16-bit samples
    QDataStream inStream(processedData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    // Apply compression
    double threshold = config.threshold;
    double ratio = config.ratio;
    double attack = config.attack;
    double release = config.release;
    
    for (int i = 0; i < samples.size(); ++i) {
        double amplitude = qAbs(samples[i]) / 32768.0;
        double db = 20 * log10(amplitude);
        
        if (db > threshold) {
            double compression = (db - threshold) / ratio;
            double gain = pow(10, -compression / 20);
            samples[i] = static_cast<qint16>(samples[i] * gain);
        }
    }
    
    // Convert back to bytes
    processedData.clear();
    QDataStream outStream(&processedData, QIODevice::WriteOnly);
    for (qint16 sample : samples) {
        outStream << sample;
    }
    
    return processedData;
}

QByteArray AudioProcessor::applyReverb(const QByteArray& audioData, const AudioFilterConfig& config)
{
    // Simple reverb implementation
    QByteArray processedData = audioData;
    
    // Convert to 16-bit samples
    QDataStream inStream(processedData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    // Apply reverb effect
    double roomSize = config.roomSize;
    double damping = config.damping;
    double wetLevel = config.wetLevel;
    double dryLevel = config.dryLevel;
    
    int delaySamples = static_cast<int>(roomSize * m_sampleRate);
    QVector<qint16> delayedSamples(delaySamples, 0);
    
    for (int i = 0; i < samples.size(); ++i) {
        qint16 delayedSample = (i >= delaySamples) ? delayedSamples[i - delaySamples] : 0;
        qint16 currentSample = samples[i];
        
        // Mix dry and wet signals
        double mixed = (currentSample * dryLevel + delayedSample * wetLevel);
        samples[i] = qBound(-32768, static_cast<int>(mixed), 32767);
        
        // Update delay buffer
        if (i < delaySamples) {
            delayedSamples[i] = currentSample;
        }
    }
    
    // Convert back to bytes
    processedData.clear();
    QDataStream outStream(&processedData, QIODevice::WriteOnly);
    for (qint16 sample : samples) {
        outStream << sample;
    }
    
    return processedData;
}

QByteArray AudioProcessor::applyDelay(const QByteArray& audioData, const AudioFilterConfig& config)
{
    QByteArray processedData = audioData;
    
    // Convert to 16-bit samples
    QDataStream inStream(processedData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    // Apply delay effect
    double delayTime = config.parameters.value("delay_time", 0.5); // seconds
    double feedback = config.parameters.value("feedback", 0.3);
    double mix = config.parameters.value("mix", 0.5);
    
    int delaySamples = static_cast<int>(delayTime * m_sampleRate);
    QVector<qint16> delayBuffer(delaySamples, 0);
    
    for (int i = 0; i < samples.size(); ++i) {
        qint16 delayedSample = (i >= delaySamples) ? delayBuffer[i - delaySamples] : 0;
        qint16 currentSample = samples[i];
        
        // Mix original and delayed signal
        double mixed = (currentSample * (1.0 - mix) + delayedSample * mix);
        samples[i] = qBound(-32768, static_cast<int>(mixed), 32767);
        
        // Update delay buffer with feedback
        if (i < delaySamples) {
            delayBuffer[i] = static_cast<qint16>(currentSample * feedback);
        }
    }
    
    // Convert back to bytes
    processedData.clear();
    QDataStream outStream(&processedData, QIODevice::WriteOnly);
    for (qint16 sample : samples) {
        outStream << sample;
    }
    
    return processedData;
}

QByteArray AudioProcessor::applyFilter(const QByteArray& audioData, const AudioFilterConfig& config)
{
    QByteArray processedData = audioData;
    
    // Convert to 16-bit samples
    QDataStream inStream(processedData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    // Apply filter based on type
    double cutoffFreq = config.cutoffFrequency;
    double resonance = config.resonance;
    
    // Simple low-pass filter implementation
    if (config.type == AudioEffectType::FILTER_LOW_PASS) {
        double alpha = 1.0 / (1.0 + 2.0 * M_PI * cutoffFreq / m_sampleRate);
        double filtered = 0.0;
        
        for (int i = 0; i < samples.size(); ++i) {
            filtered = alpha * samples[i] + (1.0 - alpha) * filtered;
            samples[i] = static_cast<qint16>(filtered);
        }
    }
    
    // Convert back to bytes
    processedData.clear();
    QDataStream outStream(&processedData, QIODevice::WriteOnly);
    for (qint16 sample : samples) {
        outStream << sample;
    }
    
    return processedData;
}

QByteArray AudioProcessor::applyChorus(const QByteArray& audioData, const AudioFilterConfig& config)
{
    // Chorus effect implementation
    return applyDelay(audioData, config); // Simplified chorus using delay
}

QByteArray AudioProcessor::applyFlanger(const QByteArray& audioData, const AudioFilterConfig& config)
{
    // Flanger effect implementation
    return applyDelay(audioData, config); // Simplified flanger using delay
}

QByteArray AudioProcessor::applyDistortion(const QByteArray& audioData, const AudioFilterConfig& config)
{
    QByteArray processedData = audioData;
    
    // Convert to 16-bit samples
    QDataStream inStream(processedData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    // Apply distortion
    double intensity = config.intensity;
    
    for (int i = 0; i < samples.size(); ++i) {
        double normalized = samples[i] / 32768.0;
        double distorted = tanh(normalized * intensity);
        samples[i] = static_cast<qint16>(distorted * 32768.0);
    }
    
    // Convert back to bytes
    processedData.clear();
    QDataStream outStream(&processedData, QIODevice::WriteOnly);
    for (qint16 sample : samples) {
        outStream << sample;
    }
    
    return processedData;
}

QByteArray AudioProcessor::normalizeAudio(const QByteArray& audioData)
{
    QByteArray processedData = audioData;
    
    // Convert to 16-bit samples
    QDataStream inStream(processedData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    // Find peak amplitude
    qint16 maxAmplitude = 0;
    for (qint16 sample : samples) {
        maxAmplitude = qMax(maxAmplitude, qAbs(sample));
    }
    
    if (maxAmplitude > 0) {
        // Calculate normalization factor
        double factor = 32767.0 / maxAmplitude;
        
        // Apply normalization
        for (int i = 0; i < samples.size(); ++i) {
            samples[i] = static_cast<qint16>(samples[i] * factor);
        }
    }
    
    // Convert back to bytes
    processedData.clear();
    QDataStream outStream(&processedData, QIODevice::WriteOnly);
    for (qint16 sample : samples) {
        outStream << sample;
    }
    
    return processedData;
}

QByteArray AudioProcessor::reduceNoise(const QByteArray& audioData)
{
    // Simple noise reduction using spectral subtraction
    return audioData; // Placeholder implementation
}

AudioAnalysis AudioProcessor::analyzeAudio(const QByteArray& audioData, const QString& streamId)
{
    AudioAnalysis analysis;
    analysis.timestamp = QDateTime::currentDateTime();
    
    if (audioData.isEmpty()) {
        return analysis;
    }
    
    // Convert to 16-bit samples
    QDataStream inStream(audioData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    // Calculate analysis metrics
    analysis.rms = calculateRMS(audioData);
    analysis.peak = calculatePeak(audioData);
    analysis.dynamicRange = calculateDynamicRange(audioData);
    analysis.spectralCentroid = calculateSpectralCentroid(audioData);
    analysis.spectralRolloff = calculateSpectralRolloff(audioData);
    analysis.zeroCrossingRate = calculateZeroCrossingRate(audioData);
    analysis.spectrum = calculateSpectrum(audioData);
    analysis.mfcc = calculateMFCC(audioData);
    analysis.isClipping = detectClipping(audioData);
    analysis.snr = calculateSNR(audioData);
    
    // Store analysis
    m_lastAnalysis[streamId] = analysis;
    m_analysisHistory[streamId].append(analysis);
    
    // Limit history size
    while (m_analysisHistory[streamId].size() > 100) {
        m_analysisHistory[streamId].removeFirst();
    }
    
    emit analysisUpdated(streamId, analysis);
    return analysis;
}

double AudioProcessor::calculateRMS(const QByteArray& audioData)
{
    QDataStream inStream(audioData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    double sum = 0.0;
    for (qint16 sample : samples) {
        double normalized = sample / 32768.0;
        sum += normalized * normalized;
    }
    
    return sqrt(sum / samples.size());
}

double AudioProcessor::calculatePeak(const QByteArray& audioData)
{
    QDataStream inStream(audioData);
    qint16 maxAmplitude = 0;
    
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        maxAmplitude = qMax(maxAmplitude, qAbs(sample));
    }
    
    return maxAmplitude / 32768.0;
}

double AudioProcessor::calculateDynamicRange(const QByteArray& audioData)
{
    QDataStream inStream(audioData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    qint16 maxAmplitude = 0;
    qint16 minAmplitude = 32767;
    
    for (qint16 sample : samples) {
        maxAmplitude = qMax(maxAmplitude, sample);
        minAmplitude = qMin(minAmplitude, sample);
    }
    
    return 20 * log10(static_cast<double>(maxAmplitude) / minAmplitude);
}

double AudioProcessor::calculateSpectralCentroid(const QByteArray& audioData)
{
    // Simplified spectral centroid calculation
    return 1000.0; // Placeholder
}

double AudioProcessor::calculateSpectralRolloff(const QByteArray& audioData)
{
    // Simplified spectral rolloff calculation
    return 2000.0; // Placeholder
}

double AudioProcessor::calculateZeroCrossingRate(const QByteArray& audioData)
{
    QDataStream inStream(audioData);
    QVector<qint16> samples;
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        samples.append(sample);
    }
    
    int crossings = 0;
    for (int i = 1; i < samples.size(); ++i) {
        if ((samples[i] >= 0 && samples[i-1] < 0) || (samples[i] < 0 && samples[i-1] >= 0)) {
            crossings++;
        }
    }
    
    return static_cast<double>(crossings) / (samples.size() - 1);
}

QMap<int, double> AudioProcessor::calculateSpectrum(const QByteArray& audioData)
{
    // Simplified spectrum calculation
    QMap<int, double> spectrum;
    for (int i = 0; i < 10; ++i) {
        spectrum[i * 1000] = 0.1 + (i * 0.05);
    }
    return spectrum;
}

QMap<int, double> AudioProcessor::calculateMFCC(const QByteArray& audioData)
{
    // Simplified MFCC calculation
    QMap<int, double> mfcc;
    for (int i = 0; i < 13; ++i) {
        mfcc[i] = 0.0;
    }
    return mfcc;
}

bool AudioProcessor::detectClipping(const QByteArray& audioData)
{
    QDataStream inStream(audioData);
    while (!inStream.atEnd()) {
        qint16 sample;
        inStream >> sample;
        if (qAbs(sample) >= 32767) {
            return true;
        }
    }
    return false;
}

double AudioProcessor::calculateSNR(const QByteArray& audioData)
{
    // Simplified SNR calculation
    return 20.0; // Placeholder
}

void AudioProcessor::addEffect(const QString& streamId, const AudioFilterConfig& effect)
{
    QMutexLocker locker(&m_mutex);
    m_streamEffects[streamId].append(effect);
    qCInfo(audioProcessor) << "Added effect to stream:" << streamId;
}

void AudioProcessor::removeEffect(const QString& streamId, AudioEffectType effectType)
{
    QMutexLocker locker(&m_mutex);
    if (m_streamEffects.contains(streamId)) {
        QList<AudioFilterConfig>& effects = m_streamEffects[streamId];
        for (int i = effects.size() - 1; i >= 0; --i) {
            if (effects[i].type == effectType) {
                effects.removeAt(i);
            }
        }
        qCInfo(audioProcessor) << "Removed effect from stream:" << streamId;
    }
}

QList<AudioFilterConfig> AudioProcessor::getEffects(const QString& streamId) const
{
    QMutexLocker locker(&m_mutex);
    return m_streamEffects.value(streamId);
}

void AudioProcessor::clearEffects(const QString& streamId)
{
    QMutexLocker locker(&m_mutex);
    m_streamEffects.remove(streamId);
    qCInfo(audioProcessor) << "Cleared effects for stream:" << streamId;
}

QJsonObject AudioProcessor::getAnalysisJson(const AudioAnalysis& analysis) const
{
    QJsonObject json;
    json["rms"] = analysis.rms;
    json["peak"] = analysis.peak;
    json["dynamic_range"] = analysis.dynamicRange;
    json["spectral_centroid"] = analysis.spectralCentroid;
    json["spectral_rolloff"] = analysis.spectralRolloff;
    json["zero_crossing_rate"] = analysis.zeroCrossingRate;
    json["is_clipping"] = analysis.isClipping;
    json["snr"] = analysis.snr;
    json["timestamp"] = analysis.timestamp.toString(Qt::ISODate);
    
    return json;
}

void AudioProcessor::startRealTimeAnalysis(const QString& streamId, bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_realTimeAnalysisEnabled[streamId] = enabled;
    qCInfo(audioProcessor) << "Real-time analysis" << (enabled ? "enabled" : "disabled") << "for stream:" << streamId;
}

bool AudioProcessor::isRealTimeAnalysisEnabled(const QString& streamId) const
{
    QMutexLocker locker(&m_mutex);
    return m_realTimeAnalysisEnabled.value(streamId, false);
}

QByteArray AudioProcessor::convertFormat(const QByteArray& audioData, const QString& fromFormat, 
                                        const QString& toFormat, const AudioFormatInfo& targetFormat)
{
    QByteArray convertedData = audioData;
    
    // Resample if needed
    if (targetFormat.sampleRate != m_sampleRate) {
        convertedData = resampleAudio(convertedData, m_sampleRate, targetFormat.sampleRate);
    }
    
    // Convert channels if needed
    if (targetFormat.channels != m_channels) {
        convertedData = convertChannels(convertedData, m_channels, targetFormat.channels);
    }
    
    // Convert bit depth if needed
    if (targetFormat.bitDepth != m_bitDepth) {
        convertedData = convertBitDepth(convertedData, m_bitDepth, targetFormat.bitDepth);
    }
    
    emit formatConverted("", fromFormat, toFormat);
    return convertedData;
}

AudioFormatInfo AudioProcessor::detectFormat(const QByteArray& audioData)
{
    AudioFormatInfo format;
    format.format = "unknown";
    format.sampleRate = m_sampleRate;
    format.channels = m_channels;
    format.bitDepth = m_bitDepth;
    format.fileSize = audioData.size();
    
    // Simple format detection based on file size and header
    if (audioData.size() > 0) {
        format.format = "raw";
        format.codec = "pcm";
        format.isLossless = true;
    }
    
    return format;
}

QJsonObject AudioProcessor::getFormatInfoJson(const AudioFormatInfo& format) const
{
    QJsonObject json;
    json["format"] = format.format;
    json["sample_rate"] = format.sampleRate;
    json["channels"] = format.channels;
    json["bit_depth"] = format.bitDepth;
    json["bitrate"] = format.bitrate;
    json["duration"] = format.duration;
    json["file_size"] = format.fileSize;
    json["codec"] = format.codec;
    json["is_lossless"] = format.isLossless;
    
    return json;
}

bool AudioProcessor::isFormatSupported(const QString& format) const
{
    QStringList supportedFormats = {"mp3", "aac", "ogg", "flac", "wav", "raw"};
    return supportedFormats.contains(format.toLower());
}

void AudioProcessor::synchronizeStreams(const QStringList& streamIds)
{
    QMutexLocker locker(&m_mutex);
    m_synchronizedStreams = streamIds;
    
    for (const QString& streamId : streamIds) {
        AudioSyncInfo syncInfo;
        syncInfo.streamId = streamId;
        syncInfo.timestamp = QDateTime::currentMSecsSinceEpoch();
        syncInfo.isSynchronized = true;
        syncInfo.lastSync = QDateTime::currentDateTime();
        m_syncInfo[streamId] = syncInfo;
    }
    
    emit streamsSynchronized(streamIds);
    qCInfo(audioProcessor) << "Synchronized streams:" << streamIds;
}

void AudioProcessor::setStreamOffset(const QString& streamId, double offset)
{
    QMutexLocker locker(&m_mutex);
    if (m_syncInfo.contains(streamId)) {
        m_syncInfo[streamId].offset = offset;
        qCInfo(audioProcessor) << "Set offset for stream:" << streamId << "=" << offset;
    }
}

double AudioProcessor::getStreamOffset(const QString& streamId) const
{
    QMutexLocker locker(&m_mutex);
    return m_syncInfo.value(streamId).offset;
}

AudioSyncInfo AudioProcessor::getSyncInfo(const QString& streamId) const
{
    QMutexLocker locker(&m_mutex);
    return m_syncInfo.value(streamId);
}

void AudioProcessor::resetSynchronization(const QString& streamId)
{
    QMutexLocker locker(&m_mutex);
    m_syncInfo.remove(streamId);
    m_synchronizedStreams.removeAll(streamId);
    qCInfo(audioProcessor) << "Reset synchronization for stream:" << streamId;
}

void AudioProcessor::setQualitySettings(const QString& streamId, const QJsonObject& settings)
{
    QMutexLocker locker(&m_mutex);
    m_qualitySettings[streamId] = settings;
    qCInfo(audioProcessor) << "Set quality settings for stream:" << streamId;
}

QJsonObject AudioProcessor::getQualitySettings(const QString& streamId) const
{
    QMutexLocker locker(&m_mutex);
    return m_qualitySettings.value(streamId);
}

void AudioProcessor::enableQualityMonitoring(const QString& streamId, bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_qualityMonitoringEnabled[streamId] = enabled;
    qCInfo(audioProcessor) << "Quality monitoring" << (enabled ? "enabled" : "disabled") << "for stream:" << streamId;
}

bool AudioProcessor::isQualityMonitoringEnabled(const QString& streamId) const
{
    QMutexLocker locker(&m_mutex);
    return m_qualityMonitoringEnabled.value(streamId, false);
}

QJsonObject AudioProcessor::getProcessingStats() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject stats;
    stats["total_streams"] = m_streamEffects.size();
    stats["total_processed_bytes"] = 0;
    stats["total_processing_time"] = 0;
    stats["total_effect_applications"] = 0;
    stats["total_format_conversions"] = 0;
    
    for (auto it = m_processedBytes.begin(); it != m_processedBytes.end(); ++it) {
        stats["total_processed_bytes"] = stats["total_processed_bytes"].toDouble() + it.value();
    }
    
    for (auto it = m_processingTime.begin(); it != m_processingTime.end(); ++it) {
        stats["total_processing_time"] = stats["total_processing_time"].toDouble() + it.value();
    }
    
    for (auto it = m_effectApplications.begin(); it != m_effectApplications.end(); ++it) {
        stats["total_effect_applications"] = stats["total_effect_applications"].toInt() + it.value();
    }
    
    for (auto it = m_formatConversions.begin(); it != m_formatConversions.end(); ++it) {
        stats["total_format_conversions"] = stats["total_format_conversions"].toInt() + it.value();
    }
    
    return stats;
}

QJsonObject AudioProcessor::getStreamProcessingStats(const QString& streamId) const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject stats;
    stats["processed_bytes"] = m_processedBytes.value(streamId, 0);
    stats["processing_time"] = m_processingTime.value(streamId, 0);
    stats["effect_applications"] = m_effectApplications.value(streamId, 0);
    stats["format_conversions"] = m_formatConversions.value(streamId, 0);
    stats["effects_count"] = m_streamEffects.value(streamId).size();
    stats["real_time_analysis"] = m_realTimeAnalysisEnabled.value(streamId, false);
    stats["quality_monitoring"] = m_qualityMonitoringEnabled.value(streamId, false);
    
    return stats;
}

void AudioProcessor::resetStats()
{
    QMutexLocker locker(&m_mutex);
    m_processedBytes.clear();
    m_processingTime.clear();
    m_effectApplications.clear();
    m_formatConversions.clear();
    qCInfo(audioProcessor) << "Audio processing statistics reset";
}

QByteArray AudioProcessor::resampleAudio(const QByteArray& audioData, int fromSampleRate, int toSampleRate)
{
    // Simplified resampling implementation
    return audioData; // Placeholder
}

QByteArray AudioProcessor::convertChannels(const QByteArray& audioData, int fromChannels, int toChannels)
{
    // Simplified channel conversion implementation
    return audioData; // Placeholder
}

QByteArray AudioProcessor::convertBitDepth(const QByteArray& audioData, int fromBitDepth, int toBitDepth)
{
    // Simplified bit depth conversion implementation
    return audioData; // Placeholder
}

QString AudioProcessor::formatToString(AudioEffectType effect) const
{
    switch (effect) {
        case AudioEffectType::EQUALIZER: return "equalizer";
        case AudioEffectType::COMPRESSOR: return "compressor";
        case AudioEffectType::REVERB: return "reverb";
        case AudioEffectType::DELAY: return "delay";
        case AudioEffectType::CHORUS: return "chorus";
        case AudioEffectType::FLANGER: return "flanger";
        case AudioEffectType::DISTORTION: return "distortion";
        case AudioEffectType::FILTER_LOW_PASS: return "low_pass_filter";
        case AudioEffectType::FILTER_HIGH_PASS: return "high_pass_filter";
        case AudioEffectType::FILTER_BAND_PASS: return "band_pass_filter";
        case AudioEffectType::NORMALIZER: return "normalizer";
        case AudioEffectType::NOISE_REDUCTION: return "noise_reduction";
        default: return "none";
    }
}

AudioEffectType AudioProcessor::stringToFormat(const QString& format) const
{
    if (format == "equalizer") return AudioEffectType::EQUALIZER;
    if (format == "compressor") return AudioEffectType::COMPRESSOR;
    if (format == "reverb") return AudioEffectType::REVERB;
    if (format == "delay") return AudioEffectType::DELAY;
    if (format == "chorus") return AudioEffectType::CHORUS;
    if (format == "flanger") return AudioEffectType::FLANGER;
    if (format == "distortion") return AudioEffectType::DISTORTION;
    if (format == "low_pass_filter") return AudioEffectType::FILTER_LOW_PASS;
    if (format == "high_pass_filter") return AudioEffectType::FILTER_HIGH_PASS;
    if (format == "band_pass_filter") return AudioEffectType::FILTER_BAND_PASS;
    if (format == "normalizer") return AudioEffectType::NORMALIZER;
    if (format == "noise_reduction") return AudioEffectType::NOISE_REDUCTION;
    return AudioEffectType::NONE;
}

void AudioProcessor::onAnalysisTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    // Perform real-time analysis for enabled streams
    QMutexLocker locker(&m_mutex);
    for (auto it = m_realTimeAnalysisEnabled.begin(); it != m_realTimeAnalysisEnabled.end(); ++it) {
        if (it.value()) {
            QString streamId = it.key();
            // Trigger analysis for this stream
            // This would typically be called with actual audio data
        }
    }
}

void AudioProcessor::onSyncTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    // Check synchronization status for synchronized streams
    QMutexLocker locker(&m_mutex);
    for (const QString& streamId : m_synchronizedStreams) {
        if (m_syncInfo.contains(streamId)) {
            AudioSyncInfo& syncInfo = m_syncInfo[streamId];
            // Update synchronization status
            syncInfo.lastSync = QDateTime::currentDateTime();
        }
    }
}

void AudioProcessor::onQualityCheckTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    // Check audio quality for monitored streams
    QMutexLocker locker(&m_mutex);
    for (auto it = m_qualityMonitoringEnabled.begin(); it != m_qualityMonitoringEnabled.end(); ++it) {
        if (it.value()) {
            QString streamId = it.key();
            // Perform quality checks
            // This would typically analyze recent audio data
        }
    }
}

} // namespace LegacyStream 