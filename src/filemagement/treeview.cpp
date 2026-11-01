#include "filesystemtreeview.h"

#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QSortFilterProxyModel>

FileSystemTreeView::FileSystemTreeView(QWidget* parent)
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

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(fileSystemModel);
    proxyModel->setRecursiveFilteringEnabled(true);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterRole(Qt::DisplayRole);

    setModel(proxyModel);

    connect(fileSystemModel, &QFileSystemModel::dataChanged, this,
            &FileSystemTreeView::onItemRenamed);
    connect(fileSystemModel, &QFileSystemModel::fileRenamed, this,
            [this](const QString& path, const QString& oldName,
                   const QString& newName) {
                QString oldPath = QDir(path).filePath(oldName);
                QString newPath = QDir(path).filePath(newName);
                emit fileRenamed(oldPath, newPath);
            });
}

void FileSystemTreeView::setupView() {
    setHeaderHidden(false);
    setColumnHidden(1, true);  // Hide size column
    setColumnHidden(2, true);  // Hide type column
    setColumnHidden(3, true);  // Hide date modified column
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    setAnimated(true);
    setIndentation(20);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::EditKeyPressed);

    // Drag and drop enabled to support dragging files from file explorer to
    // editor (creates wiki-links) and dragging files onto folders to move them.
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    connect(selectionModel(), &QItemSelectionModel::currentChanged, this,
            &FileSystemTreeView::onSelectionChanged);
}

void FileSystemTreeView::setupFileSystemWatcher() {
    fileSystemWatcher = new QFileSystemWatcher(this);

    connect(fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this,
            &FileSystemTreeView::onDirectoryChanged);
    connect(fileSystemWatcher, &QFileSystemWatcher::fileChanged, this,
            &FileSystemTreeView::onFileChanged);
}

void FileSystemTreeView::setRootPath(const QString& path) {
    if (path.isEmpty() || !QDir(path).exists()) {
        return;
    }

    if (!currentRootPath.isEmpty() &&
        fileSystemWatcher->directories().contains(currentRootPath)) {
        fileSystemWatcher->removePath(currentRootPath);
    }

    currentRootPath = path;
    currentRootSourceIndex = fileSystemModel->setRootPath(path);
    QModelIndex proxyRootIndex = proxyModel->mapFromSource(currentRootSourceIndex);
    setRootIndex(proxyRootIndex);

    if (proxyRootIndex.isValid()) {
        expand(proxyRootIndex);
    }

    fileSystemWatcher->addPath(path);

    addDirectoriesToWatcher(path);
}

QString FileSystemTreeView::currentFilePath() const {
    QModelIndex index = currentIndex();
    if (!index.isValid()) {
        return QString();
    }
    QModelIndex sourceIndex = proxyModel->mapToSource(index);
    return fileSystemModel->filePath(sourceIndex);
}

QString FileSystemTreeView::rootPath() const { return currentRootPath; }

void FileSystemTreeView::addDirectoriesToWatcher(const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        return;
    }

    // Skip system/private directories to avoid permission errors
    if (path.contains("/systemd-private-") || path.startsWith("/proc") ||
        path.startsWith("/sys") || path.startsWith("/dev")) {
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
