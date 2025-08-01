#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>

namespace LegacyStream {

namespace SSL {
    class CertificateManager;
}

namespace GUI {

class SSLWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SSLWidget(QWidget *parent = nullptr);
    ~SSLWidget();

private slots:
    void onCertificateStatusChanged();
    void onUploadCertificateClicked();
    void onGenerateSelfSignedClicked();
    void onRequestLetsEncryptClicked();
    void onConfigureCloudflareClicked();
    void onTestConnectionClicked();
    void onRenewCertificateClicked();
    void onExportCertificateClicked();
    void onDeleteCertificateClicked();
    
    void onCertificateLoaded();
    void onCertificateRenewed();
    void onCertificateExpiring(int days);
    void onRenewalCompleted(bool success);
    void onRenewalFailed(const QString& error);
    
    void updateCertificateInfo();
    void updateRenewalCountdown();

private:
    void setupUI();
    void setupOverviewTab();
    void setupManualTab();
    void setupLetsEncryptTab();
    void setupCloudflareTab();
    void setupAdvancedTab();
    
    void connectSignals();
    void updateButtonStates();
    void showCertificateDetails();
    void validateInputs();
    
    // Main layout
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Overview tab
    QWidget* m_overviewTab;
    QGroupBox* m_statusGroup;
    QLabel* m_statusIcon;
    QLabel* m_statusText;
    QLabel* m_certificateSubject;
    QLabel* m_certificateIssuer;
    QLabel* m_expiryDate;
    QLabel* m_daysUntilExpiry;
    QProgressBar* m_expiryProgress;
    QPushButton* m_renewButton;
    QPushButton* m_detailsButton;
    
    // Manual certificate tab
    QWidget* m_manualTab;
    QGroupBox* m_uploadGroup;
    QLineEdit* m_certificatePathEdit;
    QLineEdit* m_privateKeyPathEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_browseCertButton;
    QPushButton* m_browseKeyButton;
    QPushButton* m_uploadButton;
    QPushButton* m_generateSelfSignedButton;
    
    // Let's Encrypt tab
    QWidget* m_letsEncryptTab;
    QGroupBox* m_letsEncryptGroup;
    QLineEdit* m_emailEdit;
    QTextEdit* m_domainsEdit;
    QCheckBox* m_stagingCheckBox;
    QCheckBox* m_autoRenewCheckBox;
    QPushButton* m_requestCertButton;
    QLabel* m_challengeStatus;
    QProgressBar* m_requestProgress;
    
    // Cloudflare tab
    QWidget* m_cloudflareTab;
    QGroupBox* m_cloudflareGroup;
    QLineEdit* m_apiTokenEdit;
    QLineEdit* m_zoneIdEdit;
    QPushButton* m_testCloudflareButton;
    QPushButton* m_configureCloudflareButton;
    QLabel* m_cloudflareStatus;
    
    // Advanced tab
    QWidget* m_advancedTab;
    QGroupBox* m_settingsGroup;
    QComboBox* m_tlsVersionCombo;
    QComboBox* m_cipherSuiteCombo;
    QCheckBox* m_hstsCheckBox;
    QCheckBox* m_ocspStaplingCheckBox;
    QCheckBox* m_compressionCheckBox;
    QPushButton* m_exportButton;
    QPushButton* m_deleteButton;
    QPushButton* m_testConnectionButton;
    
    // Status and progress
    QLabel* m_progressLabel;
    QProgressBar* m_progressBar;
    QTimer* m_updateTimer;
    
    // Certificate manager reference
    SSL::CertificateManager* m_certificateManager;
    
    // State tracking
    bool m_isRequestInProgress = false;
    bool m_certificateValid = false;
};

} // namespace GUI
} // namespace LegacyStream