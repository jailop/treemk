#include "tabeditor.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "outlinepanel.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QScrollBar>

TabEditor::TabEditor(QWidget *parent)
    : QWidget(parent)
    , m_editor(nullptr)
    , m_preview(nullptr)
    , m_splitter(nullptr)
    , m_previewTimer(nullptr)
    , m_isModified(false)
{
    setupUI();
}

TabEditor::~TabEditor()
{
}

void TabEditor::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    // Editor
    m_editor = new MarkdownEditor(this);
    connect(m_editor, &MarkdownEditor::textChanged, this, &TabEditor::onDocumentModified);
    
    // Connect editor scroll to preview scroll
    connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &TabEditor::syncPreviewScroll);
    connect(m_editor, &MarkdownEditor::cursorPositionChanged,
            this, &TabEditor::syncPreviewScroll);
    
    // Preview only (outline is now in main window)
    m_preview = new MarkdownPreview(this);
    
    m_splitter->addWidget(m_editor);
    m_splitter->addWidget(m_preview);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(m_splitter);
    
    // Preview update timer
    m_previewTimer = new QTimer(this);
    m_previewTimer->setSingleShot(true);
    m_previewTimer->setInterval(500);
    connect(m_previewTimer, &QTimer::timeout, this, &TabEditor::updatePreview);
}

QString TabEditor::fileName() const
{
    if (m_filePath.isEmpty()) {
        return tr("Untitled");
    }
    return QFileInfo(m_filePath).fileName();
}

void TabEditor::setModified(bool modified)
{
    if (m_isModified != modified) {
        m_isModified = modified;
        emit modificationChanged(modified);
    }
}

bool TabEditor::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    
    m_editor->setPlainText(content);
    m_filePath = filePath;
    m_editor->setCurrentFilePath(filePath);
    m_isModified = false;
    
    updatePreview();
    
    emit filePathChanged(filePath);
    emit modificationChanged(false);
    
    return true;
}

bool TabEditor::saveFile()
{
    if (m_filePath.isEmpty()) {
        return false;
    }
    
    return saveFileAs(m_filePath);
}

bool TabEditor::saveFileAs(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_editor->toPlainText();
    file.close();
    
    m_filePath = filePath;
    m_editor->setCurrentFilePath(filePath);
    m_isModified = false;
    
    emit filePathChanged(filePath);
    emit modificationChanged(false);
    
    return true;
}

QString TabEditor::content() const
{
    return m_editor->toPlainText();
}

void TabEditor::setContent(const QString &content)
{
    m_editor->setPlainText(content);
    m_isModified = false;
    updatePreview();
}

void TabEditor::updatePreview()
{
    QString markdown = m_editor->toPlainText();
    
    // Set base path for images
    if (!m_filePath.isEmpty()) {
        QFileInfo fileInfo(m_filePath);
        m_preview->setBasePath(fileInfo.absolutePath());
    }
    
    m_preview->setMarkdownContent(markdown);
    // Note: Outline is now updated by MainWindow
    
    // Sync scroll position after preview refresh
    syncPreviewScroll();
}

void TabEditor::onDocumentModified()
{
    if (!m_isModified) {
        setModified(true);
    }
    
    // Restart preview timer
    m_previewTimer->start();
}

void TabEditor::syncPreviewScroll()
{
    QScrollBar *scrollBar = m_editor->verticalScrollBar();
    
    // Calculate the scroll percentage
    int maximum = scrollBar->maximum();
    if (maximum == 0) {
        return; // No scrolling needed
    }
    
    int value = scrollBar->value();
    double percentage = static_cast<double>(value) / maximum;
    
    // Sync the preview scroll position
    m_preview->scrollToPercentage(percentage);
}
