#pragma once

#include <QObject>
#include <QString>

namespace LegacyStream {

class SSLManager : public QObject
{
    Q_OBJECT

public:
    explicit SSLManager(QObject* parent = nullptr);
    ~SSLManager();

    bool initialize();
    void shutdown();
    bool isEnabled() const;

signals:
    void sslEnabledChanged(bool enabled);
    void certificateLoaded(bool success, const QString& path);
    void certificateGenerated(bool success, const QString& path);

private:
    bool m_enabled = false;
};

} // namespace LegacyStream 