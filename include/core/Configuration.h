#pragma once

#include <QObject>
#include <QSettings>
#include <QHostAddress>
#include <QSslCertificate>
#include <QSslKey>
#include <memory>

namespace LegacyStream {

class Configuration : public QObject
{
    Q_OBJECT

public:
    static Configuration& instance();
    
    void initialize(const QString& configPath);
    void save();
    void load();
    void reset();
    
    // Server configuration
    int httpPort() const { return m_httpPort; }
    void setHttpPort(int port);
    
    int httpsPort() const { return m_httpsPort; }
    void setHttpsPort(int port);
    
    QHostAddress bindAddress() const { return m_bindAddress; }
    void setBindAddress(const QHostAddress& address);
    
    int maxConnections() const { return m_maxConnections; }
    void setMaxConnections(int connections);
    
    int maxStreams() const { return m_maxStreams; }
    void setMaxStreams(int streams);
    
    // Stream configuration
    int defaultLatency() const { return m_defaultLatency; }
    void setDefaultLatency(int latency);
    
    int maxLatency() const { return m_maxLatency; }
    void setMaxLatency(int latency);
    
    int minLatency() const { return m_minLatency; }
    void setMinLatency(int latency);
    
    int bufferSize() const { return m_bufferSize; }
    void setBufferSize(int size);
    
    // SSL/TLS configuration
    bool sslEnabled() const { return m_sslEnabled; }
    void setSslEnabled(bool enabled);
    
    QString certificatePath() const { return m_certificatePath; }
    void setCertificatePath(const QString& path);
    
    QString privateKeyPath() const { return m_privateKeyPath; }
    void setPrivateKeyPath(const QString& path);
    
    QString certificatePassword() const { return m_certificatePassword; }
    void setCertificatePassword(const QString& password);
    
    bool autoRenewCertificates() const { return m_autoRenewCertificates; }
    void setAutoRenewCertificates(bool autoRenew);
    
    // Let's Encrypt configuration
    bool letsEncryptEnabled() const { return m_letsEncryptEnabled; }
    void setLetsEncryptEnabled(bool enabled);
    
    QString letsEncryptEmail() const { return m_letsEncryptEmail; }
    void setLetsEncryptEmail(const QString& email);
    
    QStringList letsEncryptDomains() const { return m_letsEncryptDomains; }
    void setLetsEncryptDomains(const QStringList& domains);
    
    bool letsEncryptStaging() const { return m_letsEncryptStaging; }
    void setLetsEncryptStaging(bool staging);
    
    // Cloudflare configuration
    bool cloudflareEnabled() const { return m_cloudflareEnabled; }
    void setCloudflareEnabled(bool enabled);
    
    QString cloudflareApiToken() const { return m_cloudflareApiToken; }
    void setCloudflareApiToken(const QString& token);
    
    QString cloudflareZoneId() const { return m_cloudflareZoneId; }
    void setCloudflareZoneId(const QString& zoneId);
    
    // Protocol configuration
    bool iceCastEnabled() const { return m_iceCastEnabled; }
    void setIceCastEnabled(bool enabled);
    
    bool shoutCastEnabled() const { return m_shoutCastEnabled; }
    void setShoutCastEnabled(bool enabled);
    
    bool hlsEnabled() const { return m_hlsEnabled; }
    void setHlsEnabled(bool enabled);
    
    int hlsSegmentDuration() const { return m_hlsSegmentDuration; }
    void setHlsSegmentDuration(int duration);
    
    int hlsPlaylistSize() const { return m_hlsPlaylistSize; }
    void setHlsPlaylistSize(int size);
    
    // Codec configuration
    QStringList enabledCodecs() const { return m_enabledCodecs; }
    void setEnabledCodecs(const QStringList& codecs);
    
    int mp3Quality() const { return m_mp3Quality; }
    void setMp3Quality(int quality);
    
    int aacBitrate() const { return m_aacBitrate; }
    void setAacBitrate(int bitrate);
    
    int oggQuality() const { return m_oggQuality; }
    void setOggQuality(int quality);
    
    // Relay configuration
    bool relayEnabled() const { return m_relayEnabled; }
    void setRelayEnabled(bool enabled);
    
    int maxRelays() const { return m_maxRelays; }
    void setMaxRelays(int relays);
    
    int relayReconnectInterval() const { return m_relayReconnectInterval; }
    void setRelayReconnectInterval(int interval);
    
    // Statistic relay configuration
    bool statisticRelayEnabled() const { return m_statisticRelayEnabled; }
    void setStatisticRelayEnabled(bool enabled);
    
    int statisticRelayUpdateInterval() const { return m_statisticRelayUpdateInterval; }
    void setStatisticRelayUpdateInterval(int interval);
    
    int maxStatisticRelays() const { return m_maxStatisticRelays; }
    void setMaxStatisticRelays(int relays);
    
    // Server authentication and location settings
    QString sourcePassword() const { return m_sourcePassword; }
    void setSourcePassword(const QString& password);
    
    QString relayPassword() const { return m_relayPassword; }
    void setRelayPassword(const QString& password);
    
    QString adminUsername() const { return m_adminUsername; }
    void setAdminUsername(const QString& username);
    
    QString adminPassword() const { return m_adminPassword; }
    void setAdminPassword(const QString& password);
    
    QString serverLocation() const { return m_serverLocation; }
    void setServerLocation(const QString& location);
    
    QString serverHostname() const { return m_serverHostname; }
    void setServerHostname(const QString& hostname);
    
    // Fallback configuration
    bool fallbackEnabled() const { return m_fallbackEnabled; }
    void setFallbackEnabled(bool enabled);
    
    QString fallbackFile() const { return m_fallbackFile; }
    void setFallbackFile(const QString& file);
    
    QString emergencyFile() const { return m_emergencyFile; }
    void setEmergencyFile(const QString& file);
    
    // Logging configuration
    QString logLevel() const { return m_logLevel; }
    void setLogLevel(const QString& level);
    
    int maxLogSize() const { return m_maxLogSize; }
    void setMaxLogSize(int size);
    
    int logRetention() const { return m_logRetention; }
    void setLogRetention(int days);
    
    // Performance configuration
    int ioThreads() const { return m_ioThreads; }
    void setIoThreads(int threads);
    
    int workerThreads() const { return m_workerThreads; }
    void setWorkerThreads(int threads);
    
    bool enableCompression() const { return m_enableCompression; }
    void setEnableCompression(bool enabled);
    
    // GUI configuration
    bool minimizeToTray() const { return m_minimizeToTray; }
    void setMinimizeToTray(bool minimize);
    
    bool startMinimized() const { return m_startMinimized; }
    void setStartMinimized(bool minimized);
    
    QString theme() const { return m_theme; }
    void setTheme(const QString& theme);

signals:
    void configurationChanged();
    void httpPortChanged(int port);
    void httpsPortChanged(int port);
    void sslConfigurationChanged();
    void codecConfigurationChanged();

private:
    Configuration();
    ~Configuration();
    
    void setDefaultValues();
    void validateSettings();
    
    std::unique_ptr<QSettings> m_settings;
    
    // Server settings
    int m_httpPort = 8000;
    int m_httpsPort = 8443;
    QHostAddress m_bindAddress = QHostAddress::Any;
    int m_maxConnections = 100000;
    int m_maxStreams = 1000;
    
    // Stream settings
    int m_defaultLatency = 5;
    int m_maxLatency = 60;
    int m_minLatency = 1;
    int m_bufferSize = 65536;
    
    // SSL settings
    bool m_sslEnabled = false;
    QString m_certificatePath;
    QString m_privateKeyPath;
    QString m_certificatePassword;
    bool m_autoRenewCertificates = true;
    
    // Let's Encrypt
    bool m_letsEncryptEnabled = false;
    QString m_letsEncryptEmail;
    QStringList m_letsEncryptDomains;
    bool m_letsEncryptStaging = false;
    
    // Cloudflare
    bool m_cloudflareEnabled = false;
    QString m_cloudflareApiToken;
    QString m_cloudflareZoneId;
    
    // Protocols
    bool m_iceCastEnabled = true;
    bool m_shoutCastEnabled = true;
    bool m_hlsEnabled = true;
    int m_hlsSegmentDuration = 6;
    int m_hlsPlaylistSize = 6;
    
    // Codecs
    QStringList m_enabledCodecs = {"mp3", "aac", "aac+", "ogg", "opus", "flac"};
    int m_mp3Quality = 128;
    int m_aacBitrate = 128;
    int m_oggQuality = 6;
    
    // Relay
    bool m_relayEnabled = true;
    int m_maxRelays = 100;
    int m_relayReconnectInterval = 5;
    
    // Statistic relay
    bool m_statisticRelayEnabled = true;
    int m_statisticRelayUpdateInterval = 30;
    int m_maxStatisticRelays = 50;
    
    // Server authentication and location settings
    QString m_sourcePassword;
    QString m_relayPassword;
    QString m_adminUsername;
    QString m_adminPassword;
    QString m_serverLocation;
    QString m_serverHostname;
    
    // Fallback
    bool m_fallbackEnabled = true;
    QString m_fallbackFile;
    QString m_emergencyFile;
    
    // Logging
    QString m_logLevel = "INFO";
    int m_maxLogSize = 10; // MB
    int m_logRetention = 30; // days
    
    // Performance
    int m_ioThreads = 4;
    int m_workerThreads = 8;
    bool m_enableCompression = true;
    
    // GUI
    bool m_minimizeToTray = true;
    bool m_startMinimized = false;
    QString m_theme = "dark";
    
    Q_DISABLE_COPY(Configuration)
};

} // namespace LegacyStream