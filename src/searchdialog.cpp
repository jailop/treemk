#include "searchdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

SearchDialog::SearchDialog(const QString &path, QWidget *parent)
    : QDialog(parent), rootPath(path)
{
    setWindowTitle(tr("Search in Files"));
    setMinimumSize(600, 400);
    
    setupUI();
}

SearchDialog::~SearchDialog()
{
}

void SearchDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Search input area
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel(tr("Search:"), this);
    searchEdit = new QLineEdit(this);
    searchButton = new QPushButton(tr("Search"), this);
    
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);
    
    mainLayout->addLayout(searchLayout);
    
    // Options
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    caseSensitiveCheck = new QCheckBox(tr("Case sensitive"), this);
    wholeWordCheck = new QCheckBox(tr("Whole word"), this);
    
    optionsLayout->addWidget(caseSensitiveCheck);
    optionsLayout->addWidget(wholeWordCheck);
    optionsLayout->addStretch();
    
    mainLayout->addLayout(optionsLayout);
    
    // Results list
    QLabel *resultsLabel = new QLabel(tr("Results:"), this);
    mainLayout->addWidget(resultsLabel);
    
    resultsView = new QListWidget(this);
    mainLayout->addWidget(resultsView);
    
    // Close button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    closeButton = new QPushButton(tr("Close"), this);
    buttonLayout->addWidget(closeButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(searchButton, &QPushButton::clicked, this, &SearchDialog::performSearch);
    connect(searchEdit, &QLineEdit::returnPressed, this, &SearchDialog::performSearch);
    connect(resultsView, &QListWidget::itemDoubleClicked, this, &SearchDialog::onResultDoubleClicked);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    searchEdit->setFocus();
}

void SearchDialog::performSearch()
{
    QString query = searchEdit->text();
    if (query.isEmpty()) {
        return;
    }
    
    resultsView->clear();
    
    bool caseSensitive = caseSensitiveCheck->isChecked();
    bool wholeWord = wholeWordCheck->isChecked();
    
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
        
        resultsView->addItem(item);
    }
    
    if (results.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem(tr("No results found"));
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        resultsView->addItem(item);
    }
}

void SearchDialog::onResultDoubleClicked()
{
    QListWidgetItem *item = resultsView->currentItem();
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

QList<SearchResult> SearchDialog::searchInFiles(const QString &query, bool caseSensitive, bool wholeWord)
{
    QList<SearchResult> results;
    scanDirectory(rootPath, query, caseSensitive, wholeWord, results);
    return results;
}

void SearchDialog::scanDirectory(const QString &dirPath, const QString &query, 
                                 bool caseSensitive, bool wholeWord, 
                                 QList<SearchResult> &results)
{
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
                QString pattern = QString("\\b%1\\b").arg(QRegularExpression::escape(query));
                QRegularExpression regex(pattern, 
                    caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
                found = regex.match(line).hasMatch();
            } else {
                found = line.contains(query, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
            }
            
            if (found) {
                SearchResult result(filePath, fileInfo.fileName(), lineNumber, line);
                results.append(result);
            }
        }
        
        file.close();
    }
    
    // Recursively scan subdirectories
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &subdirInfo : subdirs) {
        scanDirectory(subdirInfo.absoluteFilePath(), query, caseSensitive, wholeWord, results);
    }
}
