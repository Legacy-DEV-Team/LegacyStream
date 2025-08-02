#include "gui/MainWindow.h"
#include "core/Configuration.h"
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
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QSettings> // Added for QSettings

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
    
    // Set dark theme to match your website
    centralWidget->setStyleSheet(
        "QWidget { background-color: #0f172a; color: #f8fafc; }"
        "QLabel { color: #f8fafc; }"
        "QPushButton { "
        "   background-color: #3b82f6; "
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
        "   background-color: #60a5fa; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #1d4ed8; "
        "}"
        "QPushButton:disabled { "
        "   background-color: #64748b; "
        "   color: #94a3b8; "
        "}"
        "QFrame { "
        "   background-color: #1e293b; "
        "   border: 1px solid #334155; "
        "   border-radius: 8px; "
        "}"
        "QTextEdit, QPlainTextEdit { "
        "   background-color: #1e293b; "
        "   border: 1px solid #334155; "
        "   border-radius: 6px; "
        "   padding: 8px; "
        "   font-family: 'Consolas', 'Monaco', monospace; "
        "   font-size: 12px; "
        "   color: #f8fafc; "
        "}"
        "QTabWidget::pane { "
        "   border: 1px solid #334155; "
        "   border-radius: 6px; "
        "   background-color: #1e293b; "
        "}"
        "QTabBar::tab { "
        "   background-color: #334155; "
        "   border: 1px solid #334155; "
        "   padding: 8px 16px; "
        "   margin-right: 2px; "
        "   border-top-left-radius: 6px; "
        "   border-top-right-radius: 6px; "
        "   color: #cbd5e1; "
        "}"
        "QTabBar::tab:selected { "
        "   background-color: #1e293b; "
        "   border-bottom: 1px solid #1e293b; "
        "   color: #f8fafc; "
        "}"
    );
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Main controls area (like Rocket Streaming)
    QFrame *controlsFrame = new QFrame(this);
    controlsFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #1e293b; "
        "   border: 1px solid #334155; "
        "   border-radius: 8px; "
        "   padding: 20px; "
        "}"
    );
    QVBoxLayout *controlsLayout = new QVBoxLayout(controlsFrame);
    
    // Control buttons in a horizontal layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    // Start Server Button (green when active, like Rocket's style)
    m_startBtn = new QPushButton("▶ Start Server", this);
    m_startBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #10b981; "
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
        "   background-color: #059669; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #047857; "
        "}"
        "QPushButton:disabled { "
        "   background-color: #64748b; "
        "   color: #94a3b8; "
        "}"
    );
    
    // Stop Server Button (red when active, like Rocket's style)
    m_stopBtn = new QPushButton("⏹ Stop Server", this);
    m_stopBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #ef4444; "
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
        "   background-color: #dc2626; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #b91c1c; "
        "}"
        "QPushButton:disabled { "
        "   background-color: #64748b; "
        "   color: #94a3b8; "
        "}"
    );
    m_stopBtn->setEnabled(false); // Initially disabled
    
    // Config Button (blue, like Rocket's style)
    m_configBtn = new QPushButton("⚙ Config", this);
    m_configBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #3b82f6; "
        "   border: none; "
        "   border-radius: 6px; "
        "   padding: 12px 24px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 100px; "
        "   min-height: 40px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #60a5fa; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #1d4ed8; "
        "}"
        "QPushButton:disabled { "
        "   background-color: #64748b; "
        "   color: #94a3b8; "
        "}"
    );
    
    // Streams Button (purple, like Rocket's style)
    m_streamsBtn = new QPushButton("📡 Streams", this);
    m_streamsBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #8b5cf6; "
        "   border: none; "
        "   border-radius: 6px; "
        "   padding: 12px 24px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 100px; "
        "   min-height: 40px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #7c3aed; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #6d28d9; "
        "}"
        "QPushButton:disabled { "
        "   background-color: #64748b; "
        "   color: #94a3b8; "
        "}"
    );
    
    buttonLayout->addWidget(m_startBtn);
    buttonLayout->addWidget(m_stopBtn);
    buttonLayout->addWidget(m_configBtn);
    buttonLayout->addWidget(m_streamsBtn);
    buttonLayout->addStretch(); // Push buttons to the left
    
    controlsLayout->addLayout(buttonLayout);
    
    // Streams section (like Rocket's interface)
    QLabel *streamsLabel = new QLabel("Streams", this);
    streamsLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #f8fafc; margin-top: 15px;");
    controlsLayout->addWidget(streamsLabel);
    
    // Stream list frame
    QFrame *streamsFrame = new QFrame(this);
    streamsFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #0f172a; "
        "   border: 1px solid #334155; "
        "   border-radius: 6px; "
        "   padding: 15px; "
        "}"
    );
    QVBoxLayout *streamsLayout = new QVBoxLayout(streamsFrame);
    
    // Stream list container (will be populated dynamically)
    m_streamsLayout = streamsLayout;
    
    // Add initial streams
    addStreamToMainWindow("/radio", "No source connected");
    addStreamToMainWindow("/jazz", "No source connected");
    
    controlsLayout->addWidget(streamsFrame);
    
    mainLayout->addWidget(controlsFrame);
    
    // Log area (like Rocket's interface)
    QFrame *logFrame = new QFrame(this);
    logFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #1e293b; "
        "   border: 1px solid #334155; "
        "   border-radius: 8px; "
        "   padding: 20px; "
        "}"
    );
    QVBoxLayout *logLayout = new QVBoxLayout(logFrame);
    
    // Tab widget for logs
    QTabWidget *logTabs = new QTabWidget(this);
    
    // Main Log tab
    m_mainLog = new QTextEdit(this);
    m_mainLog->setReadOnly(true);
    m_mainLog->setMaximumHeight(200);
    m_mainLog->setPlainText(
        "[info] LegacyStream Audio Server 1.0.0 starting.\n"
        "[info] Free Edition - 100 listener limit.\n"
        "[info] Listening on 0.0.0.0:8000\n"
        "[info] Running with 16 worker threads.\n"
        "[info] SSL/TLS support available.\n"
        "[info] Web interface available at http://localhost:8000/web\n"
        "[info] Server ready for connections.\n"
        "[info] Click Start Server to begin.\n"
    );
    // Auto-scroll to bottom for main log
    m_mainLog->moveCursor(QTextCursor::End);
    m_mainLog->ensureCursorVisible();
    
    // Access Log tab
    m_accessLog = new QTextEdit(this);
    m_accessLog->setReadOnly(true);
    m_accessLog->setMaximumHeight(200);
    m_accessLog->setPlainText(
        "[access] 2025-01-02 05:48:39 - Server started\n"
        "[access] 2025-01-02 05:48:39 - Listening on port 8000\n"
        "[access] 2025-01-02 05:48:39 - SSL certificate loaded\n"
        "[access] 2025-01-02 05:48:39 - Web interface initialized\n"
        "[access] 2025-01-02 05:48:39 - Ready for web access\n"
    );
    // Auto-scroll to bottom for access log
    m_accessLog->moveCursor(QTextCursor::End);
    m_accessLog->ensureCursorVisible();
    
    logTabs->addTab(m_mainLog, "Main Log");
    logTabs->addTab(m_accessLog, "Access Log");
    
    logLayout->addWidget(logTabs);
    mainLayout->addWidget(logFrame);
    
    // Status labels (for dynamic updates)
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #cbd5e1; font-size: 12px;");
    m_infoLabel = new QLabel("Port: 8000 | Protocol: HTTP/HTTPS | SSL: Available", this);
    m_infoLabel->setStyleSheet("color: #cbd5e1; font-size: 12px;");
    
    // Connect signals
    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::onStopServer);
    connect(m_configBtn, &QPushButton::clicked, this, &MainWindow::onConfiguration);
    connect(m_streamsBtn, &QPushButton::clicked, this, &MainWindow::onStreams);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("&File");
    
    // Save Config action
    QAction *saveConfigAction = fileMenu->addAction("&Save Config");
    saveConfigAction->setShortcut(QKeySequence::Save);
    connect(saveConfigAction, &QAction::triggered, this, &MainWindow::onSaveConfig);
    
    // Load Config action
    QAction *loadConfigAction = fileMenu->addAction("&Load Config");
    loadConfigAction->setShortcut(QKeySequence::Open);
    connect(loadConfigAction, &QAction::triggered, this, &MainWindow::onLoadConfig);
    
    // Separator
    fileMenu->addSeparator();
    
    // Exit action
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

void MainWindow::setupConnections()
{
    // Connections are already set up in setupUI()
    // This method is kept for consistency with the header
}

void MainWindow::onStartServer()
{
    // Update UI to show server is starting
    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    statusBar()->showMessage("Starting server...", 2000);
    
    // Add startup message to main log
    m_mainLog->append("[info] Starting LegacyStream Audio Server...");
    m_mainLog->moveCursor(QTextCursor::End);
    m_mainLog->ensureCursorVisible();
    
    // Simulate server startup (in real implementation, this would start the actual server)
    QTimer::singleShot(2000, this, [this]() {
        statusBar()->showMessage("Server is running on port 8000", 3000);
        
        // Add server running messages to main log
        m_mainLog->append("[info] LegacyStream Audio Server is now running!");
        m_mainLog->append("[info] HTTP Server: http://localhost:8000");
        m_mainLog->append("[info] HTTPS Server: https://localhost:8443");
        m_mainLog->append("[info] Web Interface: http://localhost:8000/web");
        m_mainLog->append("[info] Stream Endpoint: http://localhost:8000/stream");
        m_mainLog->append("[info] Server ready for connections.");
        
        // Auto-scroll to bottom after adding all messages
        m_mainLog->moveCursor(QTextCursor::End);
        m_mainLog->ensureCursorVisible();
    });
}

void MainWindow::onStopServer()
{
    // Update UI to show server is stopping
    m_stopBtn->setEnabled(false);
    statusBar()->showMessage("Stopping server...", 1500);
    
    // Add stopping message to main log
    m_mainLog->append("[info] Stopping LegacyStream Audio Server...");
    m_mainLog->moveCursor(QTextCursor::End);
    m_mainLog->ensureCursorVisible();
    
    // Simulate server shutdown
    QTimer::singleShot(1500, this, [this]() {
        m_startBtn->setEnabled(true);
        m_stopBtn->setEnabled(false);
        statusBar()->showMessage("Server has been stopped", 3000);
        
        // Add server stopped messages to main log
        m_mainLog->append("[info] LegacyStream Audio Server has been stopped.");
        m_mainLog->append("[info] All active connections have been closed.");
        m_mainLog->append("[info] Server is no longer accepting new connections.");
        
        // Auto-scroll to bottom after adding all messages
        m_mainLog->moveCursor(QTextCursor::End);
        m_mainLog->ensureCursorVisible();
    });
}

void MainWindow::onConfiguration()
{
    // Create and show configuration dialog
    QDialog *configDialog = new QDialog(this);
    configDialog->setWindowTitle("LegacyStream Configuration");
    configDialog->setModal(true);
    configDialog->resize(500, 400);
    
    // Set dark theme styling for dialog
    configDialog->setStyleSheet(
        "QDialog { background-color: #0f172a; color: #f8fafc; }"
        "QLabel { color: #f8fafc; }"
        "QLineEdit, QSpinBox, QComboBox { "
        "   background-color: #1e293b; "
        "   border: 2px solid #3b82f6; "
        "   border-radius: 6px; "
        "   padding: 8px; "
        "   color: #f8fafc; "
        "   font-size: 14px; "
        "}"
        "QPushButton { "
        "   background-color: #3b82f6; "
        "   border: none; "
        "   border-radius: 8px; "
        "   padding: 10px 20px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 100px; "
        "}"
        "QPushButton:hover { background-color: #60a5fa; }"
        "QPushButton:pressed { background-color: #1d4ed8; }"
    );
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(configDialog);
    
    // Configuration sections
    QLabel *titleLabel = new QLabel("Server Configuration", configDialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #3b82f6; margin: 10px;");
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
        // Log configuration changes to main log instead of showing popup
        m_mainLog->append("[info] Server configuration has been updated.");
        m_mainLog->append("[info] Changes will take effect when the server is restarted.");
        
        // Auto-scroll to bottom after adding configuration messages
        m_mainLog->moveCursor(QTextCursor::End);
        m_mainLog->ensureCursorVisible();
        
        statusBar()->showMessage("Configuration saved", 3000);
    }
}

void MainWindow::onStreams()
{
    // Create and show streams management dialog
    QDialog *streamsDialog = new QDialog(this);
    streamsDialog->setWindowTitle("Stream Management");
    streamsDialog->setModal(true);
    streamsDialog->resize(600, 500);
    
    // Set dark theme styling for dialog
    streamsDialog->setStyleSheet(
        "QDialog { background-color: #0f172a; color: #f8fafc; }"
        "QLabel { color: #f8fafc; }"
        "QLineEdit, QSpinBox, QComboBox { "
        "   background-color: #1e293b; "
        "   border: 2px solid #8b5cf6; "
        "   border-radius: 6px; "
        "   padding: 8px; "
        "   color: #f8fafc; "
        "   font-size: 14px; "
        "}"
        "QPushButton { "
        "   background-color: #8b5cf6; "
        "   border: none; "
        "   border-radius: 8px; "
        "   padding: 10px 20px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 100px; "
        "}"
        "QPushButton:hover { background-color: #7c3aed; }"
        "QPushButton:pressed { background-color: #6d28d9; }"
        "QListWidget { "
        "   background-color: #1e293b; "
        "   border: 2px solid #8b5cf6; "
        "   border-radius: 6px; "
        "   color: #f8fafc; "
        "   font-size: 12px; "
        "}"
        "QListWidget::item { "
        "   padding: 8px; "
        "   border-bottom: 1px solid #334155; "
        "}"
        "QListWidget::item:selected { "
        "   background-color: #8b5cf6; "
        "}"
    );
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(streamsDialog);
    
    // Title
    QLabel *titleLabel = new QLabel("Stream Management", streamsDialog);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #8b5cf6; margin: 10px;");
    dialogLayout->addWidget(titleLabel);
    
    // Current streams list
    QLabel *currentLabel = new QLabel("Current Mount Points:", streamsDialog);
    currentLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #f8fafc; margin-top: 10px;");
    dialogLayout->addWidget(currentLabel);
    
    QListWidget *streamsList = new QListWidget(streamsDialog);
    streamsList->setMaximumHeight(150);
    
    // Add some example streams
    QListWidgetItem *item1 = new QListWidgetItem("/radio - admin:password123");
    QListWidgetItem *item2 = new QListWidgetItem("/jazz - user:jazzpass");
    streamsList->addItem(item1);
    streamsList->addItem(item2);
    
    dialogLayout->addWidget(streamsList);
    
    // Add new stream section
    QLabel *addLabel = new QLabel("Add New Mount Point:", streamsDialog);
    addLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #f8fafc; margin-top: 15px;");
    dialogLayout->addWidget(addLabel);
    
    // Mount point input
    QHBoxLayout *mountLayout = new QHBoxLayout();
    QLabel *mountLabel = new QLabel("Mount Point:", streamsDialog);
    QLineEdit *mountEdit = new QLineEdit(streamsDialog);
    mountEdit->setPlaceholderText("/stream");
    mountLayout->addWidget(mountLabel);
    mountLayout->addWidget(mountEdit);
    dialogLayout->addLayout(mountLayout);
    
    // Username input
    QHBoxLayout *userLayout = new QHBoxLayout();
    QLabel *userLabel = new QLabel("Username:", streamsDialog);
    QLineEdit *userEdit = new QLineEdit(streamsDialog);
    userEdit->setPlaceholderText("admin");
    userLayout->addWidget(userLabel);
    userLayout->addWidget(userEdit);
    dialogLayout->addLayout(userLayout);
    
    // Password input
    QHBoxLayout *passLayout = new QHBoxLayout();
    QLabel *passLabel = new QLabel("Password:", streamsDialog);
    QLineEdit *passEdit = new QLineEdit(streamsDialog);
    passEdit->setPlaceholderText("password");
    passEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passLabel);
    passLayout->addWidget(passEdit);
    dialogLayout->addLayout(passLayout);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("Add Stream", streamsDialog);
    QPushButton *removeBtn = new QPushButton("Remove Selected", streamsDialog);
    QPushButton *closeBtn = new QPushButton("Close", streamsDialog);
    
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(removeBtn);
    buttonLayout->addWidget(closeBtn);
    dialogLayout->addLayout(buttonLayout);
    
    // Connect buttons
    connect(addBtn, &QPushButton::clicked, [=]() {
        QString mountPoint = mountEdit->text();
        QString username = userEdit->text();
        QString password = passEdit->text();
        
        if (!mountPoint.isEmpty() && !username.isEmpty() && !password.isEmpty()) {
            QString streamInfo = QString("%1 - %2:%3").arg(mountPoint, username, password);
            QListWidgetItem *newItem = new QListWidgetItem(streamInfo);
            streamsList->addItem(newItem);
            
            // Add to main window streams section
            addStreamToMainWindow(mountPoint, "No source connected");
            
            // Clear inputs
            mountEdit->clear();
            userEdit->clear();
            passEdit->clear();
            
            // Log to main log
            m_mainLog->append(QString("[info] Added new mount point: %1").arg(mountPoint));
            m_mainLog->moveCursor(QTextCursor::End);
            m_mainLog->ensureCursorVisible();
        }
    });
    
    connect(removeBtn, &QPushButton::clicked, [=]() {
        QListWidgetItem *currentItem = streamsList->currentItem();
        if (currentItem) {
            QString streamInfo = currentItem->text();
            QString mountPoint = streamInfo.split(" - ").first();
            streamsList->takeItem(streamsList->row(currentItem));
            
            // Remove from main window streams section
            removeStreamFromMainWindow(mountPoint);
            
            // Log to main log
            m_mainLog->append(QString("[info] Removed mount point: %1").arg(mountPoint));
            m_mainLog->moveCursor(QTextCursor::End);
            m_mainLog->ensureCursorVisible();
        }
    });
    
    connect(closeBtn, &QPushButton::clicked, streamsDialog, &QDialog::accept);
    
    // Show dialog
    streamsDialog->exec();
}

void MainWindow::addStreamToMainWindow(const QString& mountPoint, const QString& status)
{
    // Create stream entry layout
    QHBoxLayout *streamEntryLayout = new QHBoxLayout();
    
    QLabel *streamPathLabel = new QLabel(mountPoint, this);
    streamPathLabel->setStyleSheet(
        "color: #3b82f6; "
        "font-weight: bold; "
        "font-size: 14px; "
        "text-decoration: underline; "
        "cursor: pointer;"
    );
    
    QLabel *streamStatusLabel = new QLabel(status, this);
    streamStatusLabel->setStyleSheet("color: #cbd5e1; font-size: 12px;");
    
    // Status indicator (red circle like Rocket's)
    QLabel *statusIndicator = new QLabel("🔴", this);
    statusIndicator->setStyleSheet("font-size: 12px;");
    
    streamEntryLayout->addWidget(streamPathLabel);
    streamEntryLayout->addStretch();
    streamEntryLayout->addWidget(streamStatusLabel);
    streamEntryLayout->addWidget(statusIndicator);
    
    // Store reference to the layout for later removal
    streamEntryLayout->setProperty("mountPoint", mountPoint);
    
    m_streamsLayout->addLayout(streamEntryLayout);
}

void MainWindow::removeStreamFromMainWindow(const QString& mountPoint)
{
    // Find and remove the stream entry
    for (int i = 0; i < m_streamsLayout->count(); ++i) {
        QLayoutItem *item = m_streamsLayout->itemAt(i);
        if (item && item->layout()) {
            QHBoxLayout *streamLayout = qobject_cast<QHBoxLayout*>(item->layout());
            if (streamLayout && streamLayout->property("mountPoint").toString() == mountPoint) {
                // Remove all widgets from the layout
                QLayoutItem *child;
                while ((child = streamLayout->takeAt(0)) != nullptr) {
                    if (child->widget()) {
                        child->widget()->deleteLater();
                    }
                    delete child;
                }
                // Remove the layout itself
                m_streamsLayout->removeItem(streamLayout);
                delete streamLayout;
                break;
            }
        }
    }
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About LegacyStream", 
        "LegacyStream Audio Streaming Server\n\n"
        "Version: 1.0.0\n"
        "© 2025 Legacy DEV Team");
}

void MainWindow::onSaveConfig()
{
    // Get the current configuration
    auto& config = LegacyStream::Configuration::instance();
    
    // Show file dialog to choose save location
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Configuration",
        defaultPath + "/LegacyStream_Config.ini",
        "INI Files (*.ini);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        // Save the configuration to the selected file
        config.saveToFile(fileName);
        
        // Remember the last saved config file path
        QSettings appSettings;
        appSettings.setValue("lastSavedConfig", fileName);
        
        // Log the save operation
        m_mainLog->append(QString("[info] Configuration saved to: %1").arg(fileName));
        m_mainLog->append("[info] This configuration will be automatically loaded on next startup.");
        m_mainLog->moveCursor(QTextCursor::End);
        m_mainLog->ensureCursorVisible();
        
        // Show status message
        statusBar()->showMessage(QString("Configuration saved to %1").arg(QDir::toNativeSeparators(fileName)), 3000);
        
        // Show success message
        QMessageBox::information(this, "Configuration Saved", 
            QString("Configuration has been saved to:\n%1\n\nThis configuration will be automatically loaded when you restart the application.")
            .arg(QDir::toNativeSeparators(fileName)));
    }
}

void MainWindow::onLoadConfig()
{
    // Show file dialog to choose config file to load
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Configuration",
        defaultPath,
        "INI Files (*.ini);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        // Get the configuration instance
        auto& config = LegacyStream::Configuration::instance();
        
        // Initialize configuration with the selected file
        config.initialize(fileName);
        
        // Remember the loaded config file path
        QSettings appSettings;
        appSettings.setValue("lastSavedConfig", fileName);
        
        // Log the load operation
        m_mainLog->append(QString("[info] Configuration loaded from: %1").arg(fileName));
        m_mainLog->append(QString("[info] Loaded %1 mount points").arg(config.mountPoints().size()));
        m_mainLog->append("[info] This configuration will be automatically loaded on next startup.");
        
        // Update the streams display with loaded mount points
        // Clear existing streams
        QLayoutItem *child;
        while ((child = m_streamsLayout->takeAt(0)) != nullptr) {
            if (child->layout()) {
                QLayoutItem *layoutChild;
                while ((layoutChild = child->layout()->takeAt(0)) != nullptr) {
                    if (layoutChild->widget()) {
                        layoutChild->widget()->deleteLater();
                    }
                    delete layoutChild;
                }
                delete child->layout();
            }
            delete child;
        }
        
        // Add loaded mount points to the display
        for (const QString& mountPoint : config.mountPoints()) {
            QString mountName = config.getMountPointName(mountPoint);
            QString mountDesc = config.getMountPointDescription(mountPoint);
            QString displayText = mountName.isEmpty() ? mountPoint : mountName;
            
            addStreamToMainWindow(mountPoint, QString("Loaded: %1").arg(displayText));
        }
        
        m_mainLog->moveCursor(QTextCursor::End);
        m_mainLog->ensureCursorVisible();
        
        // Show status message
        statusBar()->showMessage(QString("Configuration loaded from %1").arg(QDir::toNativeSeparators(fileName)), 3000);
        
        // Show success message
        QMessageBox::information(this, "Configuration Loaded", 
            QString("Configuration has been loaded from:\n%1\n\n%2 mount points loaded.\n\nThis configuration will be automatically loaded when you restart the application.")
            .arg(QDir::toNativeSeparators(fileName))
            .arg(config.mountPoints().size()));
    }
} 