#include "fileutils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace FileUtils {

bool ensureDirectoryExists(const QString& dirPath) {
    QDir dir;
    return dir.mkpath(dirPath);
}

FileCreationResult createFileWithDirectories(const QString& filePath,
                                             const QString& initialContent) {
    FileCreationResult result;
    result.success = false;

    QFileInfo fileInfo(filePath);
    QString dirPath = fileInfo.absolutePath();

    if (!ensureDirectoryExists(dirPath)) {
        result.errorMessage =
            QObject::tr("Failed to create directory: %1").arg(dirPath);
        return result;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.errorMessage =
            QObject::tr("Failed to create file: %1").arg(filePath);
        return result;
    }

    if (!initialContent.isEmpty()) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << initialContent;
    }

    file.close();
    result.success = true;
    return result;
}

bool isImageFile(const QString& filePath) {
    static const QStringList imageExtensions = {
        "png", "jpg", "jpeg", "gif", "bmp", "svg", "webp"
    };
    
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return imageExtensions.contains(suffix);
}

bool isMarkdownFile(const QString& filePath) {
    static const QStringList markdownExtensions = {
        "md", "markdown", "txt"
    };
    
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return markdownExtensions.contains(suffix);
}

}  // namespace FileUtils
