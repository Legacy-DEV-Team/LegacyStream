#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QIcon>
#include <iostream>
#include "gui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("LegacyStream");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Legacy DEV Team");
    
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
    
    // Show welcome message
    QMessageBox::information(&mainWindow, "Welcome", 
        "LegacyStream Audio Streaming Server\n\n"
        "Version: 1.0.0\n"
        "Status: Ready to start streaming");
    
    mainWindow.show();
    
    return app.exec();
} 