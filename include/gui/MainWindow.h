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
#include <QTextEdit>

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
    void onStreams();
    void onAbout();
    void onSaveConfig();
    void onLoadConfig();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupConnections();
    void addStreamToMainWindow(const QString& mountPoint, const QString& status);
    void removeStreamFromMainWindow(const QString& mountPoint);

    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QLabel *m_titleLabel;
    QPushButton *m_startBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_configBtn;
    QPushButton *m_streamsBtn;
    QLabel *m_statusLabel;
    QLabel *m_infoLabel;
    QTextEdit *m_mainLog;
    QTextEdit *m_accessLog;
    QVBoxLayout *m_streamsLayout;
    
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