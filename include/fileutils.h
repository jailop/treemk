#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>

namespace FileUtils {

struct FileCreationResult {
    bool success;
    QString errorMessage;
};

FileCreationResult createFileWithDirectories(
    const QString& filePath, const QString& initialContent = QString());

bool ensureDirectoryExists(const QString& dirPath);

bool isImageFile(const QString& filePath);

bool isMarkdownFile(const QString& filePath);

}  // namespace FileUtils

#endif  // FILEUTILS_H
