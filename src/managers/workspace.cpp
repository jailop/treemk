#include "managers/workspacemanager.h"

#include <QDir>
#include <QFileInfo>

#include "logic/mainfilelocator.h"

WorkspaceManager::WorkspaceManager(QObject* parent) : QObject(parent) {}

WorkspaceManager::~WorkspaceManager() {}

bool WorkspaceManager::openFolder(const QString& folderPath) {
    if (folderPath.isEmpty()) {
        return false;
    }

    QDir dir(folderPath);
    if (!dir.exists()) {
        return false;
    }

    m_currentFolder = dir.absolutePath();
    emit folderOpened(m_currentFolder);
    return true;
}
