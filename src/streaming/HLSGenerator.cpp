#include "streaming/HLSGenerator.h"
#include <QDebug>

namespace LegacyStream {

HLSGenerator::HLSGenerator(QObject *parent)
    : QObject(parent)
    , m_isRunning(false)
{
    qDebug() << "HLSGenerator initialized";
}

HLSGenerator::~HLSGenerator()
{
    qDebug() << "HLSGenerator destroyed";
}

bool HLSGenerator::initialize()
{
    qDebug() << "HLSGenerator: Initializing";
    m_isRunning = true;
    return true;
}

void HLSGenerator::shutdown()
{
    qDebug() << "HLSGenerator: Shutting down";
    m_isRunning = false;
}

bool HLSGenerator::isRunning() const
{
    return m_isRunning;
}

} // namespace LegacyStream 