#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("LegacyStream");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Legacy DEV Team");
    
    // Create main window
    QMainWindow mainWindow;
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