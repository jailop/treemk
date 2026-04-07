#ifndef WORKSPACEMANAGER_H
#define WORKSPACEMANAGER_H

#include <QObject>
#include <QString>

/**
 * Coordinates opening folders and automatically loading main/index files.
 */
class WorkspaceManager : public QObject {
    Q_OBJECT

   public:
    explicit WorkspaceManager(QObject* parent = nullptr);
    ~WorkspaceManager();

    /**
     * @brief Open folder and optionally load main file
     * @param folderPath Absolute path to folder
     * @return true if folder opened successfully
     */
    bool openFolder(const QString& folderPath);

    QString getCurrentFolder() const { return m_currentFolder; }

   signals:
    void folderOpened(const QString& folderPath);

   private:
    QString m_currentFolder;
};

#endif  // WORKSPACEMANAGER_H
