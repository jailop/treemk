#ifndef WORKSPACEMANAGER_H
#define WORKSPACEMANAGER_H

#include <QObject>
#include <QString>

/**
 * @brief Manages folder workspace and main file loading
 * 
 * Coordinates opening folders and automatically loading main/index files.
 */
class WorkspaceManager : public QObject {
    Q_OBJECT
    
public:
    explicit WorkspaceManager(QObject *parent = nullptr);
    ~WorkspaceManager();
    
    /**
     * @brief Open folder and optionally load main file
     * @param folderPath Absolute path to folder
     * @param autoOpenMain Whether to search for and open main file
     * @param mainFileName Preferred main file name (empty = use default)
     * @return true if folder opened successfully
     */
    bool openFolder(const QString &folderPath, 
                   bool autoOpenMain = true,
                   const QString &mainFileName = QString());
    
    QString getCurrentFolder() const { return m_currentFolder; }
    QString getMainFile() const { return m_mainFile; }
    
signals:
    void folderOpened(const QString &folderPath);
    void mainFileFound(const QString &filePath);
    void noMainFileFound();
    
private:
    QString m_currentFolder;
    QString m_mainFile;
};

#endif // WORKSPACEMANAGER_H
