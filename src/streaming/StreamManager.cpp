#include "streaming/StreamManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Q_LOGGING_CATEGORY(streamManager, "streamManager")

namespace LegacyStream {

StreamManager::StreamManager(QObject* parent)
    : QObject(parent)
    , m_updateTimer(new QTimer(this))
{
    qCDebug(streamManager) << "StreamManager created";
    
    // Set up update timer
    m_updateTimer->setSingleShot(false);
    m_updateTimer->setInterval(1000); // Update every second
    
    // Connect signals
    connect(m_updateTimer, &QTimer::timeout, this, &StreamManager::onUpdateTimer);
    
    // Initialize enabled codecs
    for (const QString& codec : m_supportedCodecs) {
        m_enabledCodecs[codec] = true;
    }
}

StreamManager::~StreamManager()
{
    shutdown();
}

bool StreamManager::initialize()
{
    qCDebug(streamManager) << "Initializing StreamManager";
    
    // Initialize statistics
    m_statistics["total_streams"] = 0;
    m_statistics["active_streams"] = 0;
    m_statistics["total_bytes_received"] = 0;
    m_statistics["total_bytes_sent"] = 0;
    m_statistics["total_listeners"] = 0;
    m_statistics["start_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    qCInfo(streamManager) << "StreamManager initialized successfully";
    return true;
}

void StreamManager::shutdown()
{
    if (m_isRunning.load()) {
        stop();
    }
    
    // Stop update timer
    m_updateTimer->stop();
    
    // Clear streams
    QMutexLocker locker(&m_mutex);
    m_streams.clear();
    
    qCInfo(streamManager) << "StreamManager shutdown complete";
}

bool StreamManager::start()
{
    if (m_isRunning.load()) {
        qCWarning(streamManager) << "StreamManager already running";
        return true;
    }
    
    qCInfo(streamManager) << "Starting StreamManager";
    
    // Start update timer
    m_updateTimer->start();
    
    m_isRunning.store(true);
    m_startTime = QDateTime::currentDateTime();
    
    qCInfo(streamManager) << "StreamManager started successfully";
    emit statusChanged(m_statistics);
    
    return true;
}

void StreamManager::stop()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    qCInfo(streamManager) << "Stopping StreamManager";
    
    // Stop update timer
    m_updateTimer->stop();
    
    // Deactivate all streams
    QMutexLocker locker(&m_mutex);
    for (auto it = m_streams.begin(); it != m_streams.end(); ++it) {
        it->active = false;
    }
    
    m_isRunning.store(false);
    
    qCInfo(streamManager) << "StreamManager stopped";
    emit statusChanged(m_statistics);
}

void StreamManager::addStream(const QString& mountPoint, const QString& codec, int bitrate)
{
    if (!isValidMountPoint(mountPoint)) {
        qCWarning(streamManager) << "Invalid mount point:" << mountPoint;
        return;
    }
    
    if (!isCodecSupported(codec)) {
        qCWarning(streamManager) << "Unsupported codec:" << codec;
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    StreamInfo info;
    info.mountPoint = mountPoint;
    info.codec = codec;
    info.bitrate = bitrate;
    info.active = false;
    info.startTime = QDateTime::currentDateTime();
    
    m_streams[mountPoint] = info;
    
    m_statistics["total_streams"] = m_streams.size();
    
    qCInfo(streamManager) << "Added stream:" << mountPoint << "(" << codec << "," << bitrate << "kbps)";
    
    emit streamAdded(mountPoint);
    emit statusChanged(m_statistics);
}

void StreamManager::removeStream(const QString& mountPoint)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_streams.contains(mountPoint)) {
        m_streams.remove(mountPoint);
        m_statistics["total_streams"] = m_streams.size();
        
        qCInfo(streamManager) << "Removed stream:" << mountPoint;
        
        emit streamRemoved(mountPoint);
        emit statusChanged(m_statistics);
    }
}

void StreamManager::updateStream(const QString& mountPoint, const StreamInfo& info)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_streams.contains(mountPoint)) {
        m_streams[mountPoint] = info;
        qCDebug(streamManager) << "Updated stream:" << mountPoint;
    }
}

void StreamManager::setStreamActive(const QString& mountPoint, bool active)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_streams.contains(mountPoint)) {
        m_streams[mountPoint].active = active;
        
        // Update active streams count
        m_activeStreams = 0;
        for (const StreamInfo& info : m_streams.values()) {
            if (info.active) {
                m_activeStreams++;
            }
        }
        m_statistics["active_streams"] = m_activeStreams;
        
        qCInfo(streamManager) << "Stream" << mountPoint << (active ? "activated" : "deactivated");
        emit statusChanged(m_statistics);
    }
}

bool StreamManager::isCodecSupported(const QString& codec) const
{
    return m_supportedCodecs.contains(codec.toLower());
}

QStringList StreamManager::getSupportedCodecs() const
{
    return m_supportedCodecs;
}

CodecType StreamManager::getCodecType(const QString& codec) const
{
    return stringToCodec(codec.toLower());
}

void StreamManager::enableCodec(const QString& codec, bool enabled)
{
    QString lowerCodec = codec.toLower();
    if (m_supportedCodecs.contains(lowerCodec)) {
        m_enabledCodecs[lowerCodec] = enabled;
        qCInfo(streamManager) << "Codec" << codec << (enabled ? "enabled" : "disabled");
    }
}

void StreamManager::processStreamData(const QString& mountPoint, const QByteArray& data)
{
    if (!m_isRunning.load()) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    if (!m_streams.contains(mountPoint)) {
        qCWarning(streamManager) << "Stream not found:" << mountPoint;
        return;
    }
    
    const StreamInfo& info = m_streams[mountPoint];
    CodecType codecType = getCodecType(info.codec);
    
    // Validate data
    if (!validateStreamData(data, codecType)) {
        qCWarning(streamManager) << "Invalid stream data for" << mountPoint;
        emit streamError(mountPoint, "Invalid stream data");
        return;
    }
    
    // Process codec-specific data
    processCodecData(mountPoint, data, codecType);
    
    // Update statistics
    updateStreamStatistics(mountPoint, data.size());
    
    emit streamDataReceived(mountPoint, data);
}

void StreamManager::setStreamMetadata(const QString& mountPoint, const QString& metadata)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_streams.contains(mountPoint)) {
        m_streams[mountPoint].metadata = metadata;
        qCDebug(streamManager) << "Updated metadata for" << mountPoint << ":" << metadata;
        emit streamMetadataUpdated(mountPoint, metadata);
    }
}

bool StreamManager::isRunning() const
{
    return m_isRunning.load();
}

QList<StreamInfo> StreamManager::getStreams() const
{
    QMutexLocker locker(&m_mutex);
    return m_streams.values();
}

StreamInfo StreamManager::getStreamInfo(const QString& mountPoint) const
{
    QMutexLocker locker(&m_mutex);
    return m_streams.value(mountPoint);
}

QJsonObject StreamManager::getStatusJson() const
{
    QJsonObject status = m_statistics;
    status["running"] = m_isRunning.load();
    status["total_streams"] = m_streams.size();
    status["active_streams"] = m_activeStreams;
    status["supported_codecs"] = QJsonArray::fromStringList(m_supportedCodecs);
    
    return status;
}

QJsonArray StreamManager::getStreamsJson() const
{
    QMutexLocker locker(&m_mutex);
    QJsonArray streams;
    
    for (const StreamInfo& info : m_streams.values()) {
        QJsonObject stream;
        stream["mount_point"] = info.mountPoint;
        stream["codec"] = info.codec;
        stream["bitrate"] = info.bitrate;
        stream["sample_rate"] = info.sampleRate;
        stream["channels"] = info.channels;
        stream["active"] = info.active;
        stream["bytes_received"] = info.bytesReceived;
        stream["bytes_sent"] = info.bytesSent;
        stream["listeners"] = info.listeners;
        stream["start_time"] = info.startTime.toString(Qt::ISODate);
        stream["metadata"] = info.metadata;
        streams.append(stream);
    }
    
    return streams;
}

qint64 StreamManager::getTotalBytesReceived() const
{
    return m_totalBytesReceived;
}

qint64 StreamManager::getTotalBytesSent() const
{
    return m_totalBytesSent;
}

int StreamManager::getTotalListeners() const
{
    return m_totalListeners;
}

int StreamManager::getActiveStreams() const
{
    return m_activeStreams;
}

void StreamManager::onUpdateTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Update total listeners
    m_totalListeners = 0;
    for (const StreamInfo& info : m_streams.values()) {
        m_totalListeners += info.listeners;
    }
    m_statistics["total_listeners"] = m_totalListeners;
    
    // Update total bytes
    m_statistics["total_bytes_received"] = m_totalBytesReceived;
    m_statistics["total_bytes_sent"] = m_totalBytesSent;
    
    emit statusChanged(m_statistics);
}

void StreamManager::processCodecData(const QString& mountPoint, const QByteArray& data, CodecType codec)
{
    // Process data based on codec type
    switch (codec) {
        case CodecType::MP3:
            // MP3 processing
            break;
        case CodecType::AAC:
        case CodecType::AAC_PLUS:
            // AAC processing
            break;
        case CodecType::OGG_VORBIS:
            // OGG processing
            break;
        case CodecType::OPUS:
            // Opus processing
            break;
        case CodecType::FLAC:
            // FLAC processing
            break;
        default:
            qCWarning(streamManager) << "Unknown codec type for" << mountPoint;
            break;
    }
}

bool StreamManager::validateStreamData(const QByteArray& data, CodecType codec) const
{
    if (data.isEmpty()) {
        return false;
    }
    
    // Basic validation based on codec
    switch (codec) {
        case CodecType::MP3:
            // Check for MP3 sync bytes
            return data.size() >= 4 && (data[0] == 0xFF && (data[1] & 0xE0) == 0xE0);
        case CodecType::AAC:
        case CodecType::AAC_PLUS:
            // Check for AAC ADTS header
            return data.size() >= 7 && (data[0] == 0xFF && (data[1] & 0xF0) == 0xF0);
        case CodecType::OGG_VORBIS:
            // Check for OGG page header
            return data.size() >= 4 && data.startsWith("OggS");
        case CodecType::OPUS:
            // Check for Opus header
            return data.size() >= 8 && data.startsWith("OpusHead");
        case CodecType::FLAC:
            // Check for FLAC signature
            return data.size() >= 4 && data.startsWith("fLaC");
        default:
            return true; // Accept unknown codecs
    }
}

void StreamManager::updateStreamStatistics(const QString& mountPoint, qint64 bytesReceived)
{
    if (m_streams.contains(mountPoint)) {
        m_streams[mountPoint].bytesReceived += bytesReceived;
        m_totalBytesReceived += bytesReceived;
    }
}

QString StreamManager::codecToString(CodecType codec) const
{
    switch (codec) {
        case CodecType::MP3: return "mp3";
        case CodecType::AAC: return "aac";
        case CodecType::AAC_PLUS: return "aac+";
        case CodecType::OGG_VORBIS: return "ogg";
        case CodecType::OPUS: return "opus";
        case CodecType::FLAC: return "flac";
        default: return "unknown";
    }
}

CodecType StreamManager::stringToCodec(const QString& codec) const
{
    QString lowerCodec = codec.toLower();
    if (lowerCodec == "mp3") return CodecType::MP3;
    if (lowerCodec == "aac") return CodecType::AAC;
    if (lowerCodec == "aac+") return CodecType::AAC_PLUS;
    if (lowerCodec == "ogg") return CodecType::OGG_VORBIS;
    if (lowerCodec == "opus") return CodecType::OPUS;
    if (lowerCodec == "flac") return CodecType::FLAC;
    return CodecType::UNKNOWN;
}

bool StreamManager::isValidMountPoint(const QString& mountPoint) const
{
    return !mountPoint.isEmpty() && mountPoint.startsWith("/");
}

} // namespace LegacyStream 