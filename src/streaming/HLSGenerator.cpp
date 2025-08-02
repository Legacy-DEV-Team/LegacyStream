#include "streaming/HLSGenerator.h"
#include "streaming/StreamManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QMutexLocker>
#include <QTimer>
#include <QStandardPaths>
#include <QApplication>

Q_LOGGING_CATEGORY(hlsGenerator, "hlsGenerator")

namespace LegacyStream {

HLSGenerator::HLSGenerator(QObject* parent)
    : QObject(parent)
    , m_segmentTimer(new QTimer(this))
    , m_cleanupTimer(new QTimer(this))
{
    qCDebug(hlsGenerator) << "HLSGenerator created";
    
    // Set up timers
    m_segmentTimer->setSingleShot(false);
    m_cleanupTimer->setSingleShot(false);
    
    // Connect signals
    connect(m_segmentTimer, &QTimer::timeout, this, &HLSGenerator::onSegmentTimer);
    connect(m_cleanupTimer, &QTimer::timeout, this, &HLSGenerator::onCleanupTimer);
}

HLSGenerator::~HLSGenerator()
{
    shutdown();
}

bool HLSGenerator::initialize()
{
    qCDebug(hlsGenerator) << "Initializing HLSGenerator";
    
    // Create output directory
    if (!ensureDirectoryExists(m_outputDirectory)) {
        qCCritical(hlsGenerator) << "Failed to create output directory:" << m_outputDirectory;
        return false;
    }
    
    // Initialize statistics
    m_statistics["total_segments"] = 0;
    m_statistics["total_playlists"] = 0;
    m_statistics["active_mounts"] = 0;
    m_statistics["start_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    qCInfo(hlsGenerator) << "HLSGenerator initialized successfully";
    return true;
}

void HLSGenerator::shutdown()
{
    if (m_isRunning.load()) {
        stop();
    }
    
    // Clean up resources
    m_segmentTimer->stop();
    m_cleanupTimer->stop();
    
    // Clear buffers
    m_audioBuffers.clear();
    m_bufferSizes.clear();
    m_segmentFiles.clear();
    m_lastSegmentTime.clear();
    m_segmentCounters.clear();
    
    qCInfo(hlsGenerator) << "HLSGenerator shutdown complete";
}

bool HLSGenerator::start()
{
    if (m_isRunning.load()) {
        qCWarning(hlsGenerator) << "HLSGenerator already running";
        return true;
    }
    
    if (!m_streamManager) {
        qCCritical(hlsGenerator) << "StreamManager not set";
        return false;
    }
    
    qCInfo(hlsGenerator) << "Starting HLSGenerator";
    
    // Start timers
    m_segmentTimer->start(m_segmentDuration * 1000);
    m_cleanupTimer->start(60000); // Clean up every minute
    
    m_isRunning.store(true);
    m_startTime = QDateTime::currentDateTime();
    
    // Generate initial playlists
    generateMasterPlaylist();
    for (const QString& quality : m_qualityLevels) {
        generateVariantPlaylist(quality);
    }
    
    qCInfo(hlsGenerator) << "HLSGenerator started successfully";
    emit statusChanged(m_statistics);
    
    return true;
}

void HLSGenerator::stop()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    qCInfo(hlsGenerator) << "Stopping HLSGenerator";
    
    // Stop timers
    m_segmentTimer->stop();
    m_cleanupTimer->stop();
    
    // Generate final segments for active mounts
    QMutexLocker locker(&m_mutex);
    for (const QString& mountPoint : m_audioBuffers.keys()) {
        if (!m_audioBuffers[mountPoint].isEmpty()) {
            generateSegment(mountPoint, m_audioBuffers[mountPoint]);
        }
    }
    
    m_isRunning.store(false);
    
    qCInfo(hlsGenerator) << "HLSGenerator stopped";
    emit statusChanged(m_statistics);
}

void HLSGenerator::setStreamManager(StreamManager* streamManager)
{
    m_streamManager = streamManager;
}

void HLSGenerator::setOutputDirectory(const QString& directory)
{
    m_outputDirectory = directory;
}

void HLSGenerator::setSegmentDuration(int seconds)
{
    m_segmentDuration = seconds;
    if (m_segmentTimer->isActive()) {
        m_segmentTimer->setInterval(seconds * 1000);
    }
}

void HLSGenerator::setPlaylistLength(int segments)
{
    m_playlistLength = segments;
}

void HLSGenerator::setQualityLevels(const QStringList& levels)
{
    m_qualityLevels = levels;
}

void HLSGenerator::setTargetBitrates(const QList<int>& bitrates)
{
    m_targetBitrates = bitrates;
}

bool HLSGenerator::isRunning() const
{
    return m_isRunning.load();
}

QString HLSGenerator::getMasterPlaylistUrl() const
{
    return QString("http://localhost:8000/hls/master.m3u8");
}

QString HLSGenerator::getVariantPlaylistUrl(const QString& quality) const
{
    return QString("http://localhost:8000/hls/%1.m3u8").arg(quality);
}

QJsonObject HLSGenerator::getStatusJson() const
{
    QJsonObject status = m_statistics;
    status["running"] = m_isRunning.load();
    status["segment_duration"] = m_segmentDuration;
    status["playlist_length"] = m_playlistLength;
    status["quality_levels"] = QJsonArray::fromStringList(m_qualityLevels);
    
    QJsonArray bitrates;
    for (int bitrate : m_targetBitrates) {
        bitrates.append(bitrate);
    }
    status["target_bitrates"] = bitrates;
    
    return status;
}

void HLSGenerator::generateSegment(const QString& mountPoint, const QByteArray& audioData)
{
    if (!m_isRunning.load()) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Generate segments for each quality level
    for (int i = 0; i < m_qualityLevels.size(); ++i) {
        const QString& quality = m_qualityLevels[i];
        int targetBitrate = m_targetBitrates.value(i, 128);
        
        // Simulate different quality levels (in real implementation, this would transcode)
        QByteArray qualityData = audioData;
        if (quality == "medium") {
            qualityData = audioData.mid(0, audioData.size() * 2 / 3);
        } else if (quality == "low") {
            qualityData = audioData.mid(0, audioData.size() / 2);
        }
        
        generateSegmentFile(mountPoint, quality, qualityData);
    }
    
    // Update playlists
    updatePlaylist(mountPoint);
    
    // Update statistics
    m_totalSegmentsGenerated++;
    m_statistics["total_segments"] = m_totalSegmentsGenerated;
    
    emit segmentGenerated(mountPoint, QString("hls/segments/%1.ts").arg(m_segmentCounters[mountPoint]));
    emit statusChanged(m_statistics);
}

void HLSGenerator::updatePlaylist(const QString& mountPoint)
{
    for (const QString& quality : m_qualityLevels) {
        updatePlaylistFile(mountPoint, quality);
    }
    
    m_totalPlaylistsUpdated++;
    m_statistics["total_playlists"] = m_totalPlaylistsUpdated;
    
    emit playlistUpdated(mountPoint, QString("hls/playlists/%1.m3u8").arg(mountPoint));
}

void HLSGenerator::cleanupOldSegments()
{
    cleanupExpiredSegments();
}

void HLSGenerator::onSegmentTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Process accumulated audio data for each mount point
    for (auto it = m_audioBuffers.begin(); it != m_audioBuffers.end(); ++it) {
        const QString& mountPoint = it.key();
        const QByteArray& audioData = it.value();
        
        if (!audioData.isEmpty()) {
            generateSegment(mountPoint, audioData);
            m_audioBuffers[mountPoint].clear();
        }
    }
}

void HLSGenerator::onCleanupTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    cleanupExpiredSegments();
}

void HLSGenerator::onStreamDataReceived(const QString& mountPoint, const QByteArray& data)
{
    if (!m_isRunning.load()) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // Accumulate audio data
    m_audioBuffers[mountPoint].append(data);
    m_bufferSizes[mountPoint] = m_audioBuffers[mountPoint].size();
    
    // Update statistics
    m_statistics["active_mounts"] = m_audioBuffers.size();
    m_statistics["buffer_sizes"] = QJsonObject::fromVariantMap(m_bufferSizes.toVariantMap());
}

void HLSGenerator::generateMasterPlaylist()
{
    QString playlistPath = QString("%1/master.m3u8").arg(m_outputDirectory);
    QFile file(playlistPath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(hlsGenerator) << "Failed to create master playlist:" << playlistPath;
        return;
    }
    
    QTextStream stream(&file);
    stream << "#EXTM3U\n";
    stream << "#EXT-X-VERSION:3\n";
    stream << "#EXT-X-INDEPENDENT-SEGMENTS\n\n";
    
    for (int i = 0; i < m_qualityLevels.size(); ++i) {
        const QString& quality = m_qualityLevels[i];
        int bitrate = m_targetBitrates.value(i, 128);
        
        stream << QString("#EXT-X-STREAM-INF:BANDWIDTH=%1,RESOLUTION=640x360\n")
                  .arg(bitrate * 1000);
        stream << QString("%1.m3u8\n").arg(quality);
    }
    
    file.close();
    qCDebug(hlsGenerator) << "Generated master playlist:" << playlistPath;
}

void HLSGenerator::generateVariantPlaylist(const QString& quality)
{
    QString playlistPath = QString("%1/%2.m3u8").arg(m_outputDirectory, quality);
    QFile file(playlistPath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(hlsGenerator) << "Failed to create variant playlist:" << playlistPath;
        return;
    }
    
    QTextStream stream(&file);
    stream << "#EXTM3U\n";
    stream << "#EXT-X-VERSION:3\n";
    stream << QString("#EXT-X-TARGETDURATION:%1\n").arg(m_segmentDuration);
    stream << QString("#EXT-X-MEDIA-SEQUENCE:0\n\n");
    
    file.close();
    qCDebug(hlsGenerator) << "Generated variant playlist:" << playlistPath;
}

void HLSGenerator::generateSegmentFile(const QString& mountPoint, const QString& quality, const QByteArray& data)
{
    // Ensure segments directory exists
    QString segmentsDir = QString("%1/segments").arg(m_outputDirectory);
    ensureDirectoryExists(segmentsDir);
    
    // Generate segment filename
    int sequence = m_segmentCounters.value(mountPoint, 0);
    QString filename = generateSegmentFilename(mountPoint, quality, sequence);
    QString filepath = QString("%1/%2").arg(segmentsDir, filename);
    
    // Write segment file
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        
        // Update tracking
        m_segmentCounters[mountPoint] = sequence + 1;
        m_segmentFiles[mountPoint].append(filename);
        m_lastSegmentTime[mountPoint] = QDateTime::currentDateTime();
        
        qCDebug(hlsGenerator) << "Generated segment:" << filepath;
    } else {
        qCWarning(hlsGenerator) << "Failed to write segment file:" << filepath;
    }
}

void HLSGenerator::updatePlaylistFile(const QString& mountPoint, const QString& quality)
{
    QString playlistPath = generatePlaylistFilename(mountPoint, quality);
    QFile file(playlistPath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(hlsGenerator) << "Failed to update playlist:" << playlistPath;
        return;
    }
    
    QTextStream stream(&file);
    stream << "#EXTM3U\n";
    stream << "#EXT-X-VERSION:3\n";
    stream << QString("#EXT-X-TARGETDURATION:%1\n").arg(m_segmentDuration);
    stream << QString("#EXT-X-MEDIA-SEQUENCE:%1\n\n").arg(m_segmentCounters[mountPoint] - m_playlistLength);
    
    // Add segment entries
    QStringList segments = m_segmentFiles.value(mountPoint);
    int startIndex = qMax(0, segments.size() - m_playlistLength);
    
    for (int i = startIndex; i < segments.size(); ++i) {
        stream << QString("#EXTINF:%1.0,\n").arg(m_segmentDuration);
        stream << QString("segments/%1\n").arg(segments[i]);
    }
    
    file.close();
    qCDebug(hlsGenerator) << "Updated playlist:" << playlistPath;
}

void HLSGenerator::cleanupExpiredSegments()
{
    QMutexLocker locker(&m_mutex);
    
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-m_segmentDuration * m_playlistLength * 2);
    
    for (auto it = m_lastSegmentTime.begin(); it != m_lastSegmentTime.end(); ++it) {
        const QString& mountPoint = it.key();
        const QDateTime& lastSegment = it.value();
        
        if (lastSegment < cutoff) {
            // Remove old segments for this mount point
            QStringList& segments = m_segmentFiles[mountPoint];
            while (segments.size() > m_playlistLength) {
                QString oldSegment = segments.takeFirst();
                QString segmentPath = QString("%1/segments/%2").arg(m_outputDirectory, oldSegment);
                QFile::remove(segmentPath);
            }
        }
    }
}

QString HLSGenerator::generateSegmentFilename(const QString& mountPoint, const QString& quality, int sequence) const
{
    return QString("%1_%2_%3.ts").arg(mountPoint, quality, QString::number(sequence));
}

QString HLSGenerator::generatePlaylistFilename(const QString& mountPoint, const QString& quality) const
{
    return QString("%1/%2_%3.m3u8").arg(m_outputDirectory, mountPoint, quality);
}

QString HLSGenerator::formatDuration(int seconds) const
{
    return QString("%1.0").arg(seconds);
}

QString HLSGenerator::formatBitrate(int bitrate) const
{
    return QString::number(bitrate * 1000);
}

bool HLSGenerator::ensureDirectoryExists(const QString& path) const
{
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

} // namespace LegacyStream 