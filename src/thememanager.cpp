#include "thememanager.h"
#include <QApplication>
#include <QStyleHints>
#include <QPalette>
#include <QColor>

ThemeManager* ThemeManager::s_instance = nullptr;

ThemeManager* ThemeManager::instance()
{
    if (!s_instance) {
        s_instance = new ThemeManager(qApp);
    }
    return s_instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_currentAppTheme(AppTheme::System)
    , m_currentEditorScheme(EditorColorScheme::Light)
    , m_systemIsDark(false)
{
    detectSystemTheme();
}

ThemeManager::~ThemeManager()
{
}

void ThemeManager::detectSystemTheme()
{
    // Try to detect system theme from Qt style hints
    QStyleHints *hints = QApplication::styleHints();
    m_systemIsDark = (hints->colorScheme() == Qt::ColorScheme::Dark);
}

void ThemeManager::setAppTheme(AppTheme theme)
{
    if (m_currentAppTheme != theme) {
        m_currentAppTheme = theme;
        applyAppTheme();
        emit themeChanged();
    }
}

void ThemeManager::setAppTheme(const QString &themeName)
{
    if (themeName == "system") {
        setAppTheme(AppTheme::System);
    } else if (themeName == "light") {
        setAppTheme(AppTheme::Light);
    } else if (themeName == "dark") {
        setAppTheme(AppTheme::Dark);
    }
}

void ThemeManager::setEditorColorScheme(EditorColorScheme scheme)
{
    if (m_currentEditorScheme != scheme) {
        m_currentEditorScheme = scheme;
        emit editorColorSchemeChanged();
    }
}

void ThemeManager::setEditorColorScheme(const QString &schemeName)
{
    if (schemeName == "light") {
        setEditorColorScheme(EditorColorScheme::Light);
    } else if (schemeName == "dark") {
        setEditorColorScheme(EditorColorScheme::Dark);
    } else if (schemeName == "solarized-light") {
        setEditorColorScheme(EditorColorScheme::SolarizedLight);
    } else if (schemeName == "solarized-dark") {
        setEditorColorScheme(EditorColorScheme::SolarizedDark);
    }
}

void ThemeManager::applyAppTheme()
{
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

void ThemeManager::applyLightTheme()
{
    QPalette palette;
    
    // Light theme colors
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, QColor(0, 0, 0));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(245, 245, 245));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
    palette.setColor(QPalette::Text, QColor(0, 0, 0));
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    
    qApp->setPalette(palette);
    qApp->setStyleSheet("");
}

void ThemeManager::applyDarkTheme()
{
    QPalette palette;
    
    // Dark theme colors
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, QColor(220, 220, 220));
    palette.setColor(QPalette::Base, QColor(35, 35, 35));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, QColor(25, 25, 25));
    palette.setColor(QPalette::ToolTipText, QColor(220, 220, 220));
    palette.setColor(QPalette::Text, QColor(220, 220, 220));
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::ButtonText, QColor(220, 220, 220));
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    
    qApp->setPalette(palette);
    
    // Additional stylesheet for dark theme widgets
    QString styleSheet = R"(
        QToolTip {
            color: #dcdcdc;
            background-color: #191919;
            border: 1px solid #767676;
        }
        QMenuBar {
            background-color: #353535;
            color: #dcdcdc;
        }
        QMenuBar::item:selected {
            background-color: #2a82da;
        }
        QMenu {
            background-color: #353535;
            color: #dcdcdc;
            border: 1px solid #555555;
        }
        QMenu::item:selected {
            background-color: #2a82da;
        }
        QToolBar {
            background-color: #353535;
            border: none;
        }
    )";
    
    qApp->setStyleSheet(styleSheet);
}

QPalette ThemeManager::getEditorPalette() const
{
    QPalette palette;
    
    switch (m_currentEditorScheme) {
    case EditorColorScheme::Light:
        palette.setColor(QPalette::Base, QColor(255, 255, 255));
        palette.setColor(QPalette::Text, QColor(0, 0, 0));
        break;
        
    case EditorColorScheme::Dark:
        palette.setColor(QPalette::Base, QColor(30, 30, 30));
        palette.setColor(QPalette::Text, QColor(220, 220, 220));
        break;
        
    case EditorColorScheme::SolarizedLight:
        palette.setColor(QPalette::Base, QColor(253, 246, 227)); // #fdf6e3
        palette.setColor(QPalette::Text, QColor(101, 123, 131)); // #657b83
        break;
        
    case EditorColorScheme::SolarizedDark:
        palette.setColor(QPalette::Base, QColor(0, 43, 54)); // #002b36
        palette.setColor(QPalette::Text, QColor(131, 148, 150)); // #839496
        break;
    }
    
    return palette;
}

QString ThemeManager::getEditorStyleSheet() const
{
    QString styleSheet;
    
    switch (m_currentEditorScheme) {
    case EditorColorScheme::Light:
        styleSheet = R"(
            QPlainTextEdit {
                background-color: #ffffff;
                color: #000000;
                selection-background-color: #add6ff;
                selection-color: #000000;
            }
        )";
        break;
        
    case EditorColorScheme::Dark:
        styleSheet = R"(
            QPlainTextEdit {
                background-color: #1e1e1e;
                color: #dcdcdc;
                selection-background-color: #264f78;
                selection-color: #ffffff;
            }
        )";
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
