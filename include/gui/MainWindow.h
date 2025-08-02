#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QSplitter>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QDockWidget>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include <QSettings>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QPalette>
#include <QFont>
#include <QIcon>
#include <QKeySequence>
#include <QShortcut>
#include <QToolTip>
#include <QWhatsThis>
#include <QAccessible>
#include <QAccessibleWidget>
#include <QScreen>
#include <QWindow>

namespace LegacyStream {

class StreamManager;
class Configuration;
class PerformanceManager;
class SecurityManager;
class AudioProcessor;

/**
 * @brief Theme configuration
 */
struct ThemeConfig
{
    QString name;
    QString styleSheet;
    QPalette palette;
    QFont font;
    QIcon iconSet;
    bool isDark = false;
    QString description;
};

/**
 * @brief Accessibility settings
 */
struct AccessibilitySettings
{
    bool highContrast = false;
    bool largeFonts = false;
    bool screenReader = false;
    bool keyboardNavigation = true;
    bool toolTips = true;
    bool soundEffects = false;
    int animationSpeed = 1; // 0=off, 1=normal, 2=fast
    QString focusIndicator = "blue";
};

/**
 * @brief Main window for LegacyStream GUI
 * 
 * Provides a modern, accessible, and themeable user interface
 * with advanced features for stream management and monitoring.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Theme management
    void setTheme(const QString& themeName);
    QStringList getAvailableThemes() const;
    void createCustomTheme(const QString& name, const ThemeConfig& config);
    void deleteCustomTheme(const QString& name);

    // Accessibility
    void setAccessibilitySettings(const AccessibilitySettings& settings);
    AccessibilitySettings getAccessibilitySettings() const;
    void enableHighContrast(bool enabled);
    void enableLargeFonts(bool enabled);
    void enableScreenReader(bool enabled);

    // Window management
    void showMinimized();
    void showMaximized();
    void showFullScreen();
    void restoreWindow();
    void centerOnScreen();

    // System tray
    void setupSystemTray();
    void showSystemTrayIcon(bool show);
    void setSystemTrayIcon(const QIcon& icon);
    void showSystemTrayMessage(const QString& title, const QString& message);

    // Status and information
    void updateStatusBar(const QString& message);
    void updateProgressBar(int value);
    void showNotification(const QString& title, const QString& message);

signals:
    void themeChanged(const QString& themeName);
    void accessibilityChanged(const AccessibilitySettings& settings);
    void windowStateChanged(Qt::WindowStates states);
    void systemTrayActivated(QSystemTrayIcon::ActivationReason reason);

public slots:
    void onStreamManagerUpdate();
    void onPerformanceManagerUpdate();
    void onSecurityManagerUpdate();
    void onAudioProcessorUpdate();
    void onConfigurationChanged();

protected:
    // Event handling
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onFileMenuTriggered(QAction* action);
    void onEditMenuTriggered(QAction* action);
    void onViewMenuTriggered(QAction* action);
    void onToolsMenuTriggered(QAction* action);
    void onHelpMenuTriggered(QAction* action);
    void onThemeMenuTriggered(QAction* action);
    void onAccessibilityMenuTriggered(QAction* action);
    void onSystemTrayMenuTriggered(QAction* action);
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onWindowStateChanged(Qt::WindowStates states);
    void onUpdateTimer();

private:
    // UI creation
    void createMenus();
    void createToolBars();
    void createDockWidgets();
    void createStatusBar();
    void createCentralWidget();
    void createStreamPanel();
    void createConfigurationPanel();
    void createPerformancePanel();
    void createSecurityPanel();
    void createAudioPanel();
    void createLogPanel();

    // Theme management
    void loadThemes();
    void applyTheme(const ThemeConfig& theme);
    void createDefaultThemes();
    void saveCustomTheme(const QString& name, const ThemeConfig& config);

    // Accessibility
    void setupAccessibility();
    void applyAccessibilitySettings();
    void setupKeyboardShortcuts();
    void setupFocusNavigation();
    void setupScreenReader();

    // System tray
    void createSystemTrayMenu();
    void updateSystemTrayIcon();

    // Utility functions
    void setupConnections();
    void updateWindowTitle();
    void updateWindowIcon();
    QString getWindowTitle() const;
    QIcon getWindowIcon() const;

    // Dependencies
    StreamManager* m_streamManager = nullptr;
    Configuration* m_configuration = nullptr;
    PerformanceManager* m_performanceManager = nullptr;
    SecurityManager* m_securityManager = nullptr;
    AudioProcessor* m_audioProcessor = nullptr;

    // UI components
    QWidget* m_centralWidget = nullptr;
    QTabWidget* m_mainTabWidget = nullptr;
    QSplitter* m_mainSplitter = nullptr;
    QDockWidget* m_streamDock = nullptr;
    QDockWidget* m_configDock = nullptr;
    QDockWidget* m_performanceDock = nullptr;
    QDockWidget* m_securityDock = nullptr;
    QDockWidget* m_audioDock = nullptr;
    QDockWidget* m_logDock = nullptr;

    // Menus
    QMenuBar* m_menuBar = nullptr;
    QMenu* m_fileMenu = nullptr;
    QMenu* m_editMenu = nullptr;
    QMenu* m_viewMenu = nullptr;
    QMenu* m_toolsMenu = nullptr;
    QMenu* m_helpMenu = nullptr;
    QMenu* m_themeMenu = nullptr;
    QMenu* m_accessibilityMenu = nullptr;

    // Toolbars
    QToolBar* m_mainToolBar = nullptr;
    QToolBar* m_streamToolBar = nullptr;
    QToolBar* m_configToolBar = nullptr;

    // Status bar
    QStatusBar* m_statusBar = nullptr;
    QLabel* m_statusLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;

    // System tray
    QSystemTrayIcon* m_systemTrayIcon = nullptr;
    QMenu* m_systemTrayMenu = nullptr;
    bool m_systemTrayEnabled = false;

    // Theme management
    QMap<QString, ThemeConfig> m_themes;
    QString m_currentTheme;
    ThemeConfig m_defaultTheme;

    // Accessibility
    AccessibilitySettings m_accessibilitySettings;
    QMap<QWidget*, QAccessibleWidget*> m_accessibleWidgets;

    // Window state
    QTimer* m_updateTimer = nullptr;
    Qt::WindowStates m_lastWindowState;
    QRect m_lastGeometry;
    bool m_wasMaximized = false;

    // Settings
    QSettings* m_settings = nullptr;
    bool m_autoSave = true;
    int m_autoSaveInterval = 30000; // 30 seconds

    Q_DISABLE_COPY(MainWindow)
};

} // namespace LegacyStream

#endif // MAINWINDOW_H 