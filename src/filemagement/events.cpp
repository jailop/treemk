#include "filesystemtreeview.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSortFilterProxyModel>
#include <QUrl>

void FileSystemTreeView::mouseDoubleClickEvent(QMouseEvent* event) {
    QTreeView::mouseDoubleClickEvent(event);
    QModelIndex index = indexAt(event->pos());
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
            bool ctrlPressed = (event->modifiers() & Qt::ControlModifier);
            bool shiftPressed = (event->modifiers() & Qt::ShiftModifier);

            if (ctrlPressed && shiftPressed) {
                emit openInNewWindowRequested(filePath);
            } else if (ctrlPressed) {
                emit fileOpenInNewTabRequested(filePath);
            } else {
                emit fileDoubleClicked(filePath);
            }
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    }
}

void FileSystemTreeView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_F2) {
        QModelIndex index = currentIndex();
        if (index.isValid()) {
            QModelIndex sourceIndex = proxyModel->mapToSource(index);
            renameOldPath = fileSystemModel->filePath(sourceIndex);
        }
    } else if (event->key() == Qt::Key_Return ||
               event->key() == Qt::Key_Enter) {
        QModelIndex index = currentIndex();
        if (index.isValid()) {
            QModelIndex sourceIndex = proxyModel->mapToSource(index);
            QString filePath = fileSystemModel->filePath(sourceIndex);
            QFileInfo fileInfo(filePath);

            if (fileInfo.isDir()) {
                // For directories, expand/collapse the tree view
                if (isExpanded(index)) {
                    collapse(index);
                } else {
                    expand(index);
                }
            } else if (fileInfo.isFile()) {
                // For files, behave like "Open" action
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
        event->accept();
        return;
    }
    QTreeView::keyPressEvent(event);
}
