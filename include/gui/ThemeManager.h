#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QPalette>
#include <QFont>
#include <QIcon>
#include <QStyle>
#include <QApplication>

namespace LegacyStream {

/**
 * @brief Advanced theme configuration
 */
struct AdvancedThemeConfig
{
    QString name;
    QString description;
    QString author;
    QString version;
    QString styleSheet;
    QPalette palette;
    QFont font;
    QIcon iconSet;
    bool isDark = false;
    bool isHighContrast = false;
    bool isCustom = false;
    
    // Color scheme
    QColor primaryColor;
    QColor secondaryColor;
    QColor accentColor;
    QColor backgroundColor;
    QColor textColor;
    QColor borderColor;
    
    // Advanced styling
    int borderRadius = 4;
    int borderWidth = 1;
    QString borderStyle = "solid";
    int padding = 8;
    int margin = 4;
    
    // Animation settings
    bool enableAnimations = true;
    int animationDuration = 200;
    QString animationEasing = "easeInOut";
};

/**
 * @brief Theme manager for advanced theme management
 * 
 * Provides comprehensive theme management with customization,
 * import/export capabilities, and advanced styling options.
 */
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager();

    // Theme management
    bool loadThemes();
    void saveThemes();
    QStringList getAvailableThemes() const;
    AdvancedThemeConfig getTheme(const QString& name) const;
    bool setTheme(const QString& name);
    QString getCurrentTheme() const;

    // Theme creation and customization
    void createTheme(const AdvancedThemeConfig& config);
    void updateTheme(const QString& name, const AdvancedThemeConfig& config);
    void deleteTheme(const QString& name);
    void duplicateTheme(const QString& sourceName, const QString& newName);

    // Import/Export
    bool exportTheme(const QString& name, const QString& filePath);
    bool importTheme(const QString& filePath);
    QString exportThemeToJson(const QString& name) const;
    bool importThemeFromJson(const QString& jsonData);

    // Advanced features
    void createColorScheme(const QString& name, const QColor& primary, const QColor& secondary);
    void applyColorScheme(const QString& name);
    void generateThemeFromImage(const QString& imagePath, const QString& themeName);
    void createGradientTheme(const QString& name, const QColor& startColor, const QColor& endColor);

    // Utility functions
    QColor getContrastColor(const QColor& background) const;
    double getColorContrast(const QColor& color1, const QColor& color2) const;
    bool isColorAccessible(const QColor& foreground, const QColor& background) const;
    QString generateCSS(const AdvancedThemeConfig& config) const;

signals:
    void themeChanged(const QString& themeName);
    void themeCreated(const QString& themeName);
    void themeDeleted(const QString& themeName);
    void themeImported(const QString& themeName);
    void themeExported(const QString& themeName);

private:
    // Theme storage
    QMap<QString, AdvancedThemeConfig> m_themes;
    QString m_currentTheme;

    // Default themes
    void createDefaultThemes();
    void createDarkTheme();
    void createLightTheme();
    void createHighContrastTheme();
    void createProfessionalTheme();
    void createModernTheme();

    // Utility functions
    QPalette createPalette(const QColor& primary, const QColor& secondary, bool isDark) const;
    QFont createFont(const QString& family, int size, bool isBold = false) const;
    QString generateStylesheet(const AdvancedThemeConfig& config) const;
    QColor adjustColor(const QColor& color, int lightness, int saturation = 0) const;
};

} // namespace LegacyStream

#endif // THEMEMANAGER_H 