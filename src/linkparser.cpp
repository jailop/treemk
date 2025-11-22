#include "linkparser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDir>
#include <QFileInfo>

LinkParser::LinkParser()
{
}

QVector<WikiLink> LinkParser::parseLinks(const QString &text)
{
    QVector<WikiLink> links;
    
    // Pattern for [[!target]] or [[!target|display]] or [[target]] or [[target|display]]
    QRegularExpression wikiLinkPattern("\\[\\[(!)?([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator matchIterator = wikiLinkPattern.globalMatch(text);
    
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        bool isInclusion = !match.captured(1).isEmpty();
        QString target = match.captured(2).trimmed();
        QString display = match.captured(4).trimmed();
        
        if (display.isEmpty()) {
            display = target;
        }
        
        WikiLink link(target, display, match.capturedStart(), match.capturedLength(), isInclusion);
        links.append(link);
    }
    
    return links;
}

QVector<QString> LinkParser::extractLinksFromFile(const QString &filePath)
{
    QVector<QString> links;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return links;
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    
    QVector<WikiLink> parsedLinks = parseLinks(content);
    
    for (const WikiLink &link : parsedLinks) {
        if (!links.contains(link.targetFile)) {
            links.append(link.targetFile);
        }
    }
    
    return links;
}

void LinkParser::buildLinkIndex(const QString &path)
{
    rootPath = path;
    forwardLinks.clear();
    backLinks.clear();
    
    scanDirectory(path);
}

QVector<QString> LinkParser::getBacklinks(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.completeBaseName();
    
    QVector<QString> result;
    
    // Check all files in the backlinks map
    for (auto it = forwardLinks.constBegin(); it != forwardLinks.constEnd(); ++it) {
        const QString &sourceFile = it.key();
        const QVector<QString> &targets = it.value();
        
        for (const QString &target : targets) {
            // Check if this target matches our file
            if (target.compare(baseName, Qt::CaseInsensitive) == 0) {
                if (!result.contains(sourceFile)) {
                    result.append(sourceFile);
                }
            }
        }
    }
    
    return result;
}

QString LinkParser::resolveLinkTarget(const QString &linkTarget, const QString &currentFilePath) const
{
    QString cleanTarget = linkTarget.trimmed();
    
    // Get the directory of the current file
    QFileInfo currentFileInfo(currentFilePath);
    QDir currentDir = currentFileInfo.dir();
    
    // Try exact matches relative to current file's directory
    QStringList possibleFiles;
    possibleFiles << cleanTarget + ".md"
                  << cleanTarget + ".markdown"
                  << cleanTarget;
    
    for (const QString &fileName : possibleFiles) {
        QString fullPath = currentDir.filePath(fileName);
        if (QFileInfo::exists(fullPath)) {
            return fullPath;
        }
    }
    
    // Case-insensitive search in current directory
    QStringList filters;
    filters << "*.md" << "*.markdown";
    
    QFileInfoList files = currentDir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        QString baseName = fileInfo.completeBaseName();
        if (baseName.compare(cleanTarget, Qt::CaseInsensitive) == 0) {
            return fileInfo.absoluteFilePath();
        }
    }
    
    // If not found in current directory, search in subdirectories
    QFileInfoList subdirs = currentDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &subdirInfo : subdirs) {
        QDir subdir(subdirInfo.absoluteFilePath());
        QFileInfoList subFiles = subdir.entryInfoList(filters, QDir::Files, QDir::Name);
        for (const QFileInfo &fileInfo : subFiles) {
            QString baseName = fileInfo.completeBaseName();
            if (baseName.compare(cleanTarget, Qt::CaseInsensitive) == 0) {
                return fileInfo.absoluteFilePath();
            }
        }
    }
    
    return QString();
}

void LinkParser::scanDirectory(const QString &dirPath)
{
    QDir dir(dirPath);
    
    // Process markdown files in this directory
    QStringList filters;
    filters << "*.md" << "*.markdown";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        processFile(fileInfo.absoluteFilePath());
    }
    
    // Recursively scan subdirectories
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &subdirInfo : subdirs) {
        scanDirectory(subdirInfo.absoluteFilePath());
    }
}

void LinkParser::processFile(const QString &filePath)
{
    QVector<QString> links = extractLinksFromFile(filePath);
    
    if (!links.isEmpty()) {
        forwardLinks[filePath] = links;
    }
}
