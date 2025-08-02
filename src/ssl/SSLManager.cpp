#include "ssl/SSLManager.h"
#include "core/Configuration.h"
#include "core/Logger.h"

#include <QLoggingCategory>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QMutexLocker>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslSocket>

Q_LOGGING_CATEGORY(sslManager, "sslManager")

namespace LegacyStream {

SSLManager::SSLManager(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_renewalTimer(new QTimer(this))
{
    qCDebug(sslManager) << "SSLManager created";
    
    // Set up renewal timer
    m_renewalTimer->setSingleShot(false);
    m_renewalTimer->setInterval(86400000); // Check renewal daily
    
    // Connect signals
    connect(m_renewalTimer, &QTimer::timeout, this, &SSLManager::onCertificateRenewalTimer);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &SSLManager::onLetsEncryptRequestFinished);
}

SSLManager::~SSLManager()
{
    shutdown();
}

bool SSLManager::initialize()
{
    qCDebug(sslManager) << "Initializing SSLManager";
    
    // Initialize SSL configuration
    m_sslConfiguration = QSslConfiguration::defaultConfiguration();
    m_sslConfiguration.setProtocol(QSsl::TlsV1_2);
    
    // Initialize statistics
    m_statistics["ssl_enabled"] = false;
    m_statistics["certificate_loaded"] = false;
    m_statistics["lets_encrypt_enabled"] = false;
    m_statistics["cloudflare_enabled"] = false;
    m_statistics["start_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    qCInfo(sslManager) << "SSLManager initialized successfully";
    return true;
}

void SSLManager::shutdown()
{
    if (m_isRunning.load()) {
        stop();
    }
    
    // Stop renewal timer
    m_renewalTimer->stop();
    
    qCInfo(sslManager) << "SSLManager shutdown complete";
}

bool SSLManager::start()
{
    if (m_isRunning.load()) {
        qCWarning(sslManager) << "SSLManager already running";
        return true;
    }
    
    qCInfo(sslManager) << "Starting SSLManager";
    
    // Start renewal timer
    m_renewalTimer->start();
    
    m_isRunning.store(true);
    m_startTime = QDateTime::currentDateTime();
    
    qCInfo(sslManager) << "SSLManager started successfully";
    return true;
}

void SSLManager::stop()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    qCInfo(sslManager) << "Stopping SSLManager";
    
    // Stop renewal timer
    m_renewalTimer->stop();
    
    m_isRunning.store(false);
    
    qCInfo(sslManager) << "SSLManager stopped";
}

void SSLManager::setSSLConfig(const SSLConfig& config)
{
    QMutexLocker locker(&m_mutex);
    m_sslConfig = config;
    
    // Update SSL configuration
    m_sslConfiguration.setProtocol(static_cast<QSsl::SslProtocol>(config.sslProtocol));
    
    if (!config.allowedCiphers.isEmpty()) {
        QList<QSslCipher> ciphers;
        for (const QString& cipherName : config.allowedCiphers) {
            QSslCipher cipher(cipherName);
            if (!cipher.isNull()) {
                ciphers.append(cipher);
            }
        }
        m_sslConfiguration.setCiphers(ciphers);
    }
    
    qCInfo(sslManager) << "SSL configuration updated";
}

SSLConfig SSLManager::getSSLConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_sslConfig;
}

QSslConfiguration SSLManager::getSSLConfiguration() const
{
    QMutexLocker locker(&m_mutex);
    return m_sslConfiguration;
}

bool SSLManager::loadCertificate(const QString& certPath, const QString& keyPath, const QString& password)
{
    QFile certFile(certPath);
    QFile keyFile(keyPath);
    
    if (!certFile.open(QIODevice::ReadOnly) || !keyFile.open(QIODevice::ReadOnly)) {
        qCWarning(sslManager) << "Failed to open certificate files";
        return false;
    }
    
    QByteArray certData = certFile.readAll();
    QByteArray keyData = keyFile.readAll();
    
    return loadCertificateFromMemory(certData, keyData, password);
}

bool SSLManager::loadCertificateFromMemory(const QByteArray& certData, const QByteArray& keyData, const QString& password)
{
    QMutexLocker locker(&m_mutex);
    
    // Load certificate
    QSslCertificate cert(certData);
    if (cert.isNull()) {
        qCWarning(sslManager) << "Invalid certificate data";
        return false;
    }
    
    // Load private key
    QSslKey key;
    if (!password.isEmpty()) {
        key = QSslKey(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, password.toUtf8());
    } else {
        key = QSslKey(keyData, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
    }
    
    if (key.isNull()) {
        qCWarning(sslManager) << "Invalid private key data";
        return false;
    }
    
    // Validate certificate
    if (!validateCertificate(cert)) {
        return false;
    }
    
    // Update SSL configuration
    m_sslConfiguration.setLocalCertificate(cert);
    m_sslConfiguration.setPrivateKey(key);
    
    // Update certificate info
    updateCertificateInfo();
    
    // Update statistics
    m_statistics["ssl_enabled"] = true;
    m_statistics["certificate_loaded"] = true;
    
    qCInfo(sslManager) << "Certificate loaded successfully";
    emit certificateLoaded(m_certificateInfo);
    emit statusChanged(m_statistics);
    
    return true;
}

CertificateInfo SSLManager::getCertificateInfo() const
{
    QMutexLocker locker(&m_mutex);
    return m_certificateInfo;
}

bool SSLManager::isCertificateValid() const
{
    QMutexLocker locker(&m_mutex);
    return m_certificateInfo.isValid && !m_certificateInfo.isExpired;
}

bool SSLManager::isCertificateExpired() const
{
    QMutexLocker locker(&m_mutex);
    return m_certificateInfo.isExpired;
}

int SSLManager::getDaysUntilExpiry() const
{
    QMutexLocker locker(&m_mutex);
    return m_certificateInfo.daysUntilExpiry;
}

void SSLManager::setLetsEncryptEnabled(bool enabled)
{
    m_letsEncryptEnabled = enabled;
    m_statistics["lets_encrypt_enabled"] = enabled;
    qCInfo(sslManager) << "Let's Encrypt" << (enabled ? "enabled" : "disabled");
}

void SSLManager::setLetsEncryptEmail(const QString& email)
{
    m_letsEncryptEmail = email;
}

void SSLManager::setLetsEncryptDomains(const QStringList& domains)
{
    m_letsEncryptDomains = domains;
}

void SSLManager::setLetsEncryptStaging(bool staging)
{
    m_letsEncryptStaging = staging;
}

bool SSLManager::requestLetsEncryptCertificate()
{
    if (!m_letsEncryptEnabled) {
        qCWarning(sslManager) << "Let's Encrypt not enabled";
        return false;
    }
    
    qCInfo(sslManager) << "Requesting Let's Encrypt certificate";
    
    // This would implement the full ACME protocol
    // For now, we'll just log the request
    return true;
}

bool SSLManager::renewLetsEncryptCertificate()
{
    if (!m_letsEncryptEnabled) {
        return false;
    }
    
    qCInfo(sslManager) << "Renewing Let's Encrypt certificate";
    return requestLetsEncryptCertificate();
}

void SSLManager::setCloudflareEnabled(bool enabled)
{
    m_cloudflareEnabled = enabled;
    m_statistics["cloudflare_enabled"] = enabled;
    qCInfo(sslManager) << "Cloudflare" << (enabled ? "enabled" : "disabled");
}

void SSLManager::setCloudflareApiToken(const QString& token)
{
    m_cloudflareApiToken = token;
}

void SSLManager::setCloudflareZoneId(const QString& zoneId)
{
    m_cloudflareZoneId = zoneId;
}

bool SSLManager::updateCloudflareDNS()
{
    if (!m_cloudflareEnabled) {
        return false;
    }
    
    qCInfo(sslManager) << "Updating Cloudflare DNS";
    return true;
}

bool SSLManager::isSSLEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_statistics["ssl_enabled"].toBool();
}

bool SSLManager::isRunning() const
{
    return m_isRunning.load();
}

QJsonObject SSLManager::getStatusJson() const
{
    QMutexLocker locker(&m_mutex);
    
    QJsonObject status = m_statistics;
    status["running"] = m_isRunning.load();
    status["certificate_info"] = QJsonObject{
        {"subject", m_certificateInfo.subject},
        {"issuer", m_certificateInfo.issuer},
        {"valid_from", m_certificateInfo.validFrom.toString(Qt::ISODate)},
        {"valid_until", m_certificateInfo.validUntil.toString(Qt::ISODate)},
        {"is_valid", m_certificateInfo.isValid},
        {"is_expired", m_certificateInfo.isExpired},
        {"days_until_expiry", m_certificateInfo.daysUntilExpiry}
    };
    
    return status;
}

void SSLManager::onCertificateRenewalTimer()
{
    if (!m_isRunning.load()) {
        return;
    }
    
    // Check if certificate needs renewal
    if (m_certificateInfo.daysUntilExpiry <= 30) {
        qCInfo(sslManager) << "Certificate expires in" << m_certificateInfo.daysUntilExpiry << "days, scheduling renewal";
        
        if (m_letsEncryptEnabled) {
            renewLetsEncryptCertificate();
        } else {
            renewCertificate();
        }
    }
}

void SSLManager::onLetsEncryptRequestFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        qCInfo(sslManager) << "Let's Encrypt request completed successfully";
    } else {
        qCWarning(sslManager) << "Let's Encrypt request failed:" << reply->errorString();
        emit sslError(reply->errorString());
    }
    
    reply->deleteLater();
}

void SSLManager::onCloudflareRequestFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }
    
    if (reply->error() == QNetworkReply::NoError) {
        qCInfo(sslManager) << "Cloudflare request completed successfully";
    } else {
        qCWarning(sslManager) << "Cloudflare request failed:" << reply->errorString();
    }
    
    reply->deleteLater();
}

bool SSLManager::validateCertificate(const QSslCertificate& cert)
{
    if (cert.isNull()) {
        return false;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    if (cert.effectiveDate() > now || cert.expiryDate() < now) {
        qCWarning(sslManager) << "Certificate is not valid at current time";
        return false;
    }
    
    return true;
}

bool SSLManager::renewCertificate()
{
    qCInfo(sslManager) << "Renewing certificate";
    // Implementation would depend on certificate type
    return true;
}

void SSLManager::updateCertificateInfo()
{
    QSslCertificate cert = m_sslConfiguration.localCertificate();
    if (cert.isNull()) {
        m_certificateInfo = CertificateInfo{};
        return;
    }
    
    m_certificateInfo.subject = cert.subjectDisplayName();
    m_certificateInfo.issuer = cert.issuerDisplayName();
    m_certificateInfo.validFrom = cert.effectiveDate();
    m_certificateInfo.validUntil = cert.expiryDate();
    m_certificateInfo.serialNumber = cert.serialNumber();
    m_certificateInfo.dnsNames = cert.subjectAlternativeNames().values(QSslCertificate::DNS);
    
    QDateTime now = QDateTime::currentDateTime();
    m_certificateInfo.isValid = (cert.effectiveDate() <= now && cert.expiryDate() >= now);
    m_certificateInfo.isExpired = (cert.expiryDate() < now);
    m_certificateInfo.daysUntilExpiry = now.daysTo(cert.expiryDate());
}

void SSLManager::scheduleRenewal()
{
    if (m_sslConfig.autoRenew && m_certificateInfo.daysUntilExpiry <= 30) {
        qCInfo(sslManager) << "Scheduling certificate renewal";
        // Schedule renewal
    }
}

bool SSLManager::createLetsEncryptAccount()
{
    if (!m_letsEncryptEnabled) {
        return false;
    }
    
    qCInfo(sslManager) << "Creating Let's Encrypt account";
    
    // Generate account key if not exists
    if (m_letsEncryptAccountKey.isEmpty()) {
        // Generate new account key
        m_letsEncryptAccountKey = "generated_account_key";
    }
    
    // Create account registration request
    QJsonObject accountRequest;
    accountRequest["contact"] = QJsonArray{QString("mailto:%1").arg(m_letsEncryptEmail)};
    accountRequest["termsOfServiceAgreed"] = true;
    
    // Send account creation request
    QNetworkRequest request;
    QString directoryUrl = m_letsEncryptStaging ? 
        "https://acme-staging-v02.api.letsencrypt.org/directory" :
        "https://acme-v02.api.letsencrypt.org/directory";
    
    request.setUrl(QUrl(directoryUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/jose+json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(accountRequest).toJson());
    connect(reply, &QNetworkReply::finished, this, &SSLManager::onLetsEncryptRequestFinished);
    
    return true;
}

bool SSLManager::createLetsEncryptOrder()
{
    if (!m_letsEncryptEnabled || m_letsEncryptDomains.isEmpty()) {
        return false;
    }
    
    qCInfo(sslManager) << "Creating Let's Encrypt order";
    
    // Create certificate order request
    QJsonObject orderRequest;
    QJsonArray identifiers;
    
    for (const QString& domain : m_letsEncryptDomains) {
        QJsonObject identifier;
        identifier["type"] = "dns";
        identifier["value"] = domain;
        identifiers.append(identifier);
    }
    
    orderRequest["identifiers"] = identifiers;
    
    // Send order creation request
    QNetworkRequest request;
    QString newOrderUrl = m_letsEncryptStaging ? 
        "https://acme-staging-v02.api.letsencrypt.org/acme/new-order" :
        "https://acme-v02.api.letsencrypt.org/acme/new-order";
    
    request.setUrl(QUrl(newOrderUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/jose+json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(orderRequest).toJson());
    connect(reply, &QNetworkReply::finished, this, &SSLManager::onLetsEncryptRequestFinished);
    
    return true;
}

bool SSLManager::validateLetsEncryptDomain()
{
    if (!m_letsEncryptEnabled || m_letsEncryptOrderUrl.isEmpty()) {
        return false;
    }
    
    qCInfo(sslManager) << "Validating Let's Encrypt domain";
    
    // Create domain validation challenge
    QJsonObject challengeRequest;
    challengeRequest["type"] = "http-01";
    challengeRequest["token"] = "validation_token";
    
    // Send validation request
    QNetworkRequest request;
    request.setUrl(QUrl(m_letsEncryptOrderUrl + "/challenges"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/jose+json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(challengeRequest).toJson());
    connect(reply, &QNetworkReply::finished, this, &SSLManager::onLetsEncryptRequestFinished);
    
    return true;
}

bool SSLManager::finalizeLetsEncryptCertificate()
{
    if (!m_letsEncryptEnabled || m_letsEncryptOrderUrl.isEmpty()) {
        return false;
    }
    
    qCInfo(sslManager) << "Finalizing Let's Encrypt certificate";
    
    // Create certificate finalization request
    QJsonObject finalizeRequest;
    finalizeRequest["csr"] = "base64_encoded_csr";
    
    // Send finalization request
    QNetworkRequest request;
    request.setUrl(QUrl(m_letsEncryptOrderUrl + "/finalize"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/jose+json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(finalizeRequest).toJson());
    connect(reply, &QNetworkReply::finished, this, &SSLManager::onLetsEncryptRequestFinished);
    
    return true;
}

bool SSLManager::createCloudflareDNSRecord()
{
    if (!m_cloudflareEnabled || m_cloudflareApiToken.isEmpty() || m_cloudflareZoneId.isEmpty()) {
        return false;
    }
    
    qCInfo(sslManager) << "Creating Cloudflare DNS record";
    
    // Create DNS record request
    QJsonObject dnsRecord;
    dnsRecord["type"] = "A";
    dnsRecord["name"] = "stream";
    dnsRecord["content"] = "127.0.0.1";
    dnsRecord["ttl"] = 1; // Auto
    dnsRecord["proxied"] = true;
    
    // Send DNS record creation request
    QNetworkRequest request;
    QString url = QString("https://api.cloudflare.com/client/v4/zones/%1/dns_records").arg(m_cloudflareZoneId);
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_cloudflareApiToken).toUtf8());
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(dnsRecord).toJson());
    connect(reply, &QNetworkReply::finished, this, &SSLManager::onCloudflareRequestFinished);
    
    return true;
}

bool SSLManager::deleteCloudflareDNSRecord()
{
    if (!m_cloudflareEnabled || m_cloudflareApiToken.isEmpty() || m_cloudflareZoneId.isEmpty()) {
        return false;
    }
    
    qCInfo(sslManager) << "Deleting Cloudflare DNS record";
    
    // Send DNS record deletion request
    QNetworkRequest request;
    QString url = QString("https://api.cloudflare.com/client/v4/zones/%1/dns_records/record_id").arg(m_cloudflareZoneId);
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_cloudflareApiToken).toUtf8());
    
    QNetworkReply* reply = m_networkManager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, &SSLManager::onCloudflareRequestFinished);
    
    return true;
}

bool SSLManager::updateCloudflareDNSRecord()
{
    if (!m_cloudflareEnabled || m_cloudflareApiToken.isEmpty() || m_cloudflareZoneId.isEmpty()) {
        return false;
    }
    
    qCInfo(sslManager) << "Updating Cloudflare DNS record";
    
    // Create DNS record update request
    QJsonObject dnsRecord;
    dnsRecord["type"] = "A";
    dnsRecord["name"] = "stream";
    dnsRecord["content"] = "127.0.0.1";
    dnsRecord["ttl"] = 1; // Auto
    dnsRecord["proxied"] = true;
    
    // Send DNS record update request
    QNetworkRequest request;
    QString url = QString("https://api.cloudflare.com/client/v4/zones/%1/dns_records/record_id").arg(m_cloudflareZoneId);
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_cloudflareApiToken).toUtf8());
    
    QNetworkReply* reply = m_networkManager->put(request, QJsonDocument(dnsRecord).toJson());
    connect(reply, &QNetworkReply::finished, this, &SSLManager::onCloudflareRequestFinished);
    
    return true;
}

} // namespace LegacyStream 