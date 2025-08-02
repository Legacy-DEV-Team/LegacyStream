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
    
    // Set modern dark theme
    centralWidget->setStyleSheet(
        "QWidget { background-color: #2b2b2b; color: #ffffff; }"
        "QLabel { color: #ffffff; }"
        "QPushButton { "
        "   background-color: #4a90e2; "
        "   border: none; "
        "   border-radius: 8px; "
        "   padding: 12px 24px; "
        "   font-size: 14px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 120px; "
        "   min-height: 40px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #5ba0f2; "
        "   transform: translateY(-2px); "
        "}"
        "QPushButton:pressed { "
        "   background-color: #3a80d2; "
        "}"
        "QPushButton:disabled { "
        "   background-color: #666666; "
        "   color: #aaaaaa; "
        "}"
    );
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    
    // Header with icon and title
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    // App icon in header
    QLabel *iconLabel = new QLabel(this);
    QIcon appIcon(":/icons/app_icon.png");
    if (!appIcon.isNull()) {
        iconLabel->setPixmap(appIcon.pixmap(48, 48));
    }
    headerLayout->addWidget(iconLabel);
    
    // Title with modern styling
    QLabel *titleLabel = new QLabel("LegacyStream Audio Server", this);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setStyleSheet(
        "font-size: 28px; "
        "font-weight: bold; "
        "color: #4a90e2; "
        "margin-left: 15px; "
        "text-shadow: 2px 2px 4px rgba(0,0,0,0.3);"
    );
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    
    mainLayout->addLayout(headerLayout);
    
    // Status section with modern design
    QFrame *statusFrame = new QFrame(this);
    statusFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #3a3a3a; "
        "   border-radius: 12px; "
        "   border: 2px solid #4a90e2; "
        "   padding: 20px; "
        "}"
    );
    QVBoxLayout *statusLayout = new QVBoxLayout(statusFrame);
    
    QLabel *statusTitle = new QLabel("Server Status", this);
    statusTitle->setAlignment(Qt::AlignCenter);
    statusTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #4a90e2; margin-bottom: 10px;");
    statusLayout->addWidget(statusTitle);
    
    m_statusLabel = new QLabel("🟢 Ready to Start", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-size: 16px; color: #00ff00; font-weight: bold;");
    statusLayout->addWidget(m_statusLabel);
    
    mainLayout->addWidget(statusFrame);
    
    // Control buttons with modern design
    QFrame *buttonFrame = new QFrame(this);
    buttonFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #3a3a3a; "
        "   border-radius: 12px; "
        "   padding: 20px; "
        "}"
    );
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonFrame);
    buttonLayout->setSpacing(15);
    
    // Start Server Button
    m_startBtn = new QPushButton("🚀 Start Server", this);
    m_startBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #28a745; "
        "   border: none; "
        "   border-radius: 10px; "
        "   padding: 15px 30px; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 150px; "
        "   min-height: 50px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #34ce57; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #1e7e34; "
        "}"
    );
    
    // Stop Server Button
    m_stopBtn = new QPushButton("⏹️ Stop Server", this);
    m_stopBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #dc3545; "
        "   border: none; "
        "   border-radius: 10px; "
        "   padding: 15px 30px; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 150px; "
        "   min-height: 50px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #e74c3c; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #c82333; "
        "}"
    );
    m_stopBtn->setEnabled(false); // Initially disabled
    
    // Configuration Button
    m_configBtn = new QPushButton("⚙️ Configuration", this);
    m_configBtn->setStyleSheet(
        "QPushButton { "
        "   background-color: #6c757d; "
        "   border: none; "
        "   border-radius: 10px; "
        "   padding: 15px 30px; "
        "   font-size: 16px; "
        "   font-weight: bold; "
        "   color: white; "
        "   min-width: 150px; "
        "   min-height: 50px; "
        "}"
        "QPushButton:hover { "
        "   background-color: #7a8288; "
        "}"
        "QPushButton:pressed { "
        "   background-color: #5a6268; "
        "}"
    );
    
    buttonLayout->addWidget(m_startBtn);
    buttonLayout->addWidget(m_stopBtn);
    buttonLayout->addWidget(m_configBtn);
    
    mainLayout->addWidget(buttonFrame);
    
    // Server info section
    QFrame *infoFrame = new QFrame(this);
    infoFrame->setStyleSheet(
        "QFrame { "
        "   background-color: #3a3a3a; "
        "   border-radius: 12px; "
        "   padding: 20px; "
        "}"
    );
    QVBoxLayout *infoLayout = new QVBoxLayout(infoFrame);
    
    QLabel *infoTitle = new QLabel("Server Information", this);
    infoTitle->setAlignment(Qt::AlignCenter);
    infoTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #4a90e2; margin-bottom: 10px;");
    infoLayout->addWidget(infoTitle);
    
    m_infoLabel = new QLabel("Port: 8000 | Protocol: HTTP/HTTPS | SSL: Available", this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet("font-size: 14px; color: #cccccc;");
    infoLayout->addWidget(m_infoLabel);
    
    mainLayout->addWidget(infoFrame);
    
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
    statusBar()->showMessage("Starting server...");
    QMessageBox::information(this, "Server", "Server started successfully!");
}

void MainWindow::onStopServer()
{
    statusBar()->showMessage("Stopping server...");
    QMessageBox::information(this, "Server", "Server stopped successfully!");
}

void MainWindow::onConfiguration()
{
    QMessageBox::information(this, "Configuration", "Configuration dialog would open here.");
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About LegacyStream", 
        "LegacyStream Audio Streaming Server\n\n"
        "Version: 1.0.0\n"
        "© 2025 Legacy DEV Team");
} 