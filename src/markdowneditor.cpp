#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "shortcutmanager.h"
#include <QPainter>
#include <QTextBlock>
#include <QFont>
#include <QFontMetrics>
#include <QScrollBar>
#include <QMouseEvent>
#include <QTextCursor>
#include <QRegularExpression>
#include <QApplication>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QKeyEvent>
#include <QClipboard>
#include <QImage>
#include <QDir>
#include <QDateTime>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>

MarkdownEditor::MarkdownEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    m_highlighter = new MarkdownHighlighter(document());
    
    setupEditor();
    
    connect(this, &MarkdownEditor::blockCountChanged,
            this, &MarkdownEditor::updateLineNumberAreaWidth);
    connect(this, &MarkdownEditor::updateRequest,
            this, &MarkdownEditor::updateLineNumberArea);
    connect(this, &MarkdownEditor::cursorPositionChanged,
            this, &MarkdownEditor::highlightCurrentLine);
    
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    
    connect(document(), &QTextDocument::modificationChanged,
            this, &MarkdownEditor::setModified);
}

MarkdownEditor::~MarkdownEditor()
{
}

bool MarkdownEditor::isModified() const
{
    return document()->isModified();
}

void MarkdownEditor::setModified(bool modified)
{
    document()->setModified(modified);
}

MarkdownHighlighter* MarkdownEditor::getHighlighter() const
{
    return m_highlighter;
}

QString MarkdownEditor::getLinkAtPosition(int position) const
{
    QTextCursor cursor(document());
    cursor.setPosition(position);
    
    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();
    
    // Pattern for [[target]] or [[target|display]]
    QRegularExpression wikiLinkPattern("\\[\\[([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator matchIterator = wikiLinkPattern.globalMatch(line);
    
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        int start = match.capturedStart();
        int end = start + match.capturedLength();
        
        if (posInBlock >= start && posInBlock <= end) {
            return match.captured(1).trimmed();
        }
    }
    
    return QString();
}

QString MarkdownEditor::getExternalLinkAtPosition(int position) const
{
    QTextCursor cursor(document());
    cursor.setPosition(position);
    
    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();
    
    // Pattern for [text](url)
    QRegularExpression markdownLinkPattern("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    QRegularExpressionMatchIterator matchIterator = markdownLinkPattern.globalMatch(line);
    
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        int start = match.capturedStart();
        int end = start + match.capturedLength();
        
        if (posInBlock >= start && posInBlock <= end) {
            QString url = match.captured(2).trimmed();
            // Check if it's an external link (http/https)
            if (url.startsWith("http://") || url.startsWith("https://")) {
                return url;
            }
        }
    }
    
    // Pattern for plain URLs (http:// or https://)
    QRegularExpression urlPattern("(https?://[^\\s]+)");
    QRegularExpressionMatchIterator urlIterator = urlPattern.globalMatch(line);
    
    while (urlIterator.hasNext()) {
        QRegularExpressionMatch match = urlIterator.next();
        int start = match.capturedStart();
        int end = start + match.capturedLength();
        
        if (posInBlock >= start && posInBlock < end) {
            return match.captured(1);
        }
    }
    
    return QString();
}

void MarkdownEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier)) {
        QTextCursor cursor = cursorForPosition(event->pos());
        int position = cursor.position();
        
        // Try wiki link first
        QString linkTarget = getLinkAtPosition(position);
        if (!linkTarget.isEmpty()) {
            emit wikiLinkClicked(linkTarget);
            event->accept();
            return;
        }
        
        // Try external link
        QString externalUrl = getExternalLinkAtPosition(position);
        if (!externalUrl.isEmpty()) {
            QDesktopServices::openUrl(QUrl(externalUrl));
            event->accept();
            return;
        }
    }
    
    QPlainTextEdit::mousePressEvent(event);
}

void MarkdownEditor::keyPressEvent(QKeyEvent *event)
{
    // Get shortcut manager instance
    ShortcutManager *sm = ShortcutManager::instance();
    QKeySequence pressed(event->key() | event->modifiers());
    
    // Check custom navigation shortcuts
    QTextCursor cursor = textCursor();
    QTextCursor::MoveMode moveMode = (event->modifiers() & Qt::ShiftModifier) 
        ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
    
    // Navigation actions
    if (pressed == sm->getShortcut(ShortcutManager::MoveToStartOfLine)) {
        cursor.movePosition(QTextCursor::StartOfLine, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveToEndOfLine)) {
        cursor.movePosition(QTextCursor::EndOfLine, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveToStartOfDocument)) {
        cursor.movePosition(QTextCursor::Start, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveToEndOfDocument)) {
        cursor.movePosition(QTextCursor::End, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveWordLeft)) {
        cursor.movePosition(QTextCursor::PreviousWord, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveWordRight)) {
        cursor.movePosition(QTextCursor::NextWord, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveToPreviousParagraph)) {
        cursor.movePosition(QTextCursor::PreviousBlock, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveToNextParagraph)) {
        cursor.movePosition(QTextCursor::NextBlock, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    }
    
    // Editing actions
    if (pressed == sm->getShortcut(ShortcutManager::DeleteWordLeft)) {
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::DeleteWordRight)) {
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::DeleteToStartOfLine)) {
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::DeleteToEndOfLine)) {
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        event->accept();
        return;
    }
    
    // Ctrl+Enter or Ctrl+Return to open wiki-link at cursor
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && 
        (event->modifiers() & Qt::ControlModifier)) {
        
        int position = cursor.position();
        
        // Try wiki link first
        QString linkTarget = getLinkAtPosition(position);
        if (!linkTarget.isEmpty()) {
            emit wikiLinkClicked(linkTarget);
            event->accept();
            return;
        }
        
        // Try external link
        QString externalUrl = getExternalLinkAtPosition(position);
        if (!externalUrl.isEmpty()) {
            QDesktopServices::openUrl(QUrl(externalUrl));
            event->accept();
            return;
        }
    }
    
    QPlainTextEdit::keyPressEvent(event);
}

void MarkdownEditor::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        QPlainTextEdit::dragEnterEvent(event);
    }
}

void MarkdownEditor::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        QPlainTextEdit::dragMoveEvent(event);
    }
}

void MarkdownEditor::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        
        // Get drop position
        QTextCursor cursor = cursorForPosition(event->position().toPoint());
        setTextCursor(cursor);
        
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);
                
                // Check if it's an image
                QStringList imageExtensions;
                imageExtensions << "png" << "jpg" << "jpeg" << "gif" << "bmp" << "svg";
                
                if (imageExtensions.contains(fileInfo.suffix().toLower())) {
                    // Insert as image
                    QString imageMarkdown = QString("![%1](%2)")
                        .arg(fileInfo.baseName(), filePath);
                    cursor.insertText(imageMarkdown);
                    cursor.insertText("\n");
                } else {
                    // Insert as link
                    QString linkMarkdown = QString("[%1](%2)")
                        .arg(fileInfo.fileName(), filePath);
                    cursor.insertText(linkMarkdown);
                    cursor.insertText("\n");
                }
            }
        }
        
        event->acceptProposedAction();
    } else {
        QPlainTextEdit::dropEvent(event);
    }
}

void MarkdownEditor::setupEditor()
{
    QFont font;
    font.setFamily("Sans Serif");
    font.setStyleHint(QFont::SansSerif);
    font.setPointSize(11);
    setFont(font);
    
    QFontMetrics metrics(font);
    setTabStopDistance(4 * metrics.horizontalAdvance(' '));
    
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    
    QPalette p = palette();
    p.setColor(QPalette::Base, QColor(255, 255, 255));
    p.setColor(QPalette::Text, QColor(0, 0, 0));
    setPalette(p);
}

int MarkdownEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void MarkdownEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void MarkdownEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void MarkdownEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                     lineNumberAreaWidth(), cr.height()));
}

void MarkdownEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    
    setExtraSelections(extraSelections);
}

void MarkdownEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(240, 240, 240));
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(120, 120, 120));
            painter.drawText(0, top, lineNumberArea->width() - 5, fontMetrics().height(),
                           Qt::AlignRight, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void MarkdownEditor::setCurrentFilePath(const QString &filePath)
{
    m_currentFilePath = filePath;
}

void MarkdownEditor::insertFromMimeData(const QMimeData *source)
{
    // Check if clipboard contains an image
    if (source->hasImage()) {
        QImage image = qvariant_cast<QImage>(source->imageData());
        if (!image.isNull()) {
            QString imagePath = saveImageFromClipboard(image);
            if (!imagePath.isEmpty()) {
                // Insert markdown image syntax
                QTextCursor cursor = textCursor();
                cursor.insertText(QString("![image](%1)").arg(imagePath));
                return;
            }
        }
    }
    
    // Default behavior for other content
    QPlainTextEdit::insertFromMimeData(source);
}

QString MarkdownEditor::saveImageFromClipboard(const QImage &image)
{
    // Check if we have a current file path
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Cannot Save Image"), 
                           tr("Please save the document first before pasting images."));
        return QString();
    }
    
    // Get the directory and base name of the current file
    QFileInfo fileInfo(m_currentFilePath);
    QDir fileDir = fileInfo.absoluteDir();
    QString baseName = fileInfo.baseName();
    
    // Sanitize the base name for safe folder creation (Unix and Windows)
    QString imagesDirName = baseName;
    imagesDirName.replace(QRegularExpression("[<>:\"/\\\\|?*]"), "_");
    if (imagesDirName.isEmpty()) {
        imagesDirName = "images";
    }
    QString imagesDirPath = fileDir.filePath(imagesDirName);
    QDir imagesDir(imagesDirPath);
    
    if (!imagesDir.exists()) {
        if (!fileDir.mkdir(imagesDirName)) {
            QMessageBox::warning(this, tr("Cannot Create Directory"), 
                               tr("Failed to create directory '%1'.").arg(imagesDirName));
            return QString();
        }
    }
    
    // Find the next available number in sequence
    int nextNumber = 1;
    QRegularExpression imagePattern("^image_(\\d+)\\.png$");
    
    QStringList existingFiles = imagesDir.entryList(QStringList() << "image_*.png", QDir::Files);
    for (const QString &file : existingFiles) {
        QRegularExpressionMatch match = imagePattern.match(file);
        if (match.hasMatch()) {
            int num = match.captured(1).toInt();
            if (num >= nextNumber) {
                nextNumber = num + 1;
            }
        }
    }
    
    // Suggest filename
    QString suggestedName = QString("image_%1.png").arg(nextNumber, 3, 10, QChar('0'));
    
    // Ask user for filename
    bool ok;
    QString fileName = QInputDialog::getText(this, tr("Save Image"),
                                            tr("Enter image filename:"),
                                            QLineEdit::Normal,
                                            suggestedName, &ok);
    
    if (!ok || fileName.isEmpty()) {
        return QString(); // User cancelled
    }
    
    // Ensure .png extension
    if (!fileName.endsWith(".png", Qt::CaseInsensitive)) {
        fileName += ".png";
    }
    
    // Check if file already exists
    QString fullPath = imagesDir.filePath(fileName);
    if (QFileInfo::exists(fullPath)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            tr("File Exists"),
            tr("File '%1' already exists. Overwrite?").arg(fileName),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply != QMessageBox::Yes) {
            return QString(); // User chose not to overwrite
        }
    }
    
    // Save the image
    if (image.save(fullPath, "PNG")) {
        return imagesDirName + "/" + fileName; // Return relative path with subdirectory
    }
    
    QMessageBox::warning(this, tr("Save Failed"), 
                        tr("Failed to save image to '%1'.").arg(fileName));
    return QString();
}
