#ifndef OUTLINEPANEL_H
#define OUTLINEPANEL_H

#include <QWidget>
#include <QString>

class QListWidget;

struct OutlineItem {
    int level;
    QString text;
    int lineNumber;
    
    OutlineItem(int l, const QString &t, int ln)
        : level(l), text(t), lineNumber(ln) {}
};

class OutlinePanel : public QWidget
{
    Q_OBJECT

public:
    explicit OutlinePanel(QWidget *parent = nullptr);
    ~OutlinePanel();

    void updateOutline(const QString &markdown);

signals:
    void headerClicked(int lineNumber);

private slots:
    void onItemDoubleClicked();

private:
    void setupUI();
    QList<OutlineItem> parseHeaders(const QString &markdown);
    
    QListWidget *outlineList;
};

#endif // OUTLINEPANEL_H
