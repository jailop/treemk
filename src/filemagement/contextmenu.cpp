#include "filesystemtreeview.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QProcess>
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
    connect(renameAction, &QAction::triggered, this,
            &FileSystemTreeView::renameItem);
    deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this,
            &FileSystemTreeView::deleteItem);
    cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, &FileSystemTreeView::cutItem);
    copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this,
            &FileSystemTreeView::copyItem);
    pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this,
            &FileSystemTreeView::pasteItem);
    refreshAction = new QAction(tr("Refresh"), this);
    refreshAction->setShortcut(QKeySequence::Refresh);
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

    // Only create "Open With" action on supported platforms
    openWithAction = nullptr;
#if defined(Q_OS_WIN)
    // Windows supports "Open With" dialog
    openWithAction = new QAction(tr("Open With..."), this);
    connect(openWithAction, &QAction::triggered, this,
            &FileSystemTreeView::openFileWith);
#elif defined(Q_OS_LINUX)
    // On Linux, only GNOME/GTK environments support it via gio
    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");
    if (desktopEnv.contains("GNOME", Qt::CaseInsensitive) ||
        desktopEnv.contains("GTK", Qt::CaseInsensitive) ||
        desktopEnv.contains("XFCE", Qt::CaseInsensitive) ||
        desktopEnv.contains("MATE", Qt::CaseInsensitive)) {
        // Check if gio is available
        if (QProcess::execute("which", QStringList() << "gio") == 0) {
            openWithAction = new QAction(tr("Open With..."), this);
            connect(openWithAction, &QAction::triggered, this,
                    &FileSystemTreeView::openFileWith);
        }
    }
#endif

    contextMenu->addAction(newFileAction);
    contextMenu->addAction(newFolderAction);
    contextMenu->addSeparator();
    contextMenu->addAction(openAction);
    if (openWithAction) {
        contextMenu->addAction(openWithAction);
    }
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

    // Enable/disable actions based on selection
    pasteAction->setEnabled(!clipboardPath.isEmpty());
    renameAction->setEnabled(hasSelection);
    deleteAction->setEnabled(hasSelection);
    cutAction->setEnabled(hasSelection);
    copyAction->setEnabled(hasSelection);

    // Open - for all files (md/txt files open in TreeMk, others with system
    // app)
    openAction->setEnabled(hasSelection && !isDirectory);

    // Open With - only for non-markdown files
    if (openWithAction) {
        openWithAction->setEnabled(hasSelection && isNonMarkdownFile);
    }

    // Open in new window - for md/txt files and directories
    openInNewWindowAction->setEnabled(hasSelection &&
                                      (isMarkdownFile || isDirectory));

    // Open in new tab - only for markdown files
    openInNewTabAction->setEnabled(hasSelection && isMarkdownFile);

    // Folder navigation actions - only for directories
    setCurrentFolderAction->setEnabled(hasSelection && isDirectory);

    // Parent folder action - check if we're not already at root
    QDir currentDir(currentRootPath);
    bool hasParent = currentDir.cdUp();
    goToParentAction->setEnabled(hasParent);

    contextMenu->exec(event->globalPos());
}
