#include "filesystemtreeview.h"

#include <QDir>
#include <QFileInfo>
#include <QSortFilterProxyModel>

void FileSystemTreeView::onSelectionChanged(const QModelIndex& current,
                                             const QModelIndex& previous) {
    Q_UNUSED(previous);
    if (!current.isValid()) {
        return;
    }
    QModelIndex sourceIndex = proxyModel->mapToSource(current);
    QString filePath = fileSystemModel->filePath(sourceIndex);
    QFileInfo fileInfo(filePath);
    if (fileInfo.isFile()) {
        QString suffix = fileInfo.suffix().toLower();
        bool isMarkdownFile =
            (suffix == "md" || suffix == "markdown" || suffix == "txt");

        if (isMarkdownFile) {
            if (!watchedFilePath.isEmpty() && watchedFilePath != filePath) {
                if (fileSystemWatcher->files().contains(watchedFilePath)) {
                    fileSystemWatcher->removePath(watchedFilePath);
                }
            }
            watchedFilePath = filePath;
            if (!fileSystemWatcher->files().contains(filePath)) {
                fileSystemWatcher->addPath(filePath);
            }

            emit fileSelected(filePath);
        }
    }
}

void FileSystemTreeView::onDirectoryChanged(const QString& path) {
    Q_UNUSED(path);

    // QFileSystemModel automatically handles directory changes and updates the
    // view. No additional action needed here.
}

void FileSystemTreeView::onFileChanged(const QString& path) {
    if (path == fileSavingPath) {
        fileSavingPath.clear();
        if (!fileSystemWatcher->files().contains(path)) {
            fileSystemWatcher->addPath(path);
        }
        return;
    }
    if (path == watchedFilePath) {
        if (QFile::exists(path)) {
            emit fileModifiedExternally(path);
            if (!fileSystemWatcher->files().contains(path)) {
                fileSystemWatcher->addPath(path);
            }
        } else {
            watchedFilePath.clear();
        }
    }
}

void FileSystemTreeView::onItemRenamed(const QModelIndex& topLeft,
                                       const QModelIndex& bottomRight) {
    // Only process column 0 (filename) changes
    if (topLeft.column() != 0) {
        return;
    }

    if (!renameOldPath.isEmpty()) {
        QString newPath = fileSystemModel->filePath(topLeft);

        // Only emit if paths are different and the new path exists
        if (newPath != renameOldPath && QFile::exists(newPath) &&
            !QFile::exists(renameOldPath)) {
            emit fileRenamed(renameOldPath, newPath);
            renameOldPath.clear();
        } else if (newPath == renameOldPath) {
            // User cancelled the rename, clear the old path
            renameOldPath.clear();
        }
    }
}
