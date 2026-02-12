#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QPair>

/**
 * @brief SearchEngine provides full-text search capabilities for Markdown files
 * 
 * This class implements a search engine that can:
 * - Search through multiple files
 * - Extract context around matches
 * - Return ranked results with snippets
 * - Support case-insensitive search
 * - Handle both file paths and resource paths
 * 
 * Designed to be reused across the application for:
 * - Help system search
 * - Workspace-wide search
 * - Note search functionality
 */
class SearchEngine {
public:
  /**
   * @brief SearchResult represents a single search match
   */
  struct SearchResult {
    QString filePath;        // Full path to the file
    QString fileName;        // Just the filename
    QString title;           // Document title or topic name
    QString context;         // Snippet of text around the match
    int position;            // Character position of match in file
    int lineNumber;          // Line number of the match
    QString matchedText;     // The actual text that matched
    
    SearchResult() : position(0), lineNumber(0) {}
  };
  
  SearchEngine();
  ~SearchEngine();
  
  /**
   * @brief Search for a term in a single file
   * @param filePath Path to the file (can be Qt resource path like :/...)
   * @param searchTerm The text to search for
   * @param caseSensitive Whether search should be case-sensitive
   * @return List of all matches found in the file
   */
  QList<SearchResult> searchInFile(const QString &filePath, 
                                    const QString &searchTerm,
                                    bool caseSensitive = false);
  
  /**
   * @brief Search for a term across multiple files
   * @param filePaths List of file paths to search
   * @param searchTerm The text to search for
   * @param caseSensitive Whether search should be case-sensitive
   * @return List of all matches found across all files
   */
  QList<SearchResult> searchInFiles(const QStringList &filePaths,
                                     const QString &searchTerm,
                                     bool caseSensitive = false);
  
  /**
   * @brief Set the context size (characters) to extract around matches
   * @param size Number of characters before and after the match
   */
  void setContextSize(int size);
  
  /**
   * @brief Get the current context size
   * @return Context size in characters
   */
  int contextSize() const;
  
  /**
   * @brief Set maximum number of results per file
   * @param max Maximum results (0 = unlimited)
   */
  void setMaxResultsPerFile(int max);
  
  /**
   * @brief Get maximum results per file setting
   * @return Maximum results per file
   */
  int maxResultsPerFile() const;

private:
  QString extractContext(const QString &content, int position, const QString &searchTerm);
  int findLineNumber(const QString &content, int position);
  QString extractTitle(const QString &content);
  QString sanitizeContext(const QString &context);
  
  int m_contextSize;
  int m_maxResultsPerFile;
};

#endif // SEARCHENGINE_H
