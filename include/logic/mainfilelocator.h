#ifndef MAINFILELOCATOR_H
#define MAINFILELOCATOR_H

#include <QString>
#include <QStringList>

/**
 * @brief Logic for locating main/index files in folders
 * 
 * Pure logic class with no UI dependencies, fully testable.
 */
class MainFileLocator {
public:
    /**
     * @brief Find main file in a folder
     * @param folderPath Absolute path to folder
     * @param preferredName User-configured preferred filename (e.g., "main.md")
     * @return Absolute path to found file, or empty string if not found
     */
    static QString findMainFile(const QString &folderPath, 
                               const QString &preferredName = "main.md");
    
    /**
     * @brief Get list of fallback filenames to search
     * @return List of fallback names in order of priority
     */
    static QStringList getFallbackNames();

private:
    static const QStringList FALLBACK_NAMES;
    
    /**
     * @brief Check if file exists in folder (case-insensitive on some platforms)
     * @param folderPath Folder to search in
     * @param fileName File name to search for
     * @return Absolute path if found, empty string otherwise
     */
    static QString findFile(const QString &folderPath, const QString &fileName);
};

#endif // MAINFILELOCATOR_H
