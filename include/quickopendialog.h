#ifndef QUICKOPENDIALOG_H
#define QUICKOPENDIALOG_H

#include <QDialog>
#include <QList>
#include <QString>

class QLineEdit;
class QListWidget;

class QuickOpenDialog : public QDialog {
  Q_OBJECT

public:
  explicit QuickOpenDialog(const QString &rootPath,
                           const QStringList &recentFiles,
                           QWidget *parent = nullptr);
  ~QuickOpenDialog();

  QString getSelectedFile() const;

signals:
  void fileSelected(const QString &filePath);

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void updateFileList();
  void onItemDoubleClicked();

private:
  void setupUI();
  void scanFiles(const QString &dirPath, QList<QString> &files);
  bool fuzzyMatch(const QString &pattern, const QString &text) const;

  QString rootPath;
  QStringList recentFiles;
  QList<QString> allFiles;

  QLineEdit *searchEdit;
  QListWidget *fileListWidget;
  QString selectedFile;
};

#endif // QUICKOPENDIALOG_H
