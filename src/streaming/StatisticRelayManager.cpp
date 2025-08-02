#include "streaming/StatisticRelayManager.h"
#include <QDebug>

namespace LegacyStream {

namespace StatisticRelay {

StatisticRelayManager::StatisticRelayManager(QObject *parent)
    : QObject(parent)
    , m_isRunning(false)
{
    qDebug() << "StatisticRelayManager initialized";
}

StatisticRelayManager::~StatisticRelayManager()
{
    qDebug() << "StatisticRelayManager destroyed";
}

bool StatisticRelayManager::initialize(StreamManager* streamManager)
{
    Q_UNUSED(streamManager)
    qDebug() << "StatisticRelayManager: Initializing";
    m_isRunning = true;
    return true;
}

void StatisticRelayManager::shutdown()
{
    qDebug() << "StatisticRelayManager: Shutting down";
    m_isRunning = false;
}

bool StatisticRelayManager::isRunning() const
{
    return m_isRunning;
}

} // namespace StatisticRelay

} // namespace LegacyStream 