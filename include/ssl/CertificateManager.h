#ifndef CERTIFICATEMANAGER_H
#define CERTIFICATEMANAGER_H

#include <QObject>
#include <QString>
// SSL certificate support (disabled - not available in current Qt installation)
// #include <QSslCertificate>
// #include <QSslKey>
// #include <QSslConfiguration>

class CertificateManager : public QObject
{
    Q_OBJECT

public:
    explicit CertificateManager(QObject *parent = nullptr);
    ~CertificateManager();

    // Certificate management
    bool generateSelfSignedCertificate(const QString &commonName, 
                                     const QString &organization,
                                     const QString &country,
                                     int validityDays = 365);
    
    bool loadCertificate(const QString &certificatePath, 
                        const QString &privateKeyPath);
    
    bool saveCertificate(const QString &certificatePath, 
                        const QString &privateKeyPath);
    
    // SSL Configuration (disabled - not available in current Qt installation)
    // QSslConfiguration getSSLConfiguration() const;
    bool isCertificateValid() const;
    QString getCertificateInfo() const;
    
    // Getters
    QString getCertificatePath() const { return m_certificatePath; }
    QString getPrivateKeyPath() const { return m_privateKeyPath; }
    QString getCaCertificatePath() const { return m_caCertificatePath; }

signals:
    void certificateGenerated(bool success, const QString &message);
    void certificateLoaded(bool success, const QString &message);
    void certificateSaved(bool success, const QString &message);

private:
    // SSL certificate validation (disabled - not available in current Qt installation)
    // bool validateCertificate(const QSslCertificate &cert, const QSslKey &key);
    // QString formatCertificateInfo(const QSslCertificate &cert) const;

    // QSslCertificate m_certificate;
    // QSslKey m_privateKey;
    // QSslConfiguration m_sslConfiguration;
    
    QString m_certificatePath;
    QString m_privateKeyPath;
    QString m_caCertificatePath;
    
    bool m_isValid;
};

#endif // CERTIFICATEMANAGER_H 