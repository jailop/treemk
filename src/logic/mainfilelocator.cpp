#include "logic/mainfilelocator.h"
#include <QDir>
#include <QFileInfo>

const QStringList MainFileLocator::FALLBACK_NAMES = {
    "index.md",
    "README.md"
};

QString MainFileLocator::findMainFile(const QString &folderPath, 
                                     const QString &preferredName) {
    if (folderPath.isEmpty()) {
        return QString();
    }
    
    QDir dir(folderPath);
    if (!dir.exists()) {
        return QString();
    }
    
    // Try preferred name first
    if (!preferredName.isEmpty()) {
        QString found = findFile(folderPath, preferredName);
        if (!found.isEmpty()) {
            return found;
        }
    }
    
    // Try fallback names
    for (const QString &fallbackName : FALLBACK_NAMES) {
        // Skip if fallback is same as preferred (already tried)
        if (fallbackName.compare(preferredName, Qt::CaseInsensitive) == 0) {
            continue;
        }
        
        QString found = findFile(folderPath, fallbackName);
        if (!found.isEmpty()) {
            return found;
        }
    }
    
    return QString();
}

QStringList MainFileLocator::getFallbackNames() {
    return FALLBACK_NAMES;
}

QString MainFileLocator::findFile(const QString &folderPath, const QString &fileName) {
    QDir dir(folderPath);
    QString filePath = dir.absoluteFilePath(fileName);
    QFileInfo fileInfo(filePath);
    
    if (fileInfo.exists() && fileInfo.isFile()) {
        return fileInfo.absoluteFilePath();
    }
    
    // Try case-insensitive search on case-sensitive file systems
    QStringList entries = dir.entryList(QDir::Files);
    for (const QString &entry : entries) {
        if (entry.compare(fileName, Qt::CaseInsensitive) == 0) {
            return dir.absoluteFilePath(entry);
        }
    }
    
    return QString();
}
