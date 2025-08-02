#ifndef SSLMANAGER_H
#define SSLMANAGER_H

#include <QObject>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>

namespace LegacyStream {

/**
 * @brief SSL configuration structure
 */
struct SSLConfig
{
    QString certificatePath;
    QString privateKeyPath;
    QString certificatePassword;
    bool autoRenew = true;
    bool verifyPeer = true;
    QStringList allowedCiphers;
    int sslProtocol = QSsl::TlsV1_2;
};

/**
 * @brief Certificate information
 */
struct CertificateInfo
{
    QString subject;
    QString issuer;
    QDateTime validFrom;
    QDateTime validUntil;
    QString serialNumber;
    QStringList dnsNames;
    bool isValid = false;
    bool isExpired = false;
    int daysUntilExpiry = 0;
};

/**
 * @brief SSLManager for SSL/TLS support
 * 
 * Manages SSL certificates, configuration, and secure connections.
 * Supports Let's Encrypt integration and automatic certificate renewal.
 */
class SSLManager : public QObject
{
    Q_OBJECT

public:
    explicit SSLManager(QObject* parent = nullptr);
    ~SSLManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool start();
    void stop();

    // SSL configuration
    void setSSLConfig(const SSLConfig& config);
    SSLConfig getSSLConfig() const;
    QSslConfiguration getSSLConfiguration() const;

    // Certificate management
    bool loadCertificate(const QString& certPath, const QString& keyPath, const QString& password = QString());
    bool loadCertificateFromMemory(const QByteArray& certData, const QByteArray& keyData, const QString& password = QString());
    CertificateInfo getCertificateInfo() const;
    bool isCertificateValid() const;
    bool isCertificateExpired() const;
    int getDaysUntilExpiry() const;

    // Let's Encrypt integration
    void setLetsEncryptEnabled(bool enabled);
    void setLetsEncryptEmail(const QString& email);
    void setLetsEncryptDomains(const QStringList& domains);
    void setLetsEncryptStaging(bool staging);
    bool requestLetsEncryptCertificate();
    bool renewLetsEncryptCertificate();

    // Cloudflare integration
    void setCloudflareEnabled(bool enabled);
    void setCloudflareApiToken(const QString& token);
    void setCloudflareZoneId(const QString& zoneId);
    bool updateCloudflareDNS();

    // Status and information
    bool isSSLEnabled() const;
    bool isRunning() const;
    QJsonObject getStatusJson() const;

signals:
    void certificateLoaded(const CertificateInfo& info);
    void certificateExpired(const CertificateInfo& info);
    void certificateRenewed(const CertificateInfo& info);
    void sslError(const QString& error);
    void statusChanged(const QJsonObject& status);

private slots:
    void onCertificateRenewalTimer();
    void onLetsEncryptRequestFinished();
    void onCloudflareRequestFinished();

private:
    // Core functionality
    bool validateCertificate(const QSslCertificate& cert);
    bool renewCertificate();
    void updateCertificateInfo();
    void scheduleRenewal();

    // Let's Encrypt functions
    bool createLetsEncryptAccount();
    bool createLetsEncryptOrder();
    bool validateLetsEncryptDomain();
    bool finalizeLetsEncryptCertificate();

    // Cloudflare functions
    bool createCloudflareDNSRecord();
    bool deleteCloudflareDNSRecord();
    bool updateCloudflareDNSRecord();

    // Utility functions
    QString getLetsEncryptDirectory() const;
    QString getCertificateDirectory() const;
    bool writeCertificateToFile(const QByteArray& certData, const QString& path);
    bool writePrivateKeyToFile(const QByteArray& keyData, const QString& path);

    // Configuration
    SSLConfig m_sslConfig;
    QSslConfiguration m_sslConfiguration;
    CertificateInfo m_certificateInfo;

    // State management
    QAtomicInt m_isRunning = 0;
    QTimer* m_renewalTimer = nullptr;
    QMutex m_mutex;

    // Let's Encrypt
    bool m_letsEncryptEnabled = false;
    QString m_letsEncryptEmail;
    QStringList m_letsEncryptDomains;
    bool m_letsEncryptStaging = true;
    QString m_letsEncryptAccountKey;
    QString m_letsEncryptOrderUrl;

    // Cloudflare
    bool m_cloudflareEnabled = false;
    QString m_cloudflareApiToken;
    QString m_cloudflareZoneId;

    // Network management
    QNetworkAccessManager* m_networkManager = nullptr;

    // Statistics
    QJsonObject m_statistics;
    QDateTime m_startTime;

    Q_DISABLE_COPY(SSLManager)
};

} // namespace LegacyStream

#endif // SSLMANAGER_H 