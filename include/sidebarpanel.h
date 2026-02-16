#ifndef SIDEBARPANEL_H
#define SIDEBARPANEL_H

#include <QWidget>

namespace Ui {
class SidebarPanel;
}

class QListWidget;
class QLineEdit;
class QTabWidget;
class FileSystemTreeView;
class OutlinePanel;

class SidebarPanel : public QWidget {
  Q_OBJECT

public:
  explicit SidebarPanel(QWidget *parent = nullptr);
  ~SidebarPanel();

  // Accessors for child widgets
  QTabWidget* getLeftTabWidget() const;
  QWidget* getTreePanel() const;
  QWidget* getOutlinePanel() const;
  QWidget* getBacklinksPanel() const;
  QWidget* getHistoryPanel() const;
  QListWidget* getBacklinksView() const;
  QListWidget* getHistoryView() const;
  QLineEdit* getHistoryFilterInput() const;

  // Methods to add custom widgets to panels
  void setTreeView(FileSystemTreeView* treeView);
  void setOutlineView(OutlinePanel* outlineView);

private:
  Ui::SidebarPanel *ui;
};

#endif // SIDEBARPANEL_H
