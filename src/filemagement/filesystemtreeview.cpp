#include "filesystemtreeview.h"
#include "fileutils.h"
#include <QContextMenuEvent>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QUrl>

FileSystemTreeView::FileSystemTreeView(QWidget *parent)
    : QTreeView(parent), clipboardIsCut(false) {
  setupModel();
  setupView();
  createContextMenu();
  setupFileSystemWatcher();
}

FileSystemTreeView::~FileSystemTreeView() {}

void FileSystemTreeView::setupModel() {
  fileSystemModel = new QFileSystemModel(this);
  fileSystemModel->setReadOnly(false);
  fileSystemModel->setFilter(QDir::AllDirs | QDir::Files |
                             QDir::NoDotAndDotDot);
  // QStringList filters;
  // filters << "*.md" << "*.markdown";
  // fileSystemModel->setNameFilters(filters);
  // fileSystemModel->setNameFilterDisables(false);
  setModel(fileSystemModel);

  // Try both dataChanged and fileRenamed signals
  connect(fileSystemModel, &QFileSystemModel::dataChanged, this,
          &FileSystemTreeView::onItemRenamed);
  connect(fileSystemModel, &QFileSystemModel::fileRenamed, this,
          [this](const QString &path, const QString &oldName, const QString &newName) {
    QString oldPath = QDir(path).filePath(oldName);
    QString newPath = QDir(path).filePath(newName);
    emit fileRenamed(oldPath, newPath);
  });
}

void FileSystemTreeView::setupView() {
  setHeaderHidden(false);
  setColumnHidden(1, true); // Hide size column
  setColumnHidden(2, true); // Hide type column
  setColumnHidden(3, true); // Hide date modified column
  header()->setStretchLastSection(false);
  header()->setSectionResizeMode(0, QHeaderView::Stretch);
  setAnimated(true);
  setIndentation(20);
  setSortingEnabled(true);
  sortByColumn(0, Qt::AscendingOrder);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setEditTriggers(QAbstractItemView::EditKeyPressed);

  // Drag and drop enabled to support the auto creation of 
  // wiki-links in the editor when dragging files from the
  // file explorer to the editor.
  setDragEnabled(true);
  setDragDropMode(QAbstractItemView::DragOnly);

  connect(selectionModel(), &QItemSelectionModel::currentChanged, this,
          &FileSystemTreeView::onSelectionChanged);
}

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
  connect(copyAction, &QAction::triggered, this, &FileSystemTreeView::copyItem);
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
  contextMenu->addAction(newFileAction);
  contextMenu->addAction(newFolderAction);
  contextMenu->addSeparator();
  contextMenu->addAction(openInNewWindowAction);
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

void FileSystemTreeView::setupFileSystemWatcher() {
  fileSystemWatcher = new QFileSystemWatcher(this);

  connect(fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this,
          &FileSystemTreeView::onDirectoryChanged);
  connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this,
          &FileSystemTreeView::onFileChanged);
}

void FileSystemTreeView::setRootPath(const QString &path) {
  if (path.isEmpty() || !QDir(path).exists()) {
    return;
  }

  if (!currentRootPath.isEmpty() &&
      fileSystemWatcher->directories().contains(currentRootPath)) {
    fileSystemWatcher->removePath(currentRootPath);
  }

  currentRootPath = path;
  QModelIndex rootIndex = fileSystemModel->setRootPath(path);
  setRootIndex(rootIndex);

  if (rootIndex.isValid()) {
    expand(rootIndex);
  }

  fileSystemWatcher->addPath(path);

  addDirectoriesToWatcher(path);
}

QString FileSystemTreeView::currentFilePath() const {
  QModelIndex index = currentIndex();
  if (!index.isValid()) {
    return QString();
  }
  return fileSystemModel->filePath(index);
}

QString FileSystemTreeView::rootPath() const { return currentRootPath; }

void FileSystemTreeView::onSelectionChanged(const QModelIndex &current,
                                            const QModelIndex &previous) {
  Q_UNUSED(previous);
  if (!current.isValid()) {
    return;
  }
  QString filePath = fileSystemModel->filePath(current);
  QFileInfo fileInfo(filePath);
  if (fileInfo.isFile()) {
    QString suffix = fileInfo.suffix().toLower();
    bool isMarkdownFile = (suffix == "md" || suffix == "markdown" || suffix == "txt");
    
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

void FileSystemTreeView::onDirectoryChanged(const QString &path) {
  Q_UNUSED(path);
  
  // QFileSystemModel automatically handles directory changes and updates the view.
  // No additional action needed here.
}

void FileSystemTreeView::onFileChanged(const QString &path) {
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

void FileSystemTreeView::mouseDoubleClickEvent(QMouseEvent *event) {
  QTreeView::mouseDoubleClickEvent(event);
  QModelIndex index = indexAt(event->pos());
  if (!index.isValid()) {
    return;
  }
  QString filePath = fileSystemModel->filePath(index);
  QFileInfo fileInfo(filePath);
  if (fileInfo.isFile()) {
    QString suffix = fileInfo.suffix().toLower();
    bool isMarkdownFile = (suffix == "md" || suffix == "markdown" || suffix == "txt");
    
    if (isMarkdownFile) {
      emit fileDoubleClicked(filePath);
    } else {
      QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
  }
}

void FileSystemTreeView::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_F2) {
    QModelIndex index = currentIndex();
    if (index.isValid()) {
      renameOldPath = fileSystemModel->filePath(index);
    }
  }
  QTreeView::keyPressEvent(event);
}

void FileSystemTreeView::contextMenuEvent(QContextMenuEvent *event) {
  QModelIndex index = indexAt(event->pos());
  bool hasSelection = index.isValid();
  bool isDirectory = false;
  bool isMarkdownFile = false;
  if (hasSelection) {
    QString filePath = fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);
    isDirectory = fileInfo.isDir();
    
    if (fileInfo.isFile()) {
      QString suffix = fileInfo.suffix().toLower();
      isMarkdownFile = (suffix == "md" || suffix == "markdown" || suffix == "txt");
    }
  }

  // Enable/disable actions based on selection
  pasteAction->setEnabled(!clipboardPath.isEmpty());
  renameAction->setEnabled(hasSelection);
  deleteAction->setEnabled(hasSelection);
  cutAction->setEnabled(hasSelection);
  copyAction->setEnabled(hasSelection);
  
  // Open in new window - only for md/txt files
  openInNewWindowAction->setEnabled(hasSelection && isMarkdownFile);
  
  // Folder navigation actions - only for directories
  setCurrentFolderAction->setEnabled(hasSelection && isDirectory);
  
  // Parent folder action - check if we're not already at root
  QDir currentDir(currentRootPath);
  bool hasParent = currentDir.cdUp();
  goToParentAction->setEnabled(hasParent);

  contextMenu->exec(event->globalPos());
}

void FileSystemTreeView::createNewFile() {
  QModelIndex index = currentIndex();
  QString parentPath;

  if (index.isValid()) {
    QString selectedPath = fileSystemModel->filePath(index);
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

  if (!fileName.endsWith(".md") && !fileName.endsWith(".markdown")) {
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
    QString selectedPath = fileSystemModel->filePath(index);
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

  renameOldPath = fileSystemModel->filePath(index);
  edit(index);
}

void FileSystemTreeView::deleteItem() {
  QModelIndex index = currentIndex();
  if (!index.isValid()) {
    return;
  }

  QString filePath = fileSystemModel->filePath(index);
  QFileInfo fileInfo(filePath);

  QString itemType = fileInfo.isDir() ? tr("folder") : tr("file");
  QString itemName = fileInfo.fileName();

  QMessageBox::StandardButton reply =
      QMessageBox::question(this, tr("Delete"),
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
  clipboardPath = fileSystemModel->filePath(index);
  clipboardIsCut = true;
}

void FileSystemTreeView::copyItem() {
  QModelIndex index = currentIndex();
  if (!index.isValid()) {
    return;
  }
  clipboardPath = fileSystemModel->filePath(index);
  clipboardIsCut = false;
}

void FileSystemTreeView::pasteItem() {
  if (clipboardPath.isEmpty()) {
    return;
  }
  QModelIndex index = currentIndex();
  QString targetPath;
  if (index.isValid()) {
    QString selectedPath = fileSystemModel->filePath(index);
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

bool FileSystemTreeView::copyDirectory(const QString &srcPath,
                                       const QString &dstPath) {
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
      const QFileInfo &fileInfo,
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

void FileSystemTreeView::addDirectoriesToWatcher(const QString &path) {
  QDir dir(path);
  if (!dir.exists()) {
    return;
  }

  // Skip system/private directories to avoid permission errors
  if (path.contains("/systemd-private-") || 
      path.startsWith("/proc") || 
      path.startsWith("/sys") ||
      path.startsWith("/dev")) {
    return;
  }

  if (!fileSystemWatcher->directories().contains(path)) {
    // Try to add, but don't show error if permission denied
    fileSystemWatcher->addPath(path);
    // Silently ignore errors - QFileSystemWatcher returns false on error
  }

  // NOTE: We only watch the root directory, not all subdirectories.
  // QFileSystemModel automatically handles subdirectory updates.
  // Recursively watching all subdirectories causes severe performance issues
  // with large directory trees and blocks the UI.
}

void FileSystemTreeView::refreshDirectory() {
  if (currentRootPath.isEmpty()) {
    return;
  }

  QString savedPath = currentFilePath();
  // QModelIndex currentIdx = currentIndex();

  setRootPath(currentRootPath);

  if (!savedPath.isEmpty() && QFile::exists(savedPath)) {
    QModelIndex idx = fileSystemModel->index(savedPath);
    if (idx.isValid()) {
      setCurrentIndex(idx);
      scrollTo(idx);
    }
  }
}

void FileSystemTreeView::setAsCurrentFolder() {
  QModelIndex index = currentIndex();
  if (!index.isValid()) {
    return;
  }

  QString selectedPath = fileSystemModel->filePath(index);
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

void FileSystemTreeView::openInNewWindow() {
  QModelIndex index = currentIndex();
  if (!index.isValid()) {
    return;
  }
  
  QString path = fileSystemModel->filePath(index);
  emit openInNewWindowRequested(path);
}

void FileSystemTreeView::selectFile(const QString &filePath) {
  if (filePath.isEmpty()) {
    return;
  }
  
  QModelIndex index = fileSystemModel->index(filePath);
  if (index.isValid()) {
    setCurrentIndex(index);
    scrollTo(index);
  }
}

void FileSystemTreeView::notifyFileSaving(const QString &filePath) {
  fileSavingPath = filePath;
}

void FileSystemTreeView::onItemRenamed(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
  // Only process column 0 (filename) changes
  if (topLeft.column() != 0) {
    return;
  }
  
  if (!renameOldPath.isEmpty()) {
    QString newPath = fileSystemModel->filePath(topLeft);
    
    // Only emit if paths are different and the new path exists
    if (newPath != renameOldPath && QFile::exists(newPath) && !QFile::exists(renameOldPath)) {
      emit fileRenamed(renameOldPath, newPath);
      renameOldPath.clear();
    } else if (newPath == renameOldPath) {
      // User cancelled the rename, clear the old path
      renameOldPath.clear();
    }
  }
}
