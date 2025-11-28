#ifndef FILESYSTEMTREEVIEW_H
#define FILESYSTEMTREEVIEW_H

#include <QTreeView>
#include <QFileSystemModel>
#include <QFileSystemWatcher>

class FileSystemTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit FileSystemTreeView(QWidget *parent = nullptr);
    ~FileSystemTreeView();

    void setRootPath(const QString &path);
    QString currentFilePath() const;
    QString rootPath() const;
    void notifyFileSaving(const QString &filePath);

signals:
    void fileSelected(const QString &filePath);
    void fileDoubleClicked(const QString &filePath);
    void fileModifiedExternally(const QString &filePath);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void onSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
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
    
    QString clipboardPath;
    bool clipboardIsCut;
    QString currentRootPath;
    QString watchedFilePath;
    QString fileSavingPath;
};

#endif // FILESYSTEMTREEVIEW_H
