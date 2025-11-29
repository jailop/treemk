#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QPalette>

class QApplication;

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class AppTheme {
        System,
        Light,
        Dark
    };

    enum class EditorColorScheme {
        Auto,
        Light,
        Dark,
        SolarizedLight,
        SolarizedDark
    };

    enum class PreviewColorScheme {
        Auto,
        Light,
        Dark
    };

    static ThemeManager* instance();
    
    void setAppTheme(AppTheme theme);
    void setAppTheme(const QString &themeName);
    void setEditorColorScheme(EditorColorScheme scheme);
    void setEditorColorScheme(const QString &schemeName);
    void setPreviewColorScheme(PreviewColorScheme scheme);
    void setPreviewColorScheme(const QString &schemeName);
    
    AppTheme currentAppTheme() const { return m_currentAppTheme; }
    EditorColorScheme currentEditorColorScheme() const { return m_currentEditorScheme; }
    PreviewColorScheme currentPreviewColorScheme() const { return m_currentPreviewScheme; }
    
    QString getResolvedEditorColorSchemeName() const;
    QString getResolvedPreviewColorSchemeName() const;
    
    QPalette getEditorPalette() const;
    QString getEditorStyleSheet() const;
    QString getPreviewStyleSheet() const;
    
signals:
    void themeChanged();
    void editorColorSchemeChanged();
    void previewColorSchemeChanged();

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager();
    
    void applyAppTheme();
    void applyLightTheme();
    void applyDarkTheme();
    void detectSystemTheme();
    
    static ThemeManager* s_instance;
    AppTheme m_currentAppTheme;
    EditorColorScheme m_currentEditorScheme;
    PreviewColorScheme m_currentPreviewScheme;
    bool m_systemIsDark;
};

#endif // THEMEMANAGER_H
