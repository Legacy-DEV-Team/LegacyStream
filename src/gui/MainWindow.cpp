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
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Title
    QLabel *titleLabel = new QLabel("LegacyStream Audio Server", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    mainLayout->addWidget(titleLabel);
    
    // Status
    QLabel *statusLabel = new QLabel("Status: Ready", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *startBtn = new QPushButton("Start Server", this);
    QPushButton *stopBtn = new QPushButton("Stop Server", this);
    QPushButton *configBtn = new QPushButton("Configuration", this);
    
    buttonLayout->addWidget(startBtn);
    buttonLayout->addWidget(stopBtn);
    buttonLayout->addWidget(configBtn);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartServer);
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::onStopServer);
    connect(configBtn, &QPushButton::clicked, this, &MainWindow::onConfiguration);
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
        "© 2024 Legacy DEV Team");
} 