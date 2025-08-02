#include "gui/SSLWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>

SSLWidget::SSLWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

SSLWidget::~SSLWidget()
{
}

void SSLWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // SSL Group
    m_sslGroup = new QGroupBox("SSL Certificate Management", this);
    QVBoxLayout *sslLayout = new QVBoxLayout(m_sslGroup);
    
    // Certificate path
    QHBoxLayout *certLayout = new QHBoxLayout();
    QLabel *certLabel = new QLabel("Certificate Path:", this);
    m_certificatePath = new QLineEdit(this);
    QPushButton *certBrowseBtn = new QPushButton("Browse", this);
    
    certLayout->addWidget(certLabel);
    certLayout->addWidget(m_certificatePath);
    certLayout->addWidget(certBrowseBtn);
    sslLayout->addLayout(certLayout);
    
    // Private key path
    QHBoxLayout *keyLayout = new QHBoxLayout();
    QLabel *keyLabel = new QLabel("Private Key Path:", this);
    m_privateKeyPath = new QLineEdit(this);
    QPushButton *keyBrowseBtn = new QPushButton("Browse", this);
    
    keyLayout->addWidget(keyLabel);
    keyLayout->addWidget(m_privateKeyPath);
    keyLayout->addWidget(keyBrowseBtn);
    sslLayout->addLayout(keyLayout);
    
    // CA Certificate path
    QHBoxLayout *caLayout = new QHBoxLayout();
    QLabel *caLabel = new QLabel("CA Certificate Path:", this);
    m_caCertificatePath = new QLineEdit(this);
    QPushButton *caBrowseBtn = new QPushButton("Browse", this);
    
    caLayout->addWidget(caLabel);
    caLayout->addWidget(m_caCertificatePath);
    caLayout->addWidget(caBrowseBtn);
    sslLayout->addLayout(caLayout);
    
    // Enable SSL checkbox
    m_enableSSL = new QCheckBox("Enable SSL/TLS", this);
    sslLayout->addWidget(m_enableSSL);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_generateCertBtn = new QPushButton("Generate Certificate", this);
    m_loadCertBtn = new QPushButton("Load Certificate", this);
    m_saveCertBtn = new QPushButton("Save Certificate", this);
    m_testConnectionBtn = new QPushButton("Test Connection", this);
    
    buttonLayout->addWidget(m_generateCertBtn);
    buttonLayout->addWidget(m_loadCertBtn);
    buttonLayout->addWidget(m_saveCertBtn);
    buttonLayout->addWidget(m_testConnectionBtn);
    sslLayout->addLayout(buttonLayout);
    
    // Status
    m_statusLabel = new QLabel("Status: Ready", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    sslLayout->addWidget(m_statusLabel);
    
    mainLayout->addWidget(m_sslGroup);
    
    // Connect signals
    connect(certBrowseBtn, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, 
            "Select Certificate File", "", "Certificate Files (*.crt *.pem *.cer);;All Files (*)");
        if (!fileName.isEmpty()) {
            m_certificatePath->setText(fileName);
        }
    });
    
    connect(keyBrowseBtn, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, 
            "Select Private Key File", "", "Key Files (*.key *.pem);;All Files (*)");
        if (!fileName.isEmpty()) {
            m_privateKeyPath->setText(fileName);
        }
    });
    
    connect(caBrowseBtn, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, 
            "Select CA Certificate File", "", "Certificate Files (*.crt *.pem *.cer);;All Files (*)");
        if (!fileName.isEmpty()) {
            m_caCertificatePath->setText(fileName);
        }
    });
    
    connect(m_generateCertBtn, &QPushButton::clicked, this, &SSLWidget::onGenerateCertificate);
    connect(m_loadCertBtn, &QPushButton::clicked, this, &SSLWidget::onLoadCertificate);
    connect(m_saveCertBtn, &QPushButton::clicked, this, &SSLWidget::onSaveCertificate);
    connect(m_testConnectionBtn, &QPushButton::clicked, this, &SSLWidget::onTestConnection);
}

void SSLWidget::onGenerateCertificate()
{
    m_statusLabel->setText("Status: Generating certificate...");
    QMessageBox::information(this, "SSL", "Certificate generation would be implemented here.");
    m_statusLabel->setText("Status: Certificate generated");
}

void SSLWidget::onLoadCertificate()
{
    m_statusLabel->setText("Status: Loading certificate...");
    QMessageBox::information(this, "SSL", "Certificate loading would be implemented here.");
    m_statusLabel->setText("Status: Certificate loaded");
}

void SSLWidget::onSaveCertificate()
{
    m_statusLabel->setText("Status: Saving certificate...");
    QMessageBox::information(this, "SSL", "Certificate saving would be implemented here.");
    m_statusLabel->setText("Status: Certificate saved");
}

void SSLWidget::onTestConnection()
{
    m_statusLabel->setText("Status: Testing connection...");
    QMessageBox::information(this, "SSL", "Connection test would be implemented here.");
    m_statusLabel->setText("Status: Connection test completed");
} 