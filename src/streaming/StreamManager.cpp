#include "streaming/StreamManager.h"
#include <QDebug>

namespace LegacyStream {

StreamManager::StreamManager(QObject *parent)
    : QObject(parent)
    , m_isRunning(false)
{
    qDebug() << "StreamManager initialized";
}

StreamManager::~StreamManager()
{
    qDebug() << "StreamManager destroyed";
}

bool StreamManager::initialize()
{
    qDebug() << "StreamManager: Initializing";
    m_isRunning = true;
    return true;
}

void StreamManager::shutdown()
{
    qDebug() << "StreamManager: Shutting down";
    m_isRunning = false;
}

bool StreamManager::isRunning() const
{
    return m_isRunning;
}

} // namespace LegacyStream 