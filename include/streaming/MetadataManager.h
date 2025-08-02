#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>

namespace LegacyStream {

/**
 * @brief Metadata information structure
 */
struct MetadataInfo
{
    QString title;
    QString artist;
    QString album;
    QString genre;
    QString year;
    QString comment;
    QString url;
    int bitrate = 0;
    int sampleRate = 0;
    int channels = 0;
    QDateTime timestamp;
};

/**
 * @brief MetadataManager for stream metadata management
 */
class MetadataManager : public QObject
{
    Q_OBJECT

public:
    explicit MetadataManager(QObject* parent = nullptr);
    ~MetadataManager();

    void setMetadata(const QString& mountPoint, const MetadataInfo& metadata);
    MetadataInfo getMetadata(const QString& mountPoint) const;
    void clearMetadata(const QString& mountPoint);
    QJsonObject getMetadataJson(const QString& mountPoint) const;

signals:
    void metadataUpdated(const QString& mountPoint, const MetadataInfo& metadata);

private:
    QMap<QString, MetadataInfo> m_metadata;
    mutable QMutex m_mutex;
};

} // namespace LegacyStream

#endif // METADATAMANAGER_H 