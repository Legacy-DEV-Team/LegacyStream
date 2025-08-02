#include "protocols/ProtocolManager.h"
#include <QDebug>

ProtocolManager::ProtocolManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "ProtocolManager initialized";
}

ProtocolManager::~ProtocolManager()
{
    qDebug() << "ProtocolManager destroyed";
}

bool ProtocolManager::initialize()
{
    qDebug() << "ProtocolManager: Initializing protocols";
    return true;
}

void ProtocolManager::shutdown()
{
    qDebug() << "ProtocolManager: Shutting down protocols";
} 