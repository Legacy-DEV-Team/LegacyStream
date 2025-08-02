#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <QString>

namespace LegacyStream {

/**
 * @brief Audio codec types
 */
enum class CodecType
{
    MP3,
    AAC,
    AAC_PLUS,
    OGG_VORBIS,
    OPUS,
    FLAC,
    UNKNOWN
};

/**
 * @brief Stream information
 */
struct StreamInfo
{
    QString mountPoint;
    QString codec;
    int bitrate = 128;
    int sampleRate = 44100;
    int channels = 2;
    bool active = false;
    qint64 bytesReceived = 0;
    qint64 bytesSent = 0;
    int listeners = 0;
    QDateTime startTime;
    QString metadata;
};

/**
 * @brief StreamManager for audio stream and codec management
 * 
 * Manages audio streams, codecs, and provides stream data to other components.
 * Supports multiple codecs and real-time stream processing.
 */
class StreamManager : public QObject
{
    Q_OBJECT

public:
    explicit StreamManager(QObject* parent = nullptr);
    ~StreamManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool start();
    void stop();

    // Stream management
    void addStream(const QString& mountPoint, const QString& codec, int bitrate = 128);
    void removeStream(const QString& mountPoint);
    void updateStream(const QString& mountPoint, const StreamInfo& info);
    void setStreamActive(const QString& mountPoint, bool active);

    // Codec management
    bool isCodecSupported(const QString& codec) const;
    QStringList getSupportedCodecs() const;
    CodecType getCodecType(const QString& codec) const;
    void enableCodec(const QString& codec, bool enabled);

    // Stream data
    void processStreamData(const QString& mountPoint, const QByteArray& data);
    void setStreamMetadata(const QString& mountPoint, const QString& metadata);

    // Status and information
    bool isRunning() const;
    QList<StreamInfo> getStreams() const;
    StreamInfo getStreamInfo(const QString& mountPoint) const;
    QJsonObject getStatusJson() const;
    QJsonArray getStreamsJson() const;
    int getActiveStreamCount() const;

    // Statistics
    qint64 getTotalBytesReceived() const;
    qint64 getTotalBytesSent() const;
    int getTotalListeners() const;
    int getActiveStreams() const;

signals:
    void streamAdded(const QString& mountPoint);
    void streamRemoved(const QString& mountPoint);
    void streamDataReceived(const QString& mountPoint, const QByteArray& data);
    void streamMetadataUpdated(const QString& mountPoint, const QString& metadata);
    void streamError(const QString& mountPoint, const QString& error);
    void statusChanged(const QJsonObject& status);
    void streamConnected(const QString& mountPoint);
    void streamDisconnected(const QString& mountPoint);

private slots:
    void onUpdateTimer();

private:
    // Core functionality
    void processCodecData(const QString& mountPoint, const QByteArray& data, CodecType codec);
    bool validateStreamData(const QByteArray& data, CodecType codec) const;
    void updateStreamStatistics(const QString& mountPoint, qint64 bytesReceived);

    // Utility functions
    QString codecToString(CodecType codec) const;
    CodecType stringToCodec(const QString& codec) const;
    bool isValidMountPoint(const QString& mountPoint) const;

    // Configuration
    QMap<QString, StreamInfo> m_streams;
    QMap<QString, bool> m_enabledCodecs;
    QStringList m_supportedCodecs = {"mp3", "aac", "aac+", "ogg", "opus", "flac"};

    // State management
    QAtomicInt m_isRunning = 0;
    QTimer* m_updateTimer = nullptr;
    QMutex m_mutex;

    // Statistics
    QJsonObject m_statistics;
    QDateTime m_startTime;
    qint64 m_totalBytesReceived = 0;
    qint64 m_totalBytesSent = 0;
    int m_totalListeners = 0;
    int m_activeStreams = 0;

    Q_DISABLE_COPY(StreamManager)
};

} // namespace LegacyStream

#endif // STREAMMANAGER_H 