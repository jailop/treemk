#include "managers/workspacemanager.h"
#include "logic/mainfilelocator.h"
#include <QDir>
#include <QFileInfo>

WorkspaceManager::WorkspaceManager(QObject *parent) 
    : QObject(parent) {
}

WorkspaceManager::~WorkspaceManager() {
}

bool WorkspaceManager::openFolder(const QString &folderPath, 
                                 bool autoOpenMain,
                                 const QString &mainFileName) {
    if (folderPath.isEmpty()) {
        return false;
    }
    
    QDir dir(folderPath);
    if (!dir.exists()) {
        return false;
    }
    
    m_currentFolder = dir.absolutePath();
    m_mainFile.clear();
    
    emit folderOpened(m_currentFolder);
    
    if (autoOpenMain) {
        QString preferredName = mainFileName.isEmpty() ? "main.md" : mainFileName;
        QString mainFilePath = MainFileLocator::findMainFile(m_currentFolder, preferredName);
        
        if (!mainFilePath.isEmpty()) {
            m_mainFile = mainFilePath;
            emit mainFileFound(mainFilePath);
        } else {
            emit noMainFileFound();
        }
    }
    
    return true;
}
