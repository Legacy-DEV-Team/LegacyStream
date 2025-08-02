#ifndef HLSGENERATOR_H
#define HLSGENERATOR_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QMutex>
#include <QAtomicInt>

class StreamManager;

namespace LegacyStream {

/**
 * @brief HTTP Live Streaming (HLS) generator for LegacyStream
 * 
 * Generates HLS playlists and segments for adaptive bitrate streaming.
 * Supports multiple quality levels and automatic segment management.
 */
class HLSGenerator : public QObject
{
    Q_OBJECT

public:
    explicit HLSGenerator(QObject* parent = nullptr);
    ~HLSGenerator();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool start();
    void stop();

    // Configuration
    void setStreamManager(StreamManager* streamManager);
    void setOutputDirectory(const QString& directory);
    void setSegmentDuration(int seconds);
    void setPlaylistLength(int segments);
    void setQualityLevels(const QStringList& levels);
    void setTargetBitrates(const QList<int>& bitrates);

    // Status and information
    bool isRunning() const;
    QString getMasterPlaylistUrl() const;
    QString getVariantPlaylistUrl(const QString& quality) const;
    QJsonObject getStatusJson() const;

    // Manual control
    void generateSegment(const QString& mountPoint, const QByteArray& audioData);
    void updatePlaylist(const QString& mountPoint);
    void cleanupOldSegments();

signals:
    void segmentGenerated(const QString& mountPoint, const QString& segmentPath);
    void playlistUpdated(const QString& mountPoint, const QString& playlistPath);
    void error(const QString& error);
    void statusChanged(const QJsonObject& status);

private slots:
    void onSegmentTimer();
    void onCleanupTimer();
    void onStreamDataReceived(const QString& mountPoint, const QByteArray& data);

private:
    // Core functionality
    void generateMasterPlaylist();
    void generateVariantPlaylist(const QString& quality);
    void generateSegmentFile(const QString& mountPoint, const QString& quality, const QByteArray& data);
    void updatePlaylistFile(const QString& mountPoint, const QString& quality);
    void cleanupExpiredSegments();

    // Utility functions
    QString generateSegmentFilename(const QString& mountPoint, const QString& quality, int sequence) const;
    QString generatePlaylistFilename(const QString& mountPoint, const QString& quality) const;
    QString formatDuration(int seconds) const;
    QString formatBitrate(int bitrate) const;
    bool ensureDirectoryExists(const QString& path) const;

    // Configuration
    StreamManager* m_streamManager = nullptr;
    QString m_outputDirectory = "hls";
    int m_segmentDuration = 10;  // seconds
    int m_playlistLength = 10;   // segments
    QStringList m_qualityLevels = {"high", "medium", "low"};
    QList<int> m_targetBitrates = {256, 128, 64};  // kbps

    // State management
    QAtomicInt m_isRunning = 0;
    QTimer* m_segmentTimer = nullptr;
    QTimer* m_cleanupTimer = nullptr;
    QMutex m_mutex;

    // Segment tracking
    QMap<QString, int> m_segmentCounters;  // mountPoint -> sequence number
    QMap<QString, QStringList> m_segmentFiles;  // mountPoint -> list of segment files
    QMap<QString, QDateTime> m_lastSegmentTime;  // mountPoint -> last segment time

    // Buffer management
    QMap<QString, QByteArray> m_audioBuffers;  // mountPoint -> accumulated audio data
    QMap<QString, int> m_bufferSizes;  // mountPoint -> buffer size in bytes

    // Statistics
    QJsonObject m_statistics;
    QDateTime m_startTime;
    int m_totalSegmentsGenerated = 0;
    int m_totalPlaylistsUpdated = 0;

    Q_DISABLE_COPY(HLSGenerator)
};

} // namespace LegacyStream

#endif // HLSGENERATOR_H 