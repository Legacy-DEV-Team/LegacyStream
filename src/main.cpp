#include <QApplication>
#include <QMainWindow>
#include <QIcon>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <iostream>
#include "gui/MainWindow.h"
#include "core/Configuration.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("LegacyStream");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Legacy DEV Team");
    
    // Initialize configuration with persistent storage
    QString configPath;
    QStringList args = app.arguments();
    
    // Check for custom config file path
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == "--config" && i + 1 < args.size()) {
            configPath = args[i + 1];
            break;
        }
    }
    
    // Use default config path if not specified
    if (configPath.isEmpty()) {
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(appDataPath);
        configPath = appDataPath + "/config.ini";
        
        // Check if there's a last saved config file to load
        QSettings appSettings;
        QString lastSavedConfig = appSettings.value("lastSavedConfig").toString();
        if (!lastSavedConfig.isEmpty() && QFile::exists(lastSavedConfig)) {
            qDebug() << "Loading last saved configuration from:" << lastSavedConfig;
            configPath = lastSavedConfig;
        }
    }
    
    qDebug() << "Using configuration file:" << configPath;
    
    // Initialize configuration
    auto& config = LegacyStream::Configuration::instance();
    config.initialize(configPath);
    
    qDebug() << "Configuration loaded with" << config.mountPoints().size() << "mount points";
    qDebug() << "HTTP Port:" << config.httpPort();
    qDebug() << "Theme:" << config.theme();
    
    // Set application icon
    QIcon appIcon(":/icons/app_icon.png");
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    } else {
        // Use default Qt icon if custom icon not found
        app.setWindowIcon(QIcon());
    }
    
    // Create main window
    MainWindow mainWindow;
    mainWindow.setWindowTitle("LegacyStream - Audio Streaming Server");
    mainWindow.resize(800, 600);
    
    // Apply saved window settings
    if (config.startMinimized()) {
        mainWindow.showMinimized();
    } else {
        mainWindow.show();
    }
    
    // Connect configuration changes to auto-save
    QObject::connect(&config, &LegacyStream::Configuration::configurationChanged, 
                    [&config]() {
                        qDebug() << "Configuration changed, saving...";
                        config.save();
                    });
    
    // Save configuration when application is about to quit
    QObject::connect(&app, &QApplication::aboutToQuit, 
                    [&config]() {
                        qDebug() << "Application shutting down, saving configuration...";
                        config.save();
                    });
    
    return app.exec();
} 