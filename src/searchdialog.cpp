#include "searchdialog.h"
#include "ui_searchdialog.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

SearchDialog::SearchDialog(const QString &path, QWidget *parent)
    : QDialog(parent), ui(new Ui::SearchDialog), rootPath(path) {
  ui->setupUi(this);
  
  // Connect signals
  connect(ui->searchButton, &QPushButton::clicked, this,
          &SearchDialog::performSearch);
  connect(ui->searchEdit, &QLineEdit::returnPressed, this,
          &SearchDialog::performSearch);
  connect(ui->resultsView, &QListWidget::itemDoubleClicked, this,
          &SearchDialog::onResultDoubleClicked);
  connect(ui->closeButton, &QPushButton::clicked, this, &QDialog::accept);

  ui->searchEdit->setFocus();
}

SearchDialog::~SearchDialog() {
  delete ui;
}

void SearchDialog::performSearch() {
  QString query = ui->searchEdit->text();
  if (query.isEmpty()) {
    return;
  }

  ui->resultsView->clear();

  bool caseSensitive = ui->caseSensitiveCheck->isChecked();
  bool wholeWord = ui->wholeWordCheck->isChecked();

  QList<SearchResult> results = searchInFiles(query, caseSensitive, wholeWord);

  for (const SearchResult &result : results) {
    QString displayText = QString("%1 [Line %2]: %3")
                              .arg(result.fileName)
                              .arg(result.lineNumber)
                              .arg(result.lineContent.trimmed());

    QListWidgetItem *item = new QListWidgetItem(displayText);
    item->setData(Qt::UserRole, result.filePath);
    item->setData(Qt::UserRole + 1, result.lineNumber);
    item->setToolTip(result.filePath);
    ui->resultsView->addItem(item);
  }

  if (results.isEmpty()) {
    QListWidgetItem *item = new QListWidgetItem(tr("No results found"));
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    ui->resultsView->addItem(item);
  }
}

void SearchDialog::onResultDoubleClicked() {
  QListWidgetItem *item = ui->resultsView->currentItem();
  if (!item) {
    return;
  }

  QString filePath = item->data(Qt::UserRole).toString();
  int lineNumber = item->data(Qt::UserRole + 1).toInt();

  if (!filePath.isEmpty()) {
    emit fileSelected(filePath, lineNumber);
    accept();
  }
}

QList<SearchResult> SearchDialog::searchInFiles(const QString &query,
                                                bool caseSensitive,
                                                bool wholeWord) {
  QList<SearchResult> results;
  scanDirectory(rootPath, query, caseSensitive, wholeWord, results);
  return results;
}

void SearchDialog::scanDirectory(const QString &dirPath, const QString &query,
                                 bool caseSensitive, bool wholeWord,
                                 QList<SearchResult> &results) {
  QDir dir(dirPath);

  // Process markdown files
  QStringList filters;
  filters << "*.md" << "*.markdown";

  QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
  for (const QFileInfo &fileInfo : files) {
    QString filePath = fileInfo.absoluteFilePath();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      continue;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    int lineNumber = 0;
    while (!in.atEnd()) {
      lineNumber++;
      QString line = in.readLine();

      bool found = false;
      if (wholeWord) {
        QString pattern =
            QString("\\b%1\\b").arg(QRegularExpression::escape(query));
        QRegularExpression regex(
            pattern, caseSensitive ? QRegularExpression::NoPatternOption
                                   : QRegularExpression::CaseInsensitiveOption);
        found = regex.match(line).hasMatch();
      } else {
        found = line.contains(query, caseSensitive ? Qt::CaseSensitive
                                                   : Qt::CaseInsensitive);
      }

      if (found) {
        SearchResult result(filePath, fileInfo.fileName(), lineNumber, line);
        results.append(result);
      }
    }

    file.close();
  }

  // Recursively scan subdirectories
  QFileInfoList subdirs =
      dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QFileInfo &subdirInfo : subdirs) {
    scanDirectory(subdirInfo.absoluteFilePath(), query, caseSensitive,
                  wholeWord, results);
  }
}
