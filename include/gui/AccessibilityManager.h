#ifndef ACCESSIBILITYMANAGER_H
#define ACCESSIBILITYMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QWidget>
#include <QAccessible>
#include <QAccessibleWidget>
#include <QAccessibleInterface>
#include <QKeySequence>
#include <QShortcut>
#include <QTimer>
#include <QSound>
#include <QTextToSpeech>

namespace LegacyStream {

/**
 * @brief Advanced accessibility settings
 */
struct AdvancedAccessibilitySettings
{
    // Visual accessibility
    bool highContrast = false;
    bool largeFonts = false;
    bool boldFonts = false;
    bool increasedSpacing = false;
    bool focusIndicator = true;
    QString focusIndicatorColor = "blue";
    int focusIndicatorWidth = 2;
    
    // Keyboard navigation
    bool keyboardNavigation = true;
    bool tabNavigation = true;
    bool arrowNavigation = true;
    bool shortcutKeys = true;
    bool stickyKeys = false;
    bool slowKeys = false;
    int keyRepeatDelay = 500;
    int keyRepeatRate = 30;
    
    // Screen reader support
    bool screenReader = false;
    bool announceChanges = true;
    bool announceFocus = true;
    bool announceErrors = true;
    bool announceStatus = true;
    QString screenReaderVoice = "default";
    int screenReaderRate = 0;
    int screenReaderVolume = 100;
    
    // Audio accessibility
    bool soundEffects = false;
    bool audioFeedback = false;
    bool audioAlerts = false;
    int audioVolume = 50;
    QString audioDevice = "default";
    
    // Cognitive accessibility
    bool simplifiedInterface = false;
    bool reduceMotion = false;
    bool autoComplete = true;
    bool spellCheck = true;
    bool grammarCheck = false;
    
    // Motor accessibility
    bool mouseKeys = false;
    bool clickAssist = false;
    bool hoverDelay = false;
    int hoverDelayTime = 1000;
    bool stickyMouse = false;
    
    // Language and localization
    QString language = "en";
    QString region = "US";
    bool rightToLeft = false;
    bool showTooltips = true;
    bool showContextHelp = true;
};

/**
 * @brief Accessibility event
 */
struct AccessibilityEvent
{
    QString type; // "focus", "change", "error", "status", "alert"
    QString message;
    QString widget;
    QString context;
    QDateTime timestamp;
    int priority; // 1=low, 2=medium, 3=high, 4=critical
};

/**
 * @brief Accessibility manager for advanced accessibility features
 * 
 * Provides comprehensive accessibility support including screen reader
 * integration, keyboard navigation, audio feedback, and assistive technologies.
 */
class AccessibilityManager : public QObject
{
    Q_OBJECT

public:
    explicit AccessibilityManager(QObject* parent = nullptr);
    ~AccessibilityManager();

    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    void loadSettings();
    void saveSettings();

    // Settings management
    void setAccessibilitySettings(const AdvancedAccessibilitySettings& settings);
    AdvancedAccessibilitySettings getAccessibilitySettings() const;
    void updateSetting(const QString& key, const QVariant& value);

    // Screen reader support
    void enableScreenReader(bool enabled);
    void announceText(const QString& text, int priority = 2);
    void announceFocus(QWidget* widget);
    void announceChange(const QString& change);
    void announceError(const QString& error);
    void announceStatus(const QString& status);

    // Keyboard navigation
    void setupKeyboardNavigation();
    void enableKeyboardShortcuts(bool enabled);
    void addKeyboardShortcut(const QString& action, const QKeySequence& shortcut);
    void removeKeyboardShortcut(const QString& action);
    void setupFocusNavigation(QWidget* parent);

    // Audio feedback
    void enableAudioFeedback(bool enabled);
    void playSound(const QString& soundName);
    void playAlert(const QString& alertType);
    void setAudioVolume(int volume);

    // Visual accessibility
    void applyHighContrast(bool enabled);
    void applyLargeFonts(bool enabled);
    void applyFocusIndicator(bool enabled);
    void updateFocusIndicator(const QString& color, int width);

    // Widget accessibility
    void makeWidgetAccessible(QWidget* widget, const QString& name, const QString& description);
    void updateWidgetAccessibility(QWidget* widget, const QString& name, const QString& description);
    void removeWidgetAccessibility(QWidget* widget);

    // Event handling
    void handleAccessibilityEvent(const AccessibilityEvent& event);
    void logAccessibilityEvent(const QString& type, const QString& message);

    // Utility functions
    bool isAccessible() const;
    QString getAccessibilityInfo() const;
    void generateAccessibilityReport(const QString& filePath);

signals:
    void accessibilitySettingsChanged(const AdvancedAccessibilitySettings& settings);
    void screenReaderAnnouncement(const QString& text);
    void audioFeedbackPlayed(const QString& sound);
    void accessibilityEvent(const AccessibilityEvent& event);
    void focusChanged(QWidget* widget);
    void errorAnnounced(const QString& error);

public slots:
    void onWidgetFocusChanged(QWidget* oldWidget, QWidget* newWidget);
    void onWidgetStateChanged(QWidget* widget);
    void onErrorOccurred(const QString& error);
    void onStatusChanged(const QString& status);

private slots:
    void onUpdateTimer();
    void onScreenReaderTimer();
    void onAudioFeedbackTimer();

private:
    // Core functionality
    void setupScreenReader();
    void setupAudioFeedback();
    void setupVisualAccessibility();
    void setupKeyboardAccessibility();
    void setupEventFiltering();

    // Screen reader functions
    void initializeScreenReader();
    void cleanupScreenReader();
    void queueAnnouncement(const QString& text, int priority);
    void processAnnouncementQueue();

    // Audio feedback functions
    void initializeAudioFeedback();
    void cleanupAudioFeedback();
    void loadSoundEffects();
    void playSoundEffect(const QString& soundName);

    // Visual accessibility functions
    void applyContrastSettings();
    void applyFontSettings();
    void applyFocusSettings();
    void updateWidgetStyles();

    // Keyboard accessibility functions
    void setupStickyKeys();
    void setupSlowKeys();
    void setupMouseKeys();
    void handleKeyboardEvent(QKeyEvent* event);

    // Utility functions
    QString generateAccessibleName(QWidget* widget) const;
    QString generateAccessibleDescription(QWidget* widget) const;
    bool shouldAnnounceWidget(QWidget* widget) const;
    int calculatePriority(const QString& eventType) const;

    // Settings
    AdvancedAccessibilitySettings m_settings;
    QSettings* m_settingsStorage = nullptr;

    // Screen reader
    QTextToSpeech* m_screenReader = nullptr;
    QTimer* m_screenReaderTimer = nullptr;
    QList<QPair<QString, int>> m_announcementQueue;
    bool m_screenReaderEnabled = false;

    // Audio feedback
    QMap<QString, QString> m_soundEffects;
    QTimer* m_audioFeedbackTimer = nullptr;
    bool m_audioFeedbackEnabled = false;

    // Visual accessibility
    QMap<QWidget*, QAccessibleWidget*> m_accessibleWidgets;
    QString m_focusIndicatorColor;
    int m_focusIndicatorWidth;

    // Keyboard accessibility
    QMap<QString, QShortcut*> m_keyboardShortcuts;
    bool m_stickyKeysEnabled = false;
    bool m_slowKeysEnabled = false;
    bool m_mouseKeysEnabled = false;

    // Event tracking
    QTimer* m_updateTimer = nullptr;
    QList<AccessibilityEvent> m_eventHistory;
    int m_maxEventHistory = 1000;

    // State management
    bool m_isInitialized = false;
    QWidget* m_lastFocusedWidget = nullptr;
    QString m_lastAnnouncement;

    Q_DISABLE_COPY(AccessibilityManager)
};

} // namespace LegacyStream

#endif // ACCESSIBILITYMANAGER_H 