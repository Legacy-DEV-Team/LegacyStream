#include "streaming/WebInterface.h"
#include "streaming/HttpServer.h"
#include "streaming/StreamManager.h"
#include "streaming/StatisticRelayManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>

Q_LOGGING_CATEGORY(webInterface, "webInterface")

namespace LegacyStream {
namespace WebInterface {

WebInterface::WebInterface(QObject* parent)
    : QObject(parent)
    , m_statsTimer(new QTimer(this))
{
    qCDebug(webInterface) << "WebInterface created";
}

WebInterface::~WebInterface()
{
    shutdown();
}

bool WebInterface::initialize(HttpServer* httpServer, StreamManager* streamManager, StatisticRelay::StatisticRelayManager* statisticRelayManager)
{
    if (!httpServer || !streamManager) {
        qCCritical(webInterface) << "Invalid HTTP server or stream manager";
        return false;
    }

    m_httpServer = httpServer;
    m_streamManager = streamManager;
    m_statisticRelayManager = statisticRelayManager;

    // Connect to stream manager signals
    connect(m_streamManager, &StreamManager::streamConnected,
            this, &WebInterface::onStreamConnected);
    connect(m_streamManager, &StreamManager::streamDisconnected,
            this, &WebInterface::onStreamDisconnected);
    connect(m_streamManager, &StreamManager::listenerConnected,
            this, &WebInterface::onListenerConnected);
    connect(m_streamManager, &StreamManager::listenerDisconnected,
            this, &WebInterface::onListenerDisconnected);

    // Setup statistics timer
    m_statsTimer->setInterval(m_updateInterval);
    connect(m_statsTimer, &QTimer::timeout, this, &WebInterface::updateStatistics);
    m_statsTimer->start();

    qCInfo(webInterface) << "WebInterface initialized successfully";
    return true;
}

void WebInterface::shutdown()
{
    if (m_statsTimer) {
        m_statsTimer->stop();
    }
    
    m_mountPoints.clear();
    m_listeners.clear();
    
    qCInfo(webInterface) << "WebInterface shut down";
}

void WebInterface::addMountPoint(const QString& mountPoint, const QString& protocol)
{
    MountPointInfo info;
    info.mountPoint = mountPoint;
    info.protocol = protocol;
    info.lastUpdate = QDateTime::currentDateTime();
    info.started = QDateTime::currentDateTime();
    
    // Initialize enhanced mount point information
    info.serverType = "LegacyStream Audio Server";
    info.description = "LegacyStream Audio Stream";
    info.format = protocol == "icecast" ? "MP3" : "MP3";
    info.quality = "Standard";
    info.serverName = "LegacyStream";
    info.serverGenre = "Various";
    info.isPublic = true;
    info.maxListeners = 1000;
    
    // Get server configuration
    auto& config = Configuration::instance();
    info.serverLocation = config.serverLocation();
    info.serverHostname = config.serverHostname();
    info.serverUrl = "http://" + (info.serverHostname.isEmpty() ? "localhost" : info.serverHostname);
    info.streamUrl = info.serverUrl + "/" + mountPoint;
    info.publicUrl = info.streamUrl;
    
    m_mountPoints[mountPoint] = info;
    m_listeners[mountPoint] = QStringList();
    
    qCInfo(webInterface) << "Added mount point:" << mountPoint << "(" << protocol << ")";
    emit mountPointAdded(mountPoint);
}

void WebInterface::removeMountPoint(const QString& mountPoint)
{
    m_mountPoints.remove(mountPoint);
    m_listeners.remove(mountPoint);
    
    qCInfo(webInterface) << "Removed mount point:" << mountPoint;
    emit mountPointRemoved(mountPoint);
}

void WebInterface::updateMountPointInfo(const QString& mountPoint, const MountPointInfo& info)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint] = info;
        m_mountPoints[mountPoint].lastUpdate = QDateTime::currentDateTime();
        
        emit mountPointUpdated(mountPoint);
    }
}

void WebInterface::onStreamConnected(const QString& mountPoint)
{
    qCDebug(webInterface) << "Stream connected:" << mountPoint;
    // Mount point info will be updated by stream manager
}

void WebInterface::onStreamDisconnected(const QString& mountPoint)
{
    qCDebug(webInterface) << "Stream disconnected:" << mountPoint;
    removeMountPoint(mountPoint);
}

void WebInterface::onListenerConnected(const QString& mountPoint, const QString& clientIP)
{
    if (m_listeners.contains(mountPoint)) {
        if (!m_listeners[mountPoint].contains(clientIP)) {
            m_listeners[mountPoint].append(clientIP);
            m_mountPoints[mountPoint].listeners = m_listeners[mountPoint].size();
            
            if (m_mountPoints[mountPoint].listeners > m_mountPoints[mountPoint].peakListeners) {
                m_mountPoints[mountPoint].peakListeners = m_mountPoints[mountPoint].listeners;
            }
            
            emit mountPointUpdated(mountPoint);
        }
    }
}

void WebInterface::onListenerDisconnected(const QString& mountPoint, const QString& clientIP)
{
    if (m_listeners.contains(mountPoint)) {
        m_listeners[mountPoint].removeAll(clientIP);
        m_mountPoints[mountPoint].listeners = m_listeners[mountPoint].size();
        emit mountPointUpdated(mountPoint);
    }
}

void WebInterface::updateStatistics()
{
    m_serverUptime = QDateTime::currentSecsSinceEpoch() - 
                     QDateTime::fromString("2024-01-01T00:00:00").toSecsSinceEpoch();
    
    m_totalListeners = 0;
    m_totalBytesServed = 0;
    
    for (const auto& mountPoint : m_mountPoints) {
        m_totalListeners += mountPoint.listeners;
        m_totalBytesServed += mountPoint.bytesServed;
    }
}

QJsonObject WebInterface::getMountPointsJson() const
{
    QJsonObject result;
    QJsonArray mountPointsArray;
    
    for (const auto& mountPoint : m_mountPoints) {
        QJsonObject mountPointObj;
        mountPointObj["mountPoint"] = mountPoint.mountPoint;
        mountPointObj["protocol"] = mountPoint.protocol;
        mountPointObj["codec"] = mountPoint.codec;
        mountPointObj["bitrate"] = mountPoint.bitrate;
        mountPointObj["sampleRate"] = mountPoint.sampleRate;
        mountPointObj["channels"] = mountPoint.channels;
        mountPointObj["currentSong"] = mountPoint.currentSong;
        mountPointObj["currentArtist"] = mountPoint.currentArtist;
        mountPointObj["currentAlbum"] = mountPoint.currentAlbum;
        mountPointObj["currentGenre"] = mountPoint.currentGenre;
        mountPointObj["listeners"] = mountPoint.listeners;
        mountPointObj["peakListeners"] = mountPoint.peakListeners;
        mountPointObj["bytesServed"] = QString::number(mountPoint.bytesServed);
        mountPointObj["uptime"] = mountPoint.uptime;
        mountPointObj["isLive"] = mountPoint.isLive;
        mountPointObj["hasFallback"] = mountPoint.hasFallback;
        mountPointObj["fallbackFile"] = mountPoint.fallbackFile;
        mountPointObj["lastUpdate"] = mountPoint.lastUpdate.toString(Qt::ISODate);
        
        // Enhanced mount point information
        mountPointObj["serverType"] = mountPoint.serverType;
        mountPointObj["description"] = mountPoint.description;
        mountPointObj["format"] = mountPoint.format;
        mountPointObj["started"] = mountPoint.started.toString(Qt::ISODate);
        mountPointObj["quality"] = mountPoint.quality;
        mountPointObj["serverUrl"] = mountPoint.serverUrl;
        mountPointObj["streamUrl"] = mountPoint.streamUrl;
        mountPointObj["publicUrl"] = mountPoint.publicUrl;
        mountPointObj["serverName"] = mountPoint.serverName;
        mountPointObj["serverGenre"] = mountPoint.serverGenre;
        mountPointObj["serverIrc"] = mountPoint.serverIrc;
        mountPointObj["serverIcq"] = mountPoint.serverIcq;
        mountPointObj["serverAim"] = mountPoint.serverAim;
        mountPointObj["isPublic"] = mountPoint.isPublic;
        mountPointObj["maxListeners"] = mountPoint.maxListeners;
        mountPointObj["serverLocation"] = mountPoint.serverLocation;
        mountPointObj["serverHostname"] = mountPoint.serverHostname;
        
        mountPointsArray.append(mountPointObj);
    }
    
    result["mountPoints"] = mountPointsArray;
    result["totalMountPoints"] = m_mountPoints.size();
    result["totalListeners"] = m_totalListeners;
    result["totalBytesServed"] = QString::number(m_totalBytesServed);
    result["serverUptime"] = m_serverUptime;
    
    return result;
}

QJsonObject WebInterface::getServerStatsJson() const
{
    QJsonObject result;
    result["serverName"] = "LegacyStream Audio Server";
    result["serverVersion"] = "1.0.0";
    result["totalMountPoints"] = m_mountPoints.size();
    result["totalListeners"] = m_totalListeners;
    result["totalBytesServed"] = QString::number(m_totalBytesServed);
    result["serverUptime"] = m_serverUptime;
    result["formattedUptime"] = formatDuration(m_serverUptime);
    result["formattedBytesServed"] = formatBytes(m_totalBytesServed);
    
    return result;
}

QJsonObject WebInterface::getRelayStatsJson() const
{
    if (m_statisticRelayManager) {
        return m_statisticRelayManager->getRelayStatisticsJson();
    }
    
    QJsonObject emptyStats;
    emptyStats["shoutcastRelays"] = QJsonArray();
    emptyStats["icecastRelays"] = QJsonArray();
    emptyStats["totalRelays"] = 0;
    emptyStats["activeRelays"] = 0;
    return emptyStats;
}

void WebInterface::handleRelayApiRequest(const QString& request, QString& response, QString& contentType)
{
    Q_UNUSED(request)
    
    QJsonObject relayStats = getRelayStatsJson();
    QJsonDocument doc(relayStats);
    
    response = doc.toJson(QJsonDocument::Compact);
    contentType = "application/json";
    
    qCDebug(webInterface) << "Relay API request handled";
}

QString WebInterface::generateStatusPage() const
{
    return generateMainPage();
}

QString WebInterface::generateMountPointsPage() const
{
    QString html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LegacyStream - Mount Points</title>
    <style>
        )" + getDefaultCSS() + getCustomCSS() + R"(
    </style>
</head>
<body>
    <div class="container">
        <header class="header">
            <h1>🎵 LegacyStream Audio Server</h1>
            <p>Mount Points Overview</p>
        </header>
        
        <div class="stats-widget">
            )" + generateStatisticsWidget() + R"(
        </div>
        
        <main class="main-content">
            <div class="mount-points-section">
                <h2>Active Streams</h2>
                )" + generateMountPointsTable() + R"(
            </div>
        </main>
        
        <footer class="footer">
            )" + generateCustomFooter() + R"(
        </footer>
    </div>
    
    <script>
        )" + getDefaultJavaScript() + getCustomJavaScript() + R"(
    </script>
</body>
</html>
    )";
    
    return html;
}

QString WebInterface::generateStreamPage(const QString& mountPoint) const
{
    if (!m_mountPoints.contains(mountPoint)) {
        return "<h1>Stream not found</h1>";
    }
    
    const auto& info = m_mountPoints[mountPoint];
    
    QString html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LegacyStream - )" + escapeHtml(mountPoint) + R"(</title>
    <style>
        )" + getDefaultCSS() + getCustomCSS() + R"(
    </style>
</head>
<body>
    <div class="container">
        <header class="header">
            <h1>🎵 LegacyStream Audio Server</h1>
            <p>Stream: )" + escapeHtml(mountPoint) + R"(</p>
        </header>
        
        <main class="main-content">
            <div class="stream-details">
                )" + generateStreamDetails(mountPoint) + R"(
            </div>
            
            <div class="player-section">
                <h2>Listen Now</h2>
                )" + generatePlayerEmbed(mountPoint) + R"(
            </div>
        </main>
        
        <footer class="footer">
            )" + generateCustomFooter() + R"(
        </footer>
    </div>
    
    <script>
        )" + getDefaultJavaScript() + getCustomJavaScript() + R"(
    </script>
</body>
</html>
    )";
    
    return html;
}

QString WebInterface::generateMainPage() const
{
    QString html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LegacyStream Audio Server</title>
    <style>
        )" + getDefaultCSS() + getCustomCSS() + R"(
    </style>
</head>
<body>
    <div class="container">
        <header class="header">
            )" + generateCustomHeader() + R"(
            <h1>🎵 LegacyStream Audio Server</h1>
            <p>High-Performance Audio Streaming Server</p>
        </header>
        
        <div class="stats-widget">
            )" + generateStatisticsWidget() + R"(
        </div>
        
        <main class="main-content">
            <div class="welcome-section">
                <h2>Welcome to LegacyStream</h2>
                <p>Your high-performance audio streaming server is running successfully!</p>
                <div class="features">
                    <div class="feature">
                        <span class="feature-icon">🎧</span>
                        <h3>Multiple Protocols</h3>
                        <p>Support for IceCast and SHOUTcast protocols</p>
                    </div>
                    <div class="feature">
                        <span class="feature-icon">🔒</span>
                        <h3>SSL/TLS Security</h3>
                        <p>Automatic Let's Encrypt certificate management</p>
                    </div>
                    <div class="feature">
                        <span class="feature-icon">📊</span>
                        <h3>Real-time Statistics</h3>
                        <p>Comprehensive monitoring and analytics</p>
                    </div>
                </div>
            </div>
            
            <div class="mount-points-section">
                <h2>Active Streams</h2>
                )" + generateMountPointsTable() + R"(
            </div>
        </main>
        
        <footer class="footer">
            )" + generateCustomFooter() + R"(
        </footer>
    </div>
    
    <script>
        )" + getDefaultJavaScript() + getCustomJavaScript() + R"(
    </script>
</body>
</html>
    )";
    
    return html;
}

QString WebInterface::generateMountPointsTable() const
{
    if (m_mountPoints.isEmpty()) {
        return R"(
            <div class="no-streams">
                <p>No active streams at the moment.</p>
                <p>Streams will appear here when they become available.</p>
            </div>
        )";
    }
    
    QString table = R"(
        <div class="mount-points-table">
            <table>
                <thead>
                    <tr>
                        <th>Mount Point</th>
                        <th>Server Type</th>
                        <th>Protocol</th>
                        <th>Format</th>
                        <th>Quality</th>
                        <th>Listeners</th>
                        <th>Started</th>
                        <th>Current Song</th>
                        <th>Status</th>
                        <th>Actions</th>
                    </tr>
                </thead>
                <tbody>
    )";
    
    for (const auto& mountPoint : m_mountPoints) {
        QString statusClass = mountPoint.isLive ? "status-live" : "status-fallback";
        QString statusText = mountPoint.isLive ? "Live" : "Fallback";
        
        // Format started time
        QString startedTime = mountPoint.started.isValid() ? 
            mountPoint.started.toString("MMM dd, yyyy HH:mm") : "Unknown";
        
        // Get quality indicator
        QString qualityText = mountPoint.quality.isEmpty() ? "Standard" : mountPoint.quality;
        QString qualityClass = "quality-" + qualityText.toLower();
        
        table += QString(R"(
                    <tr>
                        <td><strong>%1</strong></td>
                        <td>%2</td>
                        <td>%3 %4</td>
                        <td>%5</td>
                        <td><span class="%6">%7</span></td>
                        <td>%8</td>
                        <td>%9</td>
                        <td>%10</td>
                        <td><span class="%11">%12</span></td>
                        <td>
                            <a href="/%1" class="btn btn-primary">Listen</a>
                            <a href="/%1/status" class="btn btn-secondary">Details</a>
                        </td>
                    </tr>
        )").arg(
            escapeHtml(mountPoint.mountPoint),
            escapeHtml(mountPoint.serverType.isEmpty() ? "LegacyStream" : mountPoint.serverType),
            getProtocolIcon(mountPoint.protocol),
            escapeHtml(mountPoint.protocol.toUpper()),
            escapeHtml(mountPoint.format.isEmpty() ? mountPoint.codec.toUpper() : mountPoint.format),
            qualityClass,
            escapeHtml(qualityText),
            QString::number(mountPoint.listeners),
            escapeHtml(startedTime),
            escapeHtml(mountPoint.currentSong.isEmpty() ? "Unknown" : mountPoint.currentSong),
            statusClass,
            statusText
        );
    }
    
    table += R"(
                </tbody>
            </table>
        </div>
    )";
    
    return table;
}

QString WebInterface::generateStreamDetails(const QString& mountPoint) const
{
    if (!m_mountPoints.contains(mountPoint)) {
        return "<p>Stream not found</p>";
    }
    
    const auto& info = m_mountPoints[mountPoint];
    
    QString details = QString(R"(
        <div class="stream-info">
            <h2>%1</h2>
            
            <div class="stream-overview">
                <div class="overview-item">
                    <span class="overview-label">Server Type:</span>
                    <span class="overview-value">%2</span>
                </div>
                <div class="overview-item">
                    <span class="overview-label">Description:</span>
                    <span class="overview-value">%3</span>
                </div>
                <div class="overview-item">
                    <span class="overview-label">Quality:</span>
                    <span class="overview-value quality-%4">%5</span>
                </div>
                <div class="overview-item">
                    <span class="overview-label">Status:</span>
                    <span class="overview-value %6">%7</span>
                </div>
            </div>
            
            <div class="info-grid">
                <div class="info-item">
                    <label>Protocol:</label>
                    <span>%8 %9</span>
                </div>
                <div class="info-item">
                    <label>Format:</label>
                    <span>%10</span>
                </div>
                <div class="info-item">
                    <label>Codec:</label>
                    <span>%11</span>
                </div>
                <div class="info-item">
                    <label>Bitrate:</label>
                    <span>%12</span>
                </div>
                <div class="info-item">
                    <label>Sample Rate:</label>
                    <span>%13</span>
                </div>
                <div class="info-item">
                    <label>Channels:</label>
                    <span>%14</span>
                </div>
                <div class="info-item">
                    <label>Started:</label>
                    <span>%15</span>
                </div>
                <div class="info-item">
                    <label>Uptime:</label>
                    <span>%16</span>
                </div>
                <div class="info-item">
                    <label>Current Listeners:</label>
                    <span>%17</span>
                </div>
                <div class="info-item">
                    <label>Peak Listeners:</label>
                    <span>%18</span>
                </div>
                <div class="info-item">
                    <label>Max Listeners:</label>
                    <span>%19</span>
                </div>
                <div class="info-item">
                    <label>Bytes Served:</label>
                    <span>%20</span>
                </div>
                <div class="info-item">
                    <label>Server Location:</label>
                    <span>%21</span>
                </div>
                <div class="info-item">
                    <label>Server Hostname:</label>
                    <span>%22</span>
                </div>
                <div class="info-item">
                    <label>Public:</label>
                    <span>%23</span>
                </div>
            </div>
            
            <div class="server-info">
                <h3>Server Information</h3>
                <div class="server-grid">
                    <div class="server-item">
                        <label>Server Name:</label>
                        <span>%24</span>
                    </div>
                    <div class="server-item">
                        <label>Server Genre:</label>
                        <span>%25</span>
                    </div>
                    <div class="server-item">
                        <label>IRC Channel:</label>
                        <span>%26</span>
                    </div>
                    <div class="server-item">
                        <label>ICQ Number:</label>
                        <span>%27</span>
                    </div>
                    <div class="server-item">
                        <label>AIM Handle:</label>
                        <span>%28</span>
                    </div>
                </div>
            </div>
            
            <div class="url-info">
                <h3>Stream URLs</h3>
                <div class="url-grid">
                    <div class="url-item">
                        <label>Direct Stream:</label>
                        <code>%29</code>
                    </div>
                    <div class="url-item">
                        <label>Public URL:</label>
                        <code>%30</code>
                    </div>
                    <div class="url-item">
                        <label>Server URL:</label>
                        <code>%31</code>
                    </div>
                </div>
            </div>
            
            <div class="current-track">
                <h3>Current Track</h3>
                <div class="track-info">
                    <div class="track-title">%32</div>
                    <div class="track-artist">%33</div>
                    <div class="track-album">%34</div>
                    <div class="track-genre">%35</div>
                </div>
            </div>
        </div>
    )").arg(
        escapeHtml(info.mountPoint),
        escapeHtml(info.serverType.isEmpty() ? "LegacyStream Audio Server" : info.serverType),
        escapeHtml(info.description.isEmpty() ? "LegacyStream Audio Stream" : info.description),
        info.quality.isEmpty() ? "standard" : info.quality.toLower(),
        escapeHtml(info.quality.isEmpty() ? "Standard" : info.quality),
        info.isLive ? "status-live" : "status-fallback",
        info.isLive ? "Live" : "Fallback",
        getProtocolIcon(info.protocol),
        escapeHtml(info.protocol.toUpper()),
        escapeHtml(info.format.isEmpty() ? info.codec.toUpper() : info.format),
        escapeHtml(info.codec.toUpper()),
        escapeHtml(formatBitrate(info.bitrate)),
        escapeHtml(info.sampleRate),
        escapeHtml(info.channels),
        info.started.isValid() ? info.started.toString("MMM dd, yyyy HH:mm") : "Unknown",
        formatDuration(info.uptime),
        QString::number(info.listeners),
        QString::number(info.peakListeners),
        QString::number(info.maxListeners),
        formatBytes(info.bytesServed),
        escapeHtml(info.serverLocation.isEmpty() ? "Unknown" : info.serverLocation),
        escapeHtml(info.serverHostname.isEmpty() ? "localhost" : info.serverHostname),
        info.isPublic ? "Yes" : "No",
        escapeHtml(info.serverName.isEmpty() ? "LegacyStream" : info.serverName),
        escapeHtml(info.serverGenre.isEmpty() ? "Various" : info.serverGenre),
        escapeHtml(info.serverIrc.isEmpty() ? "None" : info.serverIrc),
        escapeHtml(info.serverIcq.isEmpty() ? "None" : info.serverIcq),
        escapeHtml(info.serverAim.isEmpty() ? "None" : info.serverAim),
        escapeHtml(info.streamUrl.isEmpty() ? "http://" + info.serverHostname + "/" + info.mountPoint : info.streamUrl),
        escapeHtml(info.publicUrl.isEmpty() ? "http://" + info.serverHostname + "/" + info.mountPoint : info.publicUrl),
        escapeHtml(info.serverUrl.isEmpty() ? "http://" + info.serverHostname : info.serverUrl),
        escapeHtml(info.currentSong.isEmpty() ? "Unknown" : info.currentSong),
        escapeHtml(info.currentArtist.isEmpty() ? "Unknown Artist" : info.currentArtist),
        escapeHtml(info.currentAlbum.isEmpty() ? "Unknown Album" : info.currentAlbum),
        escapeHtml(info.currentGenre.isEmpty() ? "Unknown Genre" : info.currentGenre)
    );
    
    return details;
}

QString WebInterface::generatePlayerEmbed(const QString& mountPoint) const
{
    QString playerHtml = QString(R"(
        <div class="player-container">
            <audio controls autoplay>
                <source src="/%1" type="audio/mpeg">
                <source src="/%1" type="audio/aac">
                Your browser does not support the audio element.
            </audio>
            
            <div class="player-info">
                <p><strong>Direct Stream URL:</strong></p>
                <code>http://%2:%3/%4</code>
                <p><strong>IceCast URL:</strong></p>
                <code>http://%2:%3/%4</code>
            </div>
            
            <div class="player-actions">
                <button onclick="copyStreamUrl()" class="btn btn-secondary">Copy URL</button>
                <button onclick="openInPlayer()" class="btn btn-primary">Open in Player</button>
            </div>
        </div>
        
        <script>
            function copyStreamUrl() {
                navigator.clipboard.writeText('http://' + window.location.host + '/%1');
                alert('Stream URL copied to clipboard!');
            }
            
            function openInPlayer() {
                window.open('http://' + window.location.host + '/%1', '_blank');
            }
        </script>
    )").arg(
        escapeHtml(mountPoint),
        "localhost",  // TODO: Get from configuration
        "8000",       // TODO: Get from configuration
        escapeHtml(mountPoint)
    );
    
    return playerHtml;
}

QString WebInterface::generateStatisticsWidget() const
{
    QString stats = QString(R"(
        <div class="stats-container">
            <div class="stat-item">
                <div class="stat-value">%1</div>
                <div class="stat-label">Active Streams</div>
            </div>
            <div class="stat-item">
                <div class="stat-value">%2</div>
                <div class="stat-label">Total Listeners</div>
            </div>
            <div class="stat-item">
                <div class="stat-value">%3</div>
                <div class="stat-label">Bytes Served</div>
            </div>
            <div class="stat-item">
                <div class="stat-value">%4</div>
                <div class="stat-label">Uptime</div>
            </div>
        </div>
    )").arg(
        QString::number(m_mountPoints.size()),
        QString::number(m_totalListeners),
        formatBytes(m_totalBytesServed),
        formatDuration(m_serverUptime)
    );
    
    return stats;
}

QString WebInterface::generateCustomHeader() const
{
    return m_customHTML.isEmpty() ? "" : m_customHTML;
}

QString WebInterface::generateCustomFooter() const
{
    return QString(R"(
        <p>&copy; 2024 LegacyStream Audio Server. Built with ❤️ using Qt and C++.</p>
        <p>Server Version: 1.0.0 | Uptime: %1</p>
    )").arg(formatDuration(m_serverUptime));
}

QString WebInterface::getDefaultCSS() const
{
    return R"(
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
            line-height: 1.6;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        .header {
            text-align: center;
            background: rgba(255, 255, 255, 0.95);
            padding: 40px 20px;
            border-radius: 15px;
            margin-bottom: 30px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
            backdrop-filter: blur(10px);
        }
        
        .header h1 {
            font-size: 2.5em;
            color: #2c3e50;
            margin-bottom: 10px;
        }
        
        .header p {
            font-size: 1.2em;
            color: #7f8c8d;
        }
        
        .stats-widget {
            background: rgba(255, 255, 255, 0.95);
            padding: 30px;
            border-radius: 15px;
            margin-bottom: 30px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
            backdrop-filter: blur(10px);
        }
        
        .stats-container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
        }
        
        .stat-item {
            text-align: center;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border-radius: 10px;
            transition: transform 0.3s ease;
        }
        
        .stat-item:hover {
            transform: translateY(-5px);
        }
        
        .stat-value {
            font-size: 2em;
            font-weight: bold;
            margin-bottom: 5px;
        }
        
        .stat-label {
            font-size: 0.9em;
            opacity: 0.9;
        }
        
        .main-content {
            background: rgba(255, 255, 255, 0.95);
            padding: 30px;
            border-radius: 15px;
            margin-bottom: 30px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);
            backdrop-filter: blur(10px);
        }
        
        .welcome-section {
            margin-bottom: 40px;
        }
        
        .welcome-section h2 {
            color: #2c3e50;
            margin-bottom: 20px;
            font-size: 1.8em;
        }
        
        .features {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-top: 30px;
        }
        
        .feature {
            text-align: center;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 10px;
            transition: transform 0.3s ease;
        }
        
        .feature:hover {
            transform: translateY(-5px);
        }
        
        .feature-icon {
            font-size: 2em;
            margin-bottom: 15px;
            display: block;
        }
        
        .feature h3 {
            color: #2c3e50;
            margin-bottom: 10px;
        }
        
        .mount-points-section h2 {
            color: #2c3e50;
            margin-bottom: 20px;
            font-size: 1.8em;
        }
        
        .mount-points-table {
            overflow-x: auto;
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            background: white;
            border-radius: 10px;
            overflow: hidden;
            box-shadow: 0 4px 16px rgba(0, 0, 0, 0.1);
        }
        
        th, td {
            padding: 15px;
            text-align: left;
            border-bottom: 1px solid #eee;
        }
        
        th {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            font-weight: 600;
        }
        
        tr:hover {
            background: #f8f9fa;
        }
        
        .status-live {
            background: #27ae60;
            color: white;
            padding: 5px 10px;
            border-radius: 15px;
            font-size: 0.8em;
            font-weight: bold;
        }
        
        .status-fallback {
            background: #e74c3c;
            color: white;
            padding: 5px 10px;
            border-radius: 15px;
            font-size: 0.8em;
            font-weight: bold;
        }
        
        .quality-high {
            background: #27ae60;
            color: white;
            padding: 5px 10px;
            border-radius: 15px;
            font-size: 0.8em;
            font-weight: bold;
        }
        
        .quality-medium {
            background: #f39c12;
            color: white;
            padding: 5px 10px;
            border-radius: 15px;
            font-size: 0.8em;
            font-weight: bold;
        }
        
        .quality-low {
            background: #e74c3c;
            color: white;
            padding: 5px 10px;
            border-radius: 15px;
            font-size: 0.8em;
            font-weight: bold;
        }
        
        .quality-standard {
            background: #3498db;
            color: white;
            padding: 5px 10px;
            border-radius: 15px;
            font-size: 0.8em;
            font-weight: bold;
        }
        
        .btn {
            display: inline-block;
            padding: 8px 16px;
            margin: 2px;
            border: none;
            border-radius: 5px;
            text-decoration: none;
            font-size: 0.9em;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        
        .btn-primary {
            background: #3498db;
            color: white;
        }
        
        .btn-primary:hover {
            background: #2980b9;
        }
        
        .btn-secondary {
            background: #95a5a6;
            color: white;
        }
        
        .btn-secondary:hover {
            background: #7f8c8d;
        }
        
        .no-streams {
            text-align: center;
            padding: 40px;
            color: #7f8c8d;
        }
        
        .stream-details {
            margin-bottom: 30px;
        }
        
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }
        
        .info-item {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
        }
        
        .info-item label {
            font-weight: bold;
            color: #2c3e50;
            display: block;
            margin-bottom: 5px;
        }
        
        .stream-overview {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
            margin-bottom: 30px;
            padding: 20px;
            background: #ecf0f1;
            border-radius: 10px;
        }
        
        .overview-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px;
            background: white;
            border-radius: 8px;
        }
        
        .overview-label {
            font-weight: bold;
            color: #2c3e50;
        }
        
        .overview-value {
            color: #34495e;
        }
        
        .server-info, .url-info {
            margin-top: 30px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 10px;
        }
        
        .server-grid, .url-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-top: 15px;
        }
        
        .server-item, .url-item {
            background: white;
            padding: 15px;
            border-radius: 8px;
        }
        
        .server-item label, .url-item label {
            font-weight: bold;
            color: #2c3e50;
            display: block;
            margin-bottom: 5px;
        }
        
        .url-item code {
            background: #2c3e50;
            color: white;
            padding: 5px 10px;
            border-radius: 5px;
            display: block;
            margin: 5px 0;
            word-break: break-all;
            font-size: 0.9em;
        }
        
        .current-track {
            margin-top: 30px;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 10px;
        }
        
        .track-info {
            margin-top: 15px;
        }
        
        .track-title {
            font-size: 1.2em;
            font-weight: bold;
            color: #2c3e50;
            margin-bottom: 5px;
        }
        
        .track-artist {
            color: #7f8c8d;
            margin-bottom: 5px;
        }
        
        .track-album, .track-genre {
            color: #95a5a6;
            font-size: 0.9em;
        }
        
        .player-container {
            text-align: center;
            padding: 30px;
            background: #f8f9fa;
            border-radius: 10px;
            margin-top: 20px;
        }
        
        audio {
            width: 100%;
            max-width: 400px;
            margin-bottom: 20px;
        }
        
        .player-info {
            margin: 20px 0;
            text-align: left;
        }
        
        .player-info code {
            background: #2c3e50;
            color: white;
            padding: 5px 10px;
            border-radius: 5px;
            display: block;
            margin: 5px 0;
            word-break: break-all;
        }
        
        .player-actions {
            margin-top: 20px;
        }
        
        .footer {
            text-align: center;
            padding: 20px;
            color: white;
            font-size: 0.9em;
        }
        
        @media (max-width: 768px) {
            .container {
                padding: 10px;
            }
            
            .header h1 {
                font-size: 2em;
            }
            
            .stats-container {
                grid-template-columns: repeat(2, 1fr);
            }
            
            .features {
                grid-template-columns: 1fr;
            }
            
            .info-grid {
                grid-template-columns: 1fr;
            }
        }
    )";
}

QString WebInterface::getDefaultJavaScript() const
{
    return R"(
        // Auto-refresh statistics every 5 seconds
        setInterval(function() {
            fetch('/api/stats')
                .then(response => response.json())
                .then(data => {
                    updateStats(data);
                })
                .catch(error => console.error('Error fetching stats:', error));
        }, 5000);
        
        function updateStats(data) {
            // Update statistics display
            const statElements = document.querySelectorAll('.stat-value');
            if (statElements.length >= 4) {
                statElements[0].textContent = data.totalMountPoints || 0;
                statElements[1].textContent = data.totalListeners || 0;
                statElements[2].textContent = formatBytes(data.totalBytesServed || 0);
                statElements[3].textContent = formatDuration(data.serverUptime || 0);
            }
        }
        
        function formatBytes(bytes) {
            if (bytes === 0) return '0 B';
            const k = 1024;
            const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
            const i = Math.floor(Math.log(bytes) / Math.log(k));
            return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
        }
        
        function formatDuration(seconds) {
            const days = Math.floor(seconds / 86400);
            const hours = Math.floor((seconds % 86400) / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);
            const secs = seconds % 60;
            
            let result = '';
            if (days > 0) result += days + 'd ';
            if (hours > 0) result += hours + 'h ';
            if (minutes > 0) result += minutes + 'm ';
            result += secs + 's';
            
            return result;
        }
        
        // Add smooth scrolling for anchor links
        document.querySelectorAll('a[href^="#"]').forEach(anchor => {
            anchor.addEventListener('click', function (e) {
                e.preventDefault();
                document.querySelector(this.getAttribute('href')).scrollIntoView({
                    behavior: 'smooth'
                });
            });
        });
        
        // Add loading states for buttons
        document.querySelectorAll('.btn').forEach(button => {
            button.addEventListener('click', function() {
                this.style.opacity = '0.7';
                setTimeout(() => {
                    this.style.opacity = '1';
                }, 200);
            });
        });
    )";
}

QString WebInterface::getCustomCSS() const
{
    return m_customCSS;
}

QString WebInterface::getCustomJavaScript() const
{
    return m_customJavaScript;
}

void WebInterface::setCustomTheme(const QString& themeName)
{
    m_customTheme = themeName;
}

void WebInterface::setCustomCSS(const QString& css)
{
    m_customCSS = css;
}

void WebInterface::setCustomJavaScript(const QString& js)
{
    m_customJavaScript = js;
}

void WebInterface::setCustomHTML(const QString& html)
{
    m_customHTML = html;
}

QString WebInterface::formatBytes(qint64 bytes) const
{
    if (bytes == 0) return "0 B";
    
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;
    
    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', 2).arg(units[unitIndex]);
}

QString WebInterface::formatDuration(qint64 seconds) const
{
    int days = seconds / 86400;
    int hours = (seconds % 86400) / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    QString result;
    if (days > 0) result += QString("%1d ").arg(days);
    if (hours > 0) result += QString("%1h ").arg(hours);
    if (minutes > 0) result += QString("%1m ").arg(minutes);
    result += QString("%1s").arg(secs);
    
    return result;
}

QString WebInterface::formatBitrate(const QString& bitrate) const
{
    if (bitrate.isEmpty()) return "Unknown";
    
    bool ok;
    int rate = bitrate.toInt(&ok);
    if (!ok) return bitrate;
    
    if (rate >= 1000) {
        return QString("%1 kbps").arg(rate / 1000);
    } else {
        return QString("%1 bps").arg(rate);
    }
}

QString WebInterface::getProtocolIcon(const QString& protocol) const
{
    if (protocol.toLower() == "icecast") {
        return "❄️";
    } else if (protocol.toLower() == "shoutcast") {
        return "📢";
    }
    return "🎵";
}

QString WebInterface::getCodecIcon(const QString& codec) const
{
    if (codec.toLower() == "mp3") {
        return "🎵";
    } else if (codec.toLower() == "aac") {
        return "🎼";
    } else if (codec.toLower() == "ogg") {
        return "🎶";
    } else if (codec.toLower() == "opus") {
        return "🎤";
    } else if (codec.toLower() == "flac") {
        return "🎧";
    }
    return "🎵";
}

QString WebInterface::escapeHtml(const QString& text) const
{
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&#39;");
    return escaped;
}

} // namespace WebInterface
} // namespace LegacyStream 