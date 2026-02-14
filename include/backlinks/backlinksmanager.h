#ifndef BACKLINKSMANAGER_H
#define BACKLINKSMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QMutex>

/**
 * @brief Manages backlinks (reverse link index) for markdown files
 * 
 * This class builds and maintains a reverse index of links, allowing
 * fast lookups of which files link to a given document.
 */
class BacklinksManager : public QObject {
  Q_OBJECT

public:
  explicit BacklinksManager(QObject *parent = nullptr);
  
  /**
   * @brief Build backlinks map from forward links
   * @param forwardLinks Map of file -> list of link targets
   */
  void buildBacklinks(const QMap<QString, QVector<QString>> &forwardLinks);
  
  /**
   * @brief Get list of files that link to the given file
   * @param filePath Absolute path to the target file
   * @return List of files that contain links to the target
   */
  QVector<QString> getBacklinks(const QString &filePath) const;
  
  /**
   * @brief Clear the backlinks index
   */
  void clear();

signals:
  void backlinksUpdated();

private:
  QMap<QString, QVector<QString>> backLinksMap;
  mutable QMutex mutex;
  
  QString normalizePath(const QString &path) const;
};

#endif // BACKLINKSMANAGER_H
