#include "filesystemtreeview.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QSortFilterProxyModel>
#include <QUrl>

void FileSystemTreeView::openInNewWindow() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString path = fileSystemModel->filePath(sourceIndex);
    emit openInNewWindowRequested(path);
}

void FileSystemTreeView::openInNewTab() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString filePath = fileSystemModel->filePath(sourceIndex);
    QFileInfo fileInfo(filePath);

    if (fileInfo.isFile()) {
        emit fileOpenInNewTabRequested(filePath);
    }
}

void FileSystemTreeView::openFile() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString filePath = fileSystemModel->filePath(sourceIndex);
    QFileInfo fileInfo(filePath);

    if (fileInfo.isFile()) {
        QString suffix = fileInfo.suffix().toLower();
        bool isMarkdownFile =
            (suffix == "md" || suffix == "markdown" || suffix == "txt");

        if (isMarkdownFile) {
            // Open markdown/txt files in TreeMk editor
            emit fileDoubleClicked(filePath);
        } else {
            // Open other files with system default application
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    }
}

void FileSystemTreeView::openFileWith() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString filePath = fileSystemModel->filePath(sourceIndex);
    QFileInfo fileInfo(filePath);

    if (!fileInfo.isFile()) {
        return;
    }

#ifdef Q_OS_WIN
    // Windows: use the "Open With" dialog
    QProcess::startDetached(
        "rundll32.exe", QStringList()
                            << "shell32.dll,OpenAs_RunDLL"
                            << QDir::toNativeSeparators(filePath));

#elif defined(Q_OS_LINUX)
    // Linux: use gio (available on GNOME/GTK environments)
    QProcess::startDetached("gio",
                            QStringList() << "open" << "--ask" << filePath);

#endif
}

void FileSystemTreeView::openInFileExplorer() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString filePath = fileSystemModel->filePath(sourceIndex);
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        QMessageBox::warning(
            this, tr("File Not Found"),
            tr("The file or folder no longer exists:\n%1").arg(filePath));
        return;
    }

    QString dirPath = fileInfo.isDir() ? fileInfo.absoluteFilePath()
                                       : fileInfo.absolutePath();

#ifdef Q_OS_LINUX
    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");

    // KDE Plasma - use Dolphin
    if (desktopEnv.contains("KDE", Qt::CaseInsensitive)) {
        if (QProcess::execute("which", QStringList() << "dolphin") == 0) {
            QProcess::startDetached("dolphin",
                                    QStringList() << "--select" << filePath);
            return;
        }
    }

    // GNOME - use Nautilus
    if (desktopEnv.contains("GNOME", Qt::CaseInsensitive)) {
        if (QProcess::execute("which", QStringList() << "nautilus") == 0) {
            QProcess::startDetached("nautilus",
                                    QStringList() << "--select" << filePath);
            return;
        }
    }

    // Try generic xdg-open to open the directory
    if (QProcess::execute("which", QStringList() << "xdg-open") == 0) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
        return;
    }

    // Fallback: just open the directory
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));

#elif defined(Q_OS_WIN)
    // Windows: use explorer.exe with /select parameter
    QProcess::startDetached("explorer.exe",
                            QStringList() << "/select,"
                                          << QDir::toNativeSeparators(filePath));

#elif defined(Q_OS_MAC)
    // macOS: use open -R to reveal in Finder
    QProcess::startDetached("open", QStringList() << "-R" << filePath);

#else
    // Generic fallback for other systems
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
#endif
}
