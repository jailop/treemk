#include "defs.h"
#include "mainwindow.h"
#include "thememanager.h"
#include <QApplication>
#include <QIcon>
#include <QSettings>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(APP_LABEL);
  app.setOrganizationName(APP_LABEL);
  app.setApplicationVersion(APP_VERSION);
  if (QIcon::themeName().isEmpty()) {
    QIcon::setThemeName("breeze");
  }
  QSettings settings(APP_LABEL, APP_LABEL);
  QString appTheme = settings.value("appearance/appTheme", "system").toString();
  ThemeManager::instance()->setAppTheme(appTheme);
  MainWindow window;
  window.show();
  return app.exec();
}
