#include "streaming/MetadataManager.h"
#include <QDebug>

namespace LegacyStream {

MetadataManager::MetadataManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "MetadataManager initialized";
}

MetadataManager::~MetadataManager()
{
    qDebug() << "MetadataManager destroyed";
}

bool MetadataManager::initialize()
{
    qDebug() << "MetadataManager: Initializing";
    return true;
}

void MetadataManager::shutdown()
{
    qDebug() << "MetadataManager: Shutting down";
}

} // namespace LegacyStream 