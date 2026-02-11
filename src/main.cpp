#include "defs.h"
#include "mainwindow.h"
#include "thememanager.h"
#include "managers/windowmanager.h"
#include "logic/aiprovider.h"
#include "logic/ollamaprovider.h"
#include "logic/systemprompts.h"
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QSettings>

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  // Filter out inotify permission warnings
  if (msg.contains("inotify_add_watch") && msg.contains("Permission denied")) {
    return;
  }
  
  // Filter out missing ai.svg icon warnings (theme icon fallback)
  if (msg.contains("Cannot open file ':/icons/ai.svg'")) {
    return;
  }
  
  // Pass through other messages to default handler
  QByteArray localMsg = msg.toLocal8Bit();
  const char *file = context.file ? context.file : "";
  const char *function = context.function ? context.function : "";
  switch (type) {
  case QtDebugMsg:
    fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
    break;
  case QtInfoMsg:
    fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
    break;
  case QtWarningMsg:
    fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
    break;
  case QtCriticalMsg:
    fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
    break;
  case QtFatalMsg:
    fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
    break;
  }
}

int main(int argc, char *argv[]) {
  // Install message handler to filter inotify warnings
  qInstallMessageHandler(messageHandler);
  
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

  // Initialize AI providers
  QString ollamaHost = qEnvironmentVariable("OLLAMA_HOST");
  if (ollamaHost.isEmpty()) {
    ollamaHost = settings.value("ai/ollama/endpoint", "http://localhost:11434").toString();
  }
  
  OllamaProvider *ollamaProvider = new OllamaProvider();
  ollamaProvider->setEndpoint(ollamaHost);
  
  QString ollamaModel = settings.value("ai/ollama/model", "llama3.2").toString();
  ollamaProvider->setModel(ollamaModel);
  
  int timeout = settings.value("ai/ollama/timeout", 60).toInt();
  ollamaProvider->setTimeout(timeout);
  
  AIProviderManager::instance()->registerProvider("ollama", ollamaProvider);
  
  QString activeProvider = settings.value("ai/provider", "ollama").toString();
  AIProviderManager::instance()->setActiveProvider(activeProvider);
  
  SystemPrompts::instance()->loadFromSettings();

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

  // Create first window via WindowManager
  WindowManager::instance()->createWindow(startupPath, startupFile);
  
  return app.exec();
}
