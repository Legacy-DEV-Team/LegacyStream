#include "ssl/SSLManager.h"
#include <QDebug>

namespace LegacyStream {

SSLManager::SSLManager(QObject* parent)
    : QObject(parent)
    , m_enabled(false)
{
    qDebug() << "SSLManager initialized";
}

SSLManager::~SSLManager()
{
    qDebug() << "SSLManager destroyed";
}

bool SSLManager::initialize()
{
    qDebug() << "SSLManager: Initializing";
    m_enabled = true;
    return true;
}

void SSLManager::shutdown()
{
    qDebug() << "SSLManager: Shutting down";
    m_enabled = false;
}

bool SSLManager::isEnabled() const
{
    return m_enabled;
}

} // namespace LegacyStream 
