#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>

#include "gui/MainWindow.h"
#include "core/ServerManager.h"
#include "core/Logger.h"
#include "core/Configuration.h"

Q_LOGGING_CATEGORY(main, "main")

void setupLogging()
{
    // Create logs directory
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);
    
    // Initialize logger
    LegacyStream::Logger::instance().initialize(logDir + "/legacystream.log");
}

void setupConfiguration()
{
    // Create config directory
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    
    // Initialize configuration
    LegacyStream::Configuration::instance().initialize(configDir + "/config.ini");
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    QApplication::setApplicationName("LegacyStream");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setApplicationDisplayName("LegacyStream Audio Server");
    QApplication::setOrganizationName("LegacyStream");
    QApplication::setOrganizationDomain("legacystream.com");
    
    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("High-Performance Audio Streaming Server");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption configOption(QStringList() << "c" << "config",
        "Configuration file path", "file");
    parser.addOption(configOption);
    
    QCommandLineOption daemonOption(QStringList() << "d" << "daemon",
        "Run as daemon (no GUI)");
    parser.addOption(daemonOption);
    
    QCommandLineOption portOption(QStringList() << "p" << "port",
        "HTTP port", "port", "8000");
    parser.addOption(portOption);
    
    QCommandLineOption sslPortOption(QStringList() << "s" << "ssl-port",
        "HTTPS port", "port", "8443");
    parser.addOption(sslPortOption);
    
    parser.process(app);
    
    // Setup logging and configuration
    setupLogging();
    setupConfiguration();
    
    qCInfo(main) << "Starting LegacyStream Audio Server v" << QApplication::applicationVersion();
    
    // Create splash screen
    QPixmap pixmap(400, 300);
    pixmap.fill(Qt::darkBlue);
    QSplashScreen splash(pixmap);
    splash.show();
    splash.showMessage("Loading LegacyStream...", Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    
    app.processEvents();
    
    // Initialize server manager
    auto& serverManager = LegacyStream::ServerManager::instance();
    
    // Set ports from command line
    if (parser.isSet(portOption)) {
        bool ok;
        int port = parser.value(portOption).toInt(&ok);
        if (ok && port > 0 && port < 65536) {
            LegacyStream::Configuration::instance().setHttpPort(port);
        }
    }
    
    if (parser.isSet(sslPortOption)) {
        bool ok;
        int port = parser.value(sslPortOption).toInt(&ok);
        if (ok && port > 0 && port < 65536) {
            LegacyStream::Configuration::instance().setHttpsPort(port);
        }
    }
    
    // Check if running as daemon
    if (parser.isSet(daemonOption)) {
        qCInfo(main) << "Running in daemon mode";
        splash.finish(nullptr);
        
        // Initialize server without GUI
        if (!serverManager.initialize()) {
            qCCritical(main) << "Failed to initialize server";
            return 1;
        }
        
        return app.exec();
    }
    
    // GUI mode
    splash.showMessage("Initializing GUI...", Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    app.processEvents();
    
    // Set application style
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Apply dark theme
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Create main window
    LegacyStream::GUI::MainWindow mainWindow;
    
    splash.showMessage("Starting server...", Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
    app.processEvents();
    
    // Initialize server
    if (!serverManager.initialize()) {
        qCCritical(main) << "Failed to initialize server";
        splash.finish(&mainWindow);
        QMessageBox::critical(&mainWindow, "Error", 
            "Failed to initialize server. Please check the logs for more information.");
        return 1;
    }
    
    // Show main window
    QTimer::singleShot(2000, [&]() {
        splash.finish(&mainWindow);
        mainWindow.show();
    });
    
    return app.exec();
}