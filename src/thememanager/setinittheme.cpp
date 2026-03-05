#include "appinit.h"
#include "thememanager.h"

void setThemeManager(QSettings& settings) {
    QString appTheme =
        settings.value("appearance/appTheme", "system").toString();
    ThemeManager::instance()->setAppTheme(appTheme);
    QString editorScheme =
        settings.value("appearance/editorColorScheme", "auto").toString();
    ThemeManager::instance()->setEditorColorScheme(editorScheme);
}
