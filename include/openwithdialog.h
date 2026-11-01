#ifndef OPENWITHDIALOG_H
#define OPENWITHDIALOG_H

#include <QDialog>
#include <QString>
#include <QStringList>

class QCheckBox;
class QLineEdit;
class QListWidget;

class OpenWithDialog : public QDialog {
   public:
    explicit OpenWithDialog(const QString& filePath, QWidget* parent = nullptr);

   private:
    void populate();
    void applyFilter();
    void launchCurrent();

    QString filePath;
    QStringList fileMimeTypes;
    QCheckBox* showAllCheck;
    QLineEdit* searchEdit;
    QListWidget* appList;
};

#endif  // OPENWITHDIALOG_H
