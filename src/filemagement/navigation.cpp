#include "filesystemtreeview.h"

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSortFilterProxyModel>

void FileSystemTreeView::refreshDirectory() {
    if (currentRootPath.isEmpty()) {
        return;
    }

    QString savedPath = currentFilePath();

    setRootPath(currentRootPath);

    if (!savedPath.isEmpty() && QFile::exists(savedPath)) {
        QModelIndex sourceIdx = fileSystemModel->index(savedPath);
        if (sourceIdx.isValid()) {
            QModelIndex proxyIdx = proxyModel->mapFromSource(sourceIdx);
            if (proxyIdx.isValid()) {
                setCurrentIndex(proxyIdx);
                scrollTo(proxyIdx);
            }
        }
    }
}

void FileSystemTreeView::setAsCurrentFolder() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString selectedPath = fileSystemModel->filePath(sourceIndex);
    QFileInfo fileInfo(selectedPath);

    if (fileInfo.isDir()) {
        setRootPath(selectedPath);
        emit folderChanged(selectedPath);
    }
}

void FileSystemTreeView::goToParentFolder() {
    if (currentRootPath.isEmpty()) {
        return;
    }

    QDir currentDir(currentRootPath);
    if (currentDir.cdUp()) {
        QString parentPath = currentDir.absolutePath();
        setRootPath(parentPath);
        emit folderChanged(parentPath);
    }
}

void FileSystemTreeView::selectFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        return;
    }

    QModelIndex sourceIndex = fileSystemModel->index(filePath);
    if (sourceIndex.isValid()) {
        QModelIndex proxyIndex = proxyModel->mapFromSource(sourceIndex);
        if (proxyIndex.isValid()) {
            setCurrentIndex(proxyIndex);
            scrollTo(proxyIndex);
        }
    }
}

void FileSystemTreeView::notifyFileSaving(const QString& filePath) {
    fileSavingPath = filePath;
}

void FileSystemTreeView::setNameFilter(const QString& filter) {
    if (filter.isEmpty()) {
        proxyModel->setFilterRegularExpression(QRegularExpression());
    } else {
        proxyModel->setFilterWildcard("*" + filter + "*");
    }

    if (currentRootSourceIndex.isValid()) {
        QModelIndex proxyRootIndex =
            proxyModel->mapFromSource(currentRootSourceIndex);
        setRootIndex(proxyRootIndex);
    }
}
