#include "filesystemtreeview.h"

#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include "fileutils.h"

void FileSystemTreeView::createNewFile() {
    QModelIndex index = currentIndex();
    QString parentPath;

    if (index.isValid()) {
        QModelIndex sourceIndex = proxyModel->mapToSource(index);
        QString selectedPath = fileSystemModel->filePath(sourceIndex);
        QFileInfo fileInfo(selectedPath);
        parentPath = fileInfo.isDir() ? selectedPath : fileInfo.absolutePath();
    } else {
        parentPath = currentRootPath;
    }

    bool ok;
    QString fileName =
        QInputDialog::getText(this, tr("New File"), tr("File name:"),
                              QLineEdit::Normal, "untitled.md", &ok);

    if (!ok || fileName.isEmpty()) {
        return;
    }

    // Only add .md extension if user hasn't provided any extension
    QFileInfo fileNameInfo(fileName);
    if (fileNameInfo.suffix().isEmpty()) {
        fileName += ".md";
    }

    QString filePath = QDir(parentPath).filePath(fileName);

    if (QFile::exists(filePath)) {
        QMessageBox::warning(this, tr("Error"), tr("File already exists!"));
        return;
    }

    FileUtils::FileCreationResult result =
        FileUtils::createFileWithDirectories(filePath, QString());

    if (!result.success) {
        QMessageBox::warning(this, tr("Error"), result.errorMessage);
        return;
    }
}

void FileSystemTreeView::createNewFolder() {
    QModelIndex index = currentIndex();
    QString parentPath;

    if (index.isValid()) {
        QModelIndex sourceIndex = proxyModel->mapToSource(index);
        QString selectedPath = fileSystemModel->filePath(sourceIndex);
        QFileInfo fileInfo(selectedPath);
        parentPath = fileInfo.isDir() ? selectedPath : fileInfo.absolutePath();
    } else {
        parentPath = currentRootPath;
    }

    bool ok;
    QString folderName =
        QInputDialog::getText(this, tr("New Folder"), tr("Folder name:"),
                              QLineEdit::Normal, "New Folder", &ok);

    if (!ok || folderName.isEmpty()) {
        return;
    }

    QString folderPath = QDir(parentPath).filePath(folderName);

    if (QDir(folderPath).exists()) {
        QMessageBox::warning(this, tr("Error"), tr("Folder already exists!"));
        return;
    }

    if (!QDir().mkdir(folderPath)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to create folder!"));
    }
}

void FileSystemTreeView::onEditStarted() {
    // Not used anymore, can be removed
}

void FileSystemTreeView::renameItem() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    renameOldPath = fileSystemModel->filePath(sourceIndex);
    edit(index);
}

void FileSystemTreeView::deleteItem() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    QString filePath = fileSystemModel->filePath(sourceIndex);
    QFileInfo fileInfo(filePath);

    QString itemType = fileInfo.isDir() ? tr("folder") : tr("file");
    QString itemName = fileInfo.fileName();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Delete"),
        tr("Are you sure you want to delete this %1?\n\n%2")
            .arg(itemType, itemName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    bool success = false;

    if (fileInfo.isDir()) {
        QDir dir(filePath);
        success = dir.removeRecursively();
    } else {
        success = QFile::remove(filePath);
    }

    if (!success) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to delete %1!").arg(itemType));
    } else {
        emit fileDeleted(filePath);
    }
}

void FileSystemTreeView::cutItem() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    clipboardPath = fileSystemModel->filePath(sourceIndex);
    clipboardIsCut = true;
}

void FileSystemTreeView::copyItem() {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return;
    }
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    clipboardPath = fileSystemModel->filePath(sourceIndex);
    clipboardIsCut = false;
}

void FileSystemTreeView::pasteItem() {
    if (clipboardPath.isEmpty()) {
        return;
    }
    QModelIndex index = currentIndex();
    QString targetPath;
    if (index.isValid()) {
        QModelIndex sourceIndex = proxyModel->mapToSource(index);
        QString selectedPath = fileSystemModel->filePath(sourceIndex);
        QFileInfo fileInfo(selectedPath);
        targetPath = fileInfo.isDir() ? selectedPath : fileInfo.absolutePath();
    } else {
        targetPath = currentRootPath;
    }
    QFileInfo sourceInfo(clipboardPath);
    QString newPath = QDir(targetPath).filePath(sourceInfo.fileName());
    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("An item with this name already exists!"));
        return;
    }
    bool success = false;
    if (clipboardIsCut) {
        if (sourceInfo.isDir()) {
            success = QDir().rename(clipboardPath, newPath);
        } else {
            success = QFile::rename(clipboardPath, newPath);
        }

        if (success) {
            clipboardPath.clear();
        }
    } else {
        if (sourceInfo.isDir()) {
            success = copyDirectory(clipboardPath, newPath);
        } else {
            success = QFile::copy(clipboardPath, newPath);
        }
    }
    if (!success) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to paste item!"));
    }
}

bool FileSystemTreeView::copyDirectory(const QString& srcPath,
                                       const QString& dstPath) {
    QDir srcDir(srcPath);
    if (!srcDir.exists()) {
        return false;
    }

    QDir dstDir(dstPath);
    if (!dstDir.exists()) {
        if (!QDir().mkdir(dstPath)) {
            return false;
        }
    }

    foreach (
        const QFileInfo& fileInfo,
        srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString srcFilePath = fileInfo.filePath();
        QString dstFilePath = dstPath + "/" + fileInfo.fileName();

        if (fileInfo.isDir()) {
            if (!copyDirectory(srcFilePath, dstFilePath)) {
                return false;
            }
        } else {
            if (!QFile::copy(srcFilePath, dstFilePath)) {
                return false;
            }
        }
    }

    return true;
}
