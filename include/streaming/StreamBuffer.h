#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include <QObject>
#include <QByteArray>
#include <QQueue>
#include <QMutex>
#include <QTimer>

namespace LegacyStream {

/**
 * @brief StreamBuffer for efficient audio buffer management
 */
class StreamBuffer : public QObject
{
    Q_OBJECT

public:
    explicit StreamBuffer(QObject* parent = nullptr);
    ~StreamBuffer();

    void setBufferSize(int size);
    void addData(const QByteArray& data);
    QByteArray getData(int maxSize = -1);
    void clear();
    int availableData() const;
    bool isEmpty() const;

private:
    QQueue<QByteArray> m_buffers;
    QMutex m_mutex;
    int m_maxSize = 1024 * 1024; // 1MB default
};

} // namespace LegacyStream

#endif // STREAMBUFFER_H 