#include "linkparser.h"
#include "backlinks/backlinksmanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QTextStream>

static const QString MARKDOWN_FILTERS[] = {"*.md", "*.markdown"};
static const int MARKDOWN_FILTER_COUNT = 2;

LinkParser::LinkParser(QObject *parent) 
    : QObject(parent), maxDepth(2), enforceHomeBoundary(true) {
  backlinksManager = new BacklinksManager(this);
}

LinkParser::~LinkParser() {
}

void LinkParser::setEnforceHomeBoundary(bool enforce) {
  enforceHomeBoundary = enforce;
}

QVector<WikiLink> LinkParser::parseLinks(const QString &text) {
  QVector<WikiLink> links;

  // Pattern for [[!target]] or [[!target|display]] or [[target]] or
  // [[target|display]]
  QRegularExpression wikiLinkPattern(
      "\\[\\[(!)?([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
  QRegularExpressionMatchIterator wikiIterator =
      wikiLinkPattern.globalMatch(text);

  while (wikiIterator.hasNext()) {
    QRegularExpressionMatch match = wikiIterator.next();
    bool isInclusion = !match.captured(1).isEmpty();
    QString target = match.captured(2).trimmed();
    QString display = match.captured(4).trimmed();

    if (display.isEmpty()) {
      display = target;
    }

    WikiLink link(target, display, match.capturedStart(),
                  match.capturedLength(), isInclusion);
    links.append(link);
  }
  
  // Pattern for markdown links: [text](url)
  // Only capture links that look like local file paths (relative or absolute)
  QRegularExpression markdownLinkPattern(
      "\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
  QRegularExpressionMatchIterator mdIterator =
      markdownLinkPattern.globalMatch(text);

  while (mdIterator.hasNext()) {
    QRegularExpressionMatch match = mdIterator.next();
    QString display = match.captured(1).trimmed();
    QString target = match.captured(2).trimmed();
    
    // Only include links that appear to be local file paths
    // Skip URLs (http://, https://, mailto:, etc.)
    if (!target.contains("://") && !target.startsWith("mailto:") && 
        !target.startsWith("#")) {
      WikiLink link(target, display, match.capturedStart(),
                    match.capturedLength(), false);
      links.append(link);
    }
  }

  return links;
}

QVector<QString> LinkParser::extractLinksFromFile(const QString &filePath) {
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

void LinkParser::buildLinkIndex(const QString &path, int depth) {
  emit indexBuildStarted();
  
  QMutexLocker locker(&mutex);
  rootPath = path;
  maxDepth = depth;
  forwardLinks.clear();
  locker.unlock();

  scanDirectory(path, 0);
  
  // Build backlinks using the BacklinksManager
  backlinksManager->buildBacklinks(forwardLinks);
  
  emit indexBuildCompleted();
}

QVector<QString> LinkParser::getBacklinks(const QString &filePath) const {
  return backlinksManager->getBacklinks(filePath);
}

QString LinkParser::resolveLinkTarget(const QString &linkTarget,
                                      const QString &currentFilePath,
                                      int searchDepth) const {
  QString cleanTarget = linkTarget.trimmed();
  QFileInfo currentFileInfo(currentFilePath);
  QDir currentDir = currentFileInfo.dir();

  QStringList possibleExtensions;
  possibleExtensions << ".md" << ".markdown" << "";

  QString result;

  for (int depth = 0; depth <= searchDepth; ++depth) {
    if (depth == 0) {
      for (const QString &ext : possibleExtensions) {
        QString fullPath = currentDir.filePath(cleanTarget + ext);
        if (QFileInfo::exists(fullPath)) {
          return fullPath;
        }
      }

      searchInDirectory(currentDir.absolutePath(), cleanTarget, result, 0, searchDepth);
      if (!result.isEmpty()) {
        return result;
      }
    } else {
      if (!currentDir.cdUp() || (enforceHomeBoundary && !isWithinHomeDirectory(currentDir.absolutePath()))) {
        break;
      }

      QFileInfoList entries = currentDir.entryInfoList(
          QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
      for (const QFileInfo &entry : entries) {
        searchInDirectory(entry.absoluteFilePath(), cleanTarget, result, 0, searchDepth);
        if (!result.isEmpty()) {
          return result;
        }
      }
    }
  }

  return QString();
}

void LinkParser::scanDirectory(const QString &dirPath, int currentDepth) {
  if (currentDepth > maxDepth || (enforceHomeBoundary && !isWithinHomeDirectory(dirPath))) {
    return;
  }

  QDir dir(dirPath);
  QStringList filters;
  for (int i = 0; i < MARKDOWN_FILTER_COUNT; ++i) {
    filters << MARKDOWN_FILTERS[i];
  }

  QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
  for (const QFileInfo &fileInfo : files) {
    processFile(fileInfo.absoluteFilePath());
  }

  QFileInfoList subdirs =
      dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QFileInfo &subdirInfo : subdirs) {
    scanDirectory(subdirInfo.absoluteFilePath(), currentDepth + 1);
  }
}

void LinkParser::processFile(const QString &filePath) {
  QVector<QString> links = extractLinksFromFile(filePath);

  // Always add file to forward links, even if empty
  // This is needed so BacklinksManager knows about all scanned files
  QMutexLocker locker(&mutex);
  forwardLinks[filePath] = links;
}

bool LinkParser::isWithinHomeDirectory(const QString &path) const {
  QString homePath = QDir::homePath();
  QString canonicalPath = QFileInfo(path).canonicalFilePath();
  QString canonicalHome = QFileInfo(homePath).canonicalFilePath();
  
  return canonicalPath.startsWith(canonicalHome);
}

void LinkParser::searchInDirectory(const QString &dirPath,
                                    const QString &targetBaseName,
                                    QString &result, int depth,
                                    int maxDepth) const {
  if (depth > maxDepth || !result.isEmpty() || (enforceHomeBoundary && !isWithinHomeDirectory(dirPath))) {
    return;
  }

  QDir dir(dirPath);
  if (!dir.exists()) {
    return;
  }

  QStringList filters;
  for (int i = 0; i < MARKDOWN_FILTER_COUNT; ++i) {
    filters << MARKDOWN_FILTERS[i];
  }

  QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
  for (const QFileInfo &fileInfo : files) {
    QString baseName = fileInfo.completeBaseName();
    if (baseName.compare(targetBaseName, Qt::CaseInsensitive) == 0) {
      result = fileInfo.absoluteFilePath();
      return;
    }
  }

  QFileInfoList subdirs =
      dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QFileInfo &subdirInfo : subdirs) {
    searchInDirectory(subdirInfo.absoluteFilePath(), targetBaseName, result,
                      depth + 1, maxDepth);
    if (!result.isEmpty()) {
      return;
    }
  }
}
