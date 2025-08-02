#include "gui/MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QTimer>
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QFrame>
#include <QTextEdit>
#include <QTabWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    
    // Set window icon
    QIcon windowIcon(":/icons/app_icon.png");
    if (!windowIcon.isNull()) {
        setWindowIcon(windowIcon);
    } else {
        // Use default icon if custom icon not found
        setWindowIcon(QIcon());
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Set modern light theme with your blue colors
    centralWidget->setStyleSheet(
        "QWidget { background-color: #f8fafc; color: #1e293b; }"
        "QLabel { color: #1e293b; }"
        "QPushButton { "
        "   background-color: #4a90e2; "
        "   border: none; "
        "   border-radius: 6px; "
        "   padding: 10px 20px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 120px; "
        "   min-height: 35px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #3b82f6; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #1d4ed8; "
        "}"
        "QPushButton:disabled { "
        "   background-color: #94a3b8; "
        "   color: #64748b; "
        "}"
        "QFrame { "
        "   background-color: #ffffff; "
        "   border: 1px solid #e2e8f0; "
        "   border-radius: 8px; "
        "}"
        "QTextEdit, QPlainTextEdit { "
        "   background-color: #ffffff; "
        "   border: 1px solid #e2e8f0; "
        "   border-radius: 6px; "
        "   padding: 8px; "
        "   font-family: 'Consolas', 'Monaco', monospace; "
        "   font-size: 12px; "
        "   color: #1e293b; "
        "}"
        "QTabWidget::pane { "
        "   border: 1px solid #e2e8f0; "
        "   border-radius: 6px; "
        "   background-color: #ffffff; "
        "}"
        "QTabBar::tab { "
        "   background-color: #f1f5f9; "
        "   border: 1px solid #e2e8f0; "
        "   padding: 8px 16px; "
        "   margin-right: 2px; "
        "   border-top-left-radius: 6px; "
        "   border-top-right-radius: 6px; "
        "}"
        "QTabBar::tab:selected { "
        "   background-color: #ffffff; "
        "   border-bottom: 1px solid #ffffff; "
        "}"
    );
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Main controls area (like Rocket Streaming)
    QFrame *controlsFrame = new QFrame(this);
    controlsFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #ffffff; "
        "   border: 1px solid #e2e8f0; "
        "   border-radius: 8px; "
        "   padding: 20px; "
        "}"
    );
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsFrame);
    
    // Control buttons in a horizontal layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    // Start Server Button (like Rocket's style)
    m_startBtn = new QPushButton("▶ Start Server", this);
    m_startBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #4a90e2; "
        "   border: none; "
        "   border-radius: 6px; "
        "   padding: 12px 24px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 120px; "
        "   min-height: 40px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #3b82f6; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #1d4ed8; "
        "}"
    );
    
    // Stop Server Button (like Rocket's style)
    m_stopBtn = new QPushButton("⏹ Stop Server", this);
    m_stopBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #4a90e2; "
        "   border: none; "
        "   border-radius: 6px; "
        "   padding: 12px 24px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 120px; "
        "   min-height: 40px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #3b82f6; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #1d4ed8; "
        "}"
    );
    m_stopBtn->setEnabled(false); // Initially disabled
    
    buttonLayout->addWidget(m_startBtn);
    buttonLayout->addWidget(m_stopBtn);
    buttonLayout->addStretch(); // Push buttons to the left
    
    controlsLayout->addLayout(buttonLayout);
    
    // Streams section (like Rocket's interface)
    QLabel *streamsLabel = new QLabel("Streams", this);
    streamsLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #1e293b; margin-top: 15px;");
    controlsLayout->addWidget(streamsLabel);
    
    // Stream list frame
    QFrame *streamsFrame = new QFrame(this);
    streamsFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #f8fafc; "
        "   border: 1px solid #e2e8f0; "
        "   border-radius: 6px; "
        "   padding: 15px; "
        "}"
    );
    QVBoxLayout *streamsLayout = new QVBoxLayout(streamsFrame);
    
    // Stream entry (like Rocket's /radio)
    QHBoxLayout *streamEntryLayout = new QHBoxLayout();
    
    QLabel *streamPathLabel = new QLabel("/radio", this);
    streamPathLabel->setStyleSheet(
        "color: #3b82f6; "
        "font-weight: bold; "
        "font-size: 14px; "
        "text-decoration: underline; "
        "cursor: pointer;"
    );
    
    QLabel *streamStatusLabel = new QLabel("No source connected", this);
    streamStatusLabel->setStyleSheet("color: #64748b; font-size: 12px;");
    
    // Status indicator (red circle like Rocket's)
    QLabel *statusIndicator = new QLabel("🔴", this);
    statusIndicator->setStyleSheet("font-size: 12px;");
    
    streamEntryLayout->addWidget(streamPathLabel);
    streamEntryLayout->addStretch();
    streamEntryLayout->addWidget(streamStatusLabel);
    streamEntryLayout->addWidget(statusIndicator);
    
    streamsLayout->addLayout(streamEntryLayout);
    controlsLayout->addWidget(streamsFrame);
    
    mainLayout->addWidget(controlsFrame);
    
    // Log area (like Rocket's interface)
    QFrame *logFrame = new QFrame(this);
    logFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #ffffff; "
        "   border: 1px solid #e2e8f0; "
        "   border-radius: 8px; "
        "   padding: 20px; "
        "}"
    );
    QVBoxLayout *logLayout = new QVBoxLayout(logFrame);
    
    // Tab widget for logs
    QTabWidget *logTabs = new QTabWidget(this);
    
    // Main Log tab
    QTextEdit *mainLog = new QTextEdit(this);
    mainLog->setReadOnly(true);
    mainLog->setMaximumHeight(200);
    mainLog->setPlainText(
        "[info] LegacyStream Audio Server 1.0.0 starting.\n"
        "[info] Free Edition - 100 listener limit.\n"
        "[info] Listening on 0.0.0.0:8000\n"
        "[info] Running with 16 worker threads.\n"
        "[info] SSL/TLS support available.\n"
        "[info] Web interface available at http://localhost:8000/web\n"
        "[info] Server ready for connections.\n"
    );
    
    // Access Log tab
    QTextEdit *accessLog = new QTextEdit(this);
    accessLog->setReadOnly(true);
    accessLog->setMaximumHeight(200);
    accessLog->setPlainText(
        "[access] 2025-01-02 05:48:39 - Server started\n"
        "[access] 2025-01-02 05:48:39 - Listening on port 8000\n"
        "[access] 2025-01-02 05:48:39 - SSL certificate loaded\n"
        "[access] 2025-01-02 05:48:39 - Web interface initialized\n"
    );
    
    logTabs->addTab(mainLog, "Main Log");
    logTabs->addTab(accessLog, "Access Log");
    
    logLayout->addWidget(logTabs);
    mainLayout->addWidget(logFrame);
    
    // Status labels (for dynamic updates)
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #64748b; font-size: 12px;");
    m_infoLabel = new QLabel("Port: 8000 | Protocol: HTTP/HTTPS | SSL: Available", this);
    m_infoLabel->setStyleSheet("color: #64748b; font-size: 12px;");
    
    // Connect signals
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::onStopServer);
    connect(m_configBtn, &QPushButton::clicked, this, &MainWindow::onConfiguration);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("&File");
    QAction *exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::onStartServer()
{
    // Update UI to show server is starting
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    statusBar()->showMessage("Starting server...", 2000);
    
    // Simulate server startup (in real implementation, this would start the actual server)
    QTimer::singleShot(2000, this, [this]() {
        statusBar()->showMessage("Server is running on port 8000", 3000);
        
        // Show success notification
        QMessageBox::information(this, "Server Started", 
            "LegacyStream Audio Server is now running!\n\n"
            "• HTTP Server: http://localhost:8000\n"
            "• HTTPS Server: https://localhost:8443\n"
            "• Web Interface: http://localhost:8000/web\n"
            "• Stream Endpoint: http://localhost:8000/stream\n\n"
            "The server is ready to accept streaming connections.");
    });
}

void MainWindow::onStopServer()
{
    // Update UI to show server is stopping
    m_stopBtn->setEnabled(false);
    statusBar()->showMessage("Stopping server...", 1500);
    
    // Simulate server shutdown
    QTimer::singleShot(1500, this, [this]() {
        m_startBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
        statusBar()->showMessage("Server has been stopped", 3000);
        
        // Show confirmation
        QMessageBox::information(this, "Server Stopped", 
            "LegacyStream Audio Server has been stopped.\n\n"
            "All active connections have been closed and the server is no longer accepting new connections.");
    });
}

void MainWindow::onConfiguration()
{
    // Create and show configuration dialog
    QDialog *configDialog = new QDialog(this);
    configDialog->setWindowTitle("LegacyStream Configuration");
    configDialog->setModal(true);
    configDialog->resize(500, 400);
    
    // Set modern styling for dialog
    configDialog->setStyleSheet(
        "QDialog { background-color: #2b2b2b; color: #ffffff; }"
        "QLabel { color: #ffffff; }"
        "QLineEdit, QSpinBox, QComboBox { "
        "   background-color: #3a3a3a; "
        "   border: 2px solid #4a90e2; "
        "   border-radius: 6px; "
        "   padding: 8px; "
        "   color: white; "
        "   font-size: 14px; "
        "}"
        "QPushButton { "
        "   background-color: #4a90e2; "
        "   border: none; "
        "   border-radius: 8px; "
        "   padding: 10px 20px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 100px; "
        "}"
        "QPushButton:hover { background-color: #5ba0f2; }"
        "QPushButton:pressed { background-color: #3a80d2; }"
    );
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(configDialog);
    
    // Configuration sections
    QLabel *titleLabel = new QLabel("Server Configuration", configDialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #4a90e2; margin: 10px;");
    dialogLayout->addWidget(titleLabel);
    
    // Port configuration
    QHBoxLayout *portLayout = new QHBoxLayout();
    QLabel *portLabel = new QLabel("HTTP Port:", configDialog);
    QSpinBox *portSpinBox = new QSpinBox(configDialog);
    portSpinBox->setRange(1024, 65535);
    portSpinBox->setValue(8000);
    portLayout->addWidget(portLabel);
    portLayout->addWidget(portSpinBox);
    dialogLayout->addLayout(portLayout);
    
    // SSL configuration
    QHBoxLayout *sslLayout = new QHBoxLayout();
    QLabel *sslLabel = new QLabel("Enable SSL:", configDialog);
    QCheckBox *sslCheckBox = new QCheckBox(configDialog);
    sslCheckBox->setChecked(true);
    sslLayout->addWidget(sslLabel);
    sslLayout->addWidget(sslCheckBox);
    dialogLayout->addLayout(sslLayout);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *saveBtn = new QPushButton("Save", configDialog);
    QPushButton *cancelBtn = new QPushButton("Cancel", configDialog);
    
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(cancelBtn);
    dialogLayout->addLayout(buttonLayout);
    
    // Connect buttons
    connect(saveBtn, &QPushButton::clicked, configDialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, configDialog, &QDialog::reject);
    
    // Show dialog
    if (configDialog->exec() == QDialog::Accepted) {
        QMessageBox::information(this, "Configuration Saved", 
            "Server configuration has been updated.\n\n"
            "Changes will take effect when the server is restarted.");
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About LegacyStream", 
        "LegacyStream Audio Streaming Server\n\n"
        "Version: 1.0.0\n"
        "© 2025 Legacy DEV Team");
} 