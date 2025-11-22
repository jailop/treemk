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

TabEditor::TabEditor(QWidget *parent)
    : QWidget(parent)
    , m_editor(nullptr)
    , m_preview(nullptr)
    , m_outline(nullptr)
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
    
    // Right side: preview and outline
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    m_preview = new MarkdownPreview(this);
    m_outline = new OutlinePanel(this);
    m_outline->setMinimumHeight(150);
    
    rightLayout->addWidget(m_preview, 3);
    rightLayout->addWidget(m_outline, 1);
    
    m_splitter->addWidget(m_editor);
    m_splitter->addWidget(rightWidget);
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
    m_outline->updateOutline(markdown);
}

void TabEditor::onDocumentModified()
{
    if (!m_isModified) {
        setModified(true);
    }
    
    // Restart preview timer
    m_previewTimer->start();
}
