#include "filesystemtreeview.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QSortFilterProxyModel>

void FileSystemTreeView::createContextMenu() {
    contextMenu = new QMenu(this);
    newFileAction = new QAction(tr("New File"), this);
    connect(newFileAction, &QAction::triggered, this,
            &FileSystemTreeView::createNewFile);
    newFolderAction = new QAction(tr("New Folder"), this);
    connect(newFolderAction, &QAction::triggered, this,
            &FileSystemTreeView::createNewFolder);
    renameAction = new QAction(tr("Rename"), this);
    renameAction->setShortcut(QKeySequence(Qt::Key_F2));
    renameAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(renameAction, &QAction::triggered, this,
            &FileSystemTreeView::renameItem);
    deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(deleteAction, &QAction::triggered, this,
            &FileSystemTreeView::deleteItem);
    cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(cutAction, &QAction::triggered, this, &FileSystemTreeView::cutItem);
    copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(copyAction, &QAction::triggered, this,
            &FileSystemTreeView::copyItem);
    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(pasteAction, &QAction::triggered, this,
            &FileSystemTreeView::pasteItem);
    refreshAction = new QAction(tr("Refresh"), this);
    refreshAction->setShortcut(QKeySequence::Refresh);
    refreshAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(refreshAction, &QAction::triggered, this,
            &FileSystemTreeView::refreshDirectory);
    setCurrentFolderAction = new QAction(tr("Set as Current Folder"), this);
    connect(setCurrentFolderAction, &QAction::triggered, this,
            &FileSystemTreeView::setAsCurrentFolder);
    goToParentAction = new QAction(tr("Go to Parent Folder"), this);
    connect(goToParentAction, &QAction::triggered, this,
            &FileSystemTreeView::goToParentFolder);
    openInNewWindowAction = new QAction(tr("Open in New Window"), this);
    connect(openInNewWindowAction, &QAction::triggered, this,
            &FileSystemTreeView::openInNewWindow);
    openInNewTabAction = new QAction(tr("Open in New Tab"), this);
    connect(openInNewTabAction, &QAction::triggered, this,
            &FileSystemTreeView::openInNewTab);
    openAction = new QAction(tr("Open"), this);
    connect(openAction, &QAction::triggered, this,
            &FileSystemTreeView::openFile);
    openInFileExplorerAction = new QAction(tr("Open in File Explorer"), this);
    connect(openInFileExplorerAction, &QAction::triggered, this,
            &FileSystemTreeView::openInFileExplorer);

    openWithAction = new QAction(tr("Open With..."), this);
    connect(openWithAction, &QAction::triggered, this,
            &FileSystemTreeView::openFileWith);

    contextMenu->addAction(newFileAction);
    contextMenu->addAction(newFolderAction);
    contextMenu->addSeparator();
    contextMenu->addAction(openAction);
    contextMenu->addAction(openWithAction);
    contextMenu->addAction(openInNewTabAction);
    contextMenu->addAction(openInNewWindowAction);
    contextMenu->addAction(openInFileExplorerAction);
    contextMenu->addSeparator();
    contextMenu->addAction(renameAction);
    contextMenu->addAction(deleteAction);
    contextMenu->addSeparator();
    contextMenu->addAction(cutAction);
    contextMenu->addAction(copyAction);
    contextMenu->addAction(pasteAction);
    contextMenu->addSeparator();
    contextMenu->addAction(setCurrentFolderAction);
    contextMenu->addAction(goToParentAction);
    contextMenu->addSeparator();
    contextMenu->addAction(refreshAction);
    addAction(newFileAction);
    addAction(newFolderAction);
    addAction(renameAction);
    addAction(deleteAction);
    addAction(cutAction);
    addAction(copyAction);
    addAction(pasteAction);
    addAction(refreshAction);
}

void FileSystemTreeView::contextMenuEvent(QContextMenuEvent* event) {
    QModelIndex index = indexAt(event->pos());
    bool hasSelection = index.isValid();
    bool isDirectory = false;
    bool isMarkdownFile = false;
    bool isNonMarkdownFile = false;
    if (hasSelection) {
        QModelIndex sourceIndex = proxyModel->mapToSource(index);
        QString filePath = fileSystemModel->filePath(sourceIndex);
        QFileInfo fileInfo(filePath);
        isDirectory = fileInfo.isDir();

        if (fileInfo.isFile()) {
            QString suffix = fileInfo.suffix().toLower();
            isMarkdownFile =
                (suffix == "md" || suffix == "markdown" || suffix == "txt");
            isNonMarkdownFile = !isMarkdownFile;
        }
    }

    pasteAction->setEnabled(!clipboardPath.isEmpty());
    renameAction->setEnabled(hasSelection);
    deleteAction->setEnabled(hasSelection);
    cutAction->setEnabled(hasSelection);
    copyAction->setEnabled(hasSelection);
    openAction->setEnabled(hasSelection && !isDirectory);
    openWithAction->setEnabled(hasSelection && isNonMarkdownFile);
    openInNewWindowAction->setEnabled(hasSelection &&
                                      (isMarkdownFile || isDirectory));
    openInNewTabAction->setEnabled(hasSelection && isMarkdownFile);
    setCurrentFolderAction->setEnabled(hasSelection && isDirectory);
    QDir currentDir(currentRootPath);
    bool hasParent = currentDir.cdUp();
    goToParentAction->setEnabled(hasParent);

    contextMenu->exec(event->globalPos());
}
