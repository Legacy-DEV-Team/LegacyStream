#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDir>
#include <QStandardPaths>
#include <QVariant>

Q_LOGGING_CATEGORY(configuration, "configuration")

namespace LegacyStream {

Configuration& Configuration::instance()
{
    static Configuration instance;
    return instance;
}

Configuration::Configuration()
    : QObject(nullptr)
{
    qCDebug(configuration) << "Configuration singleton created";
}

Configuration::~Configuration()
{
    qCDebug(configuration) << "Configuration singleton destroyed";
}

void Configuration::initialize(const QString& configPath)
{
    m_settings = std::make_unique<QSettings>(configPath, QSettings::IniFormat);
    setDefaultValues();
    load();
    validateSettings();
    
    qDebug() << "Configuration initialized from" << configPath;
}

void Configuration::save()
{
    if (!m_settings) return;
    
    // Server settings
    m_settings->setValue("server/httpPort", m_httpPort);
    m_settings->setValue("server/httpsPort", m_httpsPort);
    m_settings->setValue("server/bindAddress", m_bindAddress.toString());
    m_settings->setValue("server/maxConnections", m_maxConnections);
    m_settings->setValue("server/maxStreams", m_maxStreams);
    
    // Stream settings
    m_settings->setValue("stream/defaultLatency", m_defaultLatency);
    m_settings->setValue("stream/maxLatency", m_maxLatency);
    m_settings->setValue("stream/minLatency", m_minLatency);
    m_settings->setValue("stream/bufferSize", m_bufferSize);
    
    // SSL settings
    m_settings->setValue("ssl/enabled", m_sslEnabled);
    m_settings->setValue("ssl/certificatePath", m_certificatePath);
    m_settings->setValue("ssl/privateKeyPath", m_privateKeyPath);
    m_settings->setValue("ssl/certificatePassword", m_certificatePassword);
    m_settings->setValue("ssl/autoRenewCertificates", m_autoRenewCertificates);
    
    // Let's Encrypt
    m_settings->setValue("letsencrypt/enabled", m_letsEncryptEnabled);
    m_settings->setValue("letsencrypt/email", m_letsEncryptEmail);
    m_settings->setValue("letsencrypt/domains", m_letsEncryptDomains);
    m_settings->setValue("letsencrypt/staging", m_letsEncryptStaging);
    
    // Cloudflare
    m_settings->setValue("cloudflare/enabled", m_cloudflareEnabled);
    m_settings->setValue("cloudflare/apiToken", m_cloudflareApiToken);
    m_settings->setValue("cloudflare/zoneId", m_cloudflareZoneId);
    
    // Protocols
    m_settings->setValue("protocols/icecast", m_iceCastEnabled);
    m_settings->setValue("protocols/shoutcast", m_shoutCastEnabled);
    m_settings->setValue("protocols/hls", m_hlsEnabled);
    m_settings->setValue("protocols/hlsSegmentDuration", m_hlsSegmentDuration);
    m_settings->setValue("protocols/hlsPlaylistSize", m_hlsPlaylistSize);
    
    // Codecs
    m_settings->setValue("codecs/enabled", m_enabledCodecs);
    m_settings->setValue("codecs/mp3Quality", m_mp3Quality);
    m_settings->setValue("codecs/aacBitrate", m_aacBitrate);
    m_settings->setValue("codecs/oggQuality", m_oggQuality);
    
    // Relay
    m_settings->setValue("relay/enabled", m_relayEnabled);
    m_settings->setValue("relay/maxRelays", m_maxRelays);
    m_settings->setValue("relay/reconnectInterval", m_relayReconnectInterval);
    
    // Statistic relay
    m_settings->setValue("statisticRelay/enabled", m_statisticRelayEnabled);
    m_settings->setValue("statisticRelay/updateInterval", m_statisticRelayUpdateInterval);
    m_settings->setValue("statisticRelay/maxRelays", m_maxStatisticRelays);
    
    // Server authentication and location settings
    m_settings->setValue("server/sourcePassword", m_sourcePassword);
    m_settings->setValue("server/relayPassword", m_relayPassword);
    m_settings->setValue("server/adminUsername", m_adminUsername);
    m_settings->setValue("server/adminPassword", m_adminPassword);
    m_settings->setValue("server/location", m_serverLocation);
    m_settings->setValue("server/hostname", m_serverHostname);
    
    // Fallback settings
    m_settings->setValue("fallback/enabled", m_fallbackEnabled);
    m_settings->setValue("fallback/file", m_fallbackFile);
    m_settings->setValue("fallback/emergencyFile", m_emergencyFile);
    
    // Logging settings
    m_settings->setValue("logging/level", m_logLevel);
    m_settings->setValue("logging/maxSize", m_maxLogSize);
    m_settings->setValue("logging/retention", m_logRetention);
    
    // Performance settings
    m_settings->setValue("performance/ioThreads", m_ioThreads);
    m_settings->setValue("performance/workerThreads", m_workerThreads);
    m_settings->setValue("performance/enableCompression", m_enableCompression);
    
    // GUI settings
    m_settings->setValue("gui/minimizeToTray", m_minimizeToTray);
    m_settings->setValue("gui/startMinimized", m_startMinimized);
    m_settings->setValue("gui/theme", m_theme);
    
    // Mount points
    m_settings->beginGroup("mountPoints");
    m_settings->remove(""); // Clear existing mount points
    
    for (auto it = m_mountPoints.begin(); it != m_mountPoints.end(); ++it) {
        const QString& mountPoint = it.key();
        const QMap<QString, QVariant>& settings = it.value();
        
        m_settings->beginGroup(mountPoint);
        for (auto settingIt = settings.begin(); settingIt != settings.end(); ++settingIt) {
            m_settings->setValue(settingIt.key(), settingIt.value());
        }
        m_settings->endGroup();
    }
    m_settings->endGroup();
    
    m_settings->sync();
    qCDebug(configuration) << "Configuration saved";
}

void Configuration::saveToFile(const QString& filePath)
{
    // Create a temporary QSettings object for the specified file
    QSettings tempSettings(filePath, QSettings::IniFormat);
    
    // Server settings
    tempSettings.setValue("server/httpPort", m_httpPort);
    tempSettings.setValue("server/httpsPort", m_httpsPort);
    tempSettings.setValue("server/bindAddress", m_bindAddress.toString());
    tempSettings.setValue("server/maxConnections", m_maxConnections);
    tempSettings.setValue("server/maxStreams", m_maxStreams);
    
    // Stream settings
    tempSettings.setValue("stream/defaultLatency", m_defaultLatency);
    tempSettings.setValue("stream/maxLatency", m_maxLatency);
    tempSettings.setValue("stream/minLatency", m_minLatency);
    tempSettings.setValue("stream/bufferSize", m_bufferSize);
    
    // SSL settings
    tempSettings.setValue("ssl/enabled", m_sslEnabled);
    tempSettings.setValue("ssl/certificatePath", m_certificatePath);
    tempSettings.setValue("ssl/privateKeyPath", m_privateKeyPath);
    tempSettings.setValue("ssl/certificatePassword", m_certificatePassword);
    tempSettings.setValue("ssl/autoRenewCertificates", m_autoRenewCertificates);
    
    // Let's Encrypt
    tempSettings.setValue("letsencrypt/enabled", m_letsEncryptEnabled);
    tempSettings.setValue("letsencrypt/email", m_letsEncryptEmail);
    tempSettings.setValue("letsencrypt/domains", m_letsEncryptDomains);
    tempSettings.setValue("letsencrypt/staging", m_letsEncryptStaging);
    
    // Cloudflare
    tempSettings.setValue("cloudflare/enabled", m_cloudflareEnabled);
    tempSettings.setValue("cloudflare/apiToken", m_cloudflareApiToken);
    tempSettings.setValue("cloudflare/zoneId", m_cloudflareZoneId);
    
    // Protocols
    tempSettings.setValue("protocols/icecast", m_iceCastEnabled);
    tempSettings.setValue("protocols/shoutcast", m_shoutCastEnabled);
    tempSettings.setValue("protocols/hls", m_hlsEnabled);
    tempSettings.setValue("protocols/hlsSegmentDuration", m_hlsSegmentDuration);
    tempSettings.setValue("protocols/hlsPlaylistSize", m_hlsPlaylistSize);
    
    // Codecs
    tempSettings.setValue("codecs/enabled", m_enabledCodecs);
    tempSettings.setValue("codecs/mp3Quality", m_mp3Quality);
    tempSettings.setValue("codecs/aacBitrate", m_aacBitrate);
    tempSettings.setValue("codecs/oggQuality", m_oggQuality);
    
    // Relay
    tempSettings.setValue("relay/enabled", m_relayEnabled);
    tempSettings.setValue("relay/maxRelays", m_maxRelays);
    tempSettings.setValue("relay/reconnectInterval", m_relayReconnectInterval);
    
    // Statistic relay
    tempSettings.setValue("statisticRelay/enabled", m_statisticRelayEnabled);
    tempSettings.setValue("statisticRelay/updateInterval", m_statisticRelayUpdateInterval);
    tempSettings.setValue("statisticRelay/maxRelays", m_maxStatisticRelays);
    
    // Server authentication and location settings
    tempSettings.setValue("server/sourcePassword", m_sourcePassword);
    tempSettings.setValue("server/relayPassword", m_relayPassword);
    tempSettings.setValue("server/adminUsername", m_adminUsername);
    tempSettings.setValue("server/adminPassword", m_adminPassword);
    tempSettings.setValue("server/location", m_serverLocation);
    tempSettings.setValue("server/hostname", m_serverHostname);
    
    // Fallback settings
    tempSettings.setValue("fallback/enabled", m_fallbackEnabled);
    tempSettings.setValue("fallback/file", m_fallbackFile);
    tempSettings.setValue("fallback/emergencyFile", m_emergencyFile);
    
    // Logging settings
    tempSettings.setValue("logging/level", m_logLevel);
    tempSettings.setValue("logging/maxSize", m_maxLogSize);
    tempSettings.setValue("logging/retention", m_logRetention);
    
    // Performance settings
    tempSettings.setValue("performance/ioThreads", m_ioThreads);
    tempSettings.setValue("performance/workerThreads", m_workerThreads);
    tempSettings.setValue("performance/enableCompression", m_enableCompression);
    
    // GUI settings
    tempSettings.setValue("gui/minimizeToTray", m_minimizeToTray);
    tempSettings.setValue("gui/startMinimized", m_startMinimized);
    tempSettings.setValue("gui/theme", m_theme);
    
    // Mount points
    tempSettings.beginGroup("mountPoints");
    tempSettings.remove(""); // Clear existing mount points
    
    for (auto it = m_mountPoints.begin(); it != m_mountPoints.end(); ++it) {
        const QString& mountPoint = it.key();
        const QMap<QString, QVariant>& settings = it.value();
        
        tempSettings.beginGroup(mountPoint);
        for (auto settingIt = settings.begin(); settingIt != settings.end(); ++settingIt) {
            tempSettings.setValue(settingIt.key(), settingIt.value());
        }
        tempSettings.endGroup();
    }
    tempSettings.endGroup();
    
    tempSettings.sync();
    qCDebug(configuration) << "Configuration saved to file:" << filePath;
}

void Configuration::load()
{
    if (!m_settings) return;
    
    // Server settings
    m_httpPort = m_settings->value("server/httpPort", m_httpPort).toInt();
    m_httpsPort = m_settings->value("server/httpsPort", m_httpsPort).toInt();
    m_bindAddress = QHostAddress(m_settings->value("server/bindAddress", m_bindAddress.toString()).toString());
    m_maxConnections = m_settings->value("server/maxConnections", m_maxConnections).toInt();
    m_maxStreams = m_settings->value("server/maxStreams", m_maxStreams).toInt();
    
    // Stream settings
    m_defaultLatency = m_settings->value("stream/defaultLatency", m_defaultLatency).toInt();
    m_maxLatency = m_settings->value("stream/maxLatency", m_maxLatency).toInt();
    m_minLatency = m_settings->value("stream/minLatency", m_minLatency).toInt();
    m_bufferSize = m_settings->value("stream/bufferSize", m_bufferSize).toInt();
    
    // SSL settings
    m_sslEnabled = m_settings->value("ssl/enabled", m_sslEnabled).toBool();
    m_certificatePath = m_settings->value("ssl/certificatePath", m_certificatePath).toString();
    m_privateKeyPath = m_settings->value("ssl/privateKeyPath", m_privateKeyPath).toString();
    m_certificatePassword = m_settings->value("ssl/certificatePassword", m_certificatePassword).toString();
    m_autoRenewCertificates = m_settings->value("ssl/autoRenewCertificates", m_autoRenewCertificates).toBool();
    
    // Let's Encrypt
    m_letsEncryptEnabled = m_settings->value("letsencrypt/enabled", m_letsEncryptEnabled).toBool();
    m_letsEncryptEmail = m_settings->value("letsencrypt/email", m_letsEncryptEmail).toString();
    m_letsEncryptDomains = m_settings->value("letsencrypt/domains", m_letsEncryptDomains).toStringList();
    m_letsEncryptStaging = m_settings->value("letsencrypt/staging", m_letsEncryptStaging).toBool();
    
    // Cloudflare
    m_cloudflareEnabled = m_settings->value("cloudflare/enabled", m_cloudflareEnabled).toBool();
    m_cloudflareApiToken = m_settings->value("cloudflare/apiToken", m_cloudflareApiToken).toString();
    m_cloudflareZoneId = m_settings->value("cloudflare/zoneId", m_cloudflareZoneId).toString();
    
    // Protocols
    m_iceCastEnabled = m_settings->value("protocols/icecast", m_iceCastEnabled).toBool();
    m_shoutCastEnabled = m_settings->value("protocols/shoutcast", m_shoutCastEnabled).toBool();
    m_hlsEnabled = m_settings->value("protocols/hls", m_hlsEnabled).toBool();
    m_hlsSegmentDuration = m_settings->value("protocols/hlsSegmentDuration", m_hlsSegmentDuration).toInt();
    m_hlsPlaylistSize = m_settings->value("protocols/hlsPlaylistSize", m_hlsPlaylistSize).toInt();
    
    // Codecs
    m_enabledCodecs = m_settings->value("codecs/enabled", m_enabledCodecs).toStringList();
    m_mp3Quality = m_settings->value("codecs/mp3Quality", m_mp3Quality).toInt();
    m_aacBitrate = m_settings->value("codecs/aacBitrate", m_aacBitrate).toInt();
    m_oggQuality = m_settings->value("codecs/oggQuality", m_oggQuality).toInt();
    
    // Relay
    m_relayEnabled = m_settings->value("relay/enabled", m_relayEnabled).toBool();
    m_maxRelays = m_settings->value("relay/maxRelays", m_maxRelays).toInt();
    m_relayReconnectInterval = m_settings->value("relay/reconnectInterval", m_relayReconnectInterval).toInt();
    
    // Statistic relay
    m_statisticRelayEnabled = m_settings->value("statisticRelay/enabled", m_statisticRelayEnabled).toBool();
    m_statisticRelayUpdateInterval = m_settings->value("statisticRelay/updateInterval", m_statisticRelayUpdateInterval).toInt();
    m_maxStatisticRelays = m_settings->value("statisticRelay/maxRelays", m_maxStatisticRelays).toInt();
    
    // Server authentication and location settings
    m_sourcePassword = m_settings->value("server/sourcePassword", m_sourcePassword).toString();
    m_relayPassword = m_settings->value("server/relayPassword", m_relayPassword).toString();
    m_adminUsername = m_settings->value("server/adminUsername", m_adminUsername).toString();
    m_adminPassword = m_settings->value("server/adminPassword", m_adminPassword).toString();
    m_serverLocation = m_settings->value("server/location", m_serverLocation).toString();
    m_serverHostname = m_settings->value("server/hostname", m_serverHostname).toString();
    
    // Fallback
    m_fallbackEnabled = m_settings->value("fallback/enabled", m_fallbackEnabled).toBool();
    m_fallbackFile = m_settings->value("fallback/file", m_fallbackFile).toString();
    m_emergencyFile = m_settings->value("fallback/emergencyFile", m_emergencyFile).toString();
    
    // Logging
    m_logLevel = m_settings->value("logging/level", m_logLevel).toString();
    m_maxLogSize = m_settings->value("logging/maxSize", m_maxLogSize).toInt();
    m_logRetention = m_settings->value("logging/retention", m_logRetention).toInt();
    
    // Performance
    m_ioThreads = m_settings->value("performance/ioThreads", m_ioThreads).toInt();
    m_workerThreads = m_settings->value("performance/workerThreads", m_workerThreads).toInt();
    m_enableCompression = m_settings->value("performance/enableCompression", m_enableCompression).toBool();
    
    // GUI
    m_minimizeToTray = m_settings->value("gui/minimizeToTray", m_minimizeToTray).toBool();
    m_startMinimized = m_settings->value("gui/startMinimized", m_startMinimized).toBool();
    m_theme = m_settings->value("gui/theme", m_theme).toString();
    
    // Mount points
    m_settings->beginGroup("mountPoints");
    const QStringList mountPointKeys = m_settings->childGroups();
    for (const QString& groupName : mountPointKeys) {
        m_settings->beginGroup(groupName);
        QMap<QString, QVariant> mountPointSettings;
        for (const QString& key : m_settings->childKeys()) {
            mountPointSettings[key] = m_settings->value(key);
        }
        m_mountPoints[groupName] = mountPointSettings;
        m_settings->endGroup();
    }
    m_settings->endGroup();
    
    qCDebug(configuration) << "Configuration loaded";
}

void Configuration::reset()
{
    setDefaultValues();
    save();
    qDebug() << "Configuration reset to defaults";
}

void Configuration::setDefaultValues()
{
    // Server settings
    m_httpPort = 8000;
    m_httpsPort = 8443;
    m_bindAddress = QHostAddress::Any;
    m_maxConnections = 100000;
    m_maxStreams = 1000;
    
    // Stream settings
    m_defaultLatency = 5;
    m_maxLatency = 60;
    m_minLatency = 1;
    m_bufferSize = 65536;
    
    // SSL settings
    m_sslEnabled = false;
    m_certificatePath.clear();
    m_privateKeyPath.clear();
    m_certificatePassword.clear();
    m_autoRenewCertificates = true;
    
    // Let's Encrypt
    m_letsEncryptEnabled = false;
    m_letsEncryptEmail.clear();
    m_letsEncryptDomains.clear();
    m_letsEncryptStaging = false;
    
    // Cloudflare
    m_cloudflareEnabled = false;
    m_cloudflareApiToken.clear();
    m_cloudflareZoneId.clear();
    
    // Protocols
    m_iceCastEnabled = true;
    m_shoutCastEnabled = true;
    m_hlsEnabled = true;
    m_hlsSegmentDuration = 6;
    m_hlsPlaylistSize = 6;
    
    // Codecs
    m_enabledCodecs = {"mp3", "aac", "aac+", "ogg", "opus", "flac"};
    m_mp3Quality = 128;
    m_aacBitrate = 128;
    m_oggQuality = 6;
    
    // Relay
    m_relayEnabled = true;
    m_maxRelays = 100;
    m_relayReconnectInterval = 5;
    
    // Statistic relay
    m_statisticRelayEnabled = true;
    m_statisticRelayUpdateInterval = 30;
    m_maxStatisticRelays = 50;
    
    // Server authentication and location settings
    m_sourcePassword.clear();
    m_relayPassword.clear();
    m_adminUsername.clear();
    m_adminPassword.clear();
    m_serverLocation.clear();
    m_serverHostname.clear();
    
    // Fallback
    m_fallbackEnabled = true;
    m_fallbackFile.clear();
    m_emergencyFile.clear();
    
    // Logging
    m_logLevel = "INFO";
    m_maxLogSize = 10;
    m_logRetention = 30;
    
    // Performance
    m_ioThreads = 4;
    m_workerThreads = 8;
    m_enableCompression = true;
    
    // GUI
    m_minimizeToTray = true;
    m_startMinimized = false;
    m_theme = "dark";
    
    // Mount points - add some default mount points
    m_mountPoints.clear();
    
    // Add default mount points
    addMountPoint("/live", "icecast");
    addMountPoint("/backup", "icecast");
    addMountPoint("/classic", "shoutcast");
    addMountPoint("/rock", "shoutcast");
    
    // Configure default mount point settings
    setMountPointName("/live", "Live Stream");
    setMountPointDescription("/live", "Main live broadcast stream");
    setMountPointCodec("/live", "mp3");
    setMountPointBitrate("/live", 128);
    setMountPointQuality("/live", "128k");
    setMountPointPublic("/live", true);
    setMountPointMaxListeners("/live", 1000);
    
    setMountPointName("/backup", "Backup Stream");
    setMountPointDescription("/backup", "Backup stream for redundancy");
    setMountPointCodec("/backup", "aac");
    setMountPointBitrate("/backup", 64);
    setMountPointQuality("/backup", "64k");
    setMountPointPublic("/backup", true);
    setMountPointMaxListeners("/backup", 500);
    
    setMountPointName("/classic", "Classic Hits");
    setMountPointDescription("/classic", "Classic music stream");
    setMountPointCodec("/classic", "mp3");
    setMountPointBitrate("/classic", 96);
    setMountPointQuality("/classic", "96k");
    setMountPointPublic("/classic", true);
    setMountPointMaxListeners("/classic", 750);
    
    setMountPointName("/rock", "Rock Station");
    setMountPointDescription("/rock", "Rock music stream");
    setMountPointCodec("/rock", "aac");
    setMountPointBitrate("/rock", 128);
    setMountPointQuality("/rock", "128k");
    setMountPointPublic("/rock", true);
    setMountPointMaxListeners("/rock", 800);
}

void Configuration::validateSettings()
{
    // Validate port ranges
    if (m_httpPort < 1 || m_httpPort > 65535) {
        qCWarning(configuration) << "Invalid HTTP port:" << m_httpPort << ", using default 8000";
        m_httpPort = 8000;
    }
    
    if (m_httpsPort < 1 || m_httpsPort > 65535) {
        qCWarning(configuration) << "Invalid HTTPS port:" << m_httpsPort << ", using default 8443";
        m_httpsPort = 8443;
    }
    
    // Validate thread counts
    if (m_ioThreads < 1) {
        qCWarning(configuration) << "Invalid IO threads:" << m_ioThreads << ", using default 4";
        m_ioThreads = 4;
    }
    
    if (m_workerThreads < 1) {
        qCWarning(configuration) << "Invalid worker threads:" << m_workerThreads << ", using default 8";
        m_workerThreads = 8;
    }
    
    // Validate latencies
    if (m_minLatency < 1) {
        qCWarning(configuration) << "Invalid min latency:" << m_minLatency << ", using default 1";
        m_minLatency = 1;
    }
    
    if (m_maxLatency < m_minLatency) {
        qCWarning(configuration) << "Max latency cannot be less than min latency, using min latency";
        m_maxLatency = m_minLatency;
    }
    
    if (m_defaultLatency < m_minLatency || m_defaultLatency > m_maxLatency) {
        qCWarning(configuration) << "Default latency out of range, using min latency";
        m_defaultLatency = m_minLatency;
    }
    
    qCDebug(configuration) << "Configuration validation completed";
}

// Server configuration setters
void Configuration::setHttpPort(int port)
{
    if (m_httpPort != port) {
        m_httpPort = port;
        emit httpPortChanged(port);
        emit configurationChanged();
    }
}

void Configuration::setHttpsPort(int port)
{
    if (m_httpsPort != port) {
        m_httpsPort = port;
        emit httpsPortChanged(port);
        emit configurationChanged();
    }
}

void Configuration::setBindAddress(const QHostAddress& address)
{
    if (m_bindAddress != address) {
        m_bindAddress = address;
        emit configurationChanged();
    }
}

void Configuration::setMaxConnections(int connections)
{
    if (m_maxConnections != connections) {
        m_maxConnections = connections;
        emit configurationChanged();
    }
}

void Configuration::setMaxStreams(int streams)
{
    if (m_maxStreams != streams) {
        m_maxStreams = streams;
        emit configurationChanged();
    }
}

// Stream configuration setters
void Configuration::setDefaultLatency(int latency)
{
    if (m_defaultLatency != latency) {
        m_defaultLatency = latency;
        emit configurationChanged();
    }
}

void Configuration::setMaxLatency(int latency)
{
    if (m_maxLatency != latency) {
        m_maxLatency = latency;
        emit configurationChanged();
    }
}

void Configuration::setMinLatency(int latency)
{
    if (m_minLatency != latency) {
        m_minLatency = latency;
        emit configurationChanged();
    }
}

void Configuration::setBufferSize(int size)
{
    if (m_bufferSize != size) {
        m_bufferSize = size;
        emit configurationChanged();
    }
}

// SSL/TLS configuration setters
void Configuration::setSslEnabled(bool enabled)
{
    if (m_sslEnabled != enabled) {
        m_sslEnabled = enabled;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setCertificatePath(const QString& path)
{
    if (m_certificatePath != path) {
        m_certificatePath = path;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setPrivateKeyPath(const QString& path)
{
    if (m_privateKeyPath != path) {
        m_privateKeyPath = path;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setCertificatePassword(const QString& password)
{
    if (m_certificatePassword != password) {
        m_certificatePassword = password;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setAutoRenewCertificates(bool autoRenew)
{
    if (m_autoRenewCertificates != autoRenew) {
        m_autoRenewCertificates = autoRenew;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

// Let's Encrypt configuration setters
void Configuration::setLetsEncryptEnabled(bool enabled)
{
    if (m_letsEncryptEnabled != enabled) {
        m_letsEncryptEnabled = enabled;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setLetsEncryptEmail(const QString& email)
{
    if (m_letsEncryptEmail != email) {
        m_letsEncryptEmail = email;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setLetsEncryptDomains(const QStringList& domains)
{
    if (m_letsEncryptDomains != domains) {
        m_letsEncryptDomains = domains;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setLetsEncryptStaging(bool staging)
{
    if (m_letsEncryptStaging != staging) {
        m_letsEncryptStaging = staging;
        emit sslConfigurationChanged();
        emit configurationChanged();
    }
}

// Cloudflare configuration setters
void Configuration::setCloudflareEnabled(bool enabled)
{
    if (m_cloudflareEnabled != enabled) {
        m_cloudflareEnabled = enabled;
        emit configurationChanged();
    }
}

void Configuration::setCloudflareApiToken(const QString& token)
{
    if (m_cloudflareApiToken != token) {
        m_cloudflareApiToken = token;
        emit configurationChanged();
    }
}

void Configuration::setCloudflareZoneId(const QString& zoneId)
{
    if (m_cloudflareZoneId != zoneId) {
        m_cloudflareZoneId = zoneId;
        emit configurationChanged();
    }
}

// Protocol configuration setters
void Configuration::setIceCastEnabled(bool enabled)
{
    if (m_iceCastEnabled != enabled) {
        m_iceCastEnabled = enabled;
        emit configurationChanged();
    }
}

void Configuration::setShoutCastEnabled(bool enabled)
{
    if (m_shoutCastEnabled != enabled) {
        m_shoutCastEnabled = enabled;
        emit configurationChanged();
    }
}

void Configuration::setHlsEnabled(bool enabled)
{
    if (m_hlsEnabled != enabled) {
        m_hlsEnabled = enabled;
        emit configurationChanged();
    }
}

void Configuration::setHlsSegmentDuration(int duration)
{
    if (m_hlsSegmentDuration != duration) {
        m_hlsSegmentDuration = duration;
        emit configurationChanged();
    }
}

void Configuration::setHlsPlaylistSize(int size)
{
    if (m_hlsPlaylistSize != size) {
        m_hlsPlaylistSize = size;
        emit configurationChanged();
    }
}

// Codec configuration setters
void Configuration::setEnabledCodecs(const QStringList& codecs)
{
    if (m_enabledCodecs != codecs) {
        m_enabledCodecs = codecs;
        emit codecConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setMp3Quality(int quality)
{
    if (m_mp3Quality != quality) {
        m_mp3Quality = quality;
        emit codecConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setAacBitrate(int bitrate)
{
    if (m_aacBitrate != bitrate) {
        m_aacBitrate = bitrate;
        emit codecConfigurationChanged();
        emit configurationChanged();
    }
}

void Configuration::setOggQuality(int quality)
{
    if (m_oggQuality != quality) {
        m_oggQuality = quality;
        emit codecConfigurationChanged();
        emit configurationChanged();
    }
}

// Relay configuration setters
void Configuration::setRelayEnabled(bool enabled)
{
    if (m_relayEnabled != enabled) {
        m_relayEnabled = enabled;
        emit configurationChanged();
    }
}

void Configuration::setMaxRelays(int relays)
{
    if (m_maxRelays != relays) {
        m_maxRelays = relays;
        emit configurationChanged();
    }
}

void Configuration::setRelayReconnectInterval(int interval)
{
    if (m_relayReconnectInterval != interval) {
        m_relayReconnectInterval = interval;
        emit configurationChanged();
    }
}

// Statistic relay configuration setters
void Configuration::setStatisticRelayEnabled(bool enabled)
{
    if (m_statisticRelayEnabled != enabled) {
        m_statisticRelayEnabled = enabled;
        emit configurationChanged();
    }
}

void Configuration::setStatisticRelayUpdateInterval(int interval)
{
    if (m_statisticRelayUpdateInterval != interval) {
        m_statisticRelayUpdateInterval = interval;
        emit configurationChanged();
    }
}

void Configuration::setMaxStatisticRelays(int relays)
{
    if (m_maxStatisticRelays != relays) {
        m_maxStatisticRelays = relays;
        emit configurationChanged();
    }
}

// Server authentication and location setters
void Configuration::setSourcePassword(const QString& password)
{
    if (m_sourcePassword != password) {
        m_sourcePassword = password;
        emit configurationChanged();
    }
}

void Configuration::setRelayPassword(const QString& password)
{
    if (m_relayPassword != password) {
        m_relayPassword = password;
        emit configurationChanged();
    }
}

void Configuration::setAdminUsername(const QString& username)
{
    if (m_adminUsername != username) {
        m_adminUsername = username;
        emit configurationChanged();
    }
}

void Configuration::setAdminPassword(const QString& password)
{
    if (m_adminPassword != password) {
        m_adminPassword = password;
        emit configurationChanged();
    }
}

void Configuration::setServerLocation(const QString& location)
{
    if (m_serverLocation != location) {
        m_serverLocation = location;
        emit configurationChanged();
    }
}

void Configuration::setServerHostname(const QString& hostname)
{
    if (m_serverHostname != hostname) {
        m_serverHostname = hostname;
        emit configurationChanged();
    }
}

// Fallback configuration setters
void Configuration::setFallbackEnabled(bool enabled)
{
    if (m_fallbackEnabled != enabled) {
        m_fallbackEnabled = enabled;
        emit configurationChanged();
    }
}

void Configuration::setFallbackFile(const QString& file)
{
    if (m_fallbackFile != file) {
        m_fallbackFile = file;
        emit configurationChanged();
    }
}

void Configuration::setEmergencyFile(const QString& file)
{
    if (m_emergencyFile != file) {
        m_emergencyFile = file;
        emit configurationChanged();
    }
}

// Logging configuration setters
void Configuration::setLogLevel(const QString& level)
{
    if (m_logLevel != level) {
        m_logLevel = level;
        emit configurationChanged();
    }
}

void Configuration::setMaxLogSize(int size)
{
    if (m_maxLogSize != size) {
        m_maxLogSize = size;
        emit configurationChanged();
    }
}

void Configuration::setLogRetention(int days)
{
    if (m_logRetention != days) {
        m_logRetention = days;
        emit configurationChanged();
    }
}

// Performance configuration setters
void Configuration::setIoThreads(int threads)
{
    if (m_ioThreads != threads) {
        m_ioThreads = threads;
        emit configurationChanged();
    }
}

void Configuration::setWorkerThreads(int threads)
{
    if (m_workerThreads != threads) {
        m_workerThreads = threads;
        emit configurationChanged();
    }
}

void Configuration::setEnableCompression(bool enabled)
{
    if (m_enableCompression != enabled) {
        m_enableCompression = enabled;
        emit configurationChanged();
    }
}

// GUI configuration setters
void Configuration::setMinimizeToTray(bool minimize)
{
    if (m_minimizeToTray != minimize) {
        m_minimizeToTray = minimize;
        emit configurationChanged();
    }
}

void Configuration::setStartMinimized(bool minimized)
{
    if (m_startMinimized != minimized) {
        m_startMinimized = minimized;
        emit configurationChanged();
    }
}

void Configuration::setTheme(const QString& theme)
{
    if (m_theme != theme) {
        m_theme = theme;
        emit configurationChanged();
    }
}

// Mount point configuration methods
void Configuration::addMountPoint(const QString& mountPoint, const QString& protocol)
{
    if (!m_mountPoints.contains(mountPoint)) {
        QMap<QString, QVariant> defaultSettings;
        defaultSettings["protocol"] = protocol;
        defaultSettings["name"] = mountPoint.mid(1); // Remove leading slash
        defaultSettings["description"] = "";
        defaultSettings["codec"] = "mp3";
        defaultSettings["bitrate"] = 128;
        defaultSettings["quality"] = "128k";
        defaultSettings["public"] = true;
        defaultSettings["maxListeners"] = 1000;
        defaultSettings["fallbackFile"] = "";
        defaultSettings["enabled"] = true;
        
        m_mountPoints[mountPoint] = defaultSettings;
        emit mountPointAdded(mountPoint);
        emit configurationChanged();
    }
}

void Configuration::removeMountPoint(const QString& mountPoint)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints.remove(mountPoint);
        emit mountPointRemoved(mountPoint);
        emit configurationChanged();
    }
}

QString Configuration::getMountPointProtocol(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("protocol", "icecast").toString();
}

void Configuration::setMountPointProtocol(const QString& mountPoint, const QString& protocol)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["protocol"] = protocol;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

QString Configuration::getMountPointName(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("name", mountPoint.mid(1)).toString();
}

void Configuration::setMountPointName(const QString& mountPoint, const QString& name)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["name"] = name;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

QString Configuration::getMountPointDescription(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("description", "").toString();
}

void Configuration::setMountPointDescription(const QString& mountPoint, const QString& description)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["description"] = description;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

QString Configuration::getMountPointCodec(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("codec", "mp3").toString();
}

void Configuration::setMountPointCodec(const QString& mountPoint, const QString& codec)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["codec"] = codec;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

int Configuration::getMountPointBitrate(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("bitrate", 128).toInt();
}

void Configuration::setMountPointBitrate(const QString& mountPoint, int bitrate)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["bitrate"] = bitrate;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

QString Configuration::getMountPointQuality(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("quality", "128k").toString();
}

void Configuration::setMountPointQuality(const QString& mountPoint, const QString& quality)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["quality"] = quality;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

bool Configuration::getMountPointPublic(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("public", true).toBool();
}

void Configuration::setMountPointPublic(const QString& mountPoint, bool isPublic)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["public"] = isPublic;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

int Configuration::getMountPointMaxListeners(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("maxListeners", 1000).toInt();
}

void Configuration::setMountPointMaxListeners(const QString& mountPoint, int maxListeners)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["maxListeners"] = maxListeners;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

QString Configuration::getMountPointFallbackFile(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("fallbackFile", "").toString();
}

void Configuration::setMountPointFallbackFile(const QString& mountPoint, const QString& file)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["fallbackFile"] = file;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

bool Configuration::getMountPointEnabled(const QString& mountPoint) const
{
    return m_mountPoints.value(mountPoint).value("enabled", true).toBool();
}

void Configuration::setMountPointEnabled(const QString& mountPoint, bool enabled)
{
    if (m_mountPoints.contains(mountPoint)) {
        m_mountPoints[mountPoint]["enabled"] = enabled;
        emit mountPointUpdated(mountPoint);
        emit configurationChanged();
    }
}

} // namespace LegacyStream 
