#include <QApplication>
#include <QSettings>
#include "mainwindow.h"
#include "thememanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("TreeMk");
    app.setOrganizationName("TreeMk");
    app.setApplicationVersion("0.1.0");
    
    // Initialize theme manager and apply saved theme
    QSettings settings("TreeMk", "TreeMk");
    QString appTheme = settings.value("appearance/appTheme", "system").toString();
    ThemeManager::instance()->setAppTheme(appTheme);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
