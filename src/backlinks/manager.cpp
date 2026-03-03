#include "backlinks/backlinksmanager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>

BacklinksManager::BacklinksManager(QObject* parent) : QObject(parent) {}

void BacklinksManager::buildBacklinks(
    const QMap<QString, QVector<QString>>& forwardLinks) {
    QMutexLocker locker(&mutex);
    backLinksMap.clear();
    normalizedPathCache.clear();

    QHash<QString, QString> fileNormalizedMap;
    for (auto it = forwardLinks.constBegin(); it != forwardLinks.constEnd();
         ++it) {
        const QString& filePath = it.key();
        QString normalized = normalizePath(filePath);
        fileNormalizedMap[filePath] = normalized;
        normalizedPathCache[filePath] = normalized;
    }

    QHash<QString, QString> normalizedToFile;
    for (auto it = fileNormalizedMap.constBegin();
         it != fileNormalizedMap.constEnd(); ++it) {
        normalizedToFile[it.value()] = it.key();
    }

    for (auto it = forwardLinks.constBegin(); it != forwardLinks.constEnd();
         ++it) {
        const QString& sourceFile = it.key();
        const QVector<QString>& linkTargets = it.value();
        QFileInfo sourceInfo(sourceFile);
        QDir sourceDir = sourceInfo.dir();

        for (const QString& linkTarget : linkTargets) {
            QString link = linkTarget.trimmed();

            QStringList candidatePaths;
            QFileInfo linkInfo(link);

            if (linkInfo.suffix().isEmpty()) {
                candidatePaths << sourceDir.filePath(link + ".md");
                candidatePaths << sourceDir.filePath(link + ".markdown");
                candidatePaths << sourceDir.filePath(link);
            } else {
                candidatePaths << sourceDir.filePath(link);
            }

            for (const QString& candidate : candidatePaths) {
                QString candidateNorm = getCachedNormalizedPath(candidate);

                if (normalizedToFile.contains(candidateNorm)) {
                    QString targetFile = normalizedToFile[candidateNorm];
                    QString targetNorm = fileNormalizedMap[targetFile];

                    if (!backLinksMap[targetNorm].contains(sourceFile)) {
                        backLinksMap[targetNorm].append(sourceFile);
                    }
                    break;
                }

                bool foundMatch = false;
                for (auto fit = normalizedToFile.constBegin();
                     fit != normalizedToFile.constEnd(); ++fit) {
                    if (candidateNorm.compare(fit.key(), Qt::CaseInsensitive) ==
                        0) {
                        QString targetFile = fit.value();
                        QString targetNorm = fileNormalizedMap[targetFile];

                        if (!backLinksMap[targetNorm].contains(sourceFile)) {
                            backLinksMap[targetNorm].append(sourceFile);
                        }
                        foundMatch = true;
                        break;
                    }
                }

                if (foundMatch) {
                    break;
                }
            }
        }
    }

    emit backlinksUpdated();
}

QVector<QString> BacklinksManager::getBacklinks(const QString& filePath) const {
    QMutexLocker locker(&mutex);

    QString normalizedPath = normalizePath(filePath);

    // Try exact match first
    if (backLinksMap.contains(normalizedPath)) {
        return backLinksMap[normalizedPath];
    }

    if (backLinksMap.contains(filePath)) {
        return backLinksMap[filePath];
    }

    // Try case-insensitive match
    for (auto it = backLinksMap.constBegin(); it != backLinksMap.constEnd();
         ++it) {
        if (it.key().compare(filePath, Qt::CaseInsensitive) == 0 ||
            it.key().compare(normalizedPath, Qt::CaseInsensitive) == 0) {
            return it.value();
        }
    }

    return QVector<QString>();
}

void BacklinksManager::clear() {
    QMutexLocker locker(&mutex);
    backLinksMap.clear();
    normalizedPathCache.clear();
    emit backlinksUpdated();
}

QString BacklinksManager::normalizePath(const QString& path) const {
    QString canonical = QFileInfo(path).canonicalFilePath();
    if (!canonical.isEmpty()) {
        return canonical;
    }
    return QFileInfo(path).absoluteFilePath();
}

QString BacklinksManager::getCachedNormalizedPath(const QString& path) {
    if (normalizedPathCache.contains(path)) {
        return normalizedPathCache[path];
    }
    QString normalized = normalizePath(path);
    normalizedPathCache[path] = normalized;
    return normalized;
}

void BacklinksManager::clearCache() {
    QMutexLocker locker(&mutex);
    normalizedPathCache.clear();
}
