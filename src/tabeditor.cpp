#include "tabeditor.h"

#include <QFileInfo>
#include <QScrollBar>
#include <QSplitter>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

#include "fileutils.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "navigationhistory.h"
#include "outlinepanel.h"

TabEditor::TabEditor(QWidget* parent)
    : QWidget(parent),
      m_editor(nullptr),
      m_sharedPreview(nullptr),
      m_previewTimer(nullptr),
      m_navigationHistory(nullptr),
      m_isModified(false),
      m_ownSaved(false),
      m_lastScrollPercentage(0.0) {
    m_navigationHistory = new NavigationHistory(this);
    setupUI();
}

TabEditor::~TabEditor() {}

void TabEditor::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_editor = new MarkdownEditor(this);
    mainLayout->addWidget(m_editor);

    connect(m_editor, &MarkdownEditor::textChanged, this,
            &TabEditor::onDocumentModified);
    connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged, this,
            &TabEditor::onEditorScrolled);
    connect(m_editor, &MarkdownEditor::cursorPositionChanged, this,
            &TabEditor::onEditorScrolled);

    connect(m_editor, &MarkdownEditor::wikiLinkClicked, this,
            &TabEditor::wikiLinkClicked);
    connect(m_editor, &MarkdownEditor::markdownLinkClicked, this,
            &TabEditor::markdownLinkClicked);
    connect(m_editor, &MarkdownEditor::openLinkInNewWindowRequested, this,
            &TabEditor::openLinkInNewWindowRequested);
    connect(m_editor, &MarkdownEditor::aiAssistRequested, this,
            &TabEditor::aiAssistRequested);

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

bool TabEditor::loadFile(const QString& filePath) {
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

    m_navigationHistory->addFile(filePath);

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

bool TabEditor::saveFileAs(const QString& filePath) {
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

void TabEditor::setContent(const QString& content) {
    m_editor->setPlainText(content);
    m_editor->document()->setModified(false);
    m_isModified = false;
    updatePreview();
}

void TabEditor::onDocumentModified() {
    if (!m_isModified) {
        setModified(true);
    }

    m_previewTimer->start();
}

void TabEditor::setSharedPreview(MarkdownPreview* preview) {
    m_sharedPreview = preview;
}

void TabEditor::updatePreviewContent(MarkdownPreview* preview) {
    if (!preview) return;

    QString markdown = m_editor->toPlainText();
    if (!m_filePath.isEmpty()) {
        QFileInfo fileInfo(m_filePath);
        preview->setBasePath(fileInfo.absolutePath());
    }
    preview->setMarkdownContent(markdown);
    preview->scrollToPercentage(m_lastScrollPercentage);
}

void TabEditor::updatePreview() {
    if (m_sharedPreview) {
        updatePreviewContent(m_sharedPreview);
    }
}

void TabEditor::onEditorScrolled() {
    QScrollBar* scrollBar = m_editor->verticalScrollBar();
    int maximum = scrollBar->maximum();
    if (maximum > 0) {
        m_lastScrollPercentage =
            static_cast<double>(scrollBar->value()) / maximum;
    }
}
