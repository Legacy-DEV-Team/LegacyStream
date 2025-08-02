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
    
    // Additional methods needed by implementation
    void write(const QByteArray& data);
    QByteArray read(qint64 maxSize = -1);
    qint64 size() const;
    void setMaxSize(qint64 size);
    qint64 maxSize() const;

private:
    QQueue<QByteArray> m_buffers;
    QMutex m_mutex;
    int m_maxSize = 1024 * 1024; // 1MB default
    QByteArray m_buffer; // Additional buffer for implementation
};

} // namespace LegacyStream

#endif // STREAMBUFFER_H 