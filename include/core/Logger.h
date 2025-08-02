#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QDebug>

namespace LegacyStream {

class Logger : public QObject
{
    Q_OBJECT

public:
    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    static Logger& instance();
    
    void log(LogLevel level, const QString& message);
    void debug(const QString& message);
    void info(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);
    void critical(const QString& message);
    
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const { return m_logLevel; }

private:
    explicit Logger(QObject* parent = nullptr);
    ~Logger();
    
    LogLevel m_logLevel = Info;
    QString formatMessage(LogLevel level, const QString& message) const;
    QString levelToString(LogLevel level) const;
};

// Convenience macros
#define LOG_DEBUG(msg) LegacyStream::Logger::instance().debug(msg)
#define LOG_INFO(msg) LegacyStream::Logger::instance().info(msg)
#define LOG_WARNING(msg) LegacyStream::Logger::instance().warning(msg)
#define LOG_ERROR(msg) LegacyStream::Logger::instance().error(msg)
#define LOG_CRITICAL(msg) LegacyStream::Logger::instance().critical(msg)

} // namespace LegacyStream

#endif // LOGGER_H 