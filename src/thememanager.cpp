#include "thememanager.h"
#include "colorpalette.h"

#include <QApplication>
#include <QColor>
#include <QFile>
#include <QPalette>
#include <QStyleHints>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance() {
    if (!s_instance) {
        s_instance = new ThemeManager(qApp);
    }
    return s_instance;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent),
      m_currentAppTheme(AppTheme::System),
      m_currentEditorScheme(EditorColorScheme::Auto),
      m_currentPreviewScheme(PreviewColorScheme::Auto),
      m_systemIsDark(false) {
    detectSystemTheme();
    // Connect to system theme changes
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QStyleHints* hints = QApplication::styleHints();
    connect(hints, &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme colorScheme) {
                bool wasDark = m_systemIsDark;
                m_systemIsDark = (colorScheme == Qt::ColorScheme::Dark);
                // Only reapply if system theme is active and the theme actually
                // changed
                if (m_currentAppTheme == AppTheme::System &&
                    wasDark != m_systemIsDark) {
                    applyAppTheme();
                    emit themeChanged();
                }
            });
#endif
}

ThemeManager::~ThemeManager() {}

void ThemeManager::detectSystemTheme() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    QStyleHints* hints = QApplication::styleHints();
    m_systemIsDark = (hints->colorScheme() == Qt::ColorScheme::Dark);
#else
    m_systemIsDark = false;
#endif
}

void ThemeManager::setAppTheme(AppTheme theme) {
    if (m_currentAppTheme != theme) {
        m_currentAppTheme = theme;
        applyAppTheme();
        emit themeChanged();
    }
}

void ThemeManager::setAppTheme(const QString& themeName) {
    if (themeName == "system") {
        setAppTheme(AppTheme::System);
    } else if (themeName == "light") {
        setAppTheme(AppTheme::Light);
    } else if (themeName == "dark") {
        setAppTheme(AppTheme::Dark);
    }
}

void ThemeManager::setEditorColorScheme(EditorColorScheme scheme) {
    if (m_currentEditorScheme != scheme) {
        m_currentEditorScheme = scheme;
        emit editorColorSchemeChanged();
    }
}

void ThemeManager::setEditorColorScheme(const QString& schemeName) {
    if (schemeName == "auto") {
        setEditorColorScheme(EditorColorScheme::Auto);
    } else if (schemeName == "light") {
        setEditorColorScheme(EditorColorScheme::Light);
    } else if (schemeName == "dark") {
        setEditorColorScheme(EditorColorScheme::Dark);
    } else if (schemeName == "solarized-light") {
        setEditorColorScheme(EditorColorScheme::SolarizedLight);
    } else if (schemeName == "solarized-dark") {
        setEditorColorScheme(EditorColorScheme::SolarizedDark);
    }
}

void ThemeManager::applyAppTheme() {
    switch (m_currentAppTheme) {
        case AppTheme::System:
            detectSystemTheme();
            if (m_systemIsDark) {
                applyDarkTheme();
            } else {
                applyLightTheme();
            }
            break;
        case AppTheme::Light:
            applyLightTheme();
            break;
        case AppTheme::Dark:
            applyDarkTheme();
            break;
    }
}

void ThemeManager::applyLightTheme() {
    const auto& colors = ColorPalette::getLightTheme();
    QPalette palette;
    palette.setColor(QPalette::Window, colors.uiWindow);
    palette.setColor(QPalette::WindowText, colors.uiWindowText);
    palette.setColor(QPalette::Base, colors.uiBase);
    palette.setColor(QPalette::AlternateBase, colors.uiAlternateBase);
    palette.setColor(QPalette::ToolTipBase, colors.uiToolTipBase);
    palette.setColor(QPalette::ToolTipText, colors.uiToolTipText);
    palette.setColor(QPalette::Text, colors.text);
    palette.setColor(QPalette::Button, colors.uiButton);
    palette.setColor(QPalette::ButtonText, colors.uiButtonText);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, colors.link);
    palette.setColor(QPalette::Highlight, colors.uiHighlight);
    palette.setColor(QPalette::HighlightedText, colors.uiHighlightedText);
    qApp->setPalette(palette);
    qApp->setStyleSheet("");
}

void ThemeManager::applyDarkTheme() {
    const auto& colors = ColorPalette::getDarkTheme();
    QPalette palette;
    palette.setColor(QPalette::Window, colors.uiWindow);
    palette.setColor(QPalette::WindowText, colors.uiWindowText);
    palette.setColor(QPalette::Base, colors.uiBase);
    palette.setColor(QPalette::AlternateBase, colors.uiAlternateBase);
    palette.setColor(QPalette::ToolTipBase, colors.uiToolTipBase);
    palette.setColor(QPalette::ToolTipText, colors.uiToolTipText);
    palette.setColor(QPalette::Text, colors.text);
    palette.setColor(QPalette::Button, colors.uiButton);
    palette.setColor(QPalette::ButtonText, colors.uiButtonText);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, colors.link);
    palette.setColor(QPalette::Highlight, colors.uiHighlight);
    palette.setColor(QPalette::HighlightedText, colors.uiHighlightedText);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, colors.uiButtonTextDisabled);
    palette.setColor(QPalette::Disabled, QPalette::Text, colors.uiButtonTextDisabled);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, colors.uiButtonTextDisabled);
    qApp->setPalette(palette);
    
    QString styleSheet = QString(R"(
        QToolTip {
            color: %1;
            background-color: %2;
            border: 1px solid %3;
        }
        QMenuBar {
            background-color: %4;
            color: %1;
        }
        QMenuBar::item:selected {
            background-color: %5;
        }
        QMenu {
            background-color: %4;
            color: %1;
            border: 1px solid %6;
        }
        QMenu::item:selected {
            background-color: %5;
        }
        QToolBar {
            background-color: %4;
            border: none;
        }
    )").arg(ColorPalette::toHexString(colors.uiWindowText))
       .arg(ColorPalette::toHexString(colors.uiToolTipBase))
       .arg(ColorPalette::toHexString(colors.border))
       .arg(ColorPalette::toHexString(colors.uiWindow))
       .arg(ColorPalette::toHexString(colors.uiHighlight))
       .arg(ColorPalette::toHexString(colors.borderTable));

    qApp->setStyleSheet(styleSheet);
}

QPalette ThemeManager::getEditorPalette() const {
    QPalette palette;
    EditorColorScheme effectiveScheme = m_currentEditorScheme;
    if (effectiveScheme == EditorColorScheme::Auto) {
        bool isDark = false;
        if (m_currentAppTheme == AppTheme::System) {
            isDark = m_systemIsDark;
        } else if (m_currentAppTheme == AppTheme::Dark) {
            isDark = true;
        }
        effectiveScheme =
            isDark ? EditorColorScheme::Dark : EditorColorScheme::Light;
    }
    switch (effectiveScheme) {
        case EditorColorScheme::Auto:
            break;
        case EditorColorScheme::Light:
            {
                const auto& colors = ColorPalette::getLightTheme();
                palette.setColor(QPalette::Base, colors.background);
                palette.setColor(QPalette::Text, colors.text);
            }
            break;
        case EditorColorScheme::Dark:
            {
                const auto& colors = ColorPalette::getDarkTheme();
                palette.setColor(QPalette::Base, colors.background);
                palette.setColor(QPalette::Text, colors.text);
            }
            break;
        case EditorColorScheme::SolarizedLight:
            palette.setColor(QPalette::Base, QColor(253, 246, 227));
            palette.setColor(QPalette::Text, QColor(101, 123, 131));
            break;
        case EditorColorScheme::SolarizedDark:
            palette.setColor(QPalette::Base, QColor(0, 43, 54));
            palette.setColor(QPalette::Text, QColor(131, 148, 150));
            break;
    }
    return palette;
}

QString ThemeManager::getEditorStyleSheet() const {
    QString styleSheet;
    EditorColorScheme effectiveScheme = m_currentEditorScheme;
    if (effectiveScheme == EditorColorScheme::Auto) {
        bool isDark = false;
        if (m_currentAppTheme == AppTheme::System) {
            isDark = m_systemIsDark;
        } else if (m_currentAppTheme == AppTheme::Dark) {
            isDark = true;
        }
        effectiveScheme =
            isDark ? EditorColorScheme::Dark : EditorColorScheme::Light;
    }
    switch (effectiveScheme) {
        case EditorColorScheme::Auto:
            break;
        case EditorColorScheme::Light:
            {
                const auto& colors = ColorPalette::getLightTheme();
                styleSheet = QString(R"(
                    QPlainTextEdit {
                        background-color: %1;
                        color: %2;
                        selection-background-color: %3;
                        selection-color: %4;
                    }
                )").arg(ColorPalette::toHexString(colors.background))
                   .arg(ColorPalette::toHexString(colors.text))
                   .arg(ColorPalette::toHexString(colors.backgroundSelection))
                   .arg(ColorPalette::toHexString(colors.textSelection));
            }
            break;
        case EditorColorScheme::Dark:
            {
                const auto& colors = ColorPalette::getDarkTheme();
                styleSheet = QString(R"(
                    QPlainTextEdit {
                        background-color: %1;
                        color: %2;
                        selection-background-color: %3;
                        selection-color: %4;
                    }
                )").arg(ColorPalette::toHexString(colors.background))
                   .arg(ColorPalette::toHexString(colors.text))
                   .arg(ColorPalette::toHexString(colors.backgroundSelection))
                   .arg(ColorPalette::toHexString(colors.textSelection));
            }
            break;
        case EditorColorScheme::SolarizedLight:
            styleSheet = R"(
            QPlainTextEdit {
                background-color: #fdf6e3;
                color: #657b83;
                selection-background-color: #eee8d5;
                selection-color: #586e75;
            }
        )";
            break;
        case EditorColorScheme::SolarizedDark:
            styleSheet = R"(
            QPlainTextEdit {
                background-color: #002b36;
                color: #839496;
                selection-background-color: #073642;
                selection-color: #93a1a1;
            }
        )";
            break;
    }
    return styleSheet;
}

void ThemeManager::setPreviewColorScheme(PreviewColorScheme scheme) {
    if (m_currentPreviewScheme != scheme) {
        m_currentPreviewScheme = scheme;
        emit previewColorSchemeChanged();
    }
}

void ThemeManager::setPreviewColorScheme(const QString& schemeName) {
    if (schemeName == "auto") {
        setPreviewColorScheme(PreviewColorScheme::Auto);
    } else if (schemeName == "light") {
        setPreviewColorScheme(PreviewColorScheme::Light);
    } else if (schemeName == "dark") {
        setPreviewColorScheme(PreviewColorScheme::Dark);
    }
}

QString ThemeManager::getPreviewStyleSheet() const {
    PreviewColorScheme effectiveScheme = m_currentPreviewScheme;
    // If Auto, determine scheme based on app theme
    if (effectiveScheme == PreviewColorScheme::Auto) {
        bool isDark = false;
        if (m_currentAppTheme == AppTheme::System) {
            isDark = m_systemIsDark;
        } else if (m_currentAppTheme == AppTheme::Dark) {
            isDark = true;
        }
        effectiveScheme =
            isDark ? PreviewColorScheme::Dark : PreviewColorScheme::Light;
    }
    QString cssFileName;
    switch (effectiveScheme) {
        case PreviewColorScheme::Auto:
            // Should not reach here
            break;
        case PreviewColorScheme::Light:
            cssFileName = ":/css/theme-light.css";
            break;
        case PreviewColorScheme::Dark:
            cssFileName = ":/css/theme-dark.css";
            break;
    }
    QFile cssFile(cssFileName);
    if (!cssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to load theme CSS:" << cssFileName;
        return "";
    }
    QString styleSheet = QString::fromUtf8(cssFile.readAll());
    cssFile.close();
    return styleSheet;
}

QString ThemeManager::getResolvedEditorColorSchemeName() const {
    EditorColorScheme effectiveScheme = m_currentEditorScheme;
    // If Auto, determine scheme based on app theme
    if (effectiveScheme == EditorColorScheme::Auto) {
        bool isDark = false;
        if (m_currentAppTheme == AppTheme::System) {
            isDark = m_systemIsDark;
        } else if (m_currentAppTheme == AppTheme::Dark) {
            isDark = true;
        }
        effectiveScheme =
            isDark ? EditorColorScheme::Dark : EditorColorScheme::Light;
    }
    switch (effectiveScheme) {
        case EditorColorScheme::Auto:
            return "light";  // Fallback
        case EditorColorScheme::Light:
            return "light";
        case EditorColorScheme::Dark:
            return "dark";
        case EditorColorScheme::SolarizedLight:
            return "solarized-light";
        case EditorColorScheme::SolarizedDark:
            return "solarized-dark";
    }

    return "light";  // Fallback
}

QString ThemeManager::getResolvedPreviewColorSchemeName() const {
    PreviewColorScheme effectiveScheme = m_currentPreviewScheme;
    // If Auto, determine scheme based on app theme
    if (effectiveScheme == PreviewColorScheme::Auto) {
        bool isDark = false;
        if (m_currentAppTheme == AppTheme::System) {
            isDark = m_systemIsDark;
        } else if (m_currentAppTheme == AppTheme::Dark) {
            isDark = true;
        }
        effectiveScheme =
            isDark ? PreviewColorScheme::Dark : PreviewColorScheme::Light;
    }
    switch (effectiveScheme) {
        case PreviewColorScheme::Auto:
            return "light";  // Fallback
        case PreviewColorScheme::Light:
            return "light";
        case PreviewColorScheme::Dark:
            return "dark";
    }
    return "light";  // Fallback
}
