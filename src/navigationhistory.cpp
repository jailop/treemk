#include "navigationhistory.h"

NavigationHistory::NavigationHistory(QObject *parent)
    : QObject(parent), currentIndex(-1), navigating(false) {
}

void NavigationHistory::addFile(const QString &filePath) {
  // Don't add to history during back/forward navigation
  if (navigating) {
    return;
  }
  
  // Don't add if it's the same as current
  if (currentIndex >= 0 && currentIndex < history.size() && 
      history[currentIndex] == filePath) {
    return;
  }
  
  // Remove any forward history when adding a new entry
  if (currentIndex < history.size() - 1) {
    history.resize(currentIndex + 1);
  }
  
  // Add new entry
  history.append(filePath);
  currentIndex++;
  
  // Maintain maximum size (circular buffer behavior)
  if (history.size() > MAX_HISTORY_SIZE) {
    history.removeFirst();
    currentIndex--;
  }
  
  emit historyChanged();
  emit canGoBackChanged(canGoBack());
  emit canGoForwardChanged(canGoForward());
}

bool NavigationHistory::canGoBack() const {
  return currentIndex > 0;
}

bool NavigationHistory::canGoForward() const {
  return currentIndex >= 0 && currentIndex < history.size() - 1;
}

QString NavigationHistory::goBack() {
  if (!canGoBack()) {
    return QString();
  }
  
  navigating = true;
  currentIndex--;
  QString filePath = history[currentIndex];
  navigating = false;
  
  emit historyChanged();
  emit canGoBackChanged(canGoBack());
  emit canGoForwardChanged(canGoForward());
  
  return filePath;
}

QString NavigationHistory::goForward() {
  if (!canGoForward()) {
    return QString();
  }
  
  navigating = true;
  currentIndex++;
  QString filePath = history[currentIndex];
  navigating = false;
  
  emit historyChanged();
  emit canGoBackChanged(canGoBack());
  emit canGoForwardChanged(canGoForward());
  
  return filePath;
}

QString NavigationHistory::currentFile() const {
  if (currentIndex >= 0 && currentIndex < history.size()) {
    return history[currentIndex];
  }
  return QString();
}

QVector<QString> NavigationHistory::getHistory() const {
  return history;
}

void NavigationHistory::clear() {
  history.clear();
  currentIndex = -1;
  
  emit historyChanged();
  emit canGoBackChanged(false);
  emit canGoForwardChanged(false);
}
