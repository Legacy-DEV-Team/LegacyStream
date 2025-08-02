#include "streaming/RelayManager.h"
#include <QDebug>

namespace LegacyStream {

RelayManager::RelayManager(QObject *parent)
    : QObject(parent)
    , m_isRunning(false)
{
    qDebug() << "RelayManager initialized";
}

RelayManager::~RelayManager()
{
    qDebug() << "RelayManager destroyed";
}

bool RelayManager::initialize()
{
    qDebug() << "RelayManager: Initializing";
    m_isRunning = true;
    return true;
}

void RelayManager::shutdown()
{
    qDebug() << "RelayManager: Shutting down";
    m_isRunning = false;
}

bool RelayManager::isRunning() const
{
    return m_isRunning;
}

} // namespace LegacyStream 