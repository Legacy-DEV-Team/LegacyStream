#include "streaming/StatisticRelayManager.h"
#include "streaming/StreamManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>
#include <QThread>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QUuid>

Q_LOGGING_CATEGORY(statisticRelay, "statisticRelay")

namespace LegacyStream {
namespace StatisticRelay {

StatisticRelayManager::StatisticRelayManager(QObject* parent)
    : QObject(parent)
    , m_networkManager(std::make_unique<QNetworkAccessManager>())
    , m_updateTimer(new QTimer(this))
    , m_realTimeCollectionTimer(new QTimer(this))
    , m_webSocketServer(std::make_unique<QWebSocketServer>(QStringLiteral("LegacyStream Statistics"), QWebSocketServer::NonSecureMode, this))
{
    qCDebug(statisticRelay) << "StatisticRelayManager created";
    
    connect(m_networkManager.get(), &QNetworkAccessManager::finished,
            this, &StatisticRelayManager::onShoutcastRelayFinished);
    
    connect(m_updateTimer, &QTimer::timeout,
            this, &StatisticRelayManager::updateStatistics);
    
    // Set up real-time collection timer
    m_realTimeCollectionTimer->setSingleShot(false);
    m_realTimeCollectionTimer->setInterval(m_realTimeUpdateInterval * 1000);
    connect(m_realTimeCollectionTimer, &QTimer::timeout,
            this, &StatisticRelayManager::onRealTimeCollectionTimer);
    
    // Set up WebSocket server
    connect(m_webSocketServer.get(), &QWebSocketServer::newConnection,
            this, &StatisticRelayManager::onWebSocketNewConnection);
}

StatisticRelayManager::~StatisticRelayManager()
{
    shutdown();
}

bool StatisticRelayManager::initialize(StreamManager* streamManager)
{
    if (!streamManager) {
        qCCritical(statisticRelay) << "Invalid stream manager";
        return false;
    }

    m_streamManager = streamManager;
    
    auto& config = Configuration::instance();
    m_updateInterval = config.relayReconnectInterval();
    
    qCInfo(statisticRelay) << "StatisticRelayManager initialized successfully";
    return true;
}

void StatisticRelayManager::shutdown()
{
    stop();
    
    // Cancel all active network requests
    for (auto reply : m_activeReplies.keys()) {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    }
    m_activeReplies.clear();
    
    m_streamManager = nullptr;
    qCInfo(statisticRelay) << "StatisticRelayManager shutdown complete";
}

void StatisticRelayManager::addShoutcastRelay(const QString& name, const ShoutcastRelayConfig& config)
{
    m_shoutcastRelays[name] = config;
    m_relayConnectionStatus[name] = false;
    qCInfo(statisticRelay) << "Added Shoutcast relay:" << name << "->" << config.host << ":" << config.port;
}

void StatisticRelayManager::addIcecastRelay(const QString& name, const IcecastRelayConfig& config)
{
    m_icecastRelays[name] = config;
    m_relayConnectionStatus[name] = false;
    qCInfo(statisticRelay) << "Added Icecast relay:" << name << "->" << config.host << ":" << config.port;
}

void StatisticRelayManager::removeRelay(const QString& name)
{
    bool removed = false;
    
    if (m_shoutcastRelays.contains(name)) {
        m_shoutcastRelays.remove(name);
        removed = true;
    }
    
    if (m_icecastRelays.contains(name)) {
        m_icecastRelays.remove(name);
        removed = true;
    }
    
    m_relayConnectionStatus.remove(name);
    m_relayStatistics.remove(name);
    
    if (removed) {
        qCInfo(statisticRelay) << "Removed relay:" << name;
    }
}

void StatisticRelayManager::updateRelayConfig(const QString& name, const ShoutcastRelayConfig& config)
{
    if (m_shoutcastRelays.contains(name)) {
        m_shoutcastRelays[name] = config;
        qCInfo(statisticRelay) << "Updated Shoutcast relay config:" << name;
    }
}

void StatisticRelayManager::updateRelayConfig(const QString& name, const IcecastRelayConfig& config)
{
    if (m_icecastRelays.contains(name)) {
        m_icecastRelays[name] = config;
        qCInfo(statisticRelay) << "Updated Icecast relay config:" << name;
    }
}

void StatisticRelayManager::enableRelay(const QString& name, bool enabled)
{
    if (m_shoutcastRelays.contains(name)) {
        m_shoutcastRelays[name].enabled = enabled;
    } else if (m_icecastRelays.contains(name)) {
        m_icecastRelays[name].enabled = enabled;
    }
    
    qCInfo(statisticRelay) << "Relay" << name << (enabled ? "enabled" : "disabled");
}

QMap<QString, RelayStatistics> StatisticRelayManager::getRelayStatistics() const
{
    return m_relayStatistics;
}

QJsonObject StatisticRelayManager::getRelayStatisticsJson() const
{
    QJsonObject result;
    QJsonArray shoutcastRelays;
    QJsonArray icecastRelays;
    
    // Shoutcast relays
    for (auto it = m_shoutcastRelays.begin(); it != m_shoutcastRelays.end(); ++it) {
        QJsonObject relay;
        relay["name"] = it.key();
        relay["type"] = "shoutcast";
        relay["host"] = it.value().host;
        relay["port"] = it.value().port;
        relay["enabled"] = it.value().enabled;
        relay["mountPoint"] = it.value().mountPoint;
        relay["connected"] = m_relayConnectionStatus.value(it.key(), false);
        
        if (m_relayStatistics.contains(it.key())) {
            const auto& stats = m_relayStatistics[it.key()];
            QJsonObject statistics;
            statistics["currentListeners"] = stats.currentListeners;
            statistics["peakListeners"] = stats.peakListeners;
            statistics["bytesServed"] = QString::number(stats.bytesServed);
            statistics["uptime"] = stats.uptime;
            statistics["currentSong"] = stats.currentSong;
            statistics["currentArtist"] = stats.currentArtist;
            statistics["lastUpdate"] = stats.lastUpdate.toString(Qt::ISODate);
            relay["statistics"] = statistics;
        }
        
        shoutcastRelays.append(relay);
    }
    
    // Icecast relays
    for (auto it = m_icecastRelays.begin(); it != m_icecastRelays.end(); ++it) {
        QJsonObject relay;
        relay["name"] = it.key();
        relay["type"] = "icecast";
        relay["host"] = it.value().host;
        relay["port"] = it.value().port;
        relay["enabled"] = it.value().enabled;
        relay["alias"] = it.value().alias;
        relay["mountPoint"] = it.value().mountPoint;
        relay["connected"] = m_relayConnectionStatus.value(it.key(), false);
        
        if (m_relayStatistics.contains(it.key())) {
            const auto& stats = m_relayStatistics[it.key()];
            QJsonObject statistics;
            statistics["currentListeners"] = stats.currentListeners;
            statistics["peakListeners"] = stats.peakListeners;
            statistics["bytesServed"] = QString::number(stats.bytesServed);
            statistics["uptime"] = stats.uptime;
            statistics["currentSong"] = stats.currentSong;
            statistics["currentArtist"] = stats.currentArtist;
            statistics["lastUpdate"] = stats.lastUpdate.toString(Qt::ISODate);
            relay["statistics"] = statistics;
        }
        
        icecastRelays.append(relay);
    }
    
    result["shoutcastRelays"] = shoutcastRelays;
    result["icecastRelays"] = icecastRelays;
    result["totalRelays"] = m_shoutcastRelays.size() + m_icecastRelays.size();
    result["activeRelays"] = m_relayConnectionStatus.values().count(true);
    
    return result;
}

void StatisticRelayManager::start()
{
    if (m_isRunning) {
        qCWarning(statisticRelay) << "StatisticRelayManager already running";
        return;
    }
    
    m_isRunning = true;
    m_updateTimer->setInterval(m_updateInterval * 1000);
    m_updateTimer->start();
    
    qCInfo(statisticRelay) << "StatisticRelayManager started";
}

void StatisticRelayManager::stop()
{
    if (!m_isRunning) {
        return;
    }
    
    m_isRunning = false;
    m_updateTimer->stop();
    
    // Cancel all active requests
    for (auto reply : m_activeReplies.keys()) {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    }
    m_activeReplies.clear();
    
    qCInfo(statisticRelay) << "StatisticRelayManager stopped";
}

void StatisticRelayManager::updateStatistics()
{
    if (!m_isRunning || !m_streamManager) {
        return;
    }
    
    // Process Shoutcast relays
    for (auto it = m_shoutcastRelays.begin(); it != m_shoutcastRelays.end(); ++it) {
        if (it.value().enabled) {
            processShoutcastRelay(it.key(), it.value());
        }
    }
    
    // Process Icecast relays
    for (auto it = m_icecastRelays.begin(); it != m_icecastRelays.end(); ++it) {
        if (it.value().enabled) {
            processIcecastRelay(it.key(), it.value());
        }
    }
}

void StatisticRelayManager::processShoutcastRelay(const QString& name, const ShoutcastRelayConfig& config)
{
    if (config.mountPoint.isEmpty()) {
        qCWarning(statisticRelay) << "No mount point specified for Shoutcast relay:" << name;
        return;
    }
    
    RelayStatistics stats = collectMountPointStatistics(config.mountPoint);
    stats.protocol = "shoutcast";
    m_relayStatistics[name] = stats;
    
    sendShoutcastStatistics(name, config, stats);
}

void StatisticRelayManager::processIcecastRelay(const QString& name, const IcecastRelayConfig& config)
{
    if (config.mountPoint.isEmpty()) {
        qCWarning(statisticRelay) << "No mount point specified for Icecast relay:" << name;
        return;
    }
    
    RelayStatistics stats = collectMountPointStatistics(config.mountPoint);
    stats.protocol = "icecast";
    m_relayStatistics[name] = stats;
    
    sendIcecastStatistics(name, config, stats);
}

void StatisticRelayManager::sendShoutcastStatistics(const QString& name, const ShoutcastRelayConfig& config, const RelayStatistics& stats)
{
    QString url = buildShoutcastUrl(config);
    QString requestData = formatShoutcastRequest(config, stats);
    
    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, "LegacyStream/1.0");
    
    QNetworkReply* reply = m_networkManager->post(request, requestData.toUtf8());
    m_activeReplies[reply] = name;
    
    connect(reply, &QNetworkReply::errorOccurred,
            this, &StatisticRelayManager::onNetworkError);
    
    qCDebug(statisticRelay) << "Sending Shoutcast statistics to" << name << "at" << url;
}

void StatisticRelayManager::sendIcecastStatistics(const QString& name, const IcecastRelayConfig& config, const RelayStatistics& stats)
{
    QString url = buildIcecastUrl(config);
    QString requestData = formatIcecastRequest(config, stats);
    
    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, "LegacyStream/1.0");
    
    QNetworkReply* reply = m_networkManager->post(request, requestData.toUtf8());
    m_activeReplies[reply] = name;
    
    connect(reply, &QNetworkReply::errorOccurred,
            this, &StatisticRelayManager::onNetworkError);
    
    qCDebug(statisticRelay) << "Sending Icecast statistics to" << name << "at" << url;
}

RelayStatistics StatisticRelayManager::collectMountPointStatistics(const QString& mountPoint) const
{
    RelayStatistics stats;
    stats.mountPoint = mountPoint;
    stats.lastUpdate = QDateTime::currentDateTime();
    
    // TODO: Get actual statistics from StreamManager
    // This is a placeholder implementation
    stats.currentListeners = 0;
    stats.peakListeners = 0;
    stats.bytesServed = 0;
    stats.uptime = 0;
    stats.currentSong = "Unknown";
    stats.currentArtist = "Unknown";
    stats.currentAlbum = "";
    stats.currentGenre = "";
    stats.codec = "mp3";
    stats.bitrate = "128";
    stats.sampleRate = "44100";
    stats.channels = "2";
    stats.isLive = false;
    
    return stats;
}

QString StatisticRelayManager::formatShoutcastRequest(const ShoutcastRelayConfig& config, const RelayStatistics& stats) const
{
    QUrlQuery query;
    
    // Get configuration for server details
    auto& configInstance = Configuration::instance();
    QString serverHostname = configInstance.serverHostname();
    if (serverHostname.isEmpty()) {
        serverHostname = "localhost";
    }
    
    // Basic authentication - use relay password if available, otherwise use config password
    QString password = config.password;
    if (password.isEmpty()) {
        password = configInstance.relayPassword();
    }
    if (!password.isEmpty()) {
        query.addQueryItem("pass", password);
    }
    
    // Shoutcast v2 SID
    if (!config.sid.isEmpty()) {
        query.addQueryItem("sid", config.sid);
    }
    
    // Build server URL using configured hostname
    QString serverUrl = QString("http://%1").arg(serverHostname);
    
    // Statistics data
    query.addQueryItem("mode", "updinfo");
    query.addQueryItem("song", escapeUrl(stats.currentSong));
    query.addQueryItem("url", escapeUrl(serverUrl));
    query.addQueryItem("irc", escapeUrl(""));
    query.addQueryItem("icq", escapeUrl(""));
    query.addQueryItem("aim", escapeUrl(""));
    query.addQueryItem("genre", escapeUrl(stats.currentGenre));
    query.addQueryItem("desc", escapeUrl("LegacyStream Relay"));
    query.addQueryItem("name", escapeUrl("LegacyStream"));
    query.addQueryItem("public", "1");
    query.addQueryItem("listeners", QString::number(stats.currentListeners));
    query.addQueryItem("maxlisteners", QString::number(stats.peakListeners));
    query.addQueryItem("bitrate", stats.bitrate);
    query.addQueryItem("samplerate", stats.sampleRate);
    query.addQueryItem("channels", stats.channels);
    query.addQueryItem("servergenre", escapeUrl(stats.currentGenre));
    query.addQueryItem("serverurl", escapeUrl(serverUrl));
    query.addQueryItem("servername", escapeUrl("LegacyStream"));
    query.addQueryItem("serverdesc", escapeUrl("LegacyStream Audio Server"));
    query.addQueryItem("servertype", "audio/mpeg");
    query.addQueryItem("streamurl", escapeUrl(serverUrl));
    query.addQueryItem("streamid", "1");
    query.addQueryItem("streamtitle", escapeUrl(stats.currentSong));
    query.addQueryItem("streamartist", escapeUrl(stats.currentArtist));
    query.addQueryItem("streamalbum", escapeUrl(stats.currentAlbum));
    query.addQueryItem("streamgenre", escapeUrl(stats.currentGenre));
    query.addQueryItem("streamurl", escapeUrl(serverUrl));
    query.addQueryItem("streamirc", escapeUrl(""));
    query.addQueryItem("streamicq", escapeUrl(""));
    query.addQueryItem("streamaim", escapeUrl(""));
    query.addQueryItem("streamdesc", escapeUrl("LegacyStream Relay"));
    query.addQueryItem("streampublic", "1");
    query.addQueryItem("streamlisteners", QString::number(stats.currentListeners));
    query.addQueryItem("streammaxlisteners", QString::number(stats.peakListeners));
    query.addQueryItem("streambitrate", stats.bitrate);
    query.addQueryItem("streamsamplerate", stats.sampleRate);
    query.addQueryItem("streamchannels", stats.channels);
    query.addQueryItem("streamservergenre", escapeUrl(stats.currentGenre));
    query.addQueryItem("streamserverurl", escapeUrl(serverUrl));
    query.addQueryItem("streamservername", escapeUrl("LegacyStream"));
    query.addQueryItem("streamserverdesc", escapeUrl("LegacyStream Audio Server"));
    query.addQueryItem("streamservertype", "audio/mpeg");
    query.addQueryItem("streamstreamurl", escapeUrl(serverUrl));
    query.addQueryItem("streamstreamid", "1");
    query.addQueryItem("streamstreamtitle", escapeUrl(stats.currentSong));
    query.addQueryItem("streamstreamartist", escapeUrl(stats.currentArtist));
    query.addQueryItem("streamstreamalbum", escapeUrl(stats.currentAlbum));
    query.addQueryItem("streamstreamgenre", escapeUrl(stats.currentGenre));
    query.addQueryItem("streamstreamurl", escapeUrl(serverUrl));
    query.addQueryItem("streamstreamirc", escapeUrl(""));
    query.addQueryItem("streamstreamicq", escapeUrl(""));
    query.addQueryItem("streamstreamaim", escapeUrl(""));
    query.addQueryItem("streamstreamdesc", escapeUrl("LegacyStream Relay"));
    query.addQueryItem("streamstreampublic", "1");
    query.addQueryItem("streamstreamlisteners", QString::number(stats.currentListeners));
    query.addQueryItem("streamstreammaxlisteners", QString::number(stats.peakListeners));
    query.addQueryItem("streamstreambitrate", stats.bitrate);
    query.addQueryItem("streamstreamsamplerate", stats.sampleRate);
    query.addQueryItem("streamstreamchannels", stats.channels);
    query.addQueryItem("streamstreamservergenre", escapeUrl(stats.currentGenre));
    query.addQueryItem("streamstreamserverurl", escapeUrl(serverUrl));
    query.addQueryItem("streamstreamservername", escapeUrl("LegacyStream"));
    query.addQueryItem("streamstreamserverdesc", escapeUrl("LegacyStream Audio Server"));
    query.addQueryItem("streamstreamservertype", "audio/mpeg");
    query.addQueryItem("streamstreamstreamurl", escapeUrl(serverUrl));
    query.addQueryItem("streamstreamstreamid", "1");
    
    return query.toString(QUrl::FullyEncoded);
}

QString StatisticRelayManager::formatIcecastRequest(const IcecastRelayConfig& config, const RelayStatistics& stats) const
{
    QUrlQuery query;
    
    // Get configuration for authentication and server details
    auto& configInstance = Configuration::instance();
    QString serverHostname = configInstance.serverHostname();
    if (serverHostname.isEmpty()) {
        serverHostname = "localhost";
    }
    
    // Authentication - use admin username/password from configuration if not provided in config
    QString username = config.username;
    QString password = config.password;
    
    if (username.isEmpty()) {
        username = configInstance.adminUsername();
    }
    if (password.isEmpty()) {
        password = configInstance.adminPassword();
    }
    
    if (!username.isEmpty()) {
        query.addQueryItem("username", username);
    }
    if (!password.isEmpty()) {
        query.addQueryItem("password", password);
    }
    
    // Build server URL using configured hostname
    QString serverUrl = QString("http://%1").arg(serverHostname);
    
    // Statistics data
    query.addQueryItem("mount", config.alias);
    query.addQueryItem("listeners", QString::number(stats.currentListeners));
    query.addQueryItem("peak_listeners", QString::number(stats.peakListeners));
    query.addQueryItem("bytes_served", QString::number(stats.bytesServed));
    query.addQueryItem("uptime", QString::number(stats.uptime));
    query.addQueryItem("current_song", escapeUrl(stats.currentSong));
    query.addQueryItem("current_artist", escapeUrl(stats.currentArtist));
    query.addQueryItem("current_album", escapeUrl(stats.currentAlbum));
    query.addQueryItem("current_genre", escapeUrl(stats.currentGenre));
    query.addQueryItem("codec", stats.codec);
    query.addQueryItem("bitrate", stats.bitrate);
    query.addQueryItem("samplerate", stats.sampleRate);
    query.addQueryItem("channels", stats.channels);
    query.addQueryItem("is_live", stats.isLive ? "1" : "0");
    query.addQueryItem("server_name", escapeUrl("LegacyStream"));
    query.addQueryItem("server_description", escapeUrl("LegacyStream Audio Server"));
    query.addQueryItem("server_url", escapeUrl(serverUrl));
    query.addQueryItem("server_genre", escapeUrl(stats.currentGenre));
    query.addQueryItem("server_type", "audio/mpeg");
    query.addQueryItem("server_public", "1");
    query.addQueryItem("server_bitrate", stats.bitrate);
    query.addQueryItem("server_samplerate", stats.sampleRate);
    query.addQueryItem("server_channels", stats.channels);
    
    return query.toString(QUrl::FullyEncoded);
}

QString StatisticRelayManager::buildShoutcastUrl(const ShoutcastRelayConfig& config) const
{
    QString scheme = (config.port == 443) ? "https" : "http";
    return QString("%1://%2:%3/admin.cgi").arg(scheme, config.host, QString::number(config.port));
}

QString StatisticRelayManager::buildIcecastUrl(const IcecastRelayConfig& config) const
{
    QString scheme = (config.port == 443) ? "https" : "http";
    return QString("%1://%2:%3/admin/stats.xml").arg(scheme, config.host, QString::number(config.port));
}

QString StatisticRelayManager::escapeUrl(const QString& text) const
{
    return QUrl::toPercentEncoding(text).constData();
}

QString StatisticRelayManager::formatBytes(qint64 bytes) const
{
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;
    
    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(QString::number(size, 'f', 2), units[unitIndex]);
}

QString StatisticRelayManager::formatDuration(qint64 seconds) const
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'));
}

void StatisticRelayManager::onShoutcastRelayFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString relayName = m_activeReplies.take(reply);
    if (relayName.isEmpty()) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        m_relayConnectionStatus[relayName] = true;
        emit relayConnected(relayName, "shoutcast");
        emit statisticsRelayed(relayName, "shoutcast");
        qCDebug(statisticRelay) << "Shoutcast relay" << relayName << "successful";
    } else {
        m_relayConnectionStatus[relayName] = false;
        emit relayError(relayName, reply->errorString());
        qCWarning(statisticRelay) << "Shoutcast relay" << relayName << "error:" << reply->errorString();
    }
    
    reply->deleteLater();
}

void StatisticRelayManager::onIcecastRelayFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString relayName = m_activeReplies.take(reply);
    if (relayName.isEmpty()) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        m_relayConnectionStatus[relayName] = true;
        emit relayConnected(relayName, "icecast");
        emit statisticsRelayed(relayName, "icecast");
        qCDebug(statisticRelay) << "Icecast relay" << relayName << "successful";
    } else {
        m_relayConnectionStatus[relayName] = false;
        emit relayError(relayName, reply->errorString());
        qCWarning(statisticRelay) << "Icecast relay" << relayName << "error:" << reply->errorString();
    }
    
    reply->deleteLater();
}

void StatisticRelayManager::onNetworkError(QNetworkReply::NetworkError error)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString relayName = m_activeReplies.value(reply);
    if (!relayName.isEmpty()) {
        m_relayConnectionStatus[relayName] = false;
        emit relayError(relayName, reply->errorString());
        qCWarning(statisticRelay) << "Network error for relay" << relayName << ":" << reply->errorString();
    }
}

void StatisticRelayManager::enableRealTimeCollection(bool enabled)
{
    m_realTimeCollectionEnabled = enabled;
    
    if (enabled) {
        m_realTimeCollectionTimer->start();
        qCInfo(statisticRelay) << "Real-time statistics collection enabled";
    } else {
        m_realTimeCollectionTimer->stop();
        qCInfo(statisticRelay) << "Real-time statistics collection disabled";
    }
}

void StatisticRelayManager::setRealTimeUpdateInterval(int seconds)
{
    m_realTimeUpdateInterval = seconds;
    m_realTimeCollectionTimer->setInterval(seconds * 1000);
    qCInfo(statisticRelay) << "Real-time update interval set to" << seconds << "seconds";
}

QJsonObject StatisticRelayManager::getRealTimeStatistics() const
{
    return m_realTimeStatistics;
}

void StatisticRelayManager::broadcastStatistics(const QJsonObject& statistics)
{
    m_realTimeStatistics = statistics;
    broadcastToWebSocketClients(statistics, "statistics");
    emit realTimeStatisticsUpdated(statistics);
}

void StatisticRelayManager::startWebSocketServer(int port)
{
    if (m_webSocketServer->isListening()) {
        qCWarning(statisticRelay) << "WebSocket server already running";
        return;
    }
    
    m_webSocketPort = port;
    
    if (m_webSocketServer->listen(QHostAddress::Any, port)) {
        qCInfo(statisticRelay) << "WebSocket server started on port" << port;
    } else {
        qCWarning(statisticRelay) << "Failed to start WebSocket server on port" << port;
    }
}

void StatisticRelayManager::stopWebSocketServer()
{
    if (m_webSocketServer->isListening()) {
        m_webSocketServer->close();
        
        // Close all client connections
        for (auto it = m_webSocketClients.begin(); it != m_webSocketClients.end(); ++it) {
            it->socket->close();
        }
        m_webSocketClients.clear();
        
        qCInfo(statisticRelay) << "WebSocket server stopped";
    }
}

bool StatisticRelayManager::isWebSocketServerRunning() const
{
    return m_webSocketServer->isListening();
}

int StatisticRelayManager::getWebSocketClientCount() const
{
    return m_webSocketClients.size();
}

void StatisticRelayManager::broadcastToWebSocketClients(const QJsonObject& data, const QString& topic)
{
    if (m_webSocketClients.isEmpty()) {
        return;
    }
    
    QJsonObject message;
    message["type"] = "broadcast";
    message["topic"] = topic;
    message["data"] = data;
    message["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(message);
    QString jsonMessage = doc.toJson(QJsonDocument::Compact);
    
    for (auto it = m_webSocketClients.begin(); it != m_webSocketClients.end(); ++it) {
        WebSocketClient& client = it.value();
        
        // Check if client is subscribed to the topic
        if (topic.isEmpty() || client.subscribedTopics.contains(topic) || client.subscribedTopics.contains("*")) {
            client.socket->sendTextMessage(jsonMessage);
        }
    }
}

void StatisticRelayManager::onRealTimeCollectionTimer()
{
    if (!m_isRunning || !m_realTimeCollectionEnabled) {
        return;
    }
    
    collectRealTimeStatistics();
}

void StatisticRelayManager::onWebSocketNewConnection()
{
    QWebSocket* socket = m_webSocketServer->nextPendingConnection();
    
    if (!socket) {
        return;
    }
    
    // Create client info
    WebSocketClient client;
    client.socket = socket;
    client.id = generateClientId();
    client.connectedAt = QDateTime::currentDateTime();
    client.subscribedTopics << "*"; // Subscribe to all topics by default
    
    m_webSocketClients[socket] = client;
    
    // Connect signals
    connect(socket, &QWebSocket::disconnected, this, &StatisticRelayManager::onWebSocketClientDisconnected);
    connect(socket, &QWebSocket::textMessageReceived, this, &StatisticRelayManager::onWebSocketTextMessageReceived);
    
    // Send welcome message
    QJsonObject welcomeMessage;
    welcomeMessage["type"] = "welcome";
    welcomeMessage["client_id"] = client.id;
    welcomeMessage["server_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    welcomeMessage["available_topics"] = QJsonArray{"statistics", "performance", "alerts", "streams"};
    
    QJsonDocument doc(welcomeMessage);
    socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    
    qCInfo(statisticRelay) << "WebSocket client connected:" << client.id;
    emit webSocketClientConnected(client.id);
}

void StatisticRelayManager::onWebSocketClientDisconnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) {
        return;
    }
    
    auto it = m_webSocketClients.find(socket);
    if (it != m_webSocketClients.end()) {
        QString clientId = it->id;
        m_webSocketClients.remove(socket);
        
        qCInfo(statisticRelay) << "WebSocket client disconnected:" << clientId;
        emit webSocketClientDisconnected(clientId);
    }
    
    socket->deleteLater();
}

void StatisticRelayManager::onWebSocketTextMessageReceived(const QString& message)
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) {
        return;
    }
    
    auto it = m_webSocketClients.find(socket);
    if (it == m_webSocketClients.end()) {
        return;
    }
    
    WebSocketClient* client = &it.value();
    
    // Parse JSON message
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qCWarning(statisticRelay) << "Invalid JSON from WebSocket client:" << error.errorString();
        return;
    }
    
    QJsonObject jsonMessage = doc.object();
    processWebSocketMessage(jsonMessage, client);
}

void StatisticRelayManager::collectRealTimeStatistics()
{
    if (!m_streamManager) {
        return;
    }
    
    QJsonObject statistics = buildRealTimeStatisticsJson();
    broadcastStatistics(statistics);
}

QJsonObject StatisticRelayManager::buildRealTimeStatisticsJson() const
{
    QJsonObject statistics;
    
    // System information
    statistics["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    statistics["uptime"] = m_streamManager ? m_streamManager->isRunning() : false;
    
    // Stream statistics
    if (m_streamManager) {
        QList<StreamInfo> streams = m_streamManager->getStreams();
        QJsonArray streamsArray;
        
        for (const StreamInfo& stream : streams) {
            QJsonObject streamObj;
            streamObj["mount_point"] = stream.mountPoint;
            streamObj["codec"] = stream.codec;
            streamObj["bitrate"] = stream.bitrate;
            streamObj["listeners"] = stream.listeners;
            streamObj["active"] = stream.active;
            streamObj["bytes_sent"] = stream.bytesSent;
            streamObj["start_time"] = stream.startTime.toString(Qt::ISODate);
            streamObj["metadata"] = stream.metadata;
            
            streamsArray.append(streamObj);
        }
        
        statistics["streams"] = streamsArray;
        statistics["total_streams"] = streams.size();
        statistics["active_streams"] = m_streamManager->getActiveStreams();
        statistics["total_listeners"] = m_streamManager->getTotalListeners();
        statistics["total_bytes_sent"] = m_streamManager->getTotalBytesSent();
    }
    
    // Relay statistics
    QJsonArray relayArray;
    for (auto it = m_relayStatistics.begin(); it != m_relayStatistics.end(); ++it) {
        const RelayStatistics& relay = it.value();
        QJsonObject relayObj;
        relayObj["name"] = it.key();
        relayObj["mount_point"] = relay.mountPoint;
        relayObj["protocol"] = relay.protocol;
        relayObj["current_listeners"] = relay.currentListeners;
        relayObj["peak_listeners"] = relay.peakListeners;
        relayObj["bytes_served"] = relay.bytesServed;
        relayObj["uptime"] = relay.uptime;
        relayObj["is_live"] = relay.isLive;
        relayObj["last_update"] = relay.lastUpdate.toString(Qt::ISODate);
        
        relayArray.append(relayObj);
    }
    
    statistics["relays"] = relayArray;
    statistics["total_relays"] = m_relayStatistics.size();
    
    // WebSocket client information
    statistics["websocket_clients"] = m_webSocketClients.size();
    statistics["websocket_server_running"] = m_webSocketServer->isListening();
    statistics["websocket_port"] = m_webSocketPort;
    
    return statistics;
}

void StatisticRelayManager::processWebSocketMessage(const QJsonObject& message, WebSocketClient* client)
{
    QString type = message["type"].toString();
    
    if (type == "subscribe") {
        QStringList topics = message["topics"].toVariant().toStringList();
        client->subscribedTopics = topics;
        
        QJsonObject response;
        response["type"] = "subscribed";
        response["topics"] = QJsonArray::fromStringList(topics);
        
        QJsonDocument doc(response);
        client->socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
        
        qCInfo(statisticRelay) << "Client" << client->id << "subscribed to topics:" << topics;
        
    } else if (type == "unsubscribe") {
        QStringList topics = message["topics"].toVariant().toStringList();
        for (const QString& topic : topics) {
            client->subscribedTopics.removeAll(topic);
        }
        
        QJsonObject response;
        response["type"] = "unsubscribed";
        response["topics"] = QJsonArray::fromStringList(topics);
        
        QJsonDocument doc(response);
        client->socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
        
        qCInfo(statisticRelay) << "Client" << client->id << "unsubscribed from topics:" << topics;
        
    } else if (type == "get_statistics") {
        QJsonObject statistics = buildRealTimeStatisticsJson();
        
        QJsonObject response;
        response["type"] = "statistics";
        response["data"] = statistics;
        
        QJsonDocument doc(response);
        client->socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
        
    } else if (type == "ping") {
        QJsonObject response;
        response["type"] = "pong";
        response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        QJsonDocument doc(response);
        client->socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
        
    } else {
        qCWarning(statisticRelay) << "Unknown WebSocket message type:" << type;
    }
}

QString StatisticRelayManager::generateClientId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace StatisticRelay
} // namespace LegacyStream 