#ifndef SSLWIDGET_H
#define SSLWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>

class SSLWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SSLWidget(QWidget *parent = nullptr);
    ~SSLWidget();

private slots:
    void onGenerateCertificate();
    void onLoadCertificate();
    void onSaveCertificate();
    void onTestConnection();

private:
    void setupUI();
    void setupConnections();

    // UI Components
    QGroupBox *m_sslGroup;
    QLineEdit *m_certificatePath;
    QLineEdit *m_privateKeyPath;
    QLineEdit *m_caCertificatePath;
    QCheckBox *m_enableSSL;
    QPushButton *m_generateCertBtn;
    QPushButton *m_loadCertBtn;
    QPushButton *m_saveCertBtn;
    QPushButton *m_testConnectionBtn;
    QLabel *m_statusLabel;
};

#endif // SSLWIDGET_H 