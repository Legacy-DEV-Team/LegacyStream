#include "streaming/StreamBuffer.h"
#include <QDebug>

namespace LegacyStream {

StreamBuffer::StreamBuffer(QObject *parent)
    : QObject(parent)
    , m_maxSize(1024 * 1024) // 1MB default
{
    qDebug() << "StreamBuffer initialized";
}

StreamBuffer::~StreamBuffer()
{
    qDebug() << "StreamBuffer destroyed";
}

void StreamBuffer::write(const QByteArray& data)
{
    m_buffer.append(data);
    
    // Trim buffer if it exceeds max size
    if (m_buffer.size() > m_maxSize) {
        m_buffer = m_buffer.right(m_maxSize / 2);
    }
}

QByteArray StreamBuffer::read(qint64 maxSize)
{
    if (maxSize <= 0 || m_buffer.isEmpty()) {
        return QByteArray();
    }
    
    qint64 readSize = qMin(maxSize, static_cast<qint64>(m_buffer.size()));
    QByteArray data = m_buffer.left(readSize);
    m_buffer.remove(0, readSize);
    
    return data;
}

void StreamBuffer::clear()
{
    m_buffer.clear();
}

qint64 StreamBuffer::size() const
{
    return m_buffer.size();
}

bool StreamBuffer::isEmpty() const
{
    return m_buffer.isEmpty();
}

void StreamBuffer::setMaxSize(qint64 maxSize)
{
    m_maxSize = maxSize;
}

qint64 StreamBuffer::maxSize() const
{
    return m_maxSize;
}

} // namespace LegacyStream 