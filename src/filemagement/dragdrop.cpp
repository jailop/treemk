#include "filesystemtreeview.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QUrl>

QString FileSystemTreeView::dropTargetDir(const QPoint& pos) const {
    QModelIndex index = indexAt(pos);
    if (!index.isValid()) {
        return currentRootPath;
    }
    QFileInfo info(
        fileSystemModel->filePath(proxyModel->mapToSource(index)));
    return info.isDir() ? info.absoluteFilePath() : QString();
}

void FileSystemTreeView::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }
    QTreeView::dragEnterEvent(event);
}

void FileSystemTreeView::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasUrls()) {
        if (dropTargetDir(event->position().toPoint()).isEmpty()) {
            event->ignore();
        } else {
            event->acceptProposedAction();
        }
        return;
    }
    QTreeView::dragMoveEvent(event);
}

void FileSystemTreeView::dropEvent(QDropEvent* event) {
    if (!event->mimeData()->hasUrls()) {
        QTreeView::dropEvent(event);
        return;
    }

    QString targetDir = dropTargetDir(event->position().toPoint());
    if (targetDir.isEmpty()) {
        event->ignore();
        return;
    }

    const bool copy = (event->dropAction() == Qt::CopyAction);
    bool changed = false;

    for (const QUrl& url : event->mimeData()->urls()) {
        QString src = url.toLocalFile();
        if (src.isEmpty()) {
            continue;
        }
        QFileInfo srcInfo(src);
        QString dest = QDir(targetDir).filePath(srcInfo.fileName());

        if (QDir(srcInfo.absolutePath()) == QDir(targetDir)) {
            continue;
        }
        if (!copy && srcInfo.isDir() &&
            QDir(targetDir).absolutePath().startsWith(
                QDir(src).absolutePath())) {
            continue;
        }
        if (QFile::exists(dest) || QDir(dest).exists()) {
            QMessageBox::warning(
                this, tr("Error"),
                tr("An item with this name already exists: %1")
                    .arg(srcInfo.fileName()));
            continue;
        }

        bool success;
        if (copy) {
            success = srcInfo.isDir() ? copyDirectory(src, dest)
                                      : QFile::copy(src, dest);
        } else {
            success = QFile::rename(src, dest);
            if (!success && srcInfo.isDir()) {
                success = copyDirectory(src, dest) &&
                          QDir(src).removeRecursively();
            } else if (!success) {
                success = QFile::copy(src, dest) && QFile::remove(src);
            }
        }

        if (success) {
            emit fileRenamed(src, dest);
            changed = true;
        } else {
            QMessageBox::warning(
                this, tr("Error"),
                tr("Failed to move %1").arg(srcInfo.fileName()));
        }
    }

    if (changed) {
        addDirectoriesToWatcher(targetDir);
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
