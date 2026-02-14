#include "markdowneditor.h"

#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QRegularExpression>
#include <QScrollBar>
#include <QSettings>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTimer>
#include <QUrl>

#include "defs.h"
#include "logic/systemprompts.h"
#include "markdownhighlighter.h"
#include "shortcutmanager.h"
#include "thememanager.h"

MarkdownEditor::MarkdownEditor(QWidget* parent)
    : QTextEdit(parent), m_predictionEnabled(true), m_lineNumbersVisible(true) {
    lineNumberArea = new LineNumberArea(this);
    m_highlighter = new MarkdownHighlighter(document());

    // Initialize formatting timer for deferred hanging indent application
    m_formatTimer = new QTimer(this);
    m_formatTimer->setSingleShot(true);
    m_formatTimer->setInterval(100);  // Wait 100ms after user stops typing
    connect(m_formatTimer, &QTimer::timeout, this,
            &MarkdownEditor::applyDeferredFormatting);

    setupEditor();

    // Enable mouse tracking for cursor changes over checkboxes
    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    connect(document(), &QTextDocument::blockCountChanged, this,
            &MarkdownEditor::updateLineNumberAreaWidth);
    connect(this, &QTextEdit::cursorPositionChanged, this,
            &MarkdownEditor::highlightCurrentLine);
    connect(this, &QTextEdit::textChanged, this,
            &MarkdownEditor::onTextChanged);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    connect(document(), &QTextDocument::modificationChanged, this,
            &MarkdownEditor::setModified);

    // Connect to theme changes
    if (ThemeManager::instance()) {
        connect(ThemeManager::instance(), &ThemeManager::themeChanged, this,
                &MarkdownEditor::onThemeChanged);
        connect(ThemeManager::instance(),
                &ThemeManager::editorColorSchemeChanged, this,
                &MarkdownEditor::onThemeChanged);
    }

    // Apply initial theme
    onThemeChanged();
}

MarkdownEditor::~MarkdownEditor() {}

void MarkdownEditor::onThemeChanged() {
    // Update editor palette and stylesheet from ThemeManager
    if (ThemeManager::instance()) {
        setPalette(ThemeManager::instance()->getEditorPalette());
        setStyleSheet(ThemeManager::instance()->getEditorStyleSheet());

        // Update highlighter color scheme
        if (m_highlighter) {
            QString resolvedScheme =
                ThemeManager::instance()->getResolvedEditorColorSchemeName();
            m_highlighter->setColorScheme(resolvedScheme);
        }

        // Update line number area to repaint with new colors
        if (lineNumberArea) {
            lineNumberArea->update();
        }

        // Update current line highlighting
        highlightCurrentLine();
    }
}

bool MarkdownEditor::isModified() const { return document()->isModified(); }

void MarkdownEditor::setModified(bool modified) {
    document()->setModified(modified);
}

MarkdownHighlighter* MarkdownEditor::getHighlighter() const {
    return m_highlighter;
}

QString MarkdownEditor::getLinkAtPosition(int position) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    // Pattern for [[target]] or [[target|display]] or ![[target]] or
    // ![[target|display]]
    QRegularExpression wikiLinkPattern(
        "!?\\[\\[([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator matchIterator =
        wikiLinkPattern.globalMatch(line);

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

QString MarkdownEditor::getExternalLinkAtPosition(int position) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    // Pattern for [text](url)
    QRegularExpression markdownLinkPattern("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    QRegularExpressionMatchIterator matchIterator =
        markdownLinkPattern.globalMatch(line);

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

QString MarkdownEditor::getMarkdownLinkAtPosition(int position) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    // Pattern for [text](url) or ![text](url) (markdown links and images)
    QRegularExpression markdownLinkPattern("!?\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    QRegularExpressionMatchIterator matchIterator =
        markdownLinkPattern.globalMatch(line);

    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        int start = match.capturedStart();
        int end = start + match.capturedLength();

        if (posInBlock >= start && posInBlock <= end) {
            QString url = match.captured(2).trimmed();
            // Return any markdown link that is not an external URL
            if (!url.startsWith("http://") && !url.startsWith("https://")) {
                return url;
            }
        }
    }

    return QString();
}

void MarkdownEditor::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QTextCursor cursor = cursorForPosition(event->pos());
        int position = cursor.position();

        // Check if clicking on a task checkbox (with or without Ctrl)
        if (isClickOnCheckbox(position)) {
            toggleTaskAtPosition(position);
            event->accept();
            return;
        }

        // Ctrl+Click for links
        if (event->modifiers() & Qt::ControlModifier) {
            // Try wiki link
            QString linkTarget = getLinkAtPosition(position);
            if (!linkTarget.isEmpty()) {
                emit wikiLinkClicked(linkTarget);
                event->accept();
                return;
            }

            // Try markdown link
            QString markdownLink = getMarkdownLinkAtPosition(position);
            if (!markdownLink.isEmpty()) {
                emit markdownLinkClicked(markdownLink);
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
    }

    QTextEdit::mousePressEvent(event);
}

void MarkdownEditor::mouseMoveEvent(QMouseEvent* event) {
    // Change cursor to pointer when hovering over checkboxes
    QTextCursor cursor = cursorForPosition(event->pos());
    int position = cursor.position();

    if (isClickOnCheckbox(position)) {
        viewport()->setCursor(Qt::PointingHandCursor);
    } else {
        viewport()->setCursor(Qt::IBeamCursor);
    }

    QTextEdit::mouseMoveEvent(event);
}

void MarkdownEditor::updateWordFrequency() {
    m_wordFrequency.clear();
    m_bigramFrequency.clear();

    QString text = toPlainText();
    QRegularExpression wordRegex("\\b[a-zA-Z]{3,}\\b");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

    QStringList words;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0).toLower();
        words.append(word);
        m_wordFrequency[word]++;
    }

    // Build bigram model
    for (int i = 0; i < words.size() - 1; i++) {
        QPair<QString, QString> bigram(words[i], words[i + 1]);
        m_bigramFrequency[bigram]++;
    }
}

QString MarkdownEditor::predictWordUnigram(const QString& prefix) const {
    if (prefix.length() < 2) {
        return QString();
    }

    QString bestMatch;
    int maxFrequency = 0;

    for (auto it = m_wordFrequency.constBegin();
         it != m_wordFrequency.constEnd(); ++it) {
        const QString& word = it.key();
        if (word.startsWith(prefix, Qt::CaseInsensitive) &&
            word.length() > prefix.length()) {
            if (it.value() > maxFrequency) {
                maxFrequency = it.value();
                bestMatch = word;
            }
        }
    }

    return bestMatch;
}

QString MarkdownEditor::predictWordBigram(const QString& previousWord,
                                          const QString& prefix) const {
    if (previousWord.isEmpty() || prefix.length() < 1) {
        return QString();
    }

    QString prevLower = previousWord.toLower();
    QString prefixLower = prefix.toLower();

    QString bestMatch;
    int maxFrequency = 0;

    for (auto it = m_bigramFrequency.constBegin();
         it != m_bigramFrequency.constEnd(); ++it) {
        const QPair<QString, QString>& bigram = it.key();

        // Check if this bigram starts with the previous word and the second
        // word matches prefix
        if (bigram.first == prevLower &&
            bigram.second.startsWith(prefixLower) &&
            bigram.second.length() > prefixLower.length()) {
            if (it.value() > maxFrequency) {
                maxFrequency = it.value();
                bestMatch = bigram.second;
            }
        }
    }

    return bestMatch;
}

QString MarkdownEditor::predictWord(const QString& prefix) const {
    if (prefix.length() < 1) {
        return QString();
    }

    // Get the previous word for bigram prediction
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    QString previousWord = cursor.selectedText();

    // Try bigram prediction first (more context-specific)
    QString bigramPrediction = predictWordBigram(previousWord, prefix);

    // Try unigram prediction
    QString unigramPrediction = predictWordUnigram(prefix);

    // Choose the best prediction
    // Prioritize bigram if it exists (more contextual)
    if (!bigramPrediction.isEmpty()) {
        // If bigram frequency is high enough, prefer it
        QString prevLower = previousWord.toLower();
        QPair<QString, QString> bigram(prevLower, bigramPrediction.toLower());
        int bigramFreq = m_bigramFrequency.value(bigram, 0);
        int unigramFreq = m_wordFrequency.value(unigramPrediction.toLower(), 0);

        // Prefer bigram if it appears at least half as often as unigram
        // or if unigram doesn't exist
        if (bigramFreq * 2 >= unigramFreq || unigramPrediction.isEmpty()) {
            return bigramPrediction;
        }
    }

    return unigramPrediction;
}

void MarkdownEditor::showPrediction() {
    QSettings settings(APP_LABEL, APP_LABEL);
    if (!settings.value("editor/enableWordPrediction", true).toBool()) {
        return;
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    QString currentWord = cursor.selectedText();

    // Allow predictions with 1+ characters for bigram, 2+ for unigram
    if (currentWord.length() >= 1) {
        QString prediction = predictWord(currentWord);
        if (!prediction.isEmpty() &&
            prediction.toLower() != currentWord.toLower()) {
            m_currentPrediction = prediction.mid(currentWord.length());
            viewport()->update();
            return;
        }
    }

    m_currentPrediction.clear();
    viewport()->update();
}

void MarkdownEditor::hidePrediction() {
    m_currentPrediction.clear();
    viewport()->update();
}

void MarkdownEditor::acceptPrediction() {
    if (!m_currentPrediction.isEmpty()) {
        QTextCursor cursor = textCursor();
        cursor.insertText(m_currentPrediction);
        setTextCursor(cursor);
        m_currentPrediction.clear();
        viewport()->update();
    }
}

void MarkdownEditor::onTextChanged() {
    static int changeCount = 0;
    changeCount++;

    // Update word frequency every 10 changes to avoid performance issues
    if (changeCount % 10 == 0) {
        updateWordFrequency();
    }

    showPrediction();

    // Apply hanging indent to current block only (doesn't affect undo/redo)
    applyListHangingIndentToCurrentBlock();
}

void MarkdownEditor::paintEvent(QPaintEvent* event) {
    QTextEdit::paintEvent(event);

    // Draw prediction in gray
    if (!m_currentPrediction.isEmpty()) {
        QTextCursor cursor = textCursor();
        QRect rect = cursorRect(cursor);

        QFont font = this->font();
        QFontMetrics fm(font);

        QPainter painter(viewport());
        painter.setFont(font);
        painter.setPen(QColor(128, 128, 128, 180));  // Gray with transparency

        int x = rect.right();
        int y = rect.bottom();

        painter.drawText(x, y, m_currentPrediction);
    }
}

void MarkdownEditor::contextMenuEvent(QContextMenuEvent* event) {
    QMenu* menu = createStandardContextMenu();

    // Check if cursor is on a wiki-link or markdown link
    QTextCursor cursor = cursorForPosition(event->pos());
    int position = cursor.position();
    QString linkTarget = getLinkAtPosition(position);

    // If not a wiki-link, check for markdown link
    if (linkTarget.isEmpty()) {
        linkTarget = getMarkdownLinkAtPosition(position);
    }

    if (!linkTarget.isEmpty()) {
        // Resolve to absolute path if it's a relative link
        QString absolutePath = linkTarget;
        if (!m_currentFilePath.isEmpty() &&
            QFileInfo(linkTarget).isRelative()) {
            QDir currentDir = QFileInfo(m_currentFilePath).dir();
            absolutePath = currentDir.absoluteFilePath(linkTarget);
        }

        // Check if it's a local file that exists
        QFileInfo fileInfo(absolutePath);
        bool isLocalFile = fileInfo.exists() && fileInfo.isFile();

        menu->addSeparator();

        QAction* openNewWindowAction =
            menu->addAction(tr("Open Link in New Window"));
        connect(openNewWindowAction, &QAction::triggered, this,
                [this, linkTarget]() {
                    emit openLinkInNewWindowRequested(linkTarget);
                });

        // Always show Rename and Delete actions for local (non-http) links
        if (!linkTarget.startsWith("http://") &&
            !linkTarget.startsWith("https://")) {
            QAction* renameAction = menu->addAction(
                QIcon::fromTheme("edit-rename"), tr("Rename Linked File..."));
            renameAction->setEnabled(isLocalFile);
            if (!isLocalFile) {
                renameAction->setToolTip(
                    tr("File not found: %1").arg(absolutePath));
            }
            connect(renameAction, &QAction::triggered, this,
                    [this, absolutePath]() {
                        emit fileRenameRequested(absolutePath);
                    });

            QAction* deleteAction = menu->addAction(
                QIcon::fromTheme("edit-delete"), tr("Delete Linked File..."));
            deleteAction->setEnabled(isLocalFile);
            if (!isLocalFile) {
                deleteAction->setToolTip(
                    tr("File not found: %1").arg(absolutePath));
            }
            connect(deleteAction, &QAction::triggered, this,
                    [this, absolutePath]() {
                        emit fileDeleteRequested(absolutePath);
                    });
        }
    }

    // Add AI Assist submenu
    menu->addSeparator();
    QMenu* aiMenu = menu->addMenu(QIcon::fromTheme("edit-ai"), tr("AI Assist"));

    // Add predefined prompts
    QList<SystemPrompt> prompts =
        SystemPrompts::instance()->getEnabledPrompts();
    for (const SystemPrompt& prompt : prompts) {
        QAction* promptAction = aiMenu->addAction(prompt.name);
        connect(promptAction, &QAction::triggered, this, [this, prompt]() {
            emit aiAssistWithPromptRequested(prompt.prompt);
        });
    }

    // Add Custom option
    if (!prompts.isEmpty()) {
        aiMenu->addSeparator();
    }
    QAction* customAction = aiMenu->addAction(tr("Custom..."));
    customAction->setShortcut(QKeySequence(tr("Ctrl+Shift+A")));
    connect(customAction, &QAction::triggered, this,
            [this]() { emit aiAssistRequested(); });

    menu->exec(event->globalPos());
    delete menu;
}

void MarkdownEditor::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        QTextEdit::dragEnterEvent(event);
    }
}

void MarkdownEditor::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        QTextEdit::dragMoveEvent(event);
    }
}

void MarkdownEditor::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();

        // Get drop position
        QTextCursor cursor = cursorForPosition(event->position().toPoint());
        setTextCursor(cursor);

        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);

                // Calculate relative path if current file path is set
                QString linkPath = filePath;
                if (!m_currentFilePath.isEmpty()) {
                    QFileInfo currentFileInfo(m_currentFilePath);
                    QDir currentDir = currentFileInfo.absoluteDir();
                    linkPath = currentDir.relativeFilePath(filePath);
                }

                // Check if it's an image
                QStringList imageExtensions;
                imageExtensions << "png" << "jpg" << "jpeg" << "gif" << "bmp"
                                << "svg";

                if (imageExtensions.contains(fileInfo.suffix().toLower())) {
                    // Insert as image
                    QString imageMarkdown =
                        QString("![%1](%2)").arg(fileInfo.baseName(), linkPath);
                    cursor.insertText(imageMarkdown);
                } else {
                    // Insert as link
                    QString linkMarkdown =
                        QString("[%1](%2)").arg(fileInfo.fileName(), linkPath);
                    cursor.insertText(linkMarkdown);
                }
            }
        }

        event->acceptProposedAction();
    } else {
        QTextEdit::dropEvent(event);
    }
}

void MarkdownEditor::setupEditor() {
    QFont font;
    font.setFamily("Sans Serif");
    font.setStyleHint(QFont::SansSerif);
    font.setPointSize(11);
    setFont(font);

    QFontMetrics metrics(font);
    setTabStopDistance(4 * metrics.horizontalAdvance(' '));

    setLineWrapMode(QTextEdit::WidgetWidth);
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

    // Explicitly enable undo/redo
    setUndoRedoEnabled(true);

    // Theme is applied via onThemeChanged() - no hardcoded colors here
}

int MarkdownEditor::lineNumberAreaWidth() {
    if (!m_lineNumbersVisible) {
        return 0;
    }

    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void MarkdownEditor::setLineNumbersVisible(bool visible) {
    m_lineNumbersVisible = visible;
    lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth(0);
}

void MarkdownEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void MarkdownEditor::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(),
                               rect.height());

    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

void MarkdownEditor::resizeEvent(QResizeEvent* e) {
    QTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void MarkdownEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QString scheme = m_highlighter->getColorScheme();
        QColor lineColor;
        if (scheme.contains("dark", Qt::CaseInsensitive)) {
            lineColor = QColor(64, 64, 64);  // Subtle gray for dark themes
        } else {
            lineColor = QColor(Qt::yellow).lighter(160);
        }

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);

    // Update the highlighter with current cursor line
    if (m_highlighter) {
        m_highlighter->setCurrentCursorLine(textCursor().blockNumber());
    }
}

void MarkdownEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(lineNumberArea);

    QColor backgroundColor, textColor;
    // Detect theme
    if (palette().color(QPalette::Base).lightness() > 128) {
        // Light theme: light gray background, darker text
        backgroundColor = QColor(240, 240, 240);
        textColor = QColor(120, 120, 120);
    } else {
        // Dark theme: dark background, light text
        backgroundColor = QColor(45, 45, 45);
        textColor = QColor(160, 160, 160);
    }

    painter.fillRect(event->rect(), backgroundColor);

    QTextBlock block = document()->firstBlock();
    int blockNumber = 0;

    while (block.isValid()) {
        blockNumber = block.blockNumber();
        QTextCursor blockCursor(block);
        int blockTop = static_cast<int>(cursorRect(blockCursor).top());

        if (blockTop > event->rect().bottom()) {
            break;
        }

        if (block.isVisible() &&
            blockTop + fontMetrics().height() > event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(textColor);
            painter.drawText(0, blockTop, lineNumberArea->width() - 5,
                             fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
    }
}

void MarkdownEditor::setCurrentFilePath(const QString& filePath) {
    m_currentFilePath = filePath;
}

void MarkdownEditor::insertFromMimeData(const QMimeData* source) {
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

    // Always paste as plain text (strip HTML formatting)
    if (source->hasText()) {
        QTextCursor cursor = textCursor();
        cursor.insertText(source->text());
        return;
    }

    // Fallback to default behavior for other content types
    QTextEdit::insertFromMimeData(source);
}
