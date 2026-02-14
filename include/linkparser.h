#ifndef LINKPARSER_H
#define LINKPARSER_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QVector>

class BacklinksManager;

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

class LinkParser : public QObject {
  Q_OBJECT

public:
  LinkParser(QObject *parent = nullptr);
  ~LinkParser();

  void setEnforceHomeBoundary(bool enforce);
  QVector<WikiLink> parseLinks(const QString &text);
  QVector<QString> extractLinksFromFile(const QString &filePath);
  void buildLinkIndex(const QString &rootPath, int maxDepth);
  QVector<QString> getBacklinks(const QString &filePath) const;
  QString resolveLinkTarget(const QString &linkTarget,
                            const QString &currentFilePath, int maxDepth) const;

signals:
  void indexBuildStarted();
  void indexBuildCompleted();
  void indexBuildProgress(int current, int total);

private:
  QMap<QString, QVector<QString>> forwardLinks;
  BacklinksManager *backlinksManager;
  QString rootPath;
  int maxDepth;
  bool enforceHomeBoundary;
  mutable QMutex mutex;

  void scanDirectory(const QString &dirPath, int currentDepth);
  void processFile(const QString &filePath);
  bool isWithinHomeDirectory(const QString &path) const;
  void searchInDirectory(const QString &dirPath, const QString &targetBaseName,
                         QString &result, int depth, int maxDepth) const;
};

#endif // LINKPARSER_H
