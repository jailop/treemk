#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include <QFileSystemModel>
#include <QFileSystemWatcher>
#include <QTreeView>

/**
 * This class represents a tree view for displaying
 * and managing the file system. It provides functionalities
 * such as navigating directories, selecting files, and performing
 * file operations like creating, renaming, and deleting files and
 * folders.
 */
class FileSystemTreeView : public QTreeView {
  Q_OBJECT

public:
  explicit FileSystemTreeView(QWidget *parent = nullptr);
  ~FileSystemTreeView();

  /**
   * The app manages the concept of a "root path" which is
   * the base directory that the tree view displays. This allows
   * the app to focus on a specific part of the file system. This
   * method sets the root path for the tree view, which will update
   * the displayed directory structure accordingly.
   */
  void setRootPath(const QString &path);

  QString currentFilePath() const;

  QString rootPath() const;

  void notifyFileSaving(const QString &filePath);

  void selectFile(const QString &filePath);

signals:
  void fileSelected(const QString &filePath);
  void fileDoubleClicked(const QString &filePath);
  void fileModifiedExternally(const QString &filePath);
  void folderChanged(const QString &folderPath);
  void openInNewWindowRequested(const QString &path);
  void fileDeleted(const QString &filePath);
  void fileRenamed(const QString &oldPath, const QString &newPath);

protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void onSelectionChanged(const QModelIndex &current,
                          const QModelIndex &previous);
  void onDirectoryChanged(const QString &path);
  void onFileChanged(const QString &path);
  void createNewFile();
  void createNewFolder();
  void renameItem();
  void deleteItem();
  void cutItem();
  void copyItem();
  void pasteItem();
  void refreshDirectory();
  void setAsCurrentFolder();
  void goToParentFolder();
  void openInNewWindow();
  void openFile();
  void openFileWith();
  void onItemRenamed(const QModelIndex &topLeft, const QModelIndex &bottomRight);
  void onEditStarted();

private:
  void setupModel();
  void setupView();
  void createContextMenu();
  bool copyDirectory(const QString &srcPath, const QString &dstPath);
  void setupFileSystemWatcher();
  void addDirectoriesToWatcher(const QString &path);

  QFileSystemModel *fileSystemModel;
  QFileSystemWatcher *fileSystemWatcher;
  QMenu *contextMenu;
  QAction *newFileAction;
  QAction *newFolderAction;
  QAction *renameAction;
  QAction *deleteAction;
  QAction *cutAction;
  QAction *copyAction;
  QAction *pasteAction;
  QAction *refreshAction;
  QAction *setCurrentFolderAction;
  QAction *goToParentAction;
  QAction *openInNewWindowAction;
  QAction *openAction;
  QAction *openWithAction;

  QString clipboardPath;
  bool clipboardIsCut;
  QString currentRootPath;
  QString watchedFilePath;
  QString fileSavingPath;
  QString renameOldPath;
};

#endif // FILESYSTEMTREEVIEW_H
