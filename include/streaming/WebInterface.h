#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
// #include <QWebSocketServer>
// #include <QWebSocket>
#include <memory>

namespace LegacyStream {

class StreamManager;
class HttpServer;

namespace StatisticRelay {
    class StatisticRelayManager;
}

namespace WebInterface {

class MountPointInfo {
public:
    QString mountPoint;
    QString protocol;  // "icecast" or "shoutcast"
    QString codec;
    QString bitrate;
    QString sampleRate;
    QString channels;
    QString currentSong;
    QString currentArtist;
    QString currentAlbum;
    QString currentGenre;
    int listeners = 0;
    int peakListeners = 0;
    qint64 bytesServed = 0;
    qint64 uptime = 0;
    bool isLive = false;
    bool hasFallback = false;
    QString fallbackFile;
    QDateTime lastUpdate;
    
    // Enhanced mount point information
    QString serverType;        // "LegacyStream Audio Server"
    QString description;       // Stream description
    QString format;           // Audio format (MP3, AAC, etc.)
    QDateTime started;        // When the stream started
    QString quality;          // Stream quality (High, Medium, Low)
    QString serverUrl;        // Server URL
    QString streamUrl;        // Direct stream URL
    QString publicUrl;        // Public accessible URL
    QString serverName;       // Server name
    QString serverGenre;      // Server genre
    QString serverIrc;        // IRC channel
    QString serverIcq;        // ICQ number
    QString serverAim;        // AIM handle
    bool isPublic = true;     // Whether stream is public
    int maxListeners = 0;     // Maximum allowed listeners
    QString serverLocation;   // Server location
    QString serverHostname;   // Server hostname
};

class WebInterface : public QObject
{
    Q_OBJECT

public:
    explicit WebInterface(QObject* parent = nullptr);
    ~WebInterface();

    // Initialization
    bool initialize(HttpServer* httpServer, StreamManager* streamManager, StatisticRelay::StatisticRelayManager* statisticRelayManager = nullptr);
    void shutdown();
    bool isInitialized() const;

    // Mount point management
    void addMountPoint(const QString& mountPoint, const QString& protocol);
    void removeMountPoint(const QString& mountPoint);
    void updateMountPointInfo(const QString& mountPoint, const MountPointInfo& info);

    // Web interface configuration
    void setCustomTheme(const QString& themeName);
    void setCustomCSS(const QString& css);
    void setCustomJavaScript(const QString& js);
    void setCustomHTML(const QString& html);

    // Interactive controls
    void startStream(const QString& mountPoint);
    void stopStream(const QString& mountPoint);
    void restartStream(const QString& mountPoint);
    void setStreamQuality(const QString& mountPoint, const QString& quality);
    void setStreamBitrate(const QString& mountPoint, int bitrate);
    void setStreamMetadata(const QString& mountPoint, const QString& title, const QString& artist);

    // Statistics and monitoring
    QJsonObject getMountPointsJson() const;
    QJsonObject getServerStatsJson() const;
    QJsonObject getRelayStatsJson() const;
    QJsonObject getAnalyticsData() const;
    QString generateStatusPage() const;
    QString generateMountPointsPage() const;
    QString generateStreamPage(const QString& mountPoint) const;
    QString generateAnalyticsPage() const;
    QString generateMobilePage() const;

signals:
    void mountPointAdded(const QString& mountPoint);
    void mountPointRemoved(const QString& mountPoint);
    void mountPointUpdated(const QString& mountPoint);
    void webInterfaceRequested(const QString& path, const QString& clientIP);
    void streamControlRequested(const QString& mountPoint, const QString& action);
    void analyticsDataUpdated(const QJsonObject& data);

private slots:
    void onStreamConnected(const QString& mountPoint);
    void onStreamDisconnected(const QString& mountPoint);
    void onListenerConnected(const QString& mountPoint, const QString& clientIP);
    void onListenerDisconnected(const QString& mountPoint, const QString& clientIP);
    void updateStatistics();
    
    // WebSocket handling (disabled - Qt WebSockets not available)
    // void onWebSocketConnection();
    // void onWebSocketDisconnection();
    // void onWebSocketMessage(const QString& message);
    // void onWebSocketError(QAbstractSocket::SocketError error);
    // void broadcastToWebSockets(const QJsonObject& data);

private:
    // HTTP request handlers
    void handleRootRequest(const QString& request, QString& response, QString& contentType);
    void handleStatusRequest(const QString& request, QString& response, QString& contentType);
    void handleMountPointsRequest(const QString& request, QString& response, QString& contentType);
    void handleStreamRequest(const QString& request, QString& response, QString& contentType);
    void handleApiRequest(const QString& request, QString& response, QString& contentType);
    void handleRelayApiRequest(const QString& request, QString& response, QString& contentType);
    void handleAnalyticsRequest(const QString& request, QString& response, QString& contentType);
    void handleMobileRequest(const QString& request, QString& response, QString& contentType);
    // void handleWebSocketRequest(const QString& request, QString& response, QString& contentType);
    void handleFaviconRequest(const QString& request, QString& response, QString& contentType);
    void handleCssRequest(const QString& request, QString& response, QString& contentType);
    void handleJsRequest(const QString& request, QString& response, QString& contentType);

    // HTML generation
    QString generateMainPage() const;
    QString generateMountPointsTable() const;
    QString generateStreamDetails(const QString& mountPoint) const;
    QString generatePlayerEmbed(const QString& mountPoint) const;
    QString generateStatisticsWidget() const;
    QString generateAnalyticsWidget() const;
    QString generateInteractiveControls(const QString& mountPoint) const;
    QString generateMobileInterface() const;
    QString generateCustomHeader() const;
    QString generateCustomFooter() const;

    // CSS and JavaScript
    QString getDefaultCSS() const;
    QString getDefaultJavaScript() const;
    QString getMobileCSS() const;
    QString getMobileJavaScript() const;
    QString getAnalyticsCSS() const;
    QString getAnalyticsJavaScript() const;
    QString getCustomCSS() const;
    QString getCustomJavaScript() const;

    // Utility functions
    QString formatBytes(qint64 bytes) const;
    QString formatDuration(qint64 seconds) const;
    QString formatBitrate(const QString& bitrate) const;
    QString getProtocolIcon(const QString& protocol) const;
    QString getCodecIcon(const QString& codec) const;
    QString escapeHtml(const QString& text) const;
    QJsonObject createAnalyticsData() const;
    QJsonObject createRealTimeData() const;

    // Component references
    HttpServer* m_httpServer = nullptr;
    StreamManager* m_streamManager = nullptr;
    StatisticRelay::StatisticRelayManager* m_statisticRelayManager = nullptr;

    // WebSocket server (disabled - Qt WebSockets not available)
    // QWebSocketServer* m_webSocketServer = nullptr;
    // QList<QWebSocket*> m_webSocketClients;
    
    // State tracking
    bool m_isInitialized = false;

    // Mount point data
    QMap<QString, MountPointInfo> m_mountPoints;
    QMap<QString, QStringList> m_listeners;  // mountPoint -> list of client IPs

    // Customization
    QString m_customTheme;
    QString m_customCSS;
    QString m_customJavaScript;
    QString m_customHTML;

    // Statistics
    QTimer* m_statsTimer;
    qint64 m_totalListeners = 0;
    qint64 m_totalBytesServed = 0;
    qint64 m_serverUptime = 0;

    // Analytics data
    QJsonObject m_analyticsData;
    QTimer* m_analyticsTimer;

    // Configuration
    bool m_enableWebInterface = true;
    bool m_enableRealTimeUpdates = true;
    bool m_enablePlayerEmbed = true;
    bool m_enableStatistics = true;
    bool m_enableWebSockets = false; // Disabled - Qt WebSockets not available
    bool m_enableInteractiveControls = true;
    bool m_enableMobileResponsive = true;
    bool m_enableAnalyticsDashboard = true;
    int m_updateInterval = 1000;  // milliseconds
    int m_webSocketPort = 8081;   // WebSocket server port

    Q_DISABLE_COPY(WebInterface)
};

} // namespace WebInterface
} // namespace LegacyStream 