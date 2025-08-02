#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartServer();
    void onStopServer();
    void onConfiguration();
    void onAbout();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();

    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QLabel *m_titleLabel;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_configButton;
    QLabel *m_statusLabel;
    
    // Menu components
    QMenuBar *m_menuBar;
    QMenu *m_fileMenu;
    QMenu *m_helpMenu;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    
    // Status bar components
    QStatusBar *m_statusBar;
    QLabel *m_serverStatusLabel;
    QLabel *m_listenerCountLabel;
    QLabel *m_uptimeLabel;
    QLabel *m_bandwidthLabel;
};

#endif // MAINWINDOW_H 