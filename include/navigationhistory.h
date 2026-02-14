#ifndef NAVIGATIONHISTORY_H
#define NAVIGATIONHISTORY_H

#include <QObject>
#include <QString>
#include <QVector>

/**
 * @brief Manages navigation history for back/forward functionality
 * 
 * Uses a circular buffer with maximum size of 2048 entries.
 * Similar to browser navigation history.
 */
class NavigationHistory : public QObject {
  Q_OBJECT

public:
  explicit NavigationHistory(QObject *parent = nullptr);
  
  // Add a new file to history
  void addFile(const QString &filePath);
  
  // Navigation
  bool canGoBack() const;
  bool canGoForward() const;
  QString goBack();
  QString goForward();
  
  // Get current file
  QString currentFile() const;
  
  // Get all history for display
  QVector<QString> getHistory() const;
  
  // Clear history
  void clear();

signals:
  void historyChanged();
  void canGoBackChanged(bool canGoBack);
  void canGoForwardChanged(bool canGoForward);

private:
  static const int MAX_HISTORY_SIZE = 2048;
  
  QVector<QString> history;
  int currentIndex;
  bool navigating; // Flag to prevent adding entries during back/forward navigation
};

#endif // NAVIGATIONHISTORY_H
