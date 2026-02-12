#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>

namespace FileUtils {

struct FileCreationResult {
    bool success;
    QString errorMessage;
};

FileCreationResult createFileWithDirectories(const QString &filePath, const QString &initialContent = QString());

bool ensureDirectoryExists(const QString &dirPath);

} // namespace FileUtils

#endif // FILEUTILS_H
