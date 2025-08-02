#include "streaming/WebInterface.h"
#include <QDebug>

namespace LegacyStream {

namespace WebInterface {

WebInterface::WebInterface(QObject *parent)
    : QObject(parent)
    , m_isInitialized(false)
{
    qDebug() << "WebInterface initialized";
}

WebInterface::~WebInterface()
{
    qDebug() << "WebInterface destroyed";
}

bool WebInterface::initialize(HttpServer* httpServer, StreamManager* streamManager, StatisticRelay::StatisticRelayManager* statisticRelayManager)
{
    Q_UNUSED(httpServer)
    Q_UNUSED(streamManager)
    Q_UNUSED(statisticRelayManager)
    
    qDebug() << "WebInterface: Initializing";
    m_isInitialized = true;
    return true;
}

void WebInterface::shutdown()
{
    qDebug() << "WebInterface: Shutting down";
    m_isInitialized = false;
}

bool WebInterface::isInitialized() const
{
    return m_isInitialized;
}

} // namespace WebInterface

} // namespace LegacyStream