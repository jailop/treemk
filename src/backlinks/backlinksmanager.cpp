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

    // For each scanned file
    for (auto it = forwardLinks.constBegin(); it != forwardLinks.constEnd();
         ++it) {
        const QString& sourceFile = it.key();
        const QVector<QString>& linkTargets = it.value();
        QFileInfo sourceInfo(sourceFile);
        QDir sourceDir = sourceInfo.dir();

        // For each link in this file
        for (const QString& linkTarget : linkTargets) {
            QString link = linkTarget.trimmed();

            // Build possible absolute paths for this link
            QStringList candidatePaths;
            QFileInfo linkInfo(link);

            if (linkInfo.suffix().isEmpty()) {
                // No extension - try .md and .markdown
                candidatePaths << sourceDir.filePath(link + ".md");
                candidatePaths << sourceDir.filePath(link + ".markdown");
                candidatePaths << sourceDir.filePath(link);
            } else {
                // Has extension - use as-is
                candidatePaths << sourceDir.filePath(link);
            }

            // Normalize candidate paths
            QStringList normalizedCandidates;
            for (const QString& candidate : candidatePaths) {
                normalizedCandidates << normalizePath(candidate);
            }

            // Check each candidate against scanned files
            for (const QString& candidateNorm : normalizedCandidates) {
                for (auto fit = forwardLinks.constBegin();
                     fit != forwardLinks.constEnd(); ++fit) {
                    const QString& scannedFile = fit.key();
                    QString scannedNorm = normalizePath(scannedFile);

                    if (candidateNorm.compare(scannedNorm,
                                              Qt::CaseInsensitive) == 0) {
                        // Use normalized path as key for consistent lookups
                        if (!backLinksMap[scannedNorm].contains(sourceFile)) {
                            backLinksMap[scannedNorm].append(sourceFile);
                        }
                        goto next_link;  // Found match, move to next link
                    }
                }
            }
        next_link:;
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
    emit backlinksUpdated();
}

QString BacklinksManager::normalizePath(const QString& path) const {
    QString canonical = QFileInfo(path).canonicalFilePath();
    if (!canonical.isEmpty()) {
        return canonical;
    }
    return QFileInfo(path).absoluteFilePath();
}
