#include "core/Logger.h"
#include <QDateTime>
#include <QDebug>

namespace LegacyStream {

Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger(QObject* parent)
    : QObject(parent)
{
}

Logger::~Logger()
{
}

void Logger::log(LogLevel level, const QString& message)
{
    if (level >= m_logLevel) {
        QString formattedMessage = formatMessage(level, message);
        qDebug().noquote() << formattedMessage;
    }
}

void Logger::debug(const QString& message)
{
    log(Debug, message);
}

void Logger::info(const QString& message)
{
    log(Info, message);
}

void Logger::warning(const QString& message)
{
    log(Warning, message);
}

void Logger::error(const QString& message)
{
    log(Error, message);
}

void Logger::critical(const QString& message)
{
    log(Critical, message);
}

void Logger::setLogLevel(LogLevel level)
{
    m_logLevel = level;
}

QString Logger::formatMessage(LogLevel level, const QString& message) const
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = levelToString(level);
    return QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case Debug: return "DEBUG";
        case Info: return "INFO";
        case Warning: return "WARNING";
        case Error: return "ERROR";
        case Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

} // namespace LegacyStream 