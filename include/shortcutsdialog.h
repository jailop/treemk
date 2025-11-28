#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialog>

class QTableWidget;
class QPushButton;

class ShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutsDialog(QWidget *parent = nullptr);

private slots:
    void onCellClicked(int row, int column);
    void onResetDefaults();
    void onSave();

private:
    void setupUI();
    void loadShortcuts();
    void addShortcut(const QString &category, const QString &action, const QString &shortcut);

    QTableWidget *shortcutsTable;
    QPushButton *resetButton;
    QPushButton *saveButton;
    QPushButton *cancelButton;
};

#endif // SHORTCUTSDIALOG_H
