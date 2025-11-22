#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialog>

class QTableWidget;

class ShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutsDialog(QWidget *parent = nullptr);

private:
    void setupUI();
    void addShortcut(const QString &action, const QString &shortcut, const QString &description);

    QTableWidget *shortcutsTable;
};

#endif // SHORTCUTSDIALOG_H
