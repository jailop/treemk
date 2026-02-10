#include "defs.h"
#include "mainwindow.h"
#include "thememanager.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
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

  // Parse command-line arguments
  QString startupPath;
  QString startupFile;
  if (argc > 1) {
    QString arg = QString::fromLocal8Bit(argv[1]);
    QFileInfo fileInfo(arg);

    if (fileInfo.isDir()) {
      // Argument is a directory
      startupPath = fileInfo.absoluteFilePath();
    } else if (fileInfo.isFile() && fileInfo.suffix().toLower() == "md") {
      // Argument is a markdown file
      startupPath = fileInfo.absolutePath();
      startupFile = fileInfo.absoluteFilePath();
    } else if (!QFileInfo::exists(arg)) {
      // Argument is ".": use current working directory
      if (arg == ".") {
        startupPath = QDir::currentPath();
      } else {
        // Non-existent path provided
        startupPath = QDir::currentPath();
      }
    }
  }

  MainWindow window;
  window.setStartupArguments(startupPath, startupFile);
  window.initializeSettings();
  window.show();
  return app.exec();
}
