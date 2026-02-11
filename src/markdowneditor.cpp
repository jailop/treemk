#include "defs.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "shortcutmanager.h"
#include "thememanager.h"
#include <QApplication>
#include <QClipboard>
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

MarkdownEditor::MarkdownEditor(QWidget *parent)
    : QTextEdit(parent), m_predictionEnabled(true) {
  lineNumberArea = new LineNumberArea(this);
  m_highlighter = new MarkdownHighlighter(document());

  // Initialize formatting timer for deferred hanging indent application
  m_formatTimer = new QTimer(this);
  m_formatTimer->setSingleShot(true);
  m_formatTimer->setInterval(100); // Wait 100ms after user stops typing
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
    connect(ThemeManager::instance(), &ThemeManager::editorColorSchemeChanged,
            this, &MarkdownEditor::onThemeChanged);
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

MarkdownHighlighter *MarkdownEditor::getHighlighter() const {
  return m_highlighter;
}

QString MarkdownEditor::getLinkAtPosition(int position) const {
  QTextCursor cursor(document());
  cursor.setPosition(position);

  QString line = cursor.block().text();
  int posInBlock = cursor.positionInBlock();

  // Pattern for [[target]] or [[target|display]]
  QRegularExpression wikiLinkPattern("\\[\\[([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
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
      // Return any markdown link that is not an external URL
      if (!url.startsWith("http://") && !url.startsWith("https://")) {
        return url;
      }
    }
  }

  return QString();
}

void MarkdownEditor::mousePressEvent(QMouseEvent *event) {
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

void MarkdownEditor::mouseMoveEvent(QMouseEvent *event) {
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

QString MarkdownEditor::predictWordUnigram(const QString &prefix) const {
  if (prefix.length() < 2) {
    return QString();
  }

  QString bestMatch;
  int maxFrequency = 0;

  for (auto it = m_wordFrequency.constBegin(); it != m_wordFrequency.constEnd();
       ++it) {
    const QString &word = it.key();
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

QString MarkdownEditor::predictWordBigram(const QString &previousWord,
                                          const QString &prefix) const {
  if (previousWord.isEmpty() || prefix.length() < 1) {
    return QString();
  }

  QString prevLower = previousWord.toLower();
  QString prefixLower = prefix.toLower();

  QString bestMatch;
  int maxFrequency = 0;

  for (auto it = m_bigramFrequency.constBegin();
       it != m_bigramFrequency.constEnd(); ++it) {
    const QPair<QString, QString> &bigram = it.key();

    // Check if this bigram starts with the previous word and the second word
    // matches prefix
    if (bigram.first == prevLower && bigram.second.startsWith(prefixLower) &&
        bigram.second.length() > prefixLower.length()) {

      if (it.value() > maxFrequency) {
        maxFrequency = it.value();
        bestMatch = bigram.second;
      }
    }
  }

  return bestMatch;
}

QString MarkdownEditor::predictWord(const QString &prefix) const {
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

  // Restart the format timer - formatting will happen after user stops typing
  m_formatTimer->stop();
  m_formatTimer->start();
}

void MarkdownEditor::paintEvent(QPaintEvent *event) {
  QTextEdit::paintEvent(event);

  // Draw prediction in gray
  if (!m_currentPrediction.isEmpty()) {
    QTextCursor cursor = textCursor();
    QRect rect = cursorRect(cursor);

    QFont font = this->font();
    QFontMetrics fm(font);

    QPainter painter(viewport());
    painter.setFont(font);
    painter.setPen(QColor(128, 128, 128, 180)); // Gray with transparency

    int x = rect.right();
    int y = rect.bottom();

    painter.drawText(x, y, m_currentPrediction);
  }
}

void MarkdownEditor::keyPressEvent(QKeyEvent *event) {
  // Handle Tab key for word prediction
  if (event->key() == Qt::Key_Tab && !m_currentPrediction.isEmpty()) {
    acceptPrediction();
    event->accept();
    return;
  }

  // Get shortcut manager instance
  ShortcutManager *sm = ShortcutManager::instance();
  QKeySequence pressed(event->key() | event->modifiers());

  // Check custom navigation shortcuts
  QTextCursor cursor = textCursor();
  QTextCursor::MoveMode moveMode = (event->modifiers() & Qt::ShiftModifier)
                                       ? QTextCursor::KeepAnchor
                                       : QTextCursor::MoveAnchor;

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
  } else if (pressed ==
             sm->getShortcut(ShortcutManager::MoveToStartOfDocument)) {
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
  } else if (pressed ==
             sm->getShortcut(ShortcutManager::MoveToPreviousParagraph)) {
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

  // Auto-indent on Enter
  QSettings settings(APP_LABEL, APP_LABEL);
  bool autoIndent = settings.value("editor/autoIndent", true).toBool();
  bool autoCloseBrackets =
      settings.value("editor/autoCloseBrackets", true).toBool();

  if (autoIndent &&
      (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
    // Get current line text to calculate indentation
    QTextCursor cursor = textCursor();
    QString currentLine = cursor.block().text();
    int cursorPosInBlock = cursor.positionInBlock();

    // Check if current line is a task list item
    QRegularExpression taskPattern("^(\\s*)([-*+])\\s+\\[([ xX.]?)\\]\\s*");
    QRegularExpressionMatch taskMatch = taskPattern.match(currentLine);
    
    if (taskMatch.hasMatch()) {
      QString whitespace = taskMatch.captured(1);
      QString bullet = taskMatch.captured(2);
      
      // Check if cursor is at the end of the line
      bool isAtEnd = cursorPosInBlock >= currentLine.length();
      
      if (isAtEnd) {
        // Check if the task line contains only the checkbox (empty task item)
        QString lineAfterCheckbox = currentLine.mid(taskMatch.capturedLength());
        bool isEmptyTaskItem = lineAfterCheckbox.trimmed().isEmpty();
        
        if (isEmptyTaskItem) {
          // User pressed Enter on an empty task item - exit task mode
          cursor.select(QTextCursor::BlockUnderCursor);
          cursor.removeSelectedText();
          cursor.insertText("\n");
          setTextCursor(cursor);
          event->accept();
          return;
        } else {
          // Auto-continue task list with new unchecked item
          cursor.insertText("\n" + whitespace + bullet + " [ ] ");
          setTextCursor(cursor);
          event->accept();
          return;
        }
      }
    }

    // Check if current line is a regular list item (not task)
    QRegularExpression listPattern("^(\\s*)([-*+]|[0-9]+\\.)\\s+");
    QRegularExpressionMatch match = listPattern.match(currentLine);
    
     if (match.hasMatch()) {
       QString whitespace = match.captured(1);
       QString bullet = match.captured(2);
       
       // Check if cursor is at the end of the line (after all content)
       bool isAtEnd = cursorPosInBlock >= currentLine.length();
       
       if (isAtEnd) {
         // Check if the line contains only the bullet (empty list item)
         QString lineAfterBullet = currentLine.mid(match.capturedLength());
         bool isEmptyListItem = lineAfterBullet.trimmed().isEmpty();
         
         if (isEmptyListItem) {
           // User pressed Enter on an empty list item - exit list mode
           // Replace the empty bullet with a blank line
           cursor.select(QTextCursor::BlockUnderCursor);
           cursor.removeSelectedText();
           cursor.insertText("\n");
           setTextCursor(cursor);
           event->accept();
           return;
          } else {
            // Auto-continue list with same bullet
            cursor.insertText("\n" + whitespace + bullet + " ");
             setTextCursor(cursor);
             event->accept();
             return;
          }
       }
     }

    int indent = 0;

    if (match.hasMatch()) {
      // For list items, indent continuation to align with the text after the marker
      indent = match.capturedLength(1) + match.capturedLength(2) + 1; // marker + space
    } else {
      // Calculate leading whitespace for other cases
      for (QChar ch : currentLine) {
        if (ch == ' ')
          indent++;
        else if (ch == '\t')
          indent += 4; // Treat tab as 4 spaces
        else
          break;
      }
    }

    // Insert newline and indentation
    cursor.insertText("\n" + QString(" ").repeated(indent));
    setTextCursor(cursor);
    event->accept();
    return;
  }

  // Auto-close brackets, parentheses, quotes
  if (autoCloseBrackets) {
    QString closingChar;
    bool shouldClose = false;
    bool addSpace = false;

    if (event->text() == "(") {
      closingChar = ")";
      shouldClose = true;
    } else if (event->text() == "[") {
      // Check if this might be a task list
      QTextCursor cursor = textCursor();
      QString currentLine = cursor.block().text();
      int posInBlock = cursor.positionInBlock();
      
      // Check if we're after a list marker (-, *, +)
      QRegularExpression listMarkerPattern("^\\s*[-*+]\\s*$");
      QString textBeforeCursor = currentLine.left(posInBlock);
      
      if (listMarkerPattern.match(textBeforeCursor).hasMatch()) {
        // This is likely a task list - add space inside brackets and after
        closingChar = " ] ";
        shouldClose = true;
        addSpace = true;
      } else {
        // Regular bracket
        closingChar = "]";
        shouldClose = true;
      }
    } else if (event->text() == "{") {
      closingChar = "}";
      shouldClose = true;
    } else if (event->text() == "\"") {
      closingChar = "\"";
      shouldClose = true;
    } else if (event->text() == "'") {
      closingChar = "'";
      shouldClose = true;
    } else if (event->text() == "`") {
      closingChar = "`";
      shouldClose = true;
    }

    if (shouldClose) {
      QTextCursor cursor = textCursor();
      cursor.insertText(event->text() + closingChar);
      if (addSpace) {
        // For task lists, move cursor after the closing bracket and add space
        // Result: - [ ]| (cursor after closing bracket)
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
      } else {
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
      }
      setTextCursor(cursor);
      event->accept();
      return;
    }

    // Skip over closing bracket if typed when already at one
    QTextCursor cursor = textCursor();
    QString nextChar = cursor.block().text().mid(cursor.positionInBlock(), 1);
    if ((event->text() == ")" && nextChar == ")") ||
        (event->text() == "]" && nextChar == "]") ||
        (event->text() == "}" && nextChar == "}") ||
        (event->text() == "\"" && nextChar == "\"") ||
        (event->text() == "'" && nextChar == "'") ||
        (event->text() == "`" && nextChar == "`")) {
      cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
      setTextCursor(cursor);
      event->accept();
      return;
    }
  }

  // Handle Tab and Shift+Tab for indenting/dedenting lists and tasks
  if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
    QTextCursor cursor = textCursor();
    QString line = cursor.block().text();
    
    // Check if current line is a list or task item
    QRegularExpression listPattern("^(\\s*)([-*+]|[0-9]+\\.)\\s");
    QRegularExpressionMatch match = listPattern.match(line);
    
    if (match.hasMatch()) {
      cursor.beginEditBlock();
      
      if (event->key() == Qt::Key_Tab && !(event->modifiers() & Qt::ShiftModifier)) {
        // Tab - indent (add 2 spaces)
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.insertText("  ");
      } else if (event->key() == Qt::Key_Backtab || (event->modifiers() & Qt::ShiftModifier)) {
        // Shift+Tab - dedent (remove up to 2 spaces)
        QString whitespace = match.captured(1);
        if (!whitespace.isEmpty()) {
          cursor.movePosition(QTextCursor::StartOfBlock);
          int charsToRemove = qMin(2, whitespace.length());
          for (int i = 0; i < charsToRemove; i++) {
            cursor.deleteChar();
          }
        }
      }
      
      cursor.endEditBlock();
      event->accept();
      return;
    }
  }

  // Ctrl+Space to toggle task on current line
  // IMPORTANT: Handle this before any other space processing
  if (event->key() == Qt::Key_Space && 
      (event->modifiers() == Qt::ControlModifier || 
       event->modifiers() == (Qt::ControlModifier | Qt::KeypadModifier))) {
    QTextCursor cursor = textCursor();
    QString line = cursor.block().text();
    QRegularExpression taskPattern("([-*+])\\s+\\[([ xX.]?)\\]");
    QRegularExpressionMatch match = taskPattern.match(line);
    if (match.hasMatch()) {
      // Calculate the position of the marker character inside the brackets
      int checkboxStart = match.capturedStart(2); // Position of the marker (space, X, x, or .)
      toggleTaskAtPosition(cursor.block().position() + checkboxStart);
    }
    // Always accept and return - don't insert space
    event->accept();
    return;
  }

  // Hide prediction on certain keys
  if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
      event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
      event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
      event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown ||
      event->key() == Qt::Key_Escape) {
    hidePrediction();
  }

  QTextEdit::keyPressEvent(event);
}

void MarkdownEditor::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  } else {
    QTextEdit::dragEnterEvent(event);
  }
}

void MarkdownEditor::dragMoveEvent(QDragMoveEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  } else {
    QTextEdit::dragMoveEvent(event);
  }
}

void MarkdownEditor::dropEvent(QDropEvent *event) {
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
          QString imageMarkdown =
              QString("![%1](%2)").arg(fileInfo.baseName(), filePath);
          cursor.insertText(imageMarkdown);
          cursor.insertText("\n");
        } else {
          // Insert as link
          QString linkMarkdown =
              QString("[%1](%2)").arg(fileInfo.fileName(), filePath);
          cursor.insertText(linkMarkdown);
          cursor.insertText("\n");
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

  // Theme is applied via onThemeChanged() - no hardcoded colors here
}

int MarkdownEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, document()->blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
  return space;
}

void MarkdownEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void MarkdownEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    lineNumberArea->scroll(0, dy);
  else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void MarkdownEditor::resizeEvent(QResizeEvent *e) {
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
      lineColor = QColor(64, 64, 64); // Subtle gray for dark themes
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

void MarkdownEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
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
     
     if (block.isVisible() && blockTop + fontMetrics().height() > event->rect().top()) {
       QString number = QString::number(blockNumber + 1);
       painter.setPen(textColor);
       painter.drawText(0, blockTop, lineNumberArea->width() - 5,
                        fontMetrics().height(), Qt::AlignRight, number);
     }

     block = block.next();
   }
}

void MarkdownEditor::setCurrentFilePath(const QString &filePath) {
  m_currentFilePath = filePath;
}

void MarkdownEditor::insertFromMimeData(const QMimeData *source) {
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
  QTextEdit::insertFromMimeData(source);
}

QString MarkdownEditor::saveImageFromClipboard(const QImage &image) {
  // Check if we have a current file path
  if (m_currentFilePath.isEmpty()) {
    QMessageBox::warning(
        this, tr("Cannot Save Image"),
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
      QMessageBox::warning(
          this, tr("Cannot Create Directory"),
          tr("Failed to create directory '%1'.").arg(imagesDirName));
      return QString();
    }
  }

  // Find the next available number in sequence
  int nextNumber = 1;
  QRegularExpression imagePattern("^image_(\\d+)\\.png$");

  QStringList existingFiles =
      imagesDir.entryList(QStringList() << "image_*.png", QDir::Files);
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
  QString suggestedName =
      QString("image_%1.png").arg(nextNumber, 3, 10, QChar('0'));

  // Ask user for filename
  bool ok;
  QString fileName =
      QInputDialog::getText(this, tr("Save Image"), tr("Enter image filename:"),
                            QLineEdit::Normal, suggestedName, &ok);

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
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("File Exists"),
        tr("File '%1' already exists. Overwrite?").arg(fileName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
      return QString(); // User chose not to overwrite
    }
  }

  // Save the image
  if (image.save(fullPath, "PNG")) {
    return imagesDirName + "/" +
           fileName; // Return relative path with subdirectory
  }

  QMessageBox::warning(this, tr("Save Failed"),
                        tr("Failed to save image to '%1'.").arg(fileName));
  return QString();
}

QString MarkdownEditor::getTaskMarkerAtPosition(int position) const {
  QTextCursor cursor(document());
  cursor.setPosition(position);
  QString line = cursor.block().text();
  int posInBlock = cursor.positionInBlock();

  QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
  QRegularExpressionMatchIterator matchIterator = taskPattern.globalMatch(line);

  while (matchIterator.hasNext()) {
    QRegularExpressionMatch match = matchIterator.next();
    int start = match.capturedStart();
    int end = start + match.capturedLength();
    if (posInBlock >= start && posInBlock <= end) {
      return match.captured(1);
    }
  }
  return QString();
}

bool MarkdownEditor::isClickOnCheckbox(int position) const {
  QTextCursor cursor(document());
  cursor.setPosition(position);
  QString line = cursor.block().text();
  int posInBlock = cursor.positionInBlock();

  // Pattern to match task list: - [ ], * [ ], + [ ]
  QRegularExpression taskPattern("([-*+])\\s+\\[([ xX.]?)\\]");
  QRegularExpressionMatch match = taskPattern.match(line);

  if (match.hasMatch()) {
    // Check if click is on the checkbox part: [ ] or [X] or [.]
    int checkboxStart = match.capturedStart();
    
    // Allow clicking anywhere on "[ ]" part (including the brackets)
    int bracketStart = line.indexOf('[', checkboxStart);
    int bracketEnd = line.indexOf(']', bracketStart);
    
    if (bracketStart >= 0 && bracketEnd >= 0) {
      return (posInBlock >= bracketStart && posInBlock <= bracketEnd + 1);
    }
  }
  return false;
}

void MarkdownEditor::toggleTaskAtPosition(int position) {
  QTextCursor cursor(document());
  cursor.setPosition(position);
  QTextBlock block = cursor.block();
  QString line = block.text();

  // Match any list marker (-, *, +) followed by [ ]
  QRegularExpression taskPattern("([-*+])\\s+\\[([ xX.]?)\\]");
  QRegularExpressionMatch match = taskPattern.match(line);
  if (match.hasMatch()) {
    QString marker = match.captured(2); // The character inside brackets
    QString newMarker;
    
    // Toggle: space -> X, X/x -> space, . -> X
    if (marker == " ") {
      newMarker = "X";
    } else if (marker.toUpper() == "X") {
      newMarker = " ";
    } else if (marker == ".") {
      newMarker = "X";
    } else {
      newMarker = "X";
    }

    // Save current cursor position in the editor
    QTextCursor editorCursor = textCursor();
    int cursorPos = editorCursor.position();
    int blockPos = block.position();
    int relativePos = cursorPos - blockPos;

    // Replace the marker character
    cursor.beginEditBlock();
    cursor.setPosition(block.position() + match.capturedStart(2));
    cursor.deleteChar();
    cursor.insertText(newMarker);
    cursor.endEditBlock();

    // Restore cursor position
    editorCursor.setPosition(blockPos + relativePos);
    setTextCursor(editorCursor);

    // Update parent task states
    updateParentTask(block);
    
    // Update child task states if this task was marked as done/pending
    if (newMarker == " ") {
      // Parent was unchecked - uncheck all children
      uncheckChildTasks(block);
    }
  }
}

void MarkdownEditor::uncheckChildTasks(const QTextBlock &block) {
  QVector<QTextBlock> children = findChildBlocks(block);
  for (const QTextBlock &child : children) {
    QString line = child.text();
    QRegularExpression taskPattern("([-*+])\\s+\\[([xX.]?)\\]");
    QRegularExpressionMatch match = taskPattern.match(line);
    if (match.hasMatch()) {
      QString marker = match.captured(2);
      if (marker != " ") {
        // Change to unchecked
        QTextCursor cursor(document());
        cursor.setPosition(child.position());
        cursor.beginEditBlock();
        cursor.select(QTextCursor::BlockUnderCursor);
        QString newLine = line;
        int markerPos = match.capturedStart(2);
        newLine.replace(markerPos, match.capturedLength(2), " ");
        cursor.insertText(newLine);
        cursor.endEditBlock();
      }
    }
  }
}

void MarkdownEditor::updateParentTask(const QTextBlock &block) {
  int currentIndent = getIndentLevel(block);
  QTextBlock parentBlock = findParentBlock(block, currentIndent);
  if (parentBlock.isValid()) {
    updateTaskState(parentBlock);
    updateParentTask(parentBlock);
  }
}

QTextBlock MarkdownEditor::findParentBlock(const QTextBlock &block, int currentIndent) {
  QTextBlock current = block.previous();
  while (current.isValid()) {
    int indent = getIndentLevel(current);
    if (indent < currentIndent) {
      QString line = current.text();
      QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
      if (taskPattern.match(line).hasMatch()) {
        return current;
      }
    }
    current = current.previous();
  }
  return QTextBlock();
}

void MarkdownEditor::updateTaskState(const QTextBlock &block) {
  QVector<QTextBlock> children = findChildBlocks(block);
  bool hasDone = false;
  bool hasPending = false;
  for (const QTextBlock &child : children) {
    QString line = child.text();
    QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
    QRegularExpressionMatch match = taskPattern.match(line);
    if (match.hasMatch()) {
      QString marker = match.captured(1);
      if (marker == "X") {
        hasDone = true;
      } else if (marker == " ") {
        hasPending = true;
      } else if (marker == ".") {
        hasDone = true;
        hasPending = true;
      }
    }
  }
  QString newMarker;
  if (hasDone && hasPending) {
    newMarker = ".";
  } else if (hasDone) {
    newMarker = "X";
  } else {
    newMarker = " ";
  }

  QString line = block.text();
  QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
  QRegularExpressionMatch match = taskPattern.match(line);
  if (match.hasMatch()) {
    QString oldMarker = match.captured(1);
    if (oldMarker != newMarker) {
      QTextCursor cursor(document());
      cursor.setPosition(block.position() + match.capturedStart(1));
      cursor.setPosition(block.position() + match.capturedStart(1) + match.capturedLength(1), QTextCursor::KeepAnchor);
      cursor.insertText(newMarker);
    }
  }
}

QVector<QTextBlock> MarkdownEditor::findChildBlocks(const QTextBlock &block) {
  QVector<QTextBlock> children;
  int parentIndent = getIndentLevel(block);
  int childIndent = -1;
  QTextBlock current = block.next();
  while (current.isValid()) {
    int indent = getIndentLevel(current);
    if (indent <= parentIndent) {
      break;
    }
    QString line = current.text();
    QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
    if (taskPattern.match(line).hasMatch()) {
      if (childIndent == -1) {
        childIndent = indent;
      }
      if (indent == childIndent) {
        children.append(current);
      } else if (indent < childIndent) {
        break;
      }
    }
    current = current.next();
  }
  return children;
}

int MarkdownEditor::getIndentLevel(const QTextBlock &block) {
  QString text = block.text();
  int indent = 0;
  for (QChar ch : text) {
    if (ch == ' ') {
      indent++;
    } else if (ch == '\t') {
      indent += 4;
    } else {
      break;
    }
  }
  return indent;
}

void MarkdownEditor::applyListHangingIndent(const QTextBlock &block) {
  QString text = block.text();
  QRegularExpression listPattern("^(\\s*)([-*+]|[0-9]+\\.)\\s+");
  QRegularExpressionMatch match = listPattern.match(text);
  if (!match.hasMatch()) {
    return; // Not a list item
  }
  QFont font = this->font();
  QFontMetrics fm(font);
  QString indent = match.captured(1); // Leading whitespace
  QString bullet = match.captured(2); // Bullet marker (-, *, +, or number.)
  int bulletWidth = fm.horizontalAdvance(indent + bullet + " ");
  QTextCursor cursor(document());
  cursor.setPosition(block.position());
  QTextBlockFormat blockFormat = cursor.blockFormat();
  blockFormat.setLeftMargin(bulletWidth);     // Total left margin
  blockFormat.setTextIndent(-bulletWidth);    // Negative indent for hanging effect
  cursor.setBlockFormat(blockFormat);
}

void MarkdownEditor::applyDeferredFormatting() {
  // This method is called after user stops typing (via QTimer)
  // It safely applies hanging indent formatting to all list items
  
  // Store the current modified state
  bool wasModified = document()->isModified();
  
  // Block signals to prevent triggering textChanged and modificationChanged
  document()->blockSignals(true);
  
  QTextBlock block = document()->firstBlock();
  while (block.isValid()) {
    applyListHangingIndent(block);
    block = block.next();
  }
  
  // Restore signals
  document()->blockSignals(false);
  
  // Restore the modified state (formatting shouldn't count as modification)
  document()->setModified(wasModified);
}
