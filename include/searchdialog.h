#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>

class QLineEdit;
class QListWidget;
class QPushButton;
class QCheckBox;

struct SearchResult
{
    QString filePath;
    QString fileName;
    int lineNumber;
    QString lineContent;
    
    SearchResult(const QString &path, const QString &name, int line, const QString &content)
        : filePath(path), fileName(name), lineNumber(line), lineContent(content) {}
};

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(const QString &rootPath, QWidget *parent = nullptr);
    ~SearchDialog();

signals:
    void fileSelected(const QString &filePath, int lineNumber);

private slots:
    void performSearch();
    void onResultDoubleClicked();

private:
    void setupUI();
    QList<SearchResult> searchInFiles(const QString &query, bool caseSensitive, bool wholeWord);
    void scanDirectory(const QString &dirPath, const QString &query, bool caseSensitive, 
                      bool wholeWord, QList<SearchResult> &results);

    QString rootPath;
    
    QLineEdit *searchEdit;
    QListWidget *resultsView;
    QPushButton *searchButton;
    QPushButton *closeButton;
    QCheckBox *caseSensitiveCheck;
    QCheckBox *wholeWordCheck;
};

#endif // SEARCHDIALOG_H
