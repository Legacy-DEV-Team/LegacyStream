#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>
#include <QString>

class ProtocolManager : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolManager(QObject *parent = nullptr);
    ~ProtocolManager();

    bool initialize();
    void shutdown();

private:
    // Protocol management members
};

#endif // PROTOCOLMANAGER_H 