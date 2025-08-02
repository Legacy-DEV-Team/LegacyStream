#include "codecs/CodecManager.h"
#include <QDebug>

CodecManager::CodecManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "CodecManager initialized";
}

CodecManager::~CodecManager()
{
    qDebug() << "CodecManager destroyed";
}

bool CodecManager::initialize()
{
    qDebug() << "CodecManager: Initializing codecs";
    return true;
}

void CodecManager::shutdown()
{
    qDebug() << "CodecManager: Shutting down codecs";
} 