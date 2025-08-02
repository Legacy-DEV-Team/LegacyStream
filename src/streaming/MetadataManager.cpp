#include "streaming/MetadataManager.h"
#include <QMutexLocker>
#include <QDateTime>

namespace LegacyStream {

MetadataManager::MetadataManager(QObject* parent)
    : QObject(parent)
{
}

MetadataManager::~MetadataManager()
{
}

void MetadataManager::setMetadata(const QString& mountPoint, const MetadataInfo& metadata)
{
    QMutexLocker locker(&m_mutex);
    m_metadata[mountPoint] = metadata;
    emit metadataUpdated(mountPoint, metadata);
}

MetadataInfo MetadataManager::getMetadata(const QString& mountPoint) const
{
    QMutexLocker locker(&m_mutex);
    return m_metadata.value(mountPoint);
}

void MetadataManager::clearMetadata(const QString& mountPoint)
{
    QMutexLocker locker(&m_mutex);
    m_metadata.remove(mountPoint);
}

QJsonObject MetadataManager::getMetadataJson(const QString& mountPoint) const
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_metadata.contains(mountPoint)) {
        return QJsonObject();
    }
    
    const MetadataInfo& info = m_metadata[mountPoint];
    QJsonObject json;
    json["title"] = info.title;
    json["artist"] = info.artist;
    json["album"] = info.album;
    json["genre"] = info.genre;
    json["year"] = info.year;
    json["comment"] = info.comment;
    json["url"] = info.url;
    json["bitrate"] = info.bitrate;
    json["sample_rate"] = info.sampleRate;
    json["channels"] = info.channels;
    json["timestamp"] = info.timestamp.toString(Qt::ISODate);
    
    return json;
}

} // namespace LegacyStream 