#pragma once

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTimer>
#include <memory>

QT_BEGIN_NAMESPACE
class QLabel;
class QProgressBar;
class QPushButton;
class QTableWidget;
class QTreeWidget;
class QTextEdit;
class QGroupBox;
class QStackedWidget;
QT_END_NAMESPACE

namespace LegacyStream {

class ServerManager;

namespace GUI {

class OverviewWidget;
class StreamsWidget;
class ListenersWidget;
class CodecsWidget;
class RelaysWidget;
class SSLWidget;
class SettingsWidget;
class LogsWidget;
class StatsWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void onServerStarted();
    void onServerStopped();
    void onServerError(const QString& error);
    void updateStats();
    void toggleServer();
    void showAbout();
    void showPreferences();
    void exportLogs();
    void importConfig();
    void exportConfig();
    void checkForUpdates();
    
    // System tray
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void showMainWindow();
    void minimizeToTray();
    
    // Tab management
    void onTabChanged(int index);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupSystemTray();
    void setupTabs();
    void connectSignals();
    
    void updateWindowTitle();
    void updateServerStatus();
    void setServerControlsEnabled(bool enabled);
    
    // Central widgets
    QTabWidget* m_tabWidget;
    
    // Tab widgets
    OverviewWidget* m_overviewWidget;
    StreamsWidget* m_streamsWidget;
    ListenersWidget* m_listenersWidget;
    CodecsWidget* m_codecsWidget;
    RelaysWidget* m_relaysWidget;
    SSLWidget* m_sslWidget;
    SettingsWidget* m_settingsWidget;
    LogsWidget* m_logsWidget;
    StatsWidget* m_statsWidget;
    
    // Menu bar
    QMenu* m_fileMenu;
    QMenu* m_serverMenu;
    QMenu* m_toolsMenu;
    QMenu* m_helpMenu;
    
    // Actions
    QAction* m_startServerAction;
    QAction* m_stopServerAction;
    QAction* m_restartServerAction;
    QAction* m_preferencesAction;
    QAction* m_exportLogsAction;
    QAction* m_importConfigAction;
    QAction* m_exportConfigAction;
    QAction* m_aboutAction;
    QAction* m_exitAction;
    QAction* m_minimizeAction;
    QAction* m_checkUpdatesAction;
    
    // Status bar
    QLabel* m_statusLabel;
    QLabel* m_serverStatusLabel;
    QLabel* m_listenersLabel;
    QLabel* m_streamsLabel;
    QProgressBar* m_cpuUsageBar;
    QProgressBar* m_memoryUsageBar;
    
    // System tray
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    
    // Timers
    QTimer* m_statsUpdateTimer;
    
    // Server manager reference
    ServerManager* m_serverManager;
    
    // State
    bool m_isClosing = false;
    bool m_isMinimizedToTray = false;
};

} // namespace GUI
} // namespace LegacyStream