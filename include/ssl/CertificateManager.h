#pragma once

#include <QObject>
#include <QSslCertificate>
#include <QSslKey>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <memory>

namespace LegacyStream {
namespace SSL {

class AcmeClient;
class CloudflareApi;

class CertificateManager : public QObject
{
    Q_OBJECT

public:
    explicit CertificateManager(QObject* parent = nullptr);
    ~CertificateManager();

    enum class CertificateSource {
        Manual,
        LetsEncrypt,
        SelfSigned
    };

    enum class CertificateStatus {
        Valid,
        Expired,
        Expiring,
        Invalid,
        NotFound,
        Renewing,
        Error
    };

    struct CertificateInfo {
        QSslCertificate certificate;
        QSslKey privateKey;
        CertificateSource source = CertificateSource::Manual;
        CertificateStatus status = CertificateStatus::NotFound;
        QDateTime expiryDate;
        QDateTime renewalDate;
        QStringList domains;
        QString issuer;
        QString error;
        int daysUntilExpiry = 0;
        bool autoRenew = false;
    };

    // Certificate management
    bool loadCertificate(const QString& certPath, const QString& keyPath, const QString& password = QString());
    bool saveCertificate(const QString& certPath, const QString& keyPath, const QSslCertificate& cert, const QSslKey& key);
    bool generateSelfSignedCertificate(const QStringList& domains, int validityDays = 365);
    
    // Let's Encrypt integration
    bool requestLetsEncryptCertificate(const QStringList& domains, const QString& email, bool staging = false);
    bool renewLetsEncryptCertificate();
    void setupAutoRenewal(int daysBeforeExpiry = 30);
    
    // Cloudflare integration
    bool setupCloudflareIntegration(const QString& apiToken, const QString& zoneId);
    bool validateCloudflareCredentials();
    bool createCloudflareChallenge(const QString& domain, const QString& token, const QString& keyAuth);
    bool deleteCloudflareChallenge(const QString& domain, const QString& token);
    
    // Certificate information
    CertificateInfo getCertificateInfo() const { return m_certificateInfo; }
    bool hasCertificate() const { return !m_certificateInfo.certificate.isNull(); }
    bool isValidCertificate() const { return m_certificateInfo.status == CertificateStatus::Valid; }
    int daysUntilExpiry() const { return m_certificateInfo.daysUntilExpiry; }
    
    // Certificate validation
    bool validateCertificate(const QSslCertificate& cert, const QStringList& domains = QStringList());
    bool validatePrivateKey(const QSslKey& key, const QSslCertificate& cert);
    CertificateStatus checkCertificateStatus(const QSslCertificate& cert);
    
    // Configuration
    void setAutoRenewalEnabled(bool enabled) { m_autoRenewalEnabled = enabled; }
    bool isAutoRenewalEnabled() const { return m_autoRenewalEnabled; }
    
    void setRenewalThreshold(int days) { m_renewalThresholdDays = days; }
    int renewalThreshold() const { return m_renewalThresholdDays; }

signals:
    void certificateLoaded(const CertificateInfo& info);
    void certificateRenewed(const CertificateInfo& info);
    void certificateExpiring(int daysUntilExpiry);
    void certificateExpired();
    void renewalStarted();
    void renewalCompleted(bool success);
    void renewalFailed(const QString& error);
    void challengeCreated(const QString& domain, const QString& token);
    void challengeVerified(const QString& domain);
    void challengeFailed(const QString& domain, const QString& error);

private slots:
    void onAutoRenewalCheck();
    void onAcmeRequestFinished();
    void onCloudflareRequestFinished();
    void handleNetworkError(QNetworkReply::NetworkError error);

private:
    void updateCertificateInfo();
    void setupRenewalTimer();
    void scheduleRenewalCheck();
    bool backupCurrentCertificate();
    bool restoreBackupCertificate();
    void cleanupOldCertificates();
    
    // ACME protocol helpers
    bool initializeAcmeClient();
    bool createAcmeAccount(const QString& email);
    bool submitCertificateRequest(const QStringList& domains);
    bool completeChallengeVerification();
    
    // Cloudflare API helpers
    bool initializeCloudflareApi();
    QString getZoneId(const QString& domain);
    bool createDnsRecord(const QString& domain, const QString& name, const QString& value);
    bool deleteDnsRecord(const QString& recordId);
    
    // File management
    QString getCertificateStoragePath() const;
    QString getBackupPath() const;
    bool ensureStorageDirectory();
    
    CertificateInfo m_certificateInfo;
    
    // Auto-renewal
    bool m_autoRenewalEnabled = true;
    int m_renewalThresholdDays = 30;
    QTimer* m_renewalTimer;
    QTimer* m_expiryCheckTimer;
    
    // Network
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    
    // ACME client (Let's Encrypt)
    std::unique_ptr<AcmeClient> m_acmeClient;
    bool m_acmeStaging = false;
    QString m_acmeEmail;
    QStringList m_acmeDomains;
    
    // Cloudflare integration
    std::unique_ptr<CloudflareApi> m_cloudflareApi;
    QString m_cloudflareApiToken;
    QString m_cloudflareZoneId;
    
    // Storage paths
    QString m_certificateDir;
    QString m_certificatePath;
    QString m_privateKeyPath;
    QString m_backupDir;
    
    // State
    bool m_isRenewing = false;
    QDateTime m_lastRenewalAttempt;
    int m_renewalAttempts = 0;
    
    Q_DISABLE_COPY(CertificateManager)
};

} // namespace SSL
} // namespace LegacyStream