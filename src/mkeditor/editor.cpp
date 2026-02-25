#include "markdowneditor.h"
#include "colorpalette.h"

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
#include "regexpatterns.h"
#include "regexutils.h"
#include "shortcutmanager.h"
#include "thememanager.h"

MarkdownEditor::MarkdownEditor(QWidget* parent)
    : QTextEdit(parent), 
      m_predictionEnabled(true), 
      m_aiAssistEnabled(true), 
      m_lineNumbersVisible(true),
      m_focusModeEnabled(false),
      m_focusModeMaxWidth(900) {
    lineNumberArea = new LineNumberArea(this);
    m_highlighter = new MarkdownHighlighter(document());

    m_formatTimer = new QTimer(this);
    m_formatTimer->setSingleShot(true);
    m_formatTimer->setInterval(100);  // Wait 100ms after user stops typing
    connect(m_formatTimer, &QTimer::timeout, this,
            &MarkdownEditor::applyDeferredFormatting);

    setupEditor();

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

    if (ThemeManager::instance()) {
        connect(ThemeManager::instance(), &ThemeManager::themeChanged, this,
                &MarkdownEditor::onThemeChanged);
        connect(ThemeManager::instance(),
                &ThemeManager::editorColorSchemeChanged, this,
                &MarkdownEditor::onThemeChanged);
    }

    onThemeChanged();

    QSettings settings(APP_LABEL, APP_LABEL);
    m_predictionEnabled = settings.value("editor/enableWordPrediction", true).toBool();
    m_aiAssistEnabled = settings.value("ai/enabled", true).toBool();
}

MarkdownEditor::~MarkdownEditor() {}

void MarkdownEditor::onThemeChanged() {
    if (ThemeManager::instance()) {
        setPalette(ThemeManager::instance()->getEditorPalette());
        setStyleSheet(ThemeManager::instance()->getEditorStyleSheet());

        if (m_highlighter) {
            QString resolvedScheme =
                ThemeManager::instance()->getResolvedEditorColorSchemeName();
            m_highlighter->setColorScheme(resolvedScheme);
        }

        if (lineNumberArea) {
            lineNumberArea->update();
        }

        highlightCurrentLine();
    }
}

bool MarkdownEditor::isModified() const { return document()->isModified(); }

void MarkdownEditor::setModified(bool modified) {
    document()->setModified(modified);
}

void MarkdownEditor::setPredictionEnabled(bool enabled) {
    m_predictionEnabled = enabled;
    if (!enabled) {
        hidePrediction();
    }
}

void MarkdownEditor::setAIAssistEnabled(bool enabled) {
    m_aiAssistEnabled = enabled;
}

MarkdownHighlighter* MarkdownEditor::getHighlighter() const {
    return m_highlighter;
}

QString MarkdownEditor::getLinkAtPosition(int position) const {
    QString dummy;
    return getLinkAtPosition(position, dummy);
}

QString MarkdownEditor::getLinkAtPosition(int position, QString& displayText) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    QVector<RegexUtils::WikiLinkInfo> links = RegexUtils::parseWikiLinks(line);
    for (const auto& link : links) {
        int start = link.position;
        int end = start + link.length;

        if (posInBlock >= start && posInBlock <= end) {
            displayText = link.display;
            return link.target;
        }
    }

    displayText.clear();
    return QString();
}

QString MarkdownEditor::getExternalLinkAtPosition(int position) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    QVector<RegexUtils::MarkdownLinkInfo> links = RegexUtils::parseMarkdownLinks(line);
    for (const auto& link : links) {
        int start = link.position;
        int end = start + link.length;

        if (posInBlock >= start && posInBlock <= end) {
            if (link.url.startsWith("http://") || link.url.startsWith("https://")) {
                return link.url;
            }
        }
    }

    QRegularExpression urlPattern(RegexPatterns::URL);
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
    QString dummy;
    return getMarkdownLinkAtPosition(position, dummy);
}

QString MarkdownEditor::getMarkdownLinkAtPosition(int position, QString& labelText) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    QVector<RegexUtils::MarkdownLinkInfo> links = RegexUtils::parseMarkdownLinks(line);
    for (const auto& link : links) {
        int start = link.position;
        int end = start + link.length;

        if (posInBlock >= start && posInBlock <= end) {
            if (!link.url.startsWith("http://") && !link.url.startsWith("https://")) {
                labelText = link.text;
                return link.url;
            }
        }
    }

    labelText.clear();
    return QString();
}

void MarkdownEditor::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        QTextCursor cursor = cursorForPosition(event->pos());
        int position = cursor.position();

        if (isClickOnCheckbox(position)) {
            toggleTaskAtPosition(position);
            event->accept();
            return;
        }

        if (event->modifiers() & Qt::ControlModifier) {
            QString linkTarget = getLinkAtPosition(position);
            if (!linkTarget.isEmpty()) {
                emit wikiLinkClicked(linkTarget);
                event->accept();
                return;
            }

            QString markdownLink = getMarkdownLinkAtPosition(position);
            if (!markdownLink.isEmpty()) {
                emit markdownLinkClicked(markdownLink);
                event->accept();
                return;
            }

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
    QRegularExpression wordRegex(RegexPatterns::WORD_BOUNDARY);
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

    QStringList words;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0).toLower();
        words.append(word);
        m_wordFrequency[word]++;
    }

    for (int i = 0; i < words.size() - 1; i++) {
        QPair<QString, QString> bigram(words[i], words[i + 1]);
        m_bigramFrequency[bigram]++;
    }

    updateDirectoryWordFrequency();
}

void MarkdownEditor::updateDirectoryWordFrequency() {
    if (m_currentFilePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(m_currentFilePath);
    QDir dir = fileInfo.absoluteDir();
    
    QStringList filters;
    filters << "*.md" << "*.markdown";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    QRegularExpression wordRegex(RegexPatterns::WORD_BOUNDARY);

    for (const QFileInfo& file : files) {
        if (file.absoluteFilePath() == m_currentFilePath) {
            continue;
        }

        QFile f(file.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QString content = QString::fromUtf8(f.readAll());
        f.close();

        QRegularExpressionMatchIterator it = wordRegex.globalMatch(content);
        QStringList words;
        
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString word = match.captured(0).toLower();
            words.append(word);
            m_wordFrequency[word]++;
        }

        for (int i = 0; i < words.size() - 1; i++) {
            QPair<QString, QString> bigram(words[i], words[i + 1]);
            m_bigramFrequency[bigram]++;
        }
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

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    QString previousWord = cursor.selectedText();

    QString bigramPrediction = predictWordBigram(previousWord, prefix);

    QString unigramPrediction = predictWordUnigram(prefix);

    if (!bigramPrediction.isEmpty()) {
        QString prevLower = previousWord.toLower();
        QPair<QString, QString> bigram(prevLower, bigramPrediction.toLower());
        int bigramFreq = m_bigramFrequency.value(bigram, 0);
        int unigramFreq = m_wordFrequency.value(unigramPrediction.toLower(), 0);

        if (bigramFreq * 2 >= unigramFreq || unigramPrediction.isEmpty()) {
            return bigramPrediction;
        }
    }

    return unigramPrediction;
}

void MarkdownEditor::showPrediction() {
    if (!m_predictionEnabled) {
        return;
    }

    QTextCursor cursor = textCursor();
    
    QTextCursor checkCursor = cursor;
    checkCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QString nextChar = checkCursor.selectedText();
    
    if (!nextChar.isEmpty() && !nextChar.at(0).isSpace()) {
        m_currentPrediction.clear();
        viewport()->update();
        return;
    }

    cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    QString currentWord = cursor.selectedText();

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

    if (changeCount % 10 == 0) {
        updateWordFrequency();
    }

    showPrediction();

    applyListHangingIndentToCurrentBlock();
}

void MarkdownEditor::paintEvent(QPaintEvent* event) {
    QTextEdit::paintEvent(event);

    if (!m_currentPrediction.isEmpty()) {
        QTextCursor cursor = textCursor();
        QRect rect = cursorRect(cursor);

        QFont font = this->font();
        QFontMetrics fm(font);

        QPainter painter(viewport());
        painter.setFont(font);
        
        QString scheme = m_highlighter->getColorScheme();
        QColor predictionColor;
        if (scheme.contains("dark", Qt::CaseInsensitive)) {
            predictionColor = ColorPalette::getDarkTheme().predictionText;
        } else {
            predictionColor = ColorPalette::getLightTheme().predictionText;
        }
        predictionColor.setAlpha(180);
        painter.setPen(predictionColor);

        int x = rect.right();
        int y = rect.bottom();

        painter.drawText(x, y, m_currentPrediction);
    }
}

void MarkdownEditor::contextMenuEvent(QContextMenuEvent* event) {
    QMenu* menu = createStandardContextMenu();

    QTextCursor cursor = cursorForPosition(event->pos());
    int position = cursor.position();
    QString linkTarget = getLinkAtPosition(position);

    if (linkTarget.isEmpty()) {
        linkTarget = getMarkdownLinkAtPosition(position);
    }

    if (!linkTarget.isEmpty()) {
        QString absolutePath = linkTarget;
        if (!m_currentFilePath.isEmpty() &&
            QFileInfo(linkTarget).isRelative()) {
            QDir currentDir = QFileInfo(m_currentFilePath).dir();
            absolutePath = currentDir.absoluteFilePath(linkTarget);
        }

        QFileInfo fileInfo(absolutePath);
        bool isLocalFile = fileInfo.exists() && fileInfo.isFile();

        menu->addSeparator();

        QAction* openNewWindowAction =
            menu->addAction(tr("Open Link in New Window"));
        connect(openNewWindowAction, &QAction::triggered, this,
                [this, linkTarget]() {
                    emit openLinkInNewWindowRequested(linkTarget);
                });

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

    if (m_aiAssistEnabled) {
        menu->addSeparator();
        QMenu* aiMenu = menu->addMenu(QIcon::fromTheme("edit-ai"), tr("AI Assist"));

        QList<SystemPrompt> prompts =
            SystemPrompts::instance()->getEnabledPrompts();
        for (const SystemPrompt& prompt : prompts) {
            QAction* promptAction = aiMenu->addAction(prompt.name);
            connect(promptAction, &QAction::triggered, this, [this, prompt]() {
                emit aiAssistWithPromptRequested(prompt.prompt);
            });
        }

        if (!prompts.isEmpty()) {
            aiMenu->addSeparator();
        }
        QAction* customAction = aiMenu->addAction(tr("Custom..."));
        connect(customAction, &QAction::triggered, this,
                [this]() { emit aiAssistRequested(); });
    }

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

/**
 * If the file cannot be read or doesn't contain a valid
 * H1 header, this function returns an empty string. The function looks
 * for the first line that matches the pattern of a markdown H1 header
 * (e.g., "# Title").
 */
QString MarkdownEditor::extractMainTitle(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    QTextStream in(&file);
    QRegularExpression headerPattern(RegexPatterns::HEADER_H1);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QRegularExpressionMatch match = headerPattern.match(line);
        if (match.hasMatch()) {
            file.close();
            return match.captured(1).trimmed();
        }
    }
    file.close();
    return QString();
}

void MarkdownEditor::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        QTextCursor cursor = cursorForPosition(event->position().toPoint());
        setTextCursor(cursor);
        for (const QUrl& url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);
                QString linkPath = filePath;
                if (!m_currentFilePath.isEmpty()) {
                    QFileInfo currentFileInfo(m_currentFilePath);
                    QDir currentDir = currentFileInfo.absoluteDir();
                    linkPath = currentDir.relativeFilePath(filePath);
                }
                QStringList imageExtensions;
                imageExtensions << "png" << "jpg" << "jpeg" << "gif" << "bmp"
                                << "svg";
                if (imageExtensions.contains(fileInfo.suffix().toLower())) {
                    QString imageMarkdown =
                        QString("![%1](%2)").arg(fileInfo.baseName(), linkPath);
                    cursor.insertText(imageMarkdown);
                } else {
                    QString linkLabel = fileInfo.fileName();
                    QStringList markdownExtensions;
                    markdownExtensions << "md" << "markdown" << "txt";
                    if (markdownExtensions.contains(fileInfo.suffix().toLower())) {
                        QString mainTitle = extractMainTitle(filePath);
                        if (!mainTitle.isEmpty()) {
                            linkLabel = mainTitle;
                        }
                    }
                    QString linkMarkdown =
                        QString("[%1](%2)").arg(linkLabel, linkPath);
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

    setUndoRedoEnabled(true);
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

void MarkdownEditor::setFocusModeEnabled(bool enabled) {
    m_focusModeEnabled = enabled;
    updateLineNumberAreaWidth(0);
}

void MarkdownEditor::setFocusModeMaxWidth(int maxWidth) {
    m_focusModeMaxWidth = maxWidth;
    if (m_focusModeEnabled) {
        updateLineNumberAreaWidth(0);
    }
}

void MarkdownEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    if (!m_focusModeEnabled) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    } else {
        QRect cr = contentsRect();
        int availableWidth = cr.width();
        int leftMargin = lineNumberAreaWidth();
        int rightMargin = 0;
        
        if (availableWidth > m_focusModeMaxWidth) {
            int totalHorizontalMargin = availableWidth - m_focusModeMaxWidth;
            int sideMargin = totalHorizontalMargin / 2;
            leftMargin += sideMargin;
            rightMargin = sideMargin;
        }
        
        setViewportMargins(leftMargin, 0, rightMargin, 0);
    }
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
    
    if (m_focusModeEnabled) {
        int availableWidth = cr.width();
        int leftMargin = lineNumberAreaWidth();
        int rightMargin = 0;
        
        if (availableWidth > m_focusModeMaxWidth) {
            int totalHorizontalMargin = availableWidth - m_focusModeMaxWidth;
            int sideMargin = totalHorizontalMargin / 2;
            leftMargin += sideMargin;
            rightMargin = sideMargin;
        }
        
        setViewportMargins(leftMargin, 0, rightMargin, 0);
        lineNumberArea->setGeometry(
            QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    } else {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
        lineNumberArea->setGeometry(
            QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }
}

void MarkdownEditor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QString scheme = m_highlighter->getColorScheme();
        QColor lineColor;
        if (scheme.contains("dark", Qt::CaseInsensitive)) {
            lineColor = ColorPalette::getDarkTheme().backgroundCurrentLine;
        } else {
            lineColor = ColorPalette::getLightTheme().backgroundCurrentLine;
        }

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);

    if (m_highlighter) {
        m_highlighter->setCurrentCursorLine(textCursor().blockNumber());
    }
}

void MarkdownEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(lineNumberArea);

    QColor backgroundColor, textColor;
    if (palette().color(QPalette::Base).lightness() > 128) {
        const auto& colors = ColorPalette::getLightTheme();
        backgroundColor = colors.lineNumberBackground;
        textColor = colors.lineNumberText;
    } else {
        const auto& colors = ColorPalette::getDarkTheme();
        backgroundColor = colors.lineNumberBackground;
        textColor = colors.lineNumberText;
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
    if (source->hasImage()) {
        QImage image = qvariant_cast<QImage>(source->imageData());
        if (!image.isNull()) {
            QString imagePath = saveImageFromClipboard(image);
            if (!imagePath.isEmpty()) {
                QTextCursor cursor = textCursor();
                cursor.insertText(QString("![image](%1)").arg(imagePath));
                return;
            }
        }
    }

    if (source->hasText()) {
        QTextCursor cursor = textCursor();
        cursor.insertText(source->text());
        return;
    }

    QTextEdit::insertFromMimeData(source);
}

void MarkdownEditor::jumpToHeading(const QString& anchor) {
    QString cleanAnchor = anchor;
    if (cleanAnchor.startsWith("#")) {
        cleanAnchor = cleanAnchor.mid(1);
    }
    
    QTextBlock block = document()->firstBlock();
    QRegularExpression headingPattern(RegexPatterns::HEADER);
    
    while (block.isValid()) {
        QString text = block.text();
        QRegularExpressionMatch match = headingPattern.match(text);
        
        if (match.hasMatch()) {
            QString headingText = match.captured(2).trimmed();
            QString slug = RegexUtils::generateSlug(headingText);
            
            if (slug == cleanAnchor) {
                QTextCursor cursor(block);
                setTextCursor(cursor);
                ensureCursorVisible();
                return;
            }
        }
        
        block = block.next();
    }
    
    qWarning() << "Heading not found:" << anchor;
}
