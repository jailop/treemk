#include "quickopendialog.h"
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

QuickOpenDialog::QuickOpenDialog(const QString &path, const QStringList &recent,
                                 QWidget *parent)
    : QDialog(parent), rootPath(path), recentFiles(recent) {
  setWindowTitle(tr("Quick Open"));
  setMinimumSize(600, 400);

  setupUI();

  // Scan all markdown files
  scanFiles(rootPath, allFiles);

  updateFileList();
}

QuickOpenDialog::~QuickOpenDialog() {}

void QuickOpenDialog::setupUI() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  searchEdit = new QLineEdit(this);
  searchEdit->setPlaceholderText(tr("Type to search files..."));
  mainLayout->addWidget(searchEdit);

  fileListWidget = new QListWidget(this);
  mainLayout->addWidget(fileListWidget);

  connect(searchEdit, &QLineEdit::textChanged, this,
          &QuickOpenDialog::updateFileList);
  connect(fileListWidget, &QListWidget::itemDoubleClicked, this,
          &QuickOpenDialog::onItemDoubleClicked);

  searchEdit->setFocus();

  // Install event filter for keyboard navigation
  searchEdit->installEventFilter(this);
}

bool QuickOpenDialog::eventFilter(QObject *obj, QEvent *event) {
  if (obj == searchEdit && event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    if (keyEvent->key() == Qt::Key_Down) {
      fileListWidget->setFocus();
      if (fileListWidget->count() > 0) {
        fileListWidget->setCurrentRow(0);
      }
      return true;
    } else if (keyEvent->key() == Qt::Key_Return ||
               keyEvent->key() == Qt::Key_Enter) {
      if (fileListWidget->count() > 0) {
        fileListWidget->setCurrentRow(0);
        onItemDoubleClicked();
      }
      return true;
    }
  }

  return QDialog::eventFilter(obj, event);
}

void QuickOpenDialog::updateFileList() {
  fileListWidget->clear();

  QString pattern = searchEdit->text().toLower();

  // Show recent files first if no search pattern
  if (pattern.isEmpty()) {
    for (const QString &recent : recentFiles) {
      // Show relative path if within rootPath
      QString displayText;
      if (!rootPath.isEmpty() && recent.startsWith(rootPath + "/")) {
        displayText = recent.mid(rootPath.length() + 1) + " [Recent]";
      } else {
        QFileInfo info(recent);
        displayText = info.fileName() + " [Recent]";
      }
      
      QListWidgetItem *item = new QListWidgetItem(displayText);
      item->setData(Qt::UserRole, recent);
      item->setToolTip(recent);
      fileListWidget->addItem(item);
    }
  }

  // Show matching files
  for (const QString &filePath : allFiles) {
    // Skip if already in recent list and no search
    if (pattern.isEmpty() && recentFiles.contains(filePath)) {
      continue;
    }

    // Show relative path if within rootPath
    QString displayText;
    if (!rootPath.isEmpty() && filePath.startsWith(rootPath + "/")) {
      displayText = filePath.mid(rootPath.length() + 1);
    } else {
      QFileInfo info(filePath);
      displayText = info.fileName();
    }

    if (pattern.isEmpty() || fuzzyMatch(pattern, displayText.toLower())) {
      QListWidgetItem *item = new QListWidgetItem(displayText);
      item->setData(Qt::UserRole, filePath);
      item->setToolTip(filePath);
      fileListWidget->addItem(item);

      // Limit results
      if (fileListWidget->count() >= 50) {
        break;
      }
    }
  }
}

void QuickOpenDialog::onItemDoubleClicked() {
  QListWidgetItem *item = fileListWidget->currentItem();
  if (item) {
    selectedFile = item->data(Qt::UserRole).toString();
    emit fileSelected(selectedFile);
    accept();
  }
}

QString QuickOpenDialog::getSelectedFile() const { return selectedFile; }

void QuickOpenDialog::scanFiles(const QString &dirPath, QList<QString> &files) {
  QDir dir(dirPath);

  QStringList filters;
  filters << "*.md" << "*.markdown";

  QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files, QDir::Name);
  for (const QFileInfo &info : fileList) {
    files.append(info.absoluteFilePath());
  }

  // Recursively scan subdirectories
  QFileInfoList subdirs =
      dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
  for (const QFileInfo &subdir : subdirs) {
    scanFiles(subdir.absoluteFilePath(), files);
  }
}

bool QuickOpenDialog::fuzzyMatch(const QString &pattern,
                                 const QString &text) const {
  int patternIndex = 0;
  int textIndex = 0;

  while (patternIndex < pattern.length() && textIndex < text.length()) {
    if (pattern[patternIndex] == text[textIndex]) {
      patternIndex++;
    }
    textIndex++;
  }

  return patternIndex == pattern.length();
}
