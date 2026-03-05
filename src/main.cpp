#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QSettings>
#include "defs.h"
#include "fileutils.h"
#include "mainwindow.h"
#include "managers/windowmanager.h"
#include "appinit.h"

namespace {

/**
 * Local struct to hold the starting file system location information.
 * It contains the path to the starting directory and an optional file
 * to open if a markdown file is provided as an argument.
 */
struct FileSysStart {
    QString path;
    QString file;
};

/**
 * Determines the starting file system location based on the command line
 * arguments. If a directory is provided, it will be used as the starting
 * path. If a markdown file is provided, its parent directory will be used
 * as the starting path, and the file itself will be opened. If no valid
 * argument is provided, the current working directory will be used as the
 * starting path.
 */
FileSysStart getStartFSLocation(int argc, char* argv[]) {
    FileSysStart result;
    if (argc > 1) {
        QString arg = QString::fromLocal8Bit(argv[1]);
        QFileInfo fileInfo(arg);
        if (fileInfo.isDir()) {
            result.path = fileInfo.absoluteFilePath();
        } else if (fileInfo.isFile() &&
                FileUtils::isMarkdownFile(fileInfo.absoluteFilePath())) {
            result.path = fileInfo.absolutePath();
            result.file = fileInfo.absoluteFilePath();
        } else if (!QFileInfo::exists(arg)) {
            if (arg == "~")
                result.path = QDir::homePath();
            else if (arg.startsWith("~/"))
                result.path = QDir::homePath() + arg.mid(1);
            else if (arg == "..")
                result.path = QDir::current().absoluteFilePath("..");
            else
                result.path = QDir::currentPath();
        }
    }
    return result;
}

/**
 * Custom message handler to filter out specific warnings and errors that are
 * not relevant to the user. This helps to reduce noise in the application
 * logs and improve the user experience by hiding non-critical messages.
 */
void messageHandler(QtMsgType type, const QMessageLogContext& context,
                    const QString& msg) {
    if (msg.contains("inotify_add_watch") &&
        msg.contains("Permission denied")) {
        return;
    }
    if (msg.contains("Cannot open file ':/icons/ai.svg'")) {
        return;
    }
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(),
                    file, context.line, function);
            break;
        case QtInfoMsg:
            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(),
                    file, context.line, function);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(),
                    file, context.line, function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(),
                    file, context.line, function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(),
                    file, context.line, function);
            break;
    }
}

} // namespace

/**
 * Application entry point. It sets up the message handler, configures
 * the application settings, and starts the main window. It also
 * determines the starting file system location based on the command
 * line arguments and passes it to the main window for initialization.
 */
int main(int argc, char* argv[]) {
    qInstallMessageHandler(messageHandler);
    QApplication app(argc, argv);
    app.setApplicationName(APP_LABEL);
    app.setOrganizationName(APP_LABEL);
    app.setApplicationVersion(APP_VERSION);
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName("breeze");
    }
    QSettings settings(APP_LABEL, APP_LABEL);
    setThemeManager(settings);
    setAIProviders(settings);
    FileSysStart fsStart = getStartFSLocation(argc, argv);
    MainWindow mainWindow;
    mainWindow.setStartupArguments(fsStart.path, fsStart.file);
    mainWindow.initializeSettings();
    mainWindow.show();
    return app.exec();
}
