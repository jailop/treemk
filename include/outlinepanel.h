#ifndef OUTLINEPANEL_H
#define OUTLINEPANEL_H

#include <QString>
#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;

struct OutlineItem {
  int level;
  QString text;
  int lineNumber;

  OutlineItem(int l, const QString &t, int ln)
      : level(l), text(t), lineNumber(ln) {}
};

class OutlinePanel : public QWidget {
  Q_OBJECT

public:
  explicit OutlinePanel(QWidget *parent = nullptr);
  ~OutlinePanel();

  void updateOutline(const QString &markdown);

signals:
  void headerClicked(int lineNumber);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void onItemClicked(QTreeWidgetItem *item, int column);
  void showContextMenu(const QPoint &pos);

private:
  void setupUI();
  QList<OutlineItem> parseHeaders(const QString &markdown);
  void buildTree(const QList<OutlineItem> &headers);

  QTreeWidget *outlineTree;
};

#endif // OUTLINEPANEL_H
