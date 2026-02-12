#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include "searchengine.h"

class MarkdownPreview;
class QListWidgetItem;

namespace Ui {
class HelpDialog;
}

/**
 * @brief HelpDialog displays the integrated user guide
 * 
 * This dialog provides access to embedded help documentation using
 * the application's Markdown rendering engine. Help content is loaded
 * from Qt resources (embedded in the binary).
 * 
 * Features:
 * - Table of contents navigation
 * - Markdown rendering with preview widget
 * - Theme-aware styling
 * - Navigation history (back/home)
 */
class HelpDialog : public QDialog {
  Q_OBJECT

public:
  explicit HelpDialog(QWidget *parent = nullptr);
  ~HelpDialog();

  /**
   * @brief Open help at a specific topic
   * @param topic The help file name (without .md extension)
   */
  void showTopic(const QString &topic);

private slots:
  void onTableOfContentsItemClicked(int index);
  void onHomeClicked();
  void onBackClicked();
  void onForwardClicked();
  void onLinkClicked(const QString &link);
  void onSearchTextChanged(const QString &text);
  void onSearchResultClicked(QListWidgetItem *item);
  void onPrintClicked();

private:
  void setupContent();
  void loadHelpContent(const QString &fileName);
  void addToHistory(const QString &fileName);
  void updateNavigationButtons();
  void updateBreadcrumb();
  void performSearch(const QString &searchTerm);
  
  Ui::HelpDialog *ui;
  MarkdownPreview *contentView;
  SearchEngine *searchEngine;
  
  // Navigation
  QStringList navigationHistory;
  int historyIndex;
  
  // Help topics metadata
  struct HelpTopic {
    QString fileName;
    QString displayName;
    QString description;
  };
  QList<HelpTopic> topics;
  
  void initializeTopics();
};

#endif // HELPDIALOG_H
