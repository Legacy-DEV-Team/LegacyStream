#ifndef CODECMANAGER_H
#define CODECMANAGER_H

#include <QObject>
#include <QString>

class CodecManager : public QObject
{
    Q_OBJECT

public:
    explicit CodecManager(QObject *parent = nullptr);
    ~CodecManager();

    bool initialize();
    void shutdown();

private:
    // Codec management members
};

#endif // CODECMANAGER_H 