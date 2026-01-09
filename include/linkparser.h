#ifndef LINKPARSER_H
#define LINKPARSER_H

#include <QMap>
#include <QString>
#include <QVector>

struct WikiLink {
  QString targetFile;
  QString displayText;
  int startPos;
  int length;
  bool isInclusion;

  WikiLink() : startPos(0), length(0), isInclusion(false) {}
  WikiLink(const QString &target, const QString &display, int start, int len,
           bool inclusion = false)
      : targetFile(target), displayText(display), startPos(start), length(len),
        isInclusion(inclusion) {}
};

class LinkParser {
public:
  LinkParser();

  // Parse wiki links from markdown text
  QVector<WikiLink> parseLinks(const QString &text);

  // Extract links from a file
  QVector<QString> extractLinksFromFile(const QString &filePath);

  // Build link index for a folder
  void buildLinkIndex(const QString &rootPath);

  // Get backlinks for a file
  QVector<QString> getBacklinks(const QString &filePath) const;

  // Resolve a link target to actual file path
  QString resolveLinkTarget(const QString &linkTarget,
                            const QString &rootPath) const;

private:
  // Map from file path to list of files it links to
  QMap<QString, QVector<QString>> forwardLinks;

  // Map from file path to list of files that link to it
  QMap<QString, QVector<QString>> backLinks;

  QString rootPath;

  void scanDirectory(const QString &dirPath);
  void processFile(const QString &filePath);
};

#endif // LINKPARSER_H
