#include "streaming/StreamBuffer.h"
#include <QMutexLocker>

namespace LegacyStream {

StreamBuffer::StreamBuffer(QObject* parent)
    : QObject(parent)
{
}

StreamBuffer::~StreamBuffer()
{
    clear();
}

void StreamBuffer::setBufferSize(int size)
{
    QMutexLocker locker(&m_mutex);
    m_maxSize = size;
}

void StreamBuffer::addData(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    
    // Check if adding this data would exceed max size
    int currentSize = 0;
    for (const QByteArray& buffer : m_buffers) {
        currentSize += buffer.size();
    }
    
    if (currentSize + data.size() > m_maxSize) {
        // Remove oldest data to make room
        while (currentSize + data.size() > m_maxSize && !m_buffers.isEmpty()) {
            currentSize -= m_buffers.dequeue().size();
        }
    }
    
    m_buffers.enqueue(data);
}

QByteArray StreamBuffer::getData(int maxSize)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_buffers.isEmpty()) {
        return QByteArray();
    }
    
    QByteArray result;
    int totalSize = 0;
    
    while (!m_buffers.isEmpty() && (maxSize == -1 || totalSize < maxSize)) {
        QByteArray buffer = m_buffers.dequeue();
        if (maxSize != -1 && totalSize + buffer.size() > maxSize) {
            // Put back the excess data
            QByteArray excess = buffer.mid(maxSize - totalSize);
            m_buffers.prepend(excess);
            buffer = buffer.left(maxSize - totalSize);
        }
        result.append(buffer);
        totalSize += buffer.size();
    }
    
    return result;
}

void StreamBuffer::clear()
{
    QMutexLocker locker(&m_mutex);
    m_buffers.clear();
}

int StreamBuffer::availableData() const
{
    QMutexLocker locker(&m_mutex);
    int totalSize = 0;
    for (const QByteArray& buffer : m_buffers) {
        totalSize += buffer.size();
    }
    return totalSize;
}

bool StreamBuffer::isEmpty() const
{
    QMutexLocker locker(&m_mutex);
    return m_buffers.isEmpty();
}

} // namespace LegacyStream 