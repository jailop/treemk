#include "tabeditor.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "outlinepanel.h"
#include "fileutils.h"
#include <QFileInfo>
#include <QScrollBar>
#include <QSplitter>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

TabEditor::TabEditor(QWidget *parent)
    : QWidget(parent), m_editor(nullptr), m_preview(nullptr),
      m_splitter(nullptr), m_previewTimer(nullptr), m_isModified(false),
      m_ownSaved(false) {
  setupUI();
}

TabEditor::~TabEditor() {}

void TabEditor::setupUI() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  m_splitter = new QSplitter(Qt::Horizontal, this);
  m_editor = new MarkdownEditor(this);
  connect(m_editor, &MarkdownEditor::textChanged, this,
          &TabEditor::onDocumentModified);
  connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged, this,
          &TabEditor::syncPreviewScroll);
  connect(m_editor, &MarkdownEditor::cursorPositionChanged, this,
          &TabEditor::syncPreviewScroll);
  
  // Forward editor signals
  connect(m_editor, &MarkdownEditor::wikiLinkClicked, this,
          &TabEditor::wikiLinkClicked);
  connect(m_editor, &MarkdownEditor::markdownLinkClicked, this,
          &TabEditor::markdownLinkClicked);
  connect(m_editor, &MarkdownEditor::openLinkInNewWindowRequested, this,
          &TabEditor::openLinkInNewWindowRequested);
  connect(m_editor, &MarkdownEditor::aiAssistRequested, this,
          &TabEditor::aiAssistRequested);
  
  m_preview = new MarkdownPreview(this);
  m_preview->setMinimumWidth(300);

  m_splitter->addWidget(m_editor);
  m_splitter->addWidget(m_preview);
  m_splitter->setStretchFactor(0, 1);
  m_splitter->setStretchFactor(1, 1);
  
  // Set equal sizes for editor and preview (50/50 split)
  // Use a large initial size that will be adjusted by the layout
  QList<int> sizes;
  sizes << 1000 << 1000;  // Equal sizes
  m_splitter->setSizes(sizes);
  
  mainLayout->addWidget(m_splitter);
  m_previewTimer = new QTimer(this);
  m_previewTimer->setSingleShot(true);
  m_previewTimer->setInterval(1000);
  connect(m_previewTimer, &QTimer::timeout, this, &TabEditor::updatePreview);
}

QString TabEditor::fileName() const {
  if (m_filePath.isEmpty()) {
    return tr("Untitled");
  }
  return QFileInfo(m_filePath).fileName();
}

void TabEditor::setModified(bool modified) {
  if (m_isModified != modified) {
    m_isModified = modified;
    emit modificationChanged(modified);
  }
}

bool TabEditor::loadFile(const QString &filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }

  QTextStream in(&file);
  in.setEncoding(QStringConverter::Utf8);
  QString content = in.readAll();
  file.close();

  m_editor->setPlainText(content);
  m_editor->document()->setModified(false);
  
  m_filePath = filePath;
  m_editor->setCurrentFilePath(filePath);
  m_isModified = false;

  updatePreview();

  emit filePathChanged(filePath);
  emit modificationChanged(false);

  return true;
}

bool TabEditor::saveFile() {
  if (m_filePath.isEmpty()) {
    return false;
  }
  m_ownSaved = true;
  return saveFileAs(m_filePath);
}

bool TabEditor::saveFileAs(const QString &filePath) {
  FileUtils::FileCreationResult result = 
      FileUtils::createFileWithDirectories(filePath, m_editor->toPlainText());
  
  if (!result.success) {
    return false;
  }

  m_filePath = filePath;
  m_editor->setCurrentFilePath(filePath);
  m_isModified = false;

  emit filePathChanged(filePath);
  emit modificationChanged(false);

  return true;
}

QString TabEditor::content() const { return m_editor->toPlainText(); }

void TabEditor::setContent(const QString &content) {
  m_editor->setPlainText(content);
  m_editor->document()->setModified(false);
  m_isModified = false;
  updatePreview();
}

void TabEditor::updatePreview() {
  QString markdown = m_editor->toPlainText();
  if (!m_filePath.isEmpty()) {
    QFileInfo fileInfo(m_filePath);
    m_preview->setBasePath(fileInfo.absolutePath());
  }
  // Update scroll position before setting new content
  syncPreviewScroll();
  m_preview->setMarkdownContent(markdown);
  // No need to mark pending scroll since position is embedded in HTML
}

void TabEditor::onDocumentModified() {
  if (!m_isModified) {
    setModified(true);
  }

  // Restart preview timer
  m_previewTimer->start();
}

void TabEditor::syncPreviewScroll() {
  QScrollBar *scrollBar = m_editor->verticalScrollBar();
  int maximum = scrollBar->maximum();
  if (maximum == 0) {
    return; // No scrolling needed
  }
  int value = scrollBar->value();
  double percentage = static_cast<double>(value) / maximum;
  m_preview->scrollToPercentage(percentage);
}
